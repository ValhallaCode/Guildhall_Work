#include "Engine/Core/Job.hpp"
#include "Engine/Core/BlockAllocator.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Profiling.hpp"
#include <thread>



class JobSystem
{
public:
	ThreadSafeQueue<Job*> *queues;
	Signal **signals;
	uint queue_count;

	bool is_running;
};

static JobSystem *gJobSystem = nullptr;
static BlockAllocator* gJobAlloc = nullptr;

//------------------------------------------------------------------------
static void GenericJobThread(Signal *signal)
{
	JobConsumer consumer;
	consumer.add_category(JOB_GENERIC);

	while (gJobSystem->is_running) {
		signal->wait();
		consumer.consume_all();
	}

	consumer.consume_all();
}

//------------------------------------------------------------------------
void Job::on_finish()
{
	state = FINISHED;

	for (uint i = 0; i < dependents.size(); ++i) {
		dependents[i]->on_dependancy_finished();
	}

	dependents.clear();
}

//------------------------------------------------------------------------
void Job::on_dependancy_finished()
{
	JobDispatchAndRelease(this);
}

//------------------------------------------------------------------------
void Job::dependent_on(Job *parent)
{
	ASSERT_OR_DIE(parent->state < ENQUEUED, "Parent was not running!");

	AtomicIncrement(&num_dependencies);
	JobAcquire(this);
	parent->dependents.push_back(this);
}

//------------------------------------------------------------------------
void JobSystemStartup(uint job_category_count, int generic_thread_count /*= -1*/)
{
	int core_count = (int)std::thread::hardware_concurrency();
	if (generic_thread_count <= 0) {
		core_count += generic_thread_count;
	}
	core_count--; // one is always being created - so subtract from total wanted;

	gJobAlloc = new BlockAllocator(sizeof(Job) * job_category_count);
	
	// We need queues! 
	gJobSystem = new JobSystem();
	gJobSystem->queues = new ThreadSafeQueue<Job*>[job_category_count];
	gJobSystem->signals = new Signal*[job_category_count];
	gJobSystem->queue_count = job_category_count;
	gJobSystem->is_running = true;

	for (uint i = 0; i < job_category_count; ++i) {
		gJobSystem->signals[i] = nullptr;
	}

	// create the signal
	gJobSystem->signals[JOB_GENERIC] = new Signal();

	ThreadCreate(GenericJobThread, gJobSystem->signals[JOB_GENERIC]);
	for (int i = 0; i < core_count; ++i) {
		ThreadCreate(GenericJobThread, gJobSystem->signals[JOB_GENERIC]);
	}
}

//------------------------------------------------------------------------
void JobSystemShutdown()
{
	uint loop_count = gJobSystem->queue_count;

	for (uint index = 0; index < loop_count; ++index)
	{
		ThreadSafeQueue<Job*> queue = gJobSystem->queues[index];
		if (!queue.empty())
		{
			Job* job = nullptr;
			while (queue.pop(&job))
			{
				JobDispatchAndRelease(job);
			}
		}
	}

	delete gJobAlloc;
	delete gJobSystem;
}

//------------------------------------------------------------------------
Job* JobCreate(uint category, job_work_cb work_cb, void *user_data)
{
	Job* job = nullptr;
	job = gJobAlloc->create<Job>();
	job->type = static_cast<eJobType>(category);
	job->work_cb = work_cb;
	job->user_data = user_data;
	job->num_dependencies = 1;
	job->ref_count = 1;
	job->state = WAITING;

	return job;
}

void JobAcquire(Job *job)
{
	AtomicIncrement(&job->ref_count);
}

void JobRun(uint category, job_work_cb cb, void *user_data)
{
	Job* job = JobCreate(category, cb, user_data);
	JobDispatchAndRelease(job);

	JobConsumer consumer;
	consumer.add_category(category);
	consumer.consume_all();
}

void JobDispatch(Job *job)
{
	uint dcount = AtomicDecrement(&job->num_dependencies);
	if (dcount != 0) {
		return;
	}

	job->state = ENQUEUED;

	JobAcquire(job);
	gJobSystem->queues[job->type].push(job);

	Signal *signal = gJobSystem->signals[job->type];
	if (nullptr != signal) {
		signal->signal_all();
	}
}

void JobRelease(Job *job)
{
	uint ref_count = AtomicDecrement(&job->ref_count);
	if (0 == ref_count) {
		gJobAlloc->free(job);
	}
}

void JobWait(Job *job, JobConsumer *consumer)
{
	while (!job->is_finished()) {
		if (consumer != nullptr) {
			consumer->consume_job();
		}
	}
}

void JobWaitAndRelease(Job *job, JobConsumer *consumer)
{
	JobWait(job, consumer);
	JobRelease(job);
}

void JobDispatchAndRelease(Job *job)
{
	JobDispatch(job);
	JobRelease(job);
}

void JobSystemMainStep()
{
	JobConsumer consumer;
	consumer.add_category(JOB_MAIN);
	consumer.consume_all();
}

void JobSystemSetCategorySignal(uint cat_id, Signal *signal)
{
	if (cat_id >= gJobSystem->queue_count) {
		return;
	}

	gJobSystem->signals[cat_id] = signal;
}

ThreadSafeQueue<Job*>* JobSystemGetQueue(uint category)
{
	if (category >= gJobSystem->queue_count) {
		return nullptr;
	}

	return &(gJobSystem->queues[category]);
}

void JobConsumer::add_category(uint category)
{
	ThreadSafeQueue<Job*>* queue = JobSystemGetQueue(category);
	if (nullptr == queue)
		return;

	if (std::find(queues.begin(), queues.end(), queue) == queues.end()) {
		queues.push_back(queue);
	}
}

bool JobConsumer::consume_job()
{
	Job *job = nullptr;
	for (uint i = 0; i < queues.size(); ++i) {
		ThreadSafeQueue<Job*> *queue = queues[i];
		if (queue->pop(&job)) {

			job->state = RUNNING;

			job->work_cb(job->user_data);

			job->on_finish();
			job->dependents.clear();

			JobRelease(job);
			return true;
		}
	}

	return false;
}

uint JobConsumer::consume_all()
{
	uint count = 0;
	while (consume_job()) {
		++count;
	}

	return count;
}

uint JobConsumer::consume_for_ms(uint ms)
{
	uint count = 0;
	uint start = TimeGet_ms();
	do {

		if (!consume_job()) {
			return count;
		}

		++count;
	} while ((TimeGet_ms() - start) < ms);

	return count;
}

static void EmptyJob(void *ptr)
{
	uint *count_ptr = (uint*)ptr;
	AtomicIncrement(count_ptr);
}

static bool gDone = false;
static void OnEverythingDone(void *ptr)
{
	uint *count_ptr = (uint*)ptr;

	// assert count_ptr is 1000 [make sure all other jobs got to run first!]
	uint count = *count_ptr;
	if (count != 1000) {
		__debugbreak();
	}

	gDone = true;
}

void JobSystemTest()
{
	// Make sure creating and releasing work [does not dispatch - job does not run!]
	Job *job = JobCreate(JOB_GENERIC, EmptyJob, nullptr);
	JobRelease(job);

	// Next, lets kick off jobs, and make sure they're freeing up.
	{
		PROFILE_LOG_SCOPE("JobDispatchAndReleaseTime");
		uint count = 0;
		Job *final_job = JobCreate(JOB_GENERIC, OnEverythingDone, &count);

		for (uint i = 0; i < 1000; ++i) {
			job = JobCreate(JOB_GENERIC, EmptyJob, &count);
			final_job->dependent_on(job);
			JobDispatchAndRelease(job);
		}

		// now I can dispatch myself [if I do it first, it has no dependancies and will immediately run]
		JobDispatch(final_job);

		// wait for it to finish
		JobWaitAndRelease(final_job);

		// And after this is done, I should KNOW the count is 1000 [1001 jobs ran]
		if (count != 1000) {
			__debugbreak();
		}
	}
}