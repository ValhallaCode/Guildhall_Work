#include "Engine/Math/AABB2D.hpp"


AABB2D::~AABB2D()
{
}

AABB2D::AABB2D()
	:mins(0.f,0.f)
	, maxs(0.f,0.f)
{}

AABB2D::AABB2D(const AABB2D& copy)
	:mins(copy.mins)
	, maxs(copy.maxs)
{
}

AABB2D::AABB2D(float initialX, float initialY)
	:mins(0.f,0.f)
	, maxs(initialX, initialY)
{
}

AABB2D::AABB2D(float minX, float minY, float maxX, float maxY)
	:mins(minX, minY)
	, maxs(maxX, maxY)
{
}

AABB2D::AABB2D(const Vector2& mins, const Vector2& maxs)
	:mins(mins)
	, maxs(maxs)
{
}

AABB2D::AABB2D(const Vector2& center, float radiusX, float radiusY)
{
	float minX = center.x - radiusX;
	float minY = center.y - radiusY;
	mins = Vector2(minX, minY);

	float maxX = center.x + radiusX;
	float maxY = center.y + radiusY;
	maxs = Vector2(maxX, maxY);
}

void AABB2D::StretchToIncludePoint(const Vector2& point)
{
	float distanceBetweenMinAndPoint = CalcDistance(mins, point);
	if (distanceBetweenMinAndPoint > 0.f)
	{
		mins.x -= distanceBetweenMinAndPoint;
		mins.y -= distanceBetweenMinAndPoint;
	}
	float distanceBetweenMaxAndPoint = CalcDistance(maxs, point);
	if (distanceBetweenMaxAndPoint > 0.f)
	{
		maxs.x += distanceBetweenMaxAndPoint;
		maxs.y += distanceBetweenMaxAndPoint;
	}
}

void AABB2D::AddPadding(float xPaddingRadius, float yPaddingRadius)
{
	mins.x -= xPaddingRadius;
	mins.y -= yPaddingRadius;
	maxs.x += xPaddingRadius;
	maxs.y += yPaddingRadius;
}

void AABB2D::Translate(const Vector2& translation)
{
	mins += translation;
	maxs += translation;
}

bool AABB2D::IsPointInside(const Vector2& point) const
{
	if (point.x > maxs.x || point.y > maxs.y || point.x < mins.x || point.y < mins.y)
		return false;
	else
		return true;
}

const Vector2 AABB2D::CalcSize() const
{
	Vector2 size;
	size.x = maxs.x - mins.x;
	size.y = maxs.y - mins.y;
	return size;
}

const Vector2 AABB2D::CalcCenter() const
{
	Vector2 center;
	center.x = ((maxs.x - mins.x) / 2) + mins.x;
	center.y = ((maxs.y - mins.y) / 2) + mins.y;
	return center;
}

const Vector2 AABB2D::GetUV(const Vector2& normalizedPosition) const
{
	Vector2 result(0.f, 0.f);
	float lengthX = maxs.x - mins.x;
	float lengthY = maxs.y - mins.y;
	
	result.x = (lengthX * normalizedPosition.x) + mins.x;
	result.y = (lengthY * normalizedPosition.y) + mins.y;
	return result;
}

const Vector2 AABB2D::CalcClosestPoint(const Vector2& point)
{
	if (IsPointInside(point)) {
		return point;
	}

	//Region I
	if (point.x < mins.x && maxs.y < point.y) {
		return Vector2(mins.x, maxs.y);
	}
	//Region II
	if (point.x < mins.x && point.y < mins.y) {
		return Vector2(mins.x, mins.y);
	}
	//Region III
	if (maxs.x < point.x && point.y < mins.y) {
		return Vector2(maxs.x, mins.y);
	}
	//Region IV
	if (maxs.x < point.x && maxs.y < point.y) {
		return Vector2(maxs.x, maxs.y);
	}
	//Region V
	if (point.x < mins.x) {
		return Vector2(mins.x, point.y);
	}
	//Region VI
	if (maxs.x < point.x) {
		return Vector2(maxs.x, point.y);
	}
	//Region VII
	if (point.y < mins.y) {
		return Vector2(point.x, mins.y);
	}
	//Region VIII
	if (maxs.y < point.y) {
		return Vector2(point.x, maxs.y);
	}

	return Vector2(0.0f, 0.0f);
}

const Vector2 AABB2D::Sample(const Vector2& point) const
{
	Vector2 result(0.f, 0.f);
	float lengthX = maxs.x - mins.x;
	float lengthY = maxs.y - mins.y;
	if(lengthX > 0)
		result.x = (point.x - mins.x) / lengthX;
	if(lengthY > 0)
		result.y = (point.y - mins.y) / lengthY;
	return result;
}

const Vector2 AABB2D::GetExtends() const
{
	Vector2 dimensions = CalcSize();
	dimensions *= 0.5f;
	return dimensions;
}

const AABB2D Interpolate(const AABB2D& start, const AABB2D& end, float fractionToEnd)
{
	float fractionOfStart = 1.f - fractionToEnd;
	AABB2D blended;
	blended.mins = (fractionOfStart * start.mins) + (fractionToEnd * end.mins);
	blended.maxs = (fractionOfStart * start.maxs) + (fractionToEnd * end.maxs);
	return blended;
}

const Vector2 GetSmallestOffset(const AABB2D& obj, const AABB2D& container)
{
	Vector2 extends = obj.GetExtends();
	AABB2D inner_box = container;
	inner_box.AddPadding(-extends.x, -extends.y);

	Vector2 obj_center = obj.CalcCenter();
	Vector2 closest_point = inner_box.CalcClosestPoint(obj_center);
	return closest_point - obj_center;
}

const AABB2D MoveToBestFit(const AABB2D& obj, const AABB2D& container)
{
	Vector2 offset = GetSmallestOffset(obj, container);

	AABB2D new_box = obj;
	new_box.Translate(offset);
	return new_box;
}

const AABB2D GetBounds(const AABB2D& parent, const Vector4& anchor, const Vector4& offset)
{
	Vector2 offset_mins(offset.x, offset.y);
	Vector2 offset_maxs(offset.z, offset.w);

	Vector2 anchor_mins(anchor.x, anchor.y);
	Vector2 anchor_maxs(anchor.z, anchor.w);

	Vector2 bound_mins = parent.GetUV(anchor_mins) + offset_mins;
	Vector2 bound_maxs = parent.GetUV(anchor_maxs) + offset_maxs;

	return AABB2D(bound_mins, bound_maxs);
}

const Vector2 GetAlignmentVector(const eAlignment& alignment)
{
	switch (alignment)
	{
	case TOP_LEFT:
		return Vector2(0.0f, 1.0f);
	case TOP:
		return Vector2(0.5f, 1.0f);
	case TOP_RIGHT:
		return Vector2(1.0f, 1.0f);
	case LEFT:
		return Vector2(0.0f, 0.5f);
	case CENTER:
		return Vector2(0.5f, 0.5f);
	case RIGHT:
		return Vector2(1.0f, 0.5f);
	case BOTTOM_LEFT:
		return Vector2(0.0f, 0.0f);
	case BOTTOM:
		return Vector2(0.5f, 0.0f);
	case BOTTOM_RIGHT:
		return Vector2(1.0f, 0.0f);
	default:
		return Vector2(0.5f, 0.5f);
	}
}

const AABB2D AlignToContainer(const AABB2D& bounds, const AABB2D& container, const eAlignment& alignment)
{
	Vector2 norm_align = GetAlignmentVector(alignment);

	Vector2 distance = container.GetUV(norm_align) - bounds.GetUV(norm_align);
	AABB2D new_box = bounds;
	new_box.Translate(distance);
	return new_box;
}

void AABB2D::operator-=(const Vector2& antiTranslation)
{
	mins -= antiTranslation;
	maxs -= antiTranslation;
}

void AABB2D::operator+=(const Vector2& translation)
{
	mins += translation;
	maxs += translation;
}

const AABB2D AABB2D::operator-(const Vector2& antiTranslation) const
{
	AABB2D aabbToWorkOn;
	aabbToWorkOn.mins = this->mins - antiTranslation;
	aabbToWorkOn.maxs = this->maxs - antiTranslation;
	return aabbToWorkOn;
}

const AABB2D AABB2D::operator+(const Vector2& translation) const
{
	AABB2D aabbToWorkOn;
	aabbToWorkOn.mins = this->mins + translation;
	aabbToWorkOn.maxs = this->maxs + translation;
	return aabbToWorkOn;
}
