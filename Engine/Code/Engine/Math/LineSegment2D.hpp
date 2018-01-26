#pragma once
#include "Engine/Math/Vector2.hpp"

class LineSegment2D
{
public:
	Vector2 m_start;
	Vector2 m_end;

	~LineSegment2D();
	LineSegment2D();
	LineSegment2D(const LineSegment2D& copy);
	explicit LineSegment2D(float initialX, float initialY);
	explicit LineSegment2D(float startX, float startY, float endX, float endY);
	explicit LineSegment2D(const Vector2& start, const Vector2& end);
	explicit LineSegment2D(const Vector2& center, float radiusX, float radiusY);
	void StretchStartToPoint(const Vector2& point);
	void StretchEndToPoint(const Vector2& point);
	void AddPadding(float xPaddingRadius, float yPaddingRadius);
	void TranslateStart(const Vector2& translation);
	void TranslateEnd(const Vector2& translation);
	bool IsPointAlongLine(const Vector2& point) const;
	const float CalcLength() const;
	const Vector2 CalcMidPoint() const;
	const float CalcSlope() const;
	float CalcSlopeBetweenStartAndPoint(const Vector2& point) const;
	const Vector2 GetPointAtNormalizedSlopeOfLine(float normalizedSlope) const;
	const float GetNormalizedSlopeForPointAlongLine(const Vector2& point) const;

	const LineSegment2D operator + (const Vector2& translation) const;
	const LineSegment2D operator - (const Vector2& antiTranslation) const;
	void operator += (const Vector2& translation);
	void operator -= (const Vector2& antiTranslation);
	bool operator == (const LineSegment2D& lineToEqual) const;
	bool operator != (const LineSegment2D& lineToNotEqual) const;
	const LineSegment2D operator * (float scale) const;
	const LineSegment2D operator * (const Vector2& perAxisScaleFactors) const;
	const LineSegment2D operator / (float inverseScale) const;
	const LineSegment2D LineSegment2D::operator/(const Vector2& perAxisInverseScaleFactors) const;
	void operator *= (float scale);
	void operator *= (const Vector2& perAxisScaleFactors);
	friend const LineSegment2D Interpolate(const LineSegment2D& start, const LineSegment2D& end, float fractionToEnd);
};

const LineSegment2D Interpolate(const LineSegment2D& start, const LineSegment2D& end, float fractionToEnd);