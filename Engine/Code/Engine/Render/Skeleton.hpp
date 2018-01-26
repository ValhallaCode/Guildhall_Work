#pragma once
#include "Engine/Math/Matrix4.hpp"
#include <string>
#include <vector>

class StructuredBuffer;
class RHIDevice;
class RHIDeviceContext;
class Pose;
class BinaryStream;

static int INVALID_INDEX = -1;

class Skeleton
{
public:
	Skeleton();
	~Skeleton();
	// Reset the skeleton - clear out all bones that make up
	// this skeleton
	void Clear();

	// Adds a joint.  Can be parented to another 
	// joint within this skeleton.
	void AddJoint(const std::string& name, const std::string& parent_name, const Matrix4 &transform);

	// get number of joints/bones in this skeleton.
	unsigned int GetJointCount() const;

	// Get a joint index by name, returns
	// (uint)(-1) if it doesn't exist.
	unsigned int GetJointIndex(const std::string& name);

	std::string GetJointName(unsigned int index);
	unsigned int GetJointParent(unsigned int index);
	bool DoesJointHaveParent(unsigned int index);
	// Get the global transform for a joint.
	Matrix4 GetJointTransform(unsigned int joint_idx) const;
	Matrix4 GetJointTransform(const std::string& name) const;
	void InitializeStructuredBuffers(RHIDevice* device);
	void CalculateSkinMatrix(RHIDeviceContext* context, Pose* pose);
	void WriteToStream(BinaryStream* stream);
	void ReadFromStream(BinaryStream* stream);
public:
	// All vectors need to be parallel
	std::vector<Matrix4> m_globalTransform;
	std::vector<std::string> m_names;
	std::vector<unsigned int> m_parentsIndex;
	StructuredBuffer* m_skinTransforms;
};