#include "Engine/Core/Profiling.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/CriticalSection.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Core/Logging.hpp"
#include "Engine/EngineConfig.hpp"
#include <string>
#include <locale>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sstream>

struct profile_node
{
	profile_node()
		:tag("")
		, start(0)
		, end(0)
		, allocs(0)
		, frees(0)
		, parent(nullptr)
		, first_child(nullptr)
		, last_child(nullptr)
		, prev_sibling(nullptr)
		, next_sibling(nullptr)
	{

	}


	const char* tag;
	uint64_t start;
	uint64_t end;

	uint allocs;
	uint frees;

	profile_node* parent;
	profile_node* first_child;
	profile_node* last_child;
	profile_node* prev_sibling;
	profile_node* next_sibling;
};


static profile_node* g_profilerRoot = nullptr;
static profile_node* g_activeNode = nullptr;
static bool g_canRecord = true;
static DWORD g_threadID;
static int g_nodeTracker = 0;
static uint64_t g_frameStart = 0;
static uint64_t g_frameEnd = 0;
static bool g_resumeRequested = false;

profile_node* FindRootFromActiveNode()
{
	ASSERT_OR_DIE(g_threadID == ::GetCurrentThreadId(), "Thread Calling Find Root From Active not the same that Initialized the Profiler!");
	if (nullptr == g_activeNode)
		return nullptr;

	profile_node* current_node = g_activeNode;
	while (nullptr != current_node->parent)
	{
		current_node = current_node->parent;
	}

	return current_node;
}

void DeleteFromRootNode(profile_node* root_node)
{
	ASSERT_OR_DIE(g_threadID == ::GetCurrentThreadId(), "Thread Calling Delete From Root not the same that Initialized the Profiler!");
	profile_node* current_node = root_node;

	while (nullptr != current_node && nullptr != current_node->parent)
	{
		if (nullptr != current_node->last_child)
		{
			current_node = current_node->last_child;
			continue;
		}

		if (nullptr != current_node->prev_sibling)
		{
			profile_node* temp = current_node->prev_sibling;
			delete current_node;
			current_node = temp;
			continue;
		}

		if (nullptr != current_node->parent)
		{
			profile_node* parent = current_node->parent;
			parent->first_child = nullptr;
			parent->last_child = nullptr;
			delete current_node;
			current_node = parent;
			continue;
		}
	}

	if (nullptr != current_node)
		delete current_node;

	root_node = nullptr;
}

void ProfilerStartup()
{
#if defined PROFILED_BUILD
	g_threadID = ::GetCurrentThreadId();
#endif
}

void SetFrameStartTime(uint64_t op_count)
{
	g_frameStart = op_count;
}

void SetFrameEndTime(uint64_t op_count)
{
	g_frameEnd = op_count;
}

void ProfilerShutdown()
{
#if defined PROFILED_BUILD
	ASSERT_OR_DIE(g_threadID == ::GetCurrentThreadId(), "Thread Calling Profiler Shutdown not the same that Initialized the Profiler!");
	
	if (nullptr != g_activeNode)
	{
		profile_node* root = FindRootFromActiveNode();
		DeleteFromRootNode(root);
	}
	
	if(nullptr != g_profilerRoot)
		DeleteFromRootNode(g_profilerRoot);
#endif
}

uint64_t GetTotalFrameTime()
{
	return g_frameEnd - g_frameStart;
}

void ProfilerPause(void*)
{
	ASSERT_OR_DIE(g_threadID == ::GetCurrentThreadId(), "Thread Calling Profiler Pause not the same that Initialized the Profiler!");
	g_canRecord = false;
}

void ProfilerResume(void*)
{
	ASSERT_OR_DIE(g_threadID == ::GetCurrentThreadId(), "Thread Calling Profiler Resume not the same that Initialized the Profiler!");
	g_resumeRequested = true;
}

void ProfilerPush(char const *tag)
{
#if defined PROFILED_BUILD
	ASSERT_OR_DIE(g_threadID == ::GetCurrentThreadId(), "Thread Calling Profiler Push not the same that Initialized the Profiler!");
	if (!g_canRecord) {

		if (g_resumeRequested && g_nodeTracker == 0)
		{
			g_canRecord = true;
			g_resumeRequested = false;
		}
		else
		{
			g_nodeTracker++;
			return;
		}
	}

	profile_node* node = new profile_node();
	node->tag = tag;
	node->start = TimeGetOpCount();
	node->parent = g_activeNode;

	if(nullptr != g_activeNode)
	{
		if (nullptr == g_activeNode->first_child)
		{
			g_activeNode->first_child = node;
			g_activeNode->last_child = node;
		}
		else
		{
			g_activeNode->last_child->next_sibling = node;
			node->prev_sibling = g_activeNode->last_child;
			g_activeNode->last_child = node;
		}
	}

	g_activeNode = node;
#endif
}

void CalcAllocsAndFreesFromChildren()
{
	uint totalAllocCount = GetAllocCount();
	uint totalFreesCount = GetFrameFrees();

	profile_node* current_child = g_activeNode->first_child;
	uint childAllocCount = 0;
	uint childFreeCount = 0;

	while (nullptr != current_child)
	{
		childAllocCount += current_child->allocs;
		childFreeCount += current_child->frees;
		current_child = current_child->next_sibling;
	}

	g_activeNode->allocs = totalAllocCount - childAllocCount;
	g_activeNode->frees = totalFreesCount - childFreeCount;
}

void ProfilerPop()
{
#if defined PROFILED_BUILD
	ASSERT_OR_DIE(g_threadID == ::GetCurrentThreadId(), "Thread Calling Profiler Pop not the same that Initialized the Profiler!");
	ASSERT_OR_DIE(g_activeNode != nullptr, "Active Node was nullptr when popping!");

	if (!g_canRecord) {
		g_nodeTracker--;
	}

	g_activeNode->end = TimeGetOpCount();

	if (nullptr == g_activeNode->first_child)
	{
		g_activeNode->allocs = GetAllocCount();
		g_activeNode->frees = GetFrameFrees();
	}
	else
	{
		CalcAllocsAndFreesFromChildren();
	}

	if (nullptr == g_activeNode->parent)
	{
		// move this to our completed frames list, and free up any old frames we may not care about anymore
		if (nullptr != g_profilerRoot)
			DeleteFromRootNode(g_profilerRoot);

		g_profilerRoot = g_activeNode;
		g_activeNode = g_activeNode->parent;
	}
	else
	{
		g_activeNode = g_activeNode->parent;
	}
#endif
}

profile_node* ProfilerGetPreviousFrame()
{
	ASSERT_OR_DIE(g_threadID == ::GetCurrentThreadId(), "Thread Calling Profiler Get Previous Frame not the same that Initialized the Profiler!");
	return g_profilerRoot;
}

profile_node* ProfilerGetPreviousFrame(char const *root_tag)
{
	ASSERT_OR_DIE(g_threadID == ::GetCurrentThreadId(), "Thread Calling Profiler Get Previous Frame not the same that Initialized the Profiler!");
	profile_node* req_root = g_profilerRoot;

	while (nullptr != req_root && req_root->tag != root_tag)
	{
		if (nullptr != req_root->last_child)
		{
			req_root = req_root->last_child;
			continue;
		}

		if (nullptr != req_root->prev_sibling)
		{
			req_root = req_root->prev_sibling;
			continue;
		}

		if (nullptr != req_root->parent)
		{
			while (nullptr == req_root->parent->prev_sibling)
			{
				req_root = req_root->parent;
			}

			if (req_root->tag != root_tag)
				req_root = req_root->parent->prev_sibling;

			continue;
		}
	}

	return req_root;
}

// Reporting Section
struct profile_data
{
	profile_data()
		:tag("")
		, call_count(1)
		, total_time_as_percent(0.0f)
		, total_time("0.0 s")
		, self_time_as_percent(0.0f)
		, self_time("0.0 s")
		, alloc_count(0)
		, free_count(0)
		, parent(nullptr)
		, first_child(nullptr)
		, prev_sibling(nullptr)
		, next_sibling(nullptr)
	{

	}

	const char* tag;
	uint call_count;
	float total_time_as_percent;
	std::string total_time;
	float self_time_as_percent;
	std::string self_time;
	uint alloc_count;
	uint free_count;

	profile_data* parent;
	profile_data* first_child;
	profile_data* prev_sibling;
	profile_data* next_sibling;
};

enum ReportStyle {
	TREE_VIEW,
	FLAT_VIEW
};

static ReportStyle g_treeReportStyle = TREE_VIEW;
#include <vector>

class ProfilerReport
{
public:
	ProfilerReport(profile_node* node_head);
	uint GetFreeCount(profile_node* node);
	uint GetAllocsCount(profile_node* node);
	profile_data* CreateDataFromNode(profile_node* node);
	void CreateTreeView();
	void CreateFlatView();
	void SortByTotalTime();
	void Log();
	void SortBySelfTime();
	uint GetCallCount(profile_node* node);
	void CreateTree(const ReportStyle& style);
public:
	profile_data* m_head;
	profile_node* m_nodeHead;
	ReportStyle m_style;
	std::vector<profile_data*> m_flatList;
};

ProfilerReport::ProfilerReport(profile_node* node_head)
	:m_head(nullptr)
{
	m_nodeHead = node_head;
	//m_head = CreateDataFromNode(node_head);
}

uint64_t GetTotalTimeForNode(profile_node* node)
{
	if (nullptr != node->first_child)
	{
		// get totals from children

		profile_node* current_child = node->first_child;
		uint64_t total = node->end - node->start;

		while (nullptr != current_child)
		{
			uint64_t child_total = current_child->end - current_child->start;
			total -= child_total;
			current_child = current_child->next_sibling;
		}

		return total;
	}
	else
	{
		// return current nodes total
		return node->end - node->start;
	}
}

uint ProfilerReport::GetCallCount(profile_node* node)
{
	profile_node* current_node = m_nodeHead;
	uint count = 0;

	while (nullptr != current_node)
	{
		if (nullptr != current_node->last_child)
		{
			current_node = current_node->last_child;
			continue;
		}

		if (nullptr != current_node->prev_sibling)
		{
			if (current_node->tag == node->tag)
				++count;

			current_node = current_node->prev_sibling;
			continue;
		}

		if (nullptr != current_node->parent)
		{
			if (nullptr != current_node && current_node->tag == node->tag)
				++count;

			current_node = current_node->parent;

			if (nullptr != current_node && current_node->tag == node->tag)
				++count;
			
			if(nullptr != current_node)
			{
				profile_node* temp = current_node;

				while (nullptr != temp && nullptr == temp->prev_sibling)
				{
					temp = temp->parent;
				}

				if (nullptr == temp)
					break;

				current_node = temp->prev_sibling;
				if (current_node->tag == node->tag)
					++count;
				//if (nullptr == temp->prev_sibling)
				//	current_node = current_node->parent;
				//else
				//	current_node = current_node->prev_sibling;
			}
			
			continue;
		}
	}

	return count;
}

// if (nullptr == current_data->prev_sibling && nullptr == current_data->next_sibling)
// {
// 	has_reached_end_of_branch = true;
// 	current_data = current_data->parent;
// 	continue;
// }

uint ProfilerReport::GetAllocsCount(profile_node* node)
{
	if (nullptr != node->first_child)
	{
		// get totals from children

		profile_node* current_child = node->first_child;
		uint total = node->allocs;

		while (nullptr != current_child)
		{
			uint child_allocs = current_child->allocs;
			total -= child_allocs;
			current_child = current_child->next_sibling;
		}

		return total;
	}
	else
	{
		// return current nodes total
		return node->allocs;
	}
}

uint ProfilerReport::GetFreeCount(profile_node* node) //
{
	if (nullptr != node->first_child)
	{
		// get totals from children

		profile_node* current_child = node->first_child;
		uint total = node->frees;

		while (nullptr != current_child)
		{
			uint child_frees = current_child->frees;
			total -= child_frees;
			current_child = current_child->next_sibling;
		}

		return total;
	}
	else
	{
		// return current nodes total
		return node->frees;
	}
}

profile_data* ProfilerReport::CreateDataFromNode(profile_node* node)
{
	profile_data* data = new profile_data();
	data->tag = node->tag;
	data->call_count = GetCallCount(node);

	uint64_t total_frame_time = GetTotalFrameTime();
	uint64_t data_total_time_for_self = GetTotalTimeForNode(node);

	data->total_time_as_percent = (float)TimeOpCountToSeconds(node->end - node->start) / (float)TimeOpCountToSeconds(total_frame_time);
	std::string total_string = TimeOpCountToString(node->end - node->start);
	data->total_time = total_string;

	data->self_time_as_percent = (float)data_total_time_for_self / (float)total_frame_time;
	std::string self_string = TimeOpCountToString(data_total_time_for_self);
	data->self_time = self_string;

	data->alloc_count = node->allocs;
	data->free_count = node->frees;

	return data;
}

void SetSiblingsParent(profile_data* first_sibling, profile_data* parent)
{
	profile_data* current_data = first_sibling;

	if (nullptr != current_data->prev_sibling)
		current_data->prev_sibling->parent = parent;

	while (nullptr != current_data)
	{
		current_data->parent = parent;
		current_data = current_data->next_sibling;
	}
}

void ProfilerReport::CreateTreeView()
{
	profile_node* current_node = m_nodeHead;
	profile_data* prev_added_node = nullptr;
	profile_data* data_added_before_branch_traversal = nullptr;

	while (nullptr != current_node)
	{
		if (nullptr != current_node->last_child)
		{
			if (nullptr != prev_added_node)
				data_added_before_branch_traversal = prev_added_node;

			prev_added_node = nullptr;
			current_node = current_node->last_child;
			continue;
		}

		if (nullptr != current_node->prev_sibling)
		{
			profile_data* data = CreateDataFromNode(current_node);

			if (nullptr == prev_added_node)
			{
				prev_added_node = data;
			}
			else
			{
				data->next_sibling = prev_added_node;
				prev_added_node->prev_sibling = data;
				prev_added_node = data;
			}

			current_node = current_node->prev_sibling;
			continue;
		}

		if (nullptr != current_node->parent)
		{
			profile_data* data = CreateDataFromNode(current_node);

			if (nullptr == prev_added_node)
			{
				prev_added_node = data;
			}
			else
			{
				data->next_sibling = prev_added_node;
				prev_added_node->prev_sibling = data;
				prev_added_node = data;
			}

			current_node = current_node->parent;

			data = CreateDataFromNode(current_node);

			if (nullptr == prev_added_node)
			{
				prev_added_node = data;
			}
			else
			{
				SetSiblingsParent(prev_added_node, data);
				data->first_child = prev_added_node;
				prev_added_node = data;
			}

			if (nullptr != current_node)
			{
				profile_node* temp = current_node;

				while (nullptr != temp && nullptr == temp->prev_sibling)
				{
					temp = temp->parent;

					if (nullptr == temp)
						break;

					data = CreateDataFromNode(temp);
					data->first_child = prev_added_node;

					if (nullptr == prev_added_node->parent)
						prev_added_node->parent = data;

					if (nullptr != data_added_before_branch_traversal && nullptr != temp->parent)
					{
						data->next_sibling = data_added_before_branch_traversal;
						data_added_before_branch_traversal->prev_sibling = data;
						data_added_before_branch_traversal->parent = data->parent;
					}

					prev_added_node = data;
				}

				if (nullptr == temp)
					break;

				current_node = temp->prev_sibling;
			}

			continue;
		}
	}


	m_head = prev_added_node;

}

bool DoesDataAlreadyExist(profile_data* data_root, const char* tag_to_check)
{
	profile_data* current_data = data_root;

	while (nullptr != current_data && nullptr != data_root->first_child)
	{
		if (current_data->tag == tag_to_check)
			return true;

		if (nullptr != current_data->first_child)
		{
			current_data = current_data->first_child;
			continue;
		}

		if (nullptr != current_data->next_sibling)
		{
			current_data = current_data->next_sibling;
			continue;
		}

		if (nullptr != current_data->parent)
		{
			while (nullptr == current_data->parent->next_sibling)
			{
				current_data = current_data->parent;
				if (nullptr == current_data->parent)
					return false;
			}

			current_data = current_data->parent->next_sibling;
			continue;
		}
	}

	if (data_root->tag == tag_to_check)
		return true;

	return false;
}

void ProfilerReport::CreateFlatView()
{
	profile_node* current_node = m_nodeHead;
	profile_data* prev_added_node = nullptr;
	profile_data* data_added_before_branch_traversal = nullptr;

	while (nullptr != current_node)
	{
		bool can_create_data = true;
		if (nullptr != prev_added_node && DoesDataAlreadyExist(prev_added_node, current_node->tag))
			can_create_data = false;

		if (nullptr != current_node->last_child)
		{
			if (nullptr != prev_added_node)
				data_added_before_branch_traversal = prev_added_node;

			prev_added_node = nullptr;
			current_node = current_node->last_child;
			continue;
		}

		if (nullptr != current_node->prev_sibling)
		{
			profile_data* data = CreateDataFromNode(current_node);
			m_flatList.push_back(data);

			if (nullptr == prev_added_node)
			{
				prev_added_node = data;
			}
			else
			{
				data->next_sibling = prev_added_node;
				prev_added_node->prev_sibling = data;
				prev_added_node = data;
			}

			current_node = current_node->prev_sibling;
			continue;
		}

		if (nullptr != current_node->parent)
		{
			profile_data* data = CreateDataFromNode(current_node);
			m_flatList.push_back(data);

			if (nullptr == prev_added_node)
			{
				prev_added_node = data;
			}
			else
			{
				data->next_sibling = prev_added_node;
				prev_added_node->prev_sibling = data;
				prev_added_node = data;
			}

			current_node = current_node->parent;

			data = CreateDataFromNode(current_node);
			m_flatList.push_back(data);

			if (nullptr == prev_added_node)
			{
				prev_added_node = data;
			}
			else
			{
				SetSiblingsParent(prev_added_node, data);
				data->first_child = prev_added_node;
				prev_added_node = data;
			}

			if (nullptr != current_node)
			{
				profile_node* temp = current_node;

				while (nullptr != temp && nullptr == temp->prev_sibling)
				{
					temp = temp->parent;

					if (nullptr == temp)
						break;

					data = CreateDataFromNode(temp);
					m_flatList.push_back(data);
					data->first_child = prev_added_node;

					if (nullptr == prev_added_node->parent)
						prev_added_node->parent = data;

					if (nullptr != data_added_before_branch_traversal && nullptr != temp->parent)
					{
						data->next_sibling = data_added_before_branch_traversal;
						data_added_before_branch_traversal->prev_sibling = data;
						data_added_before_branch_traversal->parent = data->parent;
					}

					prev_added_node = data;
				}

				if (nullptr == temp)
					break;

				current_node = temp->prev_sibling;
			}

			continue;
		}
	}


	m_head = prev_added_node;
}

bool TotalTimeSort(profile_data* i, profile_data* j) { return (i->total_time_as_percent<j->total_time_as_percent); }
bool SelfTimeSort(profile_data* i, profile_data* j) { return (i->self_time_as_percent<j->self_time_as_percent); }

void ProfilerReport::SortByTotalTime()
{
	if (m_flatList.empty())
	{
		profile_data* current_data = m_head;
		bool has_reached_end_of_branch = false;
		bool is_sorting = false;

		while (nullptr != current_data)
		{
			// Step down first child until no children
			if (nullptr != current_data->first_child && !has_reached_end_of_branch)
			{
				current_data = current_data->first_child;
				continue;
			}

			// if has next, step to and walk down until no children
			if (nullptr != current_data->next_sibling && !is_sorting)
			{
				has_reached_end_of_branch = false;
				current_data = current_data->next_sibling;
				continue;
			}

			// if has prev but no next, then sort siblings
			if (nullptr != current_data->prev_sibling && nullptr == current_data->next_sibling || is_sorting)
			{
				// SORTING
					// move to prev if time is less than or equal
				profile_data* prev = current_data->prev_sibling;
				if (nullptr != prev && prev->total_time_as_percent >= current_data->total_time_as_percent)
				{
					has_reached_end_of_branch = true;
					is_sorting = true;
					current_data = current_data->prev_sibling;
				}
				else if (nullptr != prev)
				{
					// walk list to find next that is greater than me
					profile_data* lower_than_prev = current_data;
					while (lower_than_prev->total_time_as_percent > prev->total_time_as_percent)
					{
						if (nullptr != lower_than_prev->next_sibling)
							lower_than_prev = lower_than_prev->next_sibling;
						else
							break;
					}

					// Adjust Pointers
					current_data->prev_sibling = prev->prev_sibling;
					if (nullptr != current_data->prev_sibling)
						current_data->prev_sibling->next_sibling = current_data;

					if (prev->total_time_as_percent < lower_than_prev->total_time_as_percent)
					{
						prev->next_sibling = lower_than_prev->next_sibling;
						prev->prev_sibling = lower_than_prev;
						lower_than_prev->next_sibling = prev;
					}
					else
					{
						prev->next_sibling = lower_than_prev;
						lower_than_prev->prev_sibling->next_sibling = prev;
						prev->prev_sibling = lower_than_prev->prev_sibling;
						lower_than_prev->prev_sibling = prev;
					}
				}
				else
				{
					is_sorting = false;
					current_data = current_data->parent;

					if (nullptr == current_data)
						break;
				}
			}

			//step up to parent next is available, else sort parents and step to grandparents
			// if no prev and next siblings then step up to parent
			if (nullptr == current_data->prev_sibling && nullptr == current_data->next_sibling)
			{
				has_reached_end_of_branch = true;
				current_data = current_data->parent;
				continue;
			}
		}
	}
	else
	{
		std::sort(m_flatList.begin(), m_flatList.end(), TotalTimeSort);
	}
}

void ProfilerReport::SortBySelfTime()
{
	if(m_flatList.empty())
	{
		profile_data* current_data = m_head;
		bool has_reached_end_of_branch = false;
		bool is_sorting = false;

		while (nullptr != current_data)
		{
			// Step down first child until no children
			if (nullptr != current_data->first_child && !has_reached_end_of_branch)
			{
				current_data = current_data->first_child;
				continue;
			}

			// if has next, step to and walk down until no children
			if (nullptr != current_data->next_sibling && !is_sorting)
			{
				has_reached_end_of_branch = false;
				current_data = current_data->next_sibling;
				continue;
			}

			// if has prev but no next, then sort siblings
			if (nullptr != current_data->prev_sibling && nullptr == current_data->next_sibling || is_sorting)
			{
				// SORTING
				// move to prev if time is less than or equal
				profile_data* prev = current_data->prev_sibling;
				if (nullptr != prev && prev->self_time_as_percent >= current_data->self_time_as_percent)
				{
					has_reached_end_of_branch = true;
					is_sorting = true;
					current_data = current_data->prev_sibling;
				}
				else if (nullptr != prev)
				{
					// walk list to find next that is greater than me
					profile_data* lower_than_prev = current_data;
					while (lower_than_prev->self_time_as_percent > prev->self_time_as_percent)
					{
						if (nullptr != lower_than_prev->next_sibling)
							lower_than_prev = lower_than_prev->next_sibling;
						else
							break;
					}

					// Adjust Pointers
					current_data->prev_sibling = prev->prev_sibling;
					if (nullptr != current_data->prev_sibling)
						current_data->prev_sibling->next_sibling = current_data;

					if (prev->self_time_as_percent < lower_than_prev->self_time_as_percent)
					{
						prev->next_sibling = lower_than_prev->next_sibling;
						prev->prev_sibling = lower_than_prev;
						lower_than_prev->next_sibling = prev;
					}
					else
					{
						prev->next_sibling = lower_than_prev;
						lower_than_prev->prev_sibling->next_sibling = prev;
						prev->prev_sibling = lower_than_prev->prev_sibling;
						lower_than_prev->prev_sibling = prev;
					}
				}
				else
				{
					is_sorting = false;
					current_data = current_data->parent;
				}
			}

			//step up to parent next is available, else sort parents and step to grandparents
			// if no prev and next siblings then step up to parent
			if (nullptr == current_data->prev_sibling && nullptr == current_data->next_sibling)
			{
				has_reached_end_of_branch = true;
				current_data = current_data->parent;
				continue;
			}
		}
	}
	else
	{
		std::sort(m_flatList.begin(), m_flatList.end(), SelfTimeSort);
	}
}

void ProfilerReport::Log()
{
	profile_data* current_data = m_head;
	std::string tree_padding = "  ";

	std::string header_format = "%-55s %-10s %-10s %-15s %-10s %-15s %-10s %-10s";
	LogTaggedPrintf("profile", header_format.c_str(), " TAG NAME", "CALLS", "TOTAL%", "TOTAL TIME", "SELF%", "SELF TIME", "ALLOCS", "FREES");

	std::string msg_format = "%-55s %-10i %-10s %-15s %-10s %-15s %-10u %-10u";

	if(m_flatList.empty())
	{
		std::string head_tag = tree_padding + current_data->tag;

		std::string head_total_perc;
		std::stringstream head_stream_total;
		head_stream_total << std::fixed << std::setprecision(2) << (current_data->total_time_as_percent * 100.0f);
		head_total_perc = head_stream_total.str();
		head_total_perc += "%";

		std::string head_self_perc;
		std::stringstream head_stream_self;
		head_stream_self << std::fixed << std::setprecision(2) << (current_data->self_time_as_percent * 100.0f);
		head_self_perc = head_stream_self.str();
		head_self_perc += "%";

		LogTaggedPrintf("profile", msg_format.c_str(), head_tag.c_str(), current_data->call_count, head_total_perc.c_str(), current_data->total_time.c_str(), head_self_perc.c_str(), current_data->self_time.c_str(), current_data->alloc_count, current_data->free_count);

		while (nullptr != current_data)
		{
			// Add indent for each child traversal
			// Subtract when going up by parent
			// only log when traversing to child or sibling

			if (nullptr != current_data->first_child)
			{
				// Move down
				current_data = current_data->first_child;

				// Log Child
				if (m_style == TREE_VIEW)
					tree_padding += " ";

				std::string data_tag = tree_padding + current_data->tag;

				std::string total_perc;
				std::stringstream stream_total;
				stream_total << std::fixed << std::setprecision(2) << (current_data->total_time_as_percent * 100.0f);
				total_perc = stream_total.str();
				total_perc += "%";

				std::string self_perc;
				std::stringstream stream_self;
				stream_self << std::fixed << std::setprecision(2) << (current_data->self_time_as_percent * 100.0f);
				self_perc = stream_self.str();
				self_perc += "%";

				LogTaggedPrintf("profile", msg_format.c_str(), data_tag.c_str(), current_data->call_count, total_perc.c_str(), current_data->total_time.c_str(), self_perc.c_str(), current_data->self_time.c_str(), current_data->alloc_count, current_data->free_count);
				continue;
			}

			if (nullptr != current_data->next_sibling)
			{
				current_data = current_data->next_sibling;
				std::string data_tag = tree_padding + current_data->tag;
				std::string total_perc;
				std::stringstream stream_total;
				stream_total << std::fixed << std::setprecision(2) << (current_data->total_time_as_percent * 100.0f);
				total_perc = stream_total.str();
				total_perc += "%";

				std::string self_perc;
				std::stringstream stream_self;
				stream_self << std::fixed << std::setprecision(2) << (current_data->self_time_as_percent * 100.0f);
				self_perc = stream_self.str();
				self_perc += "%";

				LogTaggedPrintf("profile", msg_format.c_str(), data_tag.c_str(), current_data->call_count, total_perc.c_str(), current_data->total_time.c_str(), self_perc.c_str(), current_data->self_time.c_str(), current_data->alloc_count, current_data->free_count);
				continue;
			}
			else
			{
				profile_data* temp = current_data;
				while (nullptr != temp && nullptr == temp->parent->next_sibling)
				{
					if (m_style == TREE_VIEW)
						tree_padding.pop_back();

					temp = temp->parent;

					if (nullptr == temp->parent)
						break;
				}

				if (nullptr == temp)
					break;

				if (nullptr == temp->parent)
				{
					current_data = temp->parent;
					continue;
				}

				if (m_style == TREE_VIEW)
					tree_padding.pop_back();

				current_data = temp->parent->next_sibling;
				std::string data_tag = tree_padding + current_data->tag;
				std::string total_perc;
				std::stringstream stream_total;
				stream_total << std::fixed << std::setprecision(2) << (current_data->total_time_as_percent * 100.0f);
				total_perc = stream_total.str();
				total_perc += "%";

				std::string self_perc;
				std::stringstream stream_self;
				stream_self << std::fixed << std::setprecision(2) << (current_data->self_time_as_percent * 100.0f);
				self_perc = stream_self.str();
				self_perc += "%";

				LogTaggedPrintf("profile", msg_format.c_str(), data_tag.c_str(), current_data->call_count, total_perc.c_str(), current_data->total_time.c_str(), self_perc.c_str(), current_data->self_time.c_str(), current_data->alloc_count, current_data->free_count);
				continue;
			}

		}
	}
	else
	{
		for (profile_data* data : m_flatList)
		{
			std::string data_tag = tree_padding + data->tag;
			std::string total_perc;
			std::stringstream stream_total;
			stream_total << std::fixed << std::setprecision(2) << (current_data->total_time_as_percent * 100.0f);
			total_perc = stream_total.str();
			total_perc += "%";

			std::string self_perc;
			std::stringstream stream_self;
			stream_self << std::fixed << std::setprecision(2) << (current_data->self_time_as_percent * 100.0f);
			self_perc = stream_self.str();
			self_perc += "%";

			LogTaggedPrintf("profile", msg_format.c_str(), data_tag.c_str(), current_data->call_count, total_perc.c_str(), current_data->total_time.c_str(), self_perc.c_str(), current_data->self_time.c_str(), current_data->alloc_count, current_data->free_count);
		}
	}
}

void ProfilerReport::CreateTree(const ReportStyle& style)
{
	m_style = style;

	if (TREE_VIEW == style)
	{
		CreateTreeView();
		SortByTotalTime();
	}
	else if(FLAT_VIEW == style)
	{
		CreateFlatView();
		SortBySelfTime();
	}
}

void PrintReport(void* data)
{
#if defined PROFILED_BUILD
	arguments args = *(arguments*)data;

	std::locale local;
	std::string casedType = args.arg_list[0];
	for (unsigned int i = 0; i < casedType.length(); ++i)
		casedType[i] = std::tolower(casedType[i], local);


	ProfilerReport report(g_profilerRoot);

	ReportStyle style = TREE_VIEW;
	if (casedType == "flat_view")
		style = FLAT_VIEW;


	report.CreateTree(style);

	report.Log();
#endif
}

void RegisterProfilerCommands()
{
#if defined PROFILED_BUILD
	g_console->RegisterCommand("ProfilerPause", ProfilerPause);
	g_console->RegisterCommand("ProfilerResume", ProfilerResume);
	g_console->RegisterCommand("ProfilerReport", PrintReport);
#endif
}