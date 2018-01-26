#include "Engine/Math/MatrixStack.hpp"


MatrixStack::MatrixStack()
	:m_top{}
	, m_stack{}
{

}

MatrixStack::~MatrixStack()
{

}

void MatrixStack::PushDirect(Matrix4& matrix)
{
	m_stack.push_back(m_top);
	m_top = matrix;
}

void MatrixStack::Push(Matrix4& matrix)
{
	m_stack.push_back(m_top);
	m_top.ConcatenateTranform(matrix);
}

void MatrixStack::Pop()
{
	m_top = m_stack.back();
	m_stack.pop_back();
}

Matrix4 MatrixStack::GetTop()
{
	return m_top;
}

