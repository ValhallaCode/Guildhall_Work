#pragma once
#include "Engine/Math/Vector3.hpp"

class LineSegment3D
{
public:
	Vector3 m_start;
	Vector3 m_end;

	~LineSegment3D();
	LineSegment3D();
	LineSegment3D(const LineSegment3D& copy);
	explicit LineSegment3D(float initialX, float initialY, float initialZ);
	explicit LineSegment3D(float startX, float startY,float startZ, float endX, float endY, float endZ);
	explicit LineSegment3D(const Vector3& start, const Vector3& end);
	explicit LineSegment3D(const Vector3& center, float radiusX, float radiusY, float radiusZ);
	void StretchStartToPoint(const Vector3& point);
	void StretchEndToPoint(const Vector3& point);
	void AddPadding(float xPaddingRadius, float yPaddingRadius, float zPaddingRadius);
	void TranslateStart(const Vector3& translation);
	void TranslateEnd(const Vector3& translation);
	bool IsPointAlongLine(const Vector3& point) const;
	const float CalcLength() const;
	const Vector3 CalcMidPoint() const;
	const float CalcSlope() const;
	float CalcSlopeBetweenStartAndPoint(const Vector3& point) const;
	const Vector3 GetPointAtNormalizedSlopeOfLine(float normalizedSlope) const;
	const float GetNormalizedSlopeForPointAlongLine(const Vector3& point) const;

	const LineSegment3D operator + (const Vector3& translation) const;
	const LineSegment3D operator - (const Vector3& antiTranslation) const;
	void operator += (const Vector3& translation);
	void operator -= (const Vector3& antiTranslation);
	bool operator == (const LineSegment3D& lineToEqual) const;
	bool operator != (const LineSegment3D& lineToNotEqual) const;
	const LineSegment3D operator * (float scale) const;
	const LineSegment3D operator * (const Vector3& perAxisScaleFactors) const;
	const LineSegment3D operator / (float inverseScale) const;
	const LineSegment3D LineSegment3D::operator/(const Vector3& perAxisInverseScaleFactors) const;
	void operator *= (float scale);
	void operator *= (const Vector3& perAxisScaleFactors);
	friend const LineSegment3D Interpolate(const LineSegment3D& start, const LineSegment3D& end, float fractionToEnd);
};

const LineSegment3D Interpolate(const LineSegment3D& start, const LineSegment3D& end, float fractionToEnd);