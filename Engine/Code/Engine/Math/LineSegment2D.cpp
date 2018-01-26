#include "Engine/Math/LineSegment2D.hpp"
#include <math.h>

LineSegment2D::~LineSegment2D()
{}

LineSegment2D::LineSegment2D()
	:m_start(0.f, 0.f)
	, m_end(0.f, 0.f)
{}

LineSegment2D::LineSegment2D(const LineSegment2D& copy)
	:m_start(copy.m_start.x, copy.m_start.y)
	, m_end(copy.m_end.x, copy.m_end.y)
{}

LineSegment2D::LineSegment2D(float initialX, float initialY)
	: m_start(initialX, initialY)
	, m_end(initialX, initialY)
{}

LineSegment2D::LineSegment2D(float startX, float startY, float endX, float endY)
	:m_start(startX, startY)
	, m_end(endX, endY)
{}

LineSegment2D::LineSegment2D(const Vector2& start, const Vector2& end)
	:m_start(start.x, start.y)
	, m_end(end.x, end.y)
{}

LineSegment2D::LineSegment2D(const Vector2& center, float radiusX, float radiusY)
	:m_start(center.x - radiusX, center.y - radiusY)
	, m_end(center.x + radiusX, center.y + radiusY)
{}

void LineSegment2D::StretchStartToPoint(const Vector2& point)
{
	m_start -= point;
}

void LineSegment2D::StretchEndToPoint(const Vector2& point)
{
	m_end += point;
}

void LineSegment2D::AddPadding(float xPaddingRadius, float yPaddingRadius)
{
	m_start.x -= xPaddingRadius;
	m_start.y -= yPaddingRadius;
	m_end.x += xPaddingRadius;
	m_end.y += yPaddingRadius;
}

void LineSegment2D::TranslateStart(const Vector2& translation)
{
	m_start.x += translation.x;
	m_start.y += translation.y;
}

void LineSegment2D::TranslateEnd(const Vector2& translation)
{
	m_end.x += translation.x;
	m_end.y += translation.y;
}

bool LineSegment2D::IsPointAlongLine(const Vector2& point) const
{
	if (m_start.x > point.x || m_start.y > point.y || m_end.x < point.x || m_end.y < point.y)
		return false;
	float slopeStartToPoint = CalcSlopeBetweenStartAndPoint(point);
	float slopeOfLine = CalcSlope();
	if (slopeOfLine == slopeStartToPoint)
		return true;
	else
		return false;
}

const float LineSegment2D::CalcLength() const
{
	float changeInX = m_start.x - m_end.x;
	float changeInY = m_start.y - m_end.y;
	return (float) sqrt((changeInX * changeInX) + (changeInY * changeInY));
}

const Vector2 LineSegment2D::CalcMidPoint() const
{
	Vector2 midPoint(0.f, 0.f);
	midPoint.x = ((this->m_end.x - this->m_start.x) / 2) + this->m_start.x;
	midPoint.y = ((this->m_end.y - this->m_start.y) / 2) + this->m_start.y;
	return midPoint;
}

const float LineSegment2D::CalcSlope() const
{
	float changeInX = m_start.x - m_end.x;
	float changeInY = m_start.y - m_end.y;
	return (changeInY / changeInX);
}

float LineSegment2D::CalcSlopeBetweenStartAndPoint(const Vector2& point) const
{
	float changeInX = m_start.x - point.x;
	float changeInY = m_start.y - point.y;
	return (changeInY / changeInX);
}

const Vector2 LineSegment2D::GetPointAtNormalizedSlopeOfLine(float normalizedSlope) const
{
	Vector2 result(0.f, 0.f);
	float lengthX = m_end.x - m_start.x;
	float lengthY = m_end.y - m_start.y;

	result.x = (lengthX * normalizedSlope) + m_start.x;
	result.y = (lengthY * normalizedSlope) + m_start.y;
	return result;
}

const float LineSegment2D::GetNormalizedSlopeForPointAlongLine(const Vector2& point) const
{
	float slopeOfLine = CalcSlope();
	float slopeAtPoint = CalcSlopeBetweenStartAndPoint(point);

	return (slopeAtPoint / slopeOfLine);
}

void LineSegment2D::operator*=(const Vector2& perAxisScaleFactors)
{
	m_start *= perAxisScaleFactors;
	m_end *= perAxisScaleFactors;
}

void LineSegment2D::operator*=(float scale)
{
	m_start *= scale;
	m_end *= scale;
}

const LineSegment2D LineSegment2D::operator/(float inverseScale) const
{
	LineSegment2D result(0.f, 0.f);
	result.m_start = m_start / inverseScale;
	result.m_end = m_end / inverseScale;
	return result;
}

const LineSegment2D LineSegment2D::operator/(const Vector2& perAxisInverseScaleFactors) const
{
	LineSegment2D result(0.f, 0.f);
	result.m_start = this->m_start / perAxisInverseScaleFactors;
	result.m_end = this->m_end / perAxisInverseScaleFactors;
	return result;
}

const LineSegment2D LineSegment2D::operator*(const Vector2& perAxisScaleFactors) const
{
	LineSegment2D result(0.f, 0.f);
	result.m_start = this->m_start * perAxisScaleFactors;
	result.m_end = this->m_end * perAxisScaleFactors;
	return result;
}

const LineSegment2D LineSegment2D::operator*(float scale) const
{
	LineSegment2D result(0.f, 0.f);
	result.m_start = this->m_start * scale;
	result.m_end = this->m_end * scale;
	return result;
}

bool LineSegment2D::operator!=(const LineSegment2D& lineToNotEqual) const
{
	if (this->m_start != lineToNotEqual.m_start && this->m_end != lineToNotEqual.m_end)
		return true;
	else
		return false;
}

bool LineSegment2D::operator==(const LineSegment2D& lineToEqual) const
{
	if (this->m_start == lineToEqual.m_start && this->m_end == lineToEqual.m_end)
		return true;
	else
		return false;
}

void LineSegment2D::operator-=(const Vector2& antiTranslation)
{
	this->m_start -= antiTranslation;
	this->m_end -= antiTranslation;
}

void LineSegment2D::operator+=(const Vector2& translation)
{
	this->m_start += translation;
	this->m_end += translation;
}

const LineSegment2D LineSegment2D::operator-(const Vector2& antiTranslation) const
{
	LineSegment2D results(0.f, 0.f);
	results.m_start = this->m_start - antiTranslation;
	results.m_end = this->m_end - antiTranslation;
	return results;
}

const LineSegment2D LineSegment2D::operator+(const Vector2& translation) const
{
	LineSegment2D results(0.f, 0.f);
	results.m_start = this->m_start + translation;
	results.m_end = this->m_end + translation;
	return results;
}

const LineSegment2D Interpolate(const LineSegment2D& start, const LineSegment2D& end, float fractionToEnd)
{
	float fractionOfStart = 1.f - fractionToEnd;
	LineSegment2D blended;
	blended.m_start = (fractionOfStart * start.m_start) + (fractionToEnd * end.m_start);
	blended.m_end = (fractionOfStart * start.m_end) + (fractionToEnd * end.m_end);
	return blended;
}
