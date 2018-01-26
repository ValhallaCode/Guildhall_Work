#pragma once
#if !defined( __JOB__ )
#define __JOB__

#include "Engine/Core/ThreadSafeQueue.hpp"
#include "Engine/Core/Signal.hpp"
#include "Engine/Core/Atomic.hpp"
#include <vector>


enum eJobType : uint
{
	JOB_GENERIC = 0,
	JOB_MAIN,
	JOB_IO,
	JOB_RENDER,
	JOB_LOGGER,
	JOB_TYPE_COUNT,
};

enum eJobState : uint
{
	WAITING,
	ENQUEUED,
	RUNNING,
	FINISHED
};

typedef void(*job_work_cb)(void*);

class Job
{
public:
	eJobType type;
	job_work_cb work_cb;
	void *user_data;
	std::vector<Job*> dependents;
	uint num_dependencies;
	uint dispatch_count;
	uint ref_count;
	eJobState state;

public:
	void on_finish();
	void on_dependancy_finished();
	void dependent_on(Job *parent);
	inline bool is_finished() const { return state == FINISHED; }
};

class JobConsumer
{
public:
	void add_category(uint category);
	bool consume_job();
	uint consume_all();
	uint consume_for_ms(uint ms);
public:
	std::vector<ThreadSafeQueue<Job*>*> queues;
};

void JobSystemStartup(uint job_category_count, int generic_thread_count = -1);
void JobSystemShutdown();
Job* JobCreate(uint category, job_work_cb cb, void *user_data);
void JobDispatchAndRelease(Job *job);
void JobDispatch(Job *job);
void JobRelease(Job *job);
void JobRun(uint category, job_work_cb cb, void *user_data);
void JobSystemMainStep();
void JobSystemSetCategorySignal(uint cat_id, Signal *signal);
void JobWait(Job *job, JobConsumer *consumer);
void JobWaitAndRelease(Job *job, JobConsumer *consumer = nullptr);
void JobAcquire(Job *job);
void JobSystemTest();

#endif 
