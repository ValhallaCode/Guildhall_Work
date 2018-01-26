#pragma once
#include "Engine/Math/Matrix4.hpp"
#include <vector>


class MatrixStack
{
public:
	MatrixStack();
	~MatrixStack();
	void PushDirect(Matrix4& matrix);
	void Push(Matrix4& matrix);
	void Pop();
	Matrix4 GetTop();
public:
	Matrix4 m_top;
	std::vector<Matrix4> m_stack;
};