#include "Engine/Render/Pose.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Render/Skeleton.hpp"


Pose::Pose()
{

}

Pose::~Pose()
{

}

Matrix4 Pose::GetGlobalTransformForLocalIndex(Skeleton* skeleton, unsigned int jointIndex)
{
	//if (!skeleton->DoesJointHaveParent(jointIndex))
	//	return Matrix4();

	unsigned int parentIndex = skeleton->GetJointParent(jointIndex);

	Transform myTransform = m_localTransforms[jointIndex];
	Matrix4 myMatrix = MakeMatrixFromTransform(myTransform);

	//Transform parentTransform = m_localTransforms[parentIndex];
	//Matrix4 parentMatrix = MakeMatrixFromTransform(parentTransform);

	Matrix4 ancestry = myMatrix; // MatrixMultiplicationRowMajorAB(myMatrix, parentMatrix);

	//if(skeleton->DoesJointHaveParent(parentIndex))
	//{
		for (int iterate = parentIndex;/*skeleton->GetJointParent(parentIndex);*/ iterate != INVALID_INDEX; iterate = skeleton->GetJointParent(iterate))
		{
			Transform grandParentTransform = m_localTransforms[iterate];
			Matrix4 grandParentMatrix = MakeMatrixFromTransform(grandParentTransform);

			ancestry = MatrixMultiplicationRowMajorAB(ancestry, grandParentMatrix);
		}
	//}

	return ancestry;
}

Matrix4 Pose::MakeMatrixFromTransform(Transform& transform)
{
	Matrix4 temp;
	Matrix4 scale = temp.CreateScale(transform.scale);
	Matrix4 position = temp.CreateTranslation(transform.position);
	Matrix4 rotation(transform.rotation);

	return MatrixMultiplicationRowMajorAB(MatrixMultiplicationRowMajorAB(scale, rotation), position);
}