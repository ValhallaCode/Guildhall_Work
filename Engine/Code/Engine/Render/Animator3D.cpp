#include "Engine/Render/Animator3D.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "ThirdParty/FBX/fbx.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Core/Time.hpp"
#include <stdarg.h>
#include <vector>


Animator3D::Animator3D(const char* file_to_read, SimpleRenderer* renderer) 
	: m_currentPose(nullptr)
	, m_mesh(nullptr)
	, m_motion(nullptr)
	, m_skeleton(nullptr)
	, m_poseFromLastAnimation(nullptr)
	, m_currentSubAnimation(nullptr)
	, m_prevSubAnimation(nullptr)
	, m_currentAnimation(nullptr)
	, m_prevAnimation(nullptr)
	, m_overlapTime(0.0f)
	, m_startBlending(false)
{
	m_currentPose = new Pose();
	ReadFromFile(file_to_read, renderer);
	m_skeleton->InitializeStructuredBuffers(renderer->m_device);
}

Animator3D::Animator3D(RenderMesh* mesh, Motion* motion, Skeleton* skel)
	: m_currentPose(nullptr)
	, m_mesh(mesh)
	, m_motion(motion)
	, m_skeleton(skel)
	, m_poseFromLastAnimation(nullptr)
	, m_currentSubAnimation(nullptr)
	, m_prevSubAnimation(nullptr)
	, m_currentAnimation(nullptr)
	, m_prevAnimation(nullptr)
	, m_overlapTime(0.0f)
	, m_startBlending(false)
{
	m_currentPose = new Pose();
}

Animator3D::~Animator3D()
{

}

void Animator3D::Evaluate(float time, const ePlayMode& playMode /*= FORWARD_LOOP*/)
{
	if (m_subAnimations.empty() && m_animations.empty())
	{
		EvaluateWithoutSubAnimations(time, playMode);
	}
	else if(m_animations.empty())
	{
		EvaluateWithSubAnimations(time);
	}
	else
	{
		EvaluateWithAnimationsList(time);
	}
}

SubAnimation* Animator3D::CreateOrGetSubAnimation(const std::string& name, float beginTime /*= 0.0f*/, float endTime /*= 0.0f*/, const ePlayMode& playMode /*= FORWARD_LOOP*/)
{
	for (auto iterate = m_subAnimations.begin(); iterate != m_subAnimations.end(); ++iterate)
	{
		if (iterate->first == name)
		{
			return iterate->second;
		}
	}

	ASSERT_OR_DIE(endTime != 0, "Tried to Create a SubAnimation with an end time of 0!");
	ASSERT_OR_DIE(endTime <= m_motion->GetDuration(), "End Time Supplied is Larger than Motion Duration!");
	return CreateSubAnimationFromMotion(name, beginTime, endTime, playMode);
}

SubAnimation* Animator3D::CreateSubAnimationFromMotion(const std::string& name, float beginTime, float endTime, const ePlayMode& playMode /*= FORWARD_LOOP*/)
{
	SubAnimation* subAnim = new SubAnimation();

	subAnim->name = name;
	subAnim->beginTime = beginTime;
	subAnim->endTime = endTime;
	subAnim->motion = CreateSubMotionFromTimeSegment(beginTime, endTime);
	subAnim->blend_start = 0.0f;
	subAnim->play_mode = playMode;

	m_subAnimations.insert_or_assign(name, subAnim);
	return subAnim;
}

Motion* Animator3D::CreateSubMotionFromTimeSegment(float beginTime, float endTime)
{
	float evalBeginTime = m_motion->ClampForwardTime(beginTime) * m_motion->m_framerate;
	float evalEndTime = m_motion->ClampForwardTime(endTime) * m_motion->m_framerate;

	int beginFrame = (int)(floor(evalBeginTime));
	int endFrame = (int)(ceil(evalEndTime));
	int frameSpan = endFrame - beginFrame;

	Motion* motion = new Motion();
	motion->m_poses.resize(frameSpan);
	motion->SetFrameRate(m_motion->m_framerate);
	//Loop through frames in motion
	for (int index = 0; index < frameSpan; ++index)
	{
		motion->m_poses[index] = m_motion->m_poses[beginFrame + index];
	}

	return motion;
}

void Animator3D::EvaluateWithoutSubAnimations(float time, ePlayMode playMode)
{
	m_currentPose->m_localTransforms.clear();
	m_motion->Evaluate(m_currentPose, time, playMode);
}

void Animator3D::EvaluateWithSubAnimations(float time)
{
	if(m_prevSubAnimation != nullptr)
	{
		if (m_startBlending) {
			m_startBlending = false;
			m_currentSubAnimation->blend_start = time;
		}

		if(time - m_currentSubAnimation->blend_start <= m_overlapTime)
		{
			BlendAnimations(time);
		}
		else
		{
			m_prevSubAnimation = nullptr;
			// After finish overlap stage
			RunBaseAnimation(time);
		}
	}
	else
	{
		// After finish overlap stage
		RunBaseAnimation(time);
	}
}

void Animator3D::BlendAnimations(float time)
{
	Pose poseFromPrev;
	Pose poseFromNext;
	float begin_time;

	if (m_animations.empty())
	{
		m_prevSubAnimation->motion->Evaluate(&poseFromPrev, time - m_prevSubAnimation->blend_start, m_prevSubAnimation->play_mode);

		m_currentSubAnimation->motion->Evaluate(&poseFromNext, time - m_currentSubAnimation->blend_start, m_currentSubAnimation->play_mode);

		begin_time = m_currentSubAnimation->blend_start;
	}
	else
	{
		m_prevAnimation->motion->Evaluate(&poseFromPrev, time - m_prevAnimation->start_time, m_prevAnimation->play_mode);

		m_currentAnimation->motion->Evaluate(&poseFromNext, time - m_currentAnimation->start_time, m_currentAnimation->play_mode);

		begin_time = m_currentAnimation->start_time;
	}

	m_currentPose->m_localTransforms.clear();

	for (unsigned int index = 0; (index < poseFromPrev.m_localTransforms.size()) || (index < poseFromNext.m_localTransforms.size()); ++index)
	{
		Transform trans;
		trans.position = Interpolate(poseFromPrev.m_localTransforms[index].position, poseFromNext.m_localTransforms[index].position, (time - begin_time) / m_overlapTime); // evalFrame - firstFrame
		trans.scale = Interpolate(poseFromPrev.m_localTransforms[index].scale, poseFromNext.m_localTransforms[index].scale, (time - begin_time) / m_overlapTime);
		trans.rotation = SLERP(poseFromPrev.m_localTransforms[index].rotation, poseFromNext.m_localTransforms[index].rotation, (time - begin_time) / m_overlapTime);
		trans.rotation.Normalize();
		m_currentPose->m_localTransforms.push_back(trans);
	}
}

void Animator3D::RunBaseAnimation(float time)
{
	m_currentPose->m_localTransforms.clear();

	if (m_animations.empty())
	{
		m_currentSubAnimation->motion->Evaluate(m_currentPose, time - m_currentSubAnimation->blend_start, m_currentSubAnimation->play_mode);
	}
	else
	{
		m_currentAnimation->motion->Evaluate(m_currentPose, time - m_currentAnimation->start_time, m_currentAnimation->play_mode);
	}
}

void Animator3D::EvaluateWithAnimationsList(float time)
{
	if (m_prevAnimation != nullptr)
	{
		if (m_startBlending) {
			m_startBlending = false;
			m_currentAnimation->start_time = time;
		}

		if (time - m_currentAnimation->start_time <= m_overlapTime)
		{
			BlendAnimations(time);
		}
		else
		{
			m_prevAnimation = nullptr;
			// After finish overlap stage
			RunBaseAnimation(time);
		}
	}
	else
	{
		// After finish overlap stage
		RunBaseAnimation(time);
	}
}

void Animator3D::SetSubAnimationToPlay(const std::string& name, float overlapTime /*= 0.0f*/, float beginTime /*= 0.0f*/, float endTime /*= 0.0f*/, const ePlayMode& playMode /*= FORWARD_LOOP*/)
{
	if (m_currentSubAnimation == nullptr)
	{
		m_currentSubAnimation = CreateOrGetSubAnimation(name, beginTime, endTime, playMode);
	}
	else
	{
		if (m_currentSubAnimation->name == name)
			return;

		if (m_currentSubAnimation != nullptr) {
			m_prevSubAnimation = m_currentSubAnimation;

			m_overlapTime = overlapTime;
			m_startBlending = true;
		}

		m_currentSubAnimation = CreateOrGetSubAnimation(name, beginTime, endTime, playMode);
	}
}

void Animator3D::SetAnimationFromListToPlay(const std::string& name, float overlapTime /*= 0.0f*/, const std::string& custom_path /*= ""*/, const std::string& fbx_filePath /*= ""*/, const ePlayMode& playMode /*= FORWARD_LOOP*/, bool is_scalable /*= false*/, float frame_rate /*= 10.0f*/)
{
	if (m_currentAnimation == nullptr)
	{
		m_currentAnimation = CreateOrGetAnimation(name, custom_path, fbx_filePath, playMode);
	}
	else
	{
		if (m_currentAnimation->name == name)
			return;

		if (m_currentAnimation != nullptr) {
			m_prevAnimation = m_currentAnimation;

			m_overlapTime = overlapTime;
			m_startBlending = true;
		}

		m_currentAnimation = CreateOrGetAnimation(name, custom_path, fbx_filePath, playMode, frame_rate, is_scalable);
	}
}

Animation_t* Animator3D::CreateOrGetAnimation(const std::string& name, const std::string& custom_filePath /*= ""*/, const std::string& fbx_filePath /*= ""*/, const ePlayMode& playMode /*= FORWARD_LOOP*/, float frame_rate /*= 10.0f*/, bool is_scalable /*= false*/)
{
	for (auto iterate = m_animations.begin(); iterate != m_animations.end(); ++iterate)
	{
		if (iterate->first == name)
		{
			if(iterate->second->play_mode == playMode)
				return iterate->second;
		}
	}

	ASSERT_OR_DIE(fbx_filePath != "", "Animation Does not exist, and FBX file path is empty!");
	ASSERT_OR_DIE(custom_filePath != "", "Animation Does not exist, and Custom file path is empty!");

	Animation_t* anim = new Animation_t();
	anim->name = name;
	anim->play_mode = playMode;
	anim->start_time = 0;
	anim->motion = CreateMotionFromFilePath(fbx_filePath, custom_filePath, frame_rate);
	anim->is_scalable = is_scalable;

	m_animations.insert_or_assign(name, anim);
	return anim;
}

Motion* Animator3D::CreateMotionFromFilePath(const std::string& fbx_filePath, const std::string& custom_filePath, float frame_rate)
{
	Motion* motion = new Motion();
	ConvertFBXFileToEngineFilesOrReadEngineFile(fbx_filePath.c_str(), custom_filePath.c_str(), nullptr, m_skeleton, motion, (uint)frame_rate, IMPORT_MOTION);
	return motion;
}

void Animator3D::ReadFromFile(const char* xml_filePath, SimpleRenderer* renderer)
{
	tinyxml2::XMLDocument document;
	tinyxml2::XMLError result = document.LoadFile(xml_filePath);
	if (result != tinyxml2::XML_SUCCESS)
	{
		return;
	}

	auto xmlRoot = document.RootElement();
	if (xmlRoot == nullptr)
	{
		return;
	}

	auto skelEle = xmlRoot->FirstChildElement("Skeleton");
	std::string customPath = ParseXmlAttribute(*skelEle, "custom_file_path", std::string("No_Path_Found!"));
	std::string fbxPath = ParseXmlAttribute(*skelEle, "fbx_file_path", std::string("No_Path_Found!"));
	m_skeleton = new Skeleton();
	ConvertFBXFileToEngineFilesOrReadEngineFile(fbxPath.c_str(), customPath.c_str(), nullptr, m_skeleton, nullptr, 10, IMPORT_SKELETON);
	

	auto renderMeshEle = xmlRoot->FirstChildElement("RenderMesh");
	m_mesh = new RenderMesh();
	for (auto iterEle = renderMeshEle->FirstChildElement("Mesh"); iterEle != nullptr; iterEle = iterEle->NextSiblingElement("Mesh"))
	{
		auto meshEle = renderMeshEle->FirstChildElement("Mesh");
		std::string mesh_name = ParseXmlAttribute(*meshEle, "name", std::string("Name_Not_Found!"));
		std::string mesh_customPath = ParseXmlAttribute(*meshEle, "custom_file_path", std::string("No_Path_Found!"));
		std::string mesh_fbxPath = ParseXmlAttribute(*meshEle, "fbx_file_path", std::string("No_Path_Found!"));

		Mesh* mesh = CreateOrGetMesh(mesh_name);
		ConvertFBXFileToEngineFilesOrReadEngineFile(mesh_fbxPath.c_str(), mesh_customPath.c_str(), mesh, m_skeleton, nullptr, 10, IMPORT_MESH);
		m_mesh->InsertMesh(mesh_name, *mesh);
	}

	for (auto iterEle = renderMeshEle->FirstChildElement("Material"); iterEle != nullptr; iterEle = iterEle->NextSiblingElement("Material"))
	{
		std::string mat_name = ParseXmlAttribute(*iterEle, "name", std::string("Name_Not_Found!"));
		std::string mat_xmlPath = ParseXmlAttribute(*iterEle, "xml_file_path", std::string("No_Path_Found!"));

		Material* mat = CreateOrGetMaterial(mat_name, renderer, mat_xmlPath);
		m_mesh->InsertMaterial(mat_name, *mat);
	}

	auto motionsEle = xmlRoot->FirstChildElement("Motions");
	for (auto iterEle = motionsEle->FirstChildElement(); iterEle != nullptr; iterEle = iterEle->NextSiblingElement())
	{
		std::string name = iterEle->Name();

		float frame_rate = ParseXmlAttribute(*iterEle, "frame_rate", 10.0f);
		std::string motion_customPath = ParseXmlAttribute(*iterEle, "custom_file_path", std::string("No_Path_Found!"));
		std::string motion_fbxPath = ParseXmlAttribute(*iterEle, "fbx_file_path", std::string("No_Path_Found!"));

		std::string play_mode_string = ParseXmlAttribute(*iterEle, "play_mode", std::string("forward_loop"));
		ePlayMode play_mode = ConvertStringToPlayMode(play_mode_string);

		bool scalable = ParseXmlAttribute(*iterEle, "is_scalable", false);

		if (name == "Default" || name == "default")
		{
			m_motion = CreateOrGetMotion(name);
			m_motion->m_name = name;
			ConvertFBXFileToEngineFilesOrReadEngineFile(motion_fbxPath.c_str(), motion_customPath.c_str(), nullptr, m_skeleton, m_motion, (uint)frame_rate, IMPORT_MOTION);
		}
		else
		{
			CreateOrGetAnimation(name, motion_customPath, motion_fbxPath, play_mode, frame_rate, scalable);
		}
	}

	if (m_motion != nullptr)
	{
		auto subAnimEle = xmlRoot->FirstChildElement("SubAnimations");
		for (auto iterEle = subAnimEle->FirstChildElement(); iterEle != nullptr; iterEle = iterEle->NextSiblingElement())
		{
			std::string name = iterEle->Name();
			float start_time = ParseXmlAttribute(*iterEle, "begin_time", 0.0f);
			float end_time = ParseXmlAttribute(*iterEle, "end_time", m_motion->GetDuration());
			std::string play_mode_string = ParseXmlAttribute(*iterEle, "play_mode", std::string("forward_loop"));
			ePlayMode play_mode = ConvertStringToPlayMode(play_mode_string);

			CreateSubAnimationFromMotion(name, start_time, end_time, play_mode);
		}
	}
}

Blend_Tree* Animator3D::CreateBlendTree(const std::string& tree_name, int anim_name_count, ...)
{
	for (auto iterate = m_trees.begin(); iterate != m_trees.end(); ++iterate)
	{
		if (iterate->first == tree_name)
		{
			return iterate->second;
		}
	}

	va_list params;
	va_start(params, anim_name_count);

	std::vector<const char*> anim_name_list;
	for (int i = 0; i < anim_name_count; ++i)
	{
		const char* anim_name = va_arg(params, const char*);
		anim_name_list.push_back(anim_name);
	}
	anim_name_list.shrink_to_fit();
	va_end(params);

	Blend_Tree* tree = new Blend_Tree();
	tree->m_animCount = anim_name_count;
	tree->m_animationsList.resize(anim_name_count);
	tree->prev_start_anim = nullptr;
	tree->prev_end_anim = nullptr;
	tree->can_scale = false;
	tree->prev_scale_val = 0.0f;

	for (unsigned int index = 0; index < anim_name_list.size(); ++index)
	{
		auto anim_iterate = m_animations.find(anim_name_list[index]);

		ASSERT_OR_DIE(anim_iterate != m_animations.end(), "Currently Attempted Animation To Blend Does Not Exist in the Animator!");

		tree->m_animationsList[index] = anim_iterate->second;
	}

	return tree;
}

void Animator3D::LinearEvaluationOfBlendTree(Blend_Tree* tree, float normalized_blend_value)
{
	m_currentPose->m_localTransforms.clear();
	GetLinearEvaluatedPoseForBlendtree(tree, normalized_blend_value, m_currentPose);
}

void Animator3D::GetLinearEvaluatedPoseForBlendtree(Blend_Tree* tree, float normalized_blend_value, Pose* out_pose)
{
	normalized_blend_value = ClampNormalizedFloat(normalized_blend_value);
	float step_value = 1.0f / ((float)tree->m_animCount - 1.0f);

	float start_value_for_index = 0.0f;
	float end_value_for_index = step_value;
	int start_index = 0;
	int end_index = 1;

	while (!(start_value_for_index <= normalized_blend_value && normalized_blend_value <= end_value_for_index))
	{
		start_value_for_index += step_value;
		end_value_for_index += step_value;
		++start_index;
		++end_index;
	}

	Animation_t* start_anim;
	Animation_t* end_anim;

	start_anim = tree->m_animationsList[start_index];
	end_anim = tree->m_animationsList[end_index];

	if (tree->prev_start_anim != nullptr && tree->prev_end_anim != nullptr)
	{
		if (start_anim->name != tree->prev_start_anim->name && end_anim->name != tree->prev_end_anim->name)
		{
			tree->prev_start_anim = start_anim;
			tree->prev_end_anim = end_anim;
		}
	}
	else
	{
		tree->prev_start_anim = start_anim;
		tree->prev_end_anim = end_anim;
	}

	float inter_val = (normalized_blend_value - start_value_for_index) / step_value;
	inter_val = ClampNormalizedFloat(inter_val);

	float start_anim_duration = start_anim->motion->GetDuration();

	float end_anim_duration = end_anim->motion->GetDuration();

	float inter_duration = LERP(start_anim_duration, end_anim_duration, inter_val);
	float ds = CalculateDeltaSeconds();

	tree->prev_scale_val += (ds / inter_duration);
	tree->prev_scale_val = ClampNormalizedFloat(tree->prev_scale_val);

	Pose prev_pose;
	start_anim->motion->Evaluate(&prev_pose, tree->prev_scale_val * start_anim_duration, start_anim->play_mode);

	Pose next_pose;
	end_anim->motion->Evaluate(&next_pose, tree->prev_scale_val * end_anim_duration, end_anim->play_mode);

	BlendPoses(&prev_pose, &next_pose, inter_val, out_pose);
	
	if (tree->prev_scale_val == 1.0f)
		tree->prev_scale_val = 0.0f;
}

Blend_Graph* Animator3D::CreateOrGetBlendGraph(const std::string& name, Blend_Tree* north /*= nullptr*/, Blend_Tree* south /*= nullptr*/, Blend_Tree* east /*= nullptr*/, Blend_Tree* west /*= nullptr*/)
{
	for (auto iterate = m_graphs.begin(); iterate != m_graphs.end(); ++iterate)
	{
		if (iterate->first == name)
		{
			return iterate->second;
		}
	}

	bool all_nullptr = north == nullptr && south == nullptr && east == nullptr && west == nullptr;
	ASSERT_OR_DIE(!all_nullptr, "No Blend trees given for creation of " + name + " Graph!");

	Blend_Graph* graph = new Blend_Graph();
	graph->name = name;
	graph->north = north;
	graph->south = south;
	graph->east = east;
	graph->west = west;

	m_graphs.insert_or_assign(name, graph);
	return graph;
}

void Animator3D::EvaluateBlendGraph(const Blend_Graph* graph, Vector2& neg_one_to_one_evals)
{
	float x_val = neg_one_to_one_evals.x;
	float y_val = neg_one_to_one_evals.y;

	y_val = 0.0f;

	bool horizontal_pose_set = false;
	bool vertical_pose_set = false;

	Pose horiz_pose;
	Pose vert_pose;

	if (x_val > 0.0f)
	{
		// East Bound
		if (graph->east != nullptr)
		{
			horizontal_pose_set = true;
			float norm_x_val = ClampNormalizedFloat(x_val);
			GetLinearEvaluatedPoseForBlendtree(graph->east, norm_x_val, &horiz_pose);
		}
		else
		{
			if (graph->west != nullptr)
			{
				horizontal_pose_set = true;
				GetLinearEvaluatedPoseForBlendtree(graph->west, 0.0f, &horiz_pose);
			}
		}
	}
	else if(x_val < 0.0f)
	{
		// West Bound
		if (graph->west != nullptr)
		{
			horizontal_pose_set = true;
			float abs_x = abs(x_val);
			float norm_x_val = ClampNormalizedFloat(abs_x);
			GetLinearEvaluatedPoseForBlendtree(graph->west, norm_x_val, &horiz_pose);
		}
		else
		{
			if (graph->east != nullptr)
			{
				horizontal_pose_set = true;
				GetLinearEvaluatedPoseForBlendtree(graph->east, 0.0f, &horiz_pose);
			}
		}
	}
	else
	{
		if (graph->east != nullptr && graph->west != nullptr)
		{
			horizontal_pose_set = true;
			Pose east;
			Pose west;

			GetLinearEvaluatedPoseForBlendtree(graph->east, 0.0f, &east);
			GetLinearEvaluatedPoseForBlendtree(graph->west, 0.0f, &west);
			// Maybe figure out interp val Here
			BlendPoses(&east, &west, 0.5f, &horiz_pose);
		}
		else if (graph->east != nullptr)
		{
			horizontal_pose_set = true;
			GetLinearEvaluatedPoseForBlendtree(graph->east, 0.0f, &horiz_pose);
		}
		else if (graph->west != nullptr)
		{
			horizontal_pose_set = true;
			GetLinearEvaluatedPoseForBlendtree(graph->west, 0.0f, &horiz_pose);
		}
	}

	if (y_val > 0.0f)
	{
		// North Bound
		if (graph->north != nullptr)
		{
			vertical_pose_set = true;
			float norm_y_val = ClampNormalizedFloat(y_val);
			GetLinearEvaluatedPoseForBlendtree(graph->north, norm_y_val, &vert_pose);
		}
		else
		{
			if (graph->south != nullptr)
			{
				vertical_pose_set = true;
				GetLinearEvaluatedPoseForBlendtree(graph->south, 0.0f, &vert_pose);
			}
		}
	}
	else if (y_val < 0.0f)
	{
		// South Bound
		if (graph->south != nullptr)
		{
			vertical_pose_set = true;
			float abs_y = abs(y_val);
			float norm_y_val = ClampNormalizedFloat(abs_y);
			GetLinearEvaluatedPoseForBlendtree(graph->south, norm_y_val, &vert_pose);
		}
		else
		{
			if (graph->north != nullptr)
			{
				vertical_pose_set = true;
				GetLinearEvaluatedPoseForBlendtree(graph->north, 0.0f, &vert_pose);
			}
		}
	}
	else
	{
		if (graph->north != nullptr && graph->south != nullptr)
		{
			//vertical_pose_set = true;
			Pose north;
			Pose south;

			GetLinearEvaluatedPoseForBlendtree(graph->north, 0.0f, &north);
			GetLinearEvaluatedPoseForBlendtree(graph->south, 0.0f, &south);
			// Maybe figure out interp val Here
			BlendPoses(&north, &south, 0.5f, &vert_pose);
		}
		else if (graph->north != nullptr)
		{
			vertical_pose_set = true;
			GetLinearEvaluatedPoseForBlendtree(graph->north, 0.0f, &vert_pose);
		}
		else if (graph->south != nullptr)
		{
			vertical_pose_set = true;
			GetLinearEvaluatedPoseForBlendtree(graph->south, 0.0f, &vert_pose);
		}
	}


	if (horizontal_pose_set && vertical_pose_set)
	{
		m_currentPose->m_localTransforms.clear();
		BlendPoses(&horiz_pose, &vert_pose, abs(y_val), m_currentPose);
	}
	else if (horizontal_pose_set)
	{
		m_currentPose->m_localTransforms.clear();
		m_currentPose->m_localTransforms.reserve(horiz_pose.m_localTransforms.size());
		for (auto iterate : horiz_pose.m_localTransforms)
		{
			m_currentPose->m_localTransforms.push_back(iterate);
		}
	}
	else if (vertical_pose_set)
	{
		m_currentPose->m_localTransforms.clear();
		m_currentPose->m_localTransforms.reserve(vert_pose.m_localTransforms.size());
		for (auto iterate : vert_pose.m_localTransforms)
		{
			m_currentPose->m_localTransforms.push_back(iterate);
		}
	}


}

void Animator3D::BlendPoses(Pose* first_pose, Pose* second_pose, float iterp_val, Pose* out_pose)
{
	for (unsigned int index = 0; (index < first_pose->m_localTransforms.size()) || (index < second_pose->m_localTransforms.size()); ++index)
	{
		Transform trans;
		trans.position = Interpolate(first_pose->m_localTransforms[index].position, second_pose->m_localTransforms[index].position, iterp_val);
		trans.scale = Interpolate(first_pose->m_localTransforms[index].scale, second_pose->m_localTransforms[index].scale, iterp_val);
		trans.rotation = SLERP(first_pose->m_localTransforms[index].rotation, second_pose->m_localTransforms[index].rotation, iterp_val);
		trans.rotation.Normalize();
		out_pose->m_localTransforms.push_back(trans);
	}
}

