#include "Engine/Math/LineSegment3D.hpp"
#include <math.h>

LineSegment3D::~LineSegment3D()
{
}

LineSegment3D::LineSegment3D()
	:m_start(0.f, 0.f, 0.f)
	, m_end(0.f, 0.f, 0.f)
{
}

LineSegment3D::LineSegment3D(const LineSegment3D& copy)
	:m_start(copy.m_start.x, copy.m_start.y, copy.m_start.z)
	, m_end(copy.m_end.x, copy.m_end.y, copy.m_end.z)
{
}

LineSegment3D::LineSegment3D(float initialX, float initialY, float initialZ)
	:m_start(initialX, initialY, initialZ)
	,m_end(initialX, initialY, initialZ)
{
}

LineSegment3D::LineSegment3D(float startX, float startY, float startZ, float endX, float endY, float endZ)
	:m_start(startX, startY, startZ)
	, m_end(endX, endY, endZ)
{
}

LineSegment3D::LineSegment3D(const Vector3& start, const Vector3& end)
	:m_start(start)
	, m_end(end)
{
}

LineSegment3D::LineSegment3D(const Vector3& center, float radiusX, float radiusY, float radiusZ)
	:m_start(center.x - radiusX, center.y - radiusY, center.z - radiusZ)
	,m_end(center.x + radiusX, center.y + radiusY, center.z + radiusZ)
{
}

void LineSegment3D::StretchStartToPoint(const Vector3& point)
{
	m_start -= point;
}

void LineSegment3D::StretchEndToPoint(const Vector3& point)
{
	m_end += point;
}

void LineSegment3D::AddPadding(float xPaddingRadius, float yPaddingRadius, float zPaddingRadius)
{
	m_start.x -= xPaddingRadius;
	m_start.y -= yPaddingRadius;
	m_start.z -= zPaddingRadius;

	m_end.x += xPaddingRadius;
	m_end.y += yPaddingRadius;
	m_end.z += zPaddingRadius;
}

void LineSegment3D::TranslateStart(const Vector3& translation)
{
	m_start += translation;
}

void LineSegment3D::TranslateEnd(const Vector3& translation)
{
	m_end += translation;
}

bool LineSegment3D::IsPointAlongLine(const Vector3& point) const
{
	if (m_start.x > point.x || m_start.y > point.y || m_start.z > point.z || m_end.x < point.x || m_end.y < point.y || m_end.z < point.z)
		return false;
	float slopeStartToPoint = CalcSlopeBetweenStartAndPoint(point);
	float slopeOfLine = CalcSlope();
	if (slopeOfLine == slopeStartToPoint)
		return true;
	else
		return false;
}

const float LineSegment3D::CalcLength() const
{
	float changeInX = m_start.x - m_end.x;
	float changeInY = m_start.y - m_end.y;
	float changeInZ = m_start.z - m_end.z;
	return (float)sqrt((changeInX * changeInX) + (changeInY * changeInY) + (changeInZ * changeInZ));
}

const Vector3 LineSegment3D::CalcMidPoint() const
{
	Vector3 midPoint(0.f, 0.f, 0.f);
	midPoint.x = ((this->m_end.x - this->m_start.x) / 2) + this->m_start.x;
	midPoint.y = ((this->m_end.y - this->m_start.y) / 2) + this->m_start.y;
	midPoint.z = ((this->m_end.z - this->m_start.z) / 2) + this->m_start.z;
	return midPoint;
}

const float LineSegment3D::CalcSlope() const
{
	float changeInX = m_start.x - m_end.x;
	float changeInY = m_start.y - m_end.y;
	float runXY = (float)sqrt((changeInX * changeInX) + (changeInY * changeInY));
	float rise = m_end.z;
	return rise / runXY;
}

float LineSegment3D::CalcSlopeBetweenStartAndPoint(const Vector3& point) const
{
	float changeInX = m_start.x - point.x;
	float changeInY = m_start.y - point.y;
	float runXY = (float)sqrt((changeInX * changeInX) + (changeInY * changeInY));
	float rise = m_end.z;
	return rise / runXY;
}

const Vector3 LineSegment3D::GetPointAtNormalizedSlopeOfLine(float normalizedSlope) const
{
	Vector3 result(0.f, 0.f, 0.f);
	float lengthX = m_end.x - m_start.x;
	float lengthY = m_end.y - m_start.y;
	float lengthZ = m_end.z - m_start.z;

	result.x = (lengthX * normalizedSlope) + m_start.x;
	result.y = (lengthY * normalizedSlope) + m_start.y;
	result.z = (lengthZ * normalizedSlope) + m_start.z;
	return result;
}

const float LineSegment3D::GetNormalizedSlopeForPointAlongLine(const Vector3& point) const
{
	float slopeOfLine = CalcSlope();
	float slopeAtPoint = CalcSlopeBetweenStartAndPoint(point);

	return (slopeAtPoint / slopeOfLine);
}

void LineSegment3D::operator*=(const Vector3& perAxisScaleFactors)
{
	m_start *= perAxisScaleFactors;
	m_end *= perAxisScaleFactors;
}

const LineSegment3D LineSegment3D::operator/(float inverseScale) const
{
	LineSegment3D result(0.f, 0.f, 0.f);
	result.m_start = m_start / inverseScale;
	result.m_end = m_end / inverseScale;
	return result;
}

const LineSegment3D LineSegment3D::operator/(const Vector3& perAxisInverseScaleFactors) const
{
	LineSegment3D result(0.f, 0.f, 0.f);
	result.m_start = this->m_start / perAxisInverseScaleFactors;
	result.m_end = this->m_end / perAxisInverseScaleFactors;
	return result;
}

const LineSegment3D LineSegment3D::operator*(const Vector3& perAxisScaleFactors) const
{
	LineSegment3D result(0.f, 0.f, 0.f);
	result.m_start = this->m_start * perAxisScaleFactors;
	result.m_end = this->m_end * perAxisScaleFactors;
	return result;
}

const LineSegment3D LineSegment3D::operator*(float scale) const
{
	LineSegment3D result(0.f, 0.f, 0.f);
	result.m_start = this->m_start * scale;
	result.m_end = this->m_end * scale;
	return result;
}

bool LineSegment3D::operator!=(const LineSegment3D& lineToNotEqual) const
{
	if (this->m_start != lineToNotEqual.m_start && this->m_end != lineToNotEqual.m_end)
		return true;
	else
		return false;
}

bool LineSegment3D::operator==(const LineSegment3D& lineToEqual) const
{
	if (this->m_start == lineToEqual.m_start && this->m_end == lineToEqual.m_end)
		return true;
	else
		return false;
}

void LineSegment3D::operator-=(const Vector3& antiTranslation)
{
	this->m_start -= antiTranslation;
	this->m_end -= antiTranslation;
}

void LineSegment3D::operator+=(const Vector3& translation)
{
	this->m_start += translation;
	this->m_end += translation;
}

const LineSegment3D LineSegment3D::operator-(const Vector3& antiTranslation) const
{
	LineSegment3D results(0.f, 0.f, 0.f);
	results.m_start = this->m_start - antiTranslation;
	results.m_end = this->m_end - antiTranslation;
	return results;
}

const LineSegment3D LineSegment3D::operator+(const Vector3& translation) const
{
	LineSegment3D results(0.f, 0.f, 0.f);
	results.m_start = this->m_start + translation;
	results.m_end = this->m_end + translation;
	return results;
}

void LineSegment3D::operator*=(float scale)
{
	m_start *= scale;
	m_end *= scale;
}

const LineSegment3D Interpolate(const LineSegment3D& start, const LineSegment3D& end, float fractionToEnd)
{
	float fractionOfStart = 1.f - fractionToEnd;
	LineSegment3D blended;
	blended.m_start = (fractionOfStart * start.m_start) + (fractionToEnd * end.m_start);
	blended.m_end = (fractionOfStart * start.m_end) + (fractionToEnd * end.m_end);
	return blended;
}
