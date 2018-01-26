#include "Engine/Render/Skeleton.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/RHI/StructuredBuffer.hpp"
#include "Engine/RHI/RHI.hpp"
#include "Engine/Render/Pose.hpp"
#include "Engine/Input/BinaryStream.hpp"

Skeleton::Skeleton()
{

}

Skeleton::~Skeleton()
{

}

void Skeleton::Clear()
{
	m_globalTransform.clear();
	m_names.clear();
}

void Skeleton::AddJoint(const std::string& name, const std::string& parent_name, const Matrix4 &transform)
{
	m_names.push_back(name);
	m_globalTransform.push_back(transform);

	unsigned int parentIndex = GetJointIndex(parent_name);
	m_parentsIndex.push_back(parentIndex);
}

unsigned int Skeleton::GetJointCount() const
{
	return m_globalTransform.size();
}

unsigned int Skeleton::GetJointIndex(const std::string& name)
{
	for (unsigned int index = 0; index < m_names.size(); ++index)
	{
		std::string storedName = m_names[index];
		if (name == storedName)
		{
			return index;
		}
	}
	return INVALID_INDEX;
}

std::string Skeleton::GetJointName(unsigned int index)
{
	ASSERT_OR_DIE(index < m_names.size(), "Index is outside of name array!");
	return m_names[index];
}

unsigned int Skeleton::GetJointParent(unsigned int index)
{
	return m_parentsIndex[index];
}

bool Skeleton::DoesJointHaveParent(unsigned int index)
{
	return m_parentsIndex[index] != (unsigned int)INVALID_INDEX;
}

Matrix4 Skeleton::GetJointTransform(unsigned int joint_idx) const
{
	return m_globalTransform[joint_idx];
}

Matrix4 Skeleton::GetJointTransform(const std::string& name) const
{
	for (unsigned int index = 0; index < m_names.size(); ++index)
	{
		std::string storedName = m_names[index];
		if (name == storedName)
		{
			return m_globalTransform[index];
		}
	}
	return Matrix4();
}

void Skeleton::InitializeStructuredBuffers(RHIDevice* device)
{
	std::vector<Matrix4> matrices;
	matrices.resize(m_globalTransform.size());
	m_skinTransforms = new StructuredBuffer(device, matrices.data(), sizeof(Matrix4), matrices.size());
}

void Skeleton::CalculateSkinMatrix(RHIDeviceContext* context, Pose* pose)
{
	unsigned int jointCount = GetJointCount();

	std::vector<Matrix4> skinMatrices;
	skinMatrices.reserve(jointCount);

	for (unsigned int index = 0; index < jointCount; ++index)
	{
		Matrix4 currentWorld = pose->GetGlobalTransformForLocalIndex(this,index);
		Matrix4 initialPose = m_globalTransform[index];

		Matrix4 invInitPose = initialPose.GetInverse();

		Matrix4 bindPose = MatrixMultiplicationRowMajorAB(invInitPose, currentWorld);
		bindPose.Transpose();
		skinMatrices.push_back(bindPose);
	}

	m_skinTransforms->Update(context, skinMatrices.data());
}

void Skeleton::WriteToStream(BinaryStream* stream)
{
	size_t globalSize = m_globalTransform.size();
	size_t namesSize = m_names.size();
	size_t parentIndexSize = m_parentsIndex.size();

	stream->write(globalSize);
	for (uint globalIndex = 0; globalIndex < m_globalTransform.size(); ++globalIndex)
	{
		Matrix4& matrix = m_globalTransform[globalIndex];
		stream->write(matrix);
	}

	stream->write(namesSize);
	for (uint nameIndex = 0; nameIndex < m_names.size(); ++nameIndex)
	{
		std::string& name = m_names[nameIndex];
		stream->write(name.size());
		stream->write(name);
	}

	stream->write(parentIndexSize);
	for (uint indexVal = 0; indexVal < m_parentsIndex.size(); ++indexVal)
	{
		unsigned int parentIndex = m_parentsIndex[indexVal];
		stream->write(parentIndex);
	}
}

void Skeleton::ReadFromStream(BinaryStream* stream)
{
	size_t globalSize;
	size_t namesSize;
	size_t parentIndexSize;

	stream->read(&globalSize);
	m_globalTransform.reserve(globalSize);
	for (uint globalIndex = 0; globalIndex < globalSize; ++globalIndex)
	{
		Matrix4 matrix;
		stream->read(&matrix);
		m_globalTransform.push_back(matrix);
	}

	stream->read(&namesSize);
	m_names.reserve(namesSize);
	for (uint nameIndex = 0; nameIndex < namesSize; ++nameIndex)
	{
		std::string name;
		size_t stringSize; 
		stream->read(&stringSize);
		name.resize(stringSize);
		stream->read(&name);
		m_names.push_back(name);
	}

	stream->read(&parentIndexSize);
	m_parentsIndex.reserve(parentIndexSize);
	for (uint indexVal = 0; indexVal < parentIndexSize; ++indexVal)
	{
		unsigned int parentIndex;
		stream->read(&parentIndex);
		m_parentsIndex.push_back(parentIndex);
	}

}