#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Quaternion.hpp"
#include <vector>

class Matrix4;
class Skeleton;

struct Transform
{ 
	Vector3 position;
	Vector3 scale;    
	Quaternion rotation; 
}; 


class Pose {
public:
	Pose();
	~Pose();
	Matrix4 GetGlobalTransformForLocalIndex(Skeleton* skeleton, unsigned int jointIndex);
	Matrix4 MakeMatrixFromTransform(Transform& transform);
public:
	std::vector<Transform> m_localTransforms;
};