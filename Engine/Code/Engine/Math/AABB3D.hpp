#pragma once
#include "Engine/Math/Vector3.hpp"

class AABB3D
{
public:
	Vector3 mins;
	Vector3 maxs;
	static const AABB3D ZERO_TO_ONE;

	~AABB3D();
	AABB3D();
	AABB3D(const AABB3D& copy);
	explicit AABB3D(float initialX, float initialY, float initialZ);
	explicit AABB3D(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
	explicit AABB3D(const Vector3& mins, const Vector3& maxs);
	explicit AABB3D(const Vector3& center, float radiusX, float radiusY, float radiusZ);
	void StretchToIncludePoint(const Vector3& point);
	void AddPadding(float xPaddingRadius, float yPaddingRadius, float zPaddingRadius);
	void Translate(const Vector3& translation);
	bool IsPointInside(const Vector3& point) const;
	const Vector3 CalcSize() const;
	const Vector3 CalcCenter() const;
	const Vector3 GetPointAtNormalizedPositionWithinBox(const Vector3& normalizedPosition) const;
	const Vector3 GetNormalizedPositionForPointWithinBox(const Vector3& point) const;
	friend const AABB3D Interpolate(const AABB3D& start, const AABB3D& end, float fractionToEnd);

	const AABB3D operator + (const Vector3& translation) const;
	const AABB3D operator - (const Vector3& antiTranslation) const;
	void operator += (const Vector3& translation);
	void operator -= (const Vector3& antiTranslation);
};
const AABB3D Interpolate(const AABB3D& start, const AABB3D& end, float fractionToEnd);