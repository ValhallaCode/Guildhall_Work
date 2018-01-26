#pragma once
#include "Engine/Math/Vector2.hpp"


class OBB2D
{
public:
	Vector2 m_center;
	Vector2 m_halfDimensions;
	Vector2 m_rightNormal;

	OBB2D();
	~OBB2D();
	OBB2D(const OBB2D& copy);
	OBB2D(Vector2 center, Vector2 radii, Vector2 rightNormal);
	void AddPadding(float xPadding, float yPadding);
	void Translate(const Vector2& translation);
	bool IsPointInside(const Vector2& refPoint) const;
	Vector2 CalcMin();
	Vector2 CalcMax();
	const Vector2 CalcSize() const;
	const Vector2 GetPointAtNormalizedPositionWithinBox(const Vector2& normalizedPosition);
	const Vector2 GetNormalizedPositionForPointWithinBox(const Vector2& point);
	friend const OBB2D Interpolate(const OBB2D& start, const OBB2D& end, float fractionToEnd);

	void operator += (const Vector2& translation);
	void operator -= (const Vector2& antiTranslation);
};
const OBB2D Interpolate(const OBB2D& start, const OBB2D& end, float fractionToEnd);