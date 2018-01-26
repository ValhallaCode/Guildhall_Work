#include "Engine/Math/AABB3D.hpp"

AABB3D::~AABB3D()
{
}

AABB3D::AABB3D()
	:mins(0.f, 0.f, 0.f)
	, maxs(0.f, 0.f, 0.f)
{
}

AABB3D::AABB3D(const AABB3D& copy)
	:mins(copy.mins.x, copy.mins.y, copy.mins.z)
	, maxs(copy.maxs.x, copy.maxs.y, copy.maxs.z)
{
}

AABB3D::AABB3D(float initialX, float initialY, float initialZ)
	:mins(0.f, 0.f, 0.f)
	,maxs(initialX, initialY, initialZ)
{
}

AABB3D::AABB3D(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
	:mins(minX, minY, minZ)
	, maxs(maxX, maxY, maxZ)
{
}

AABB3D::AABB3D(const Vector3& mins, const Vector3& maxs)
	:mins(mins.x, mins.y, mins.z)
	, maxs(maxs.x, maxs.y, maxs.z)
{
}

AABB3D::AABB3D(const Vector3& center, float radiusX, float radiusY, float radiusZ)
	:mins(center.x - radiusX, center.y - radiusY, center.z - radiusZ)
	, maxs(center.x + radiusX, center.y + radiusY, center.z + radiusZ)
{
}

void AABB3D::StretchToIncludePoint(const Vector3& point)
{
	float distanceBetweenMinAndPoint = CalcDistance(mins, point);
	if (distanceBetweenMinAndPoint > 0.f)
	{
		mins.x -= distanceBetweenMinAndPoint;
		mins.y -= distanceBetweenMinAndPoint;
		mins.z -= distanceBetweenMinAndPoint;
	}
	float distanceBetweenMaxAndPoint = CalcDistance(maxs, point);
	if (distanceBetweenMaxAndPoint > 0.f)
	{
		maxs.x += distanceBetweenMaxAndPoint;
		maxs.y += distanceBetweenMaxAndPoint;
		maxs.z += distanceBetweenMaxAndPoint;
	}
}

void AABB3D::AddPadding(float xPaddingRadius, float yPaddingRadius, float zPaddingRadius)
{
	mins.x -= xPaddingRadius;
	mins.y -= yPaddingRadius;
	mins.z -= zPaddingRadius;
	maxs.x += xPaddingRadius;
	maxs.y += yPaddingRadius;
	maxs.z += zPaddingRadius;
}

void AABB3D::Translate(const Vector3& translation)
{
	mins += translation;
	maxs += translation;
}

bool AABB3D::IsPointInside(const Vector3& point) const
{
	if (point.x > maxs.x || point.y > maxs.y || point.z > maxs.z || point.x < mins.x || point.y < mins.y || point.z < mins.z)
		return false;
	else
		return true;
}

const Vector3 AABB3D::CalcSize() const
{
	Vector3 size;
	size.x = maxs.x - mins.x;
	size.y = maxs.y - mins.y;
	size.z = maxs.z - mins.z;
	return size;
}

const Vector3 AABB3D::CalcCenter() const
{
	Vector3 center;
	center.x = ((maxs.x - mins.x) / 2) + mins.x;
	center.y = ((maxs.y - mins.y) / 2) + mins.y;
	center.z = ((maxs.z - mins.z) / 2) + mins.z;
	return center;
}

const Vector3 AABB3D::GetPointAtNormalizedPositionWithinBox(const Vector3& normalizedPosition) const
{
	Vector3 result(0.f, 0.f, 0.f);
	float lengthX = maxs.x - mins.x;
	float lengthY = maxs.y - mins.y;
	float lengthZ = maxs.z - mins.z;

	result.x = (lengthX * normalizedPosition.x) + mins.x;
	result.y = (lengthY * normalizedPosition.y) + mins.y;
	result.z = (lengthZ * normalizedPosition.z) + mins.z;
	return result;
}

const Vector3 AABB3D::GetNormalizedPositionForPointWithinBox(const Vector3& point) const
{
	Vector3 result(0.f, 0.f, 0.f);
	float lengthX = maxs.x - mins.x;
	float lengthY = maxs.y - mins.y;
	float lengthZ = maxs.z - mins.z;
	if (lengthX > 0)
		result.x = (point.x - mins.x) / lengthX;
	if (lengthY > 0)
		result.y = (point.y - mins.y) / lengthY;
	if (lengthZ > 0)
		result.z = (point.z - mins.z) / lengthZ;
	return result;
}

void AABB3D::operator-=(const Vector3& antiTranslation)
{
	mins -= antiTranslation;
	maxs -= antiTranslation;
}

void AABB3D::operator+=(const Vector3& translation)
{
	mins += translation;
	maxs += translation;
}

const AABB3D AABB3D::operator-(const Vector3& antiTranslation) const
{
	AABB3D aabbToWorkOn;
	aabbToWorkOn.mins = this->mins - antiTranslation;
	aabbToWorkOn.maxs = this->maxs - antiTranslation;
	return aabbToWorkOn;
}

const AABB3D AABB3D::operator+(const Vector3& translation) const
{
	AABB3D aabbToWorkOn;
	aabbToWorkOn.mins = this->mins + translation;
	aabbToWorkOn.maxs = this->maxs + translation;
	return aabbToWorkOn;
}

const AABB3D Interpolate(const AABB3D& start, const AABB3D& end, float fractionToEnd)
{
	float fractionOfStart = 1.f - fractionToEnd;
	AABB3D blended;
	blended.mins = (fractionOfStart * start.mins) + (fractionToEnd * end.mins);
	blended.maxs = (fractionOfStart * start.maxs) + (fractionToEnd * end.maxs);
	return blended;
}

