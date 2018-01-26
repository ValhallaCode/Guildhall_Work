#pragma once
#include "Engine/Render/Motion.hpp"
#include "Engine/Render/Skeleton.hpp"
#include "Engine/RHI/RenderMesh.hpp"
#include <string>
#include <map>

class SimpleRenderer;
class ShaderProgram;
class KerningFont;

struct SubAnimation
{
	std::string name;
	float beginTime;
	float endTime;
	Motion* motion;
	float blend_start;
	ePlayMode play_mode;
};

struct Animation_t
{
	std::string name;
	Motion* motion;
	float start_time;
	ePlayMode play_mode;
	bool is_scalable;
};

struct Blend_Tree
{
	int m_animCount;
	std::vector<Animation_t*> m_animationsList;
	Animation_t* prev_start_anim;
	Animation_t* prev_end_anim;
	bool can_scale;
	float prev_scale_val;
};

struct Blend_Graph
{
	std::string name;
	Blend_Tree* north;
	Blend_Tree* south;
	Blend_Tree* east;
	Blend_Tree* west;
};

class Animator3D
{
public:
	Animator3D(const char* file_to_read, SimpleRenderer* renderer);
	Animator3D(RenderMesh* mesh, Motion* motion, Skeleton* skel);
	~Animator3D();
	void Evaluate(float time, const ePlayMode& playMode = FORWARD_LOOP);
	SubAnimation* CreateOrGetSubAnimation(const std::string& name, float beginTime = 0.0f, float endTime = 0.0f, const ePlayMode& playMode = FORWARD_LOOP);
	SubAnimation* CreateSubAnimationFromMotion(const std::string& name, float beginTime, float endTime, const ePlayMode& playMode = FORWARD_LOOP);
	Motion* CreateSubMotionFromTimeSegment(float beginTime, float endTime);
	void EvaluateWithoutSubAnimations(float time, ePlayMode playMode);
	void EvaluateWithSubAnimations(float time);
	void BlendAnimations(float time);
	void RunBaseAnimation(float time);
	void EvaluateWithAnimationsList(float time);
	void SetSubAnimationToPlay(const std::string& name, float overlapTime = 0.0f, float beginTime = 0.0f, float endTime = 0.0f, const ePlayMode& playMode = FORWARD_LOOP);
	void SetAnimationFromListToPlay(const std::string& name, float overlapTime = 0.0f, const std::string& custom_path = "", const std::string& fbx_filePath = "", const ePlayMode& playMode = FORWARD_LOOP, bool is_scalable = false, float frame_rate = 10.0f);
	Animation_t* CreateOrGetAnimation(const std::string& name, const std::string& custom_filePath = "", const std::string& fbx_filePath = "", const ePlayMode& playMode = FORWARD_LOOP, float frame_rate = 10.0f, bool is_scalable = false);
	Motion* CreateMotionFromFilePath(const std::string& fbx_filePath, const std::string& custom_filePath, float frame_rate);
	void ReadFromFile(const char* xml_filePath, SimpleRenderer* renderer);
	Blend_Tree* CreateBlendTree(const std::string& tree_name, int anim_name_count, ...);
	void LinearEvaluationOfBlendTree(Blend_Tree* tree, float normalized_blend_value);
	void GetLinearEvaluatedPoseForBlendtree(Blend_Tree* tree, float normalized_blend_value, Pose* out_pose);
	Blend_Graph* CreateOrGetBlendGraph(const std::string& name, Blend_Tree* north = nullptr, Blend_Tree* south = nullptr, Blend_Tree* east = nullptr, Blend_Tree* west = nullptr);
	void EvaluateBlendGraph(const Blend_Graph* graph, Vector2& neg_one_to_one_evals);
	void BlendPoses(Pose* first_pose, Pose* second_pose, float iterp_val, Pose* out_pose);
public:
	Pose* m_currentPose;
	RenderMesh* m_mesh;
	Motion* m_motion;
	Skeleton* m_skeleton;
	std::map<std::string, SubAnimation*> m_subAnimations;
	std::map<std::string, Animation_t*> m_animations;
	std::map<std::string, Blend_Tree*> m_trees;
	std::map<std::string, Blend_Graph*> m_graphs;
	Pose* m_poseFromLastAnimation;
	SubAnimation* m_currentSubAnimation;
	SubAnimation* m_prevSubAnimation;
	Animation_t* m_currentAnimation;
	Animation_t* m_prevAnimation;
	float m_overlapTime;
	bool m_startBlending;

	//Temp
private:
	Mesh* m_meshRef;
};
