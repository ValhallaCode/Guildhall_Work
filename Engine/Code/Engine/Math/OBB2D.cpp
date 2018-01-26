#include "Engine/Math/OBB2D.hpp"


OBB2D::OBB2D()
	:m_center(1.f, 1.f)
	,m_halfDimensions(1.f, 1.f)
	,m_rightNormal(1.f, 0.f)
{
}

OBB2D::OBB2D(const OBB2D& copy)
	:m_center(copy.m_center)
	,m_halfDimensions(copy.m_halfDimensions)
	,m_rightNormal(copy.m_rightNormal)
{
}

OBB2D::OBB2D(Vector2 center, Vector2 radii, Vector2 rightNormal)
	:m_center(center)
	,m_halfDimensions(radii)
	,m_rightNormal(rightNormal)
{
}

void OBB2D::AddPadding(float xPadding, float yPadding)
{
	m_halfDimensions.x += xPadding;
	m_halfDimensions.y += yPadding;
}

void OBB2D::Translate(const Vector2& translation)
{
	m_center += translation;
}

bool OBB2D::IsPointInside(const Vector2& refPoint) const
{
	float distToPoint = CalcDistance(m_center, refPoint);
	return (m_halfDimensions.x > distToPoint && m_halfDimensions.y > distToPoint);
}

Vector2 OBB2D::CalcMin()
{
	Vector2 mins;
	Vector2 downwardNormal = Vector2(m_rightNormal.x, -m_rightNormal.y);

	Vector2 leftEdge = m_halfDimensions.x * (-1.f * m_rightNormal);
	Vector2 bottomEdge = m_halfDimensions.y * downwardNormal;
	Vector2 vectorToPoint = leftEdge + bottomEdge;
	mins = vectorToPoint + m_center;
	return mins;
}

Vector2 OBB2D::CalcMax() 
{
	Vector2 maxs;
	Vector2 upwardNormal = Vector2(-m_rightNormal.y, m_rightNormal.x);

	Vector2 rightEdge = m_halfDimensions.x * m_rightNormal;
	Vector2 topEdge = m_halfDimensions.y * upwardNormal;
	Vector2 vectorToPoint = rightEdge + topEdge;
	maxs = vectorToPoint + m_center;
	return maxs;
}

const Vector2 OBB2D::CalcSize() const
{
	Vector2 size;
	size.x = 2.f * m_halfDimensions.x;
	size.y = 2.f * m_halfDimensions.y;
	return size;
}

const Vector2 OBB2D::GetPointAtNormalizedPositionWithinBox(const Vector2& normalizedPosition) 
{
	Vector2 result(0.f, 0.f);
	float lengthX = 2.f * m_halfDimensions.x;
	float lengthY = 2.f * m_halfDimensions.y;

	Vector2 mins = this->CalcMin();

	result.x = (lengthX * normalizedPosition.x) + mins.x;
	result.y = (lengthY * normalizedPosition.y) + mins.y;
	return result;
}

const Vector2 OBB2D::GetNormalizedPositionForPointWithinBox(const Vector2& point) 
{
	Vector2 result(0.f, 0.f);
	float lengthX = 2.f * m_halfDimensions.x;
	float lengthY = 2.f * m_halfDimensions.y;

	Vector2 mins = this->CalcMin();

	if (lengthX > 0)
		result.x = (point.x - mins.x) / lengthX;
	if (lengthY > 0)
		result.y = (point.y - mins.y) / lengthY;
	return result;
}

void OBB2D::operator-=(const Vector2& antiTranslation)
{
	Vector2 mins = this->CalcMin();
	Vector2 maxs = this->CalcMax();

	mins -= antiTranslation;
	maxs -= antiTranslation;
}

void OBB2D::operator+=(const Vector2& translation)
{
	Vector2 mins = this->CalcMin();
	Vector2 maxs = this->CalcMax();

	mins += translation;
	maxs += translation;
}

OBB2D::~OBB2D()
{
}

const OBB2D Interpolate(const OBB2D& start, const OBB2D& end, float fractionToEnd)
{
	float fractionOfStart = 1.f - fractionToEnd;
	OBB2D blended;
	blended.m_center.x = (fractionOfStart * start.m_center.x) + (fractionToEnd * end.m_center.x);
	blended.m_center.y = (fractionOfStart * start.m_center.y) + (fractionToEnd * end.m_center.y);
	blended.m_halfDimensions.x = (fractionOfStart * start.m_halfDimensions.x) + (fractionToEnd * end.m_halfDimensions.x);
	blended.m_halfDimensions.y = (fractionOfStart * start.m_halfDimensions.y) + (fractionToEnd * end.m_halfDimensions.y);
	blended.m_rightNormal.x = (fractionOfStart * start.m_rightNormal.x) + (fractionToEnd * end.m_rightNormal.x);
	blended.m_rightNormal.y = (fractionOfStart * start.m_rightNormal.y) + (fractionToEnd * end.m_rightNormal.y);
	return blended;
}
