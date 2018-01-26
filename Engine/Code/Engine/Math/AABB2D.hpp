#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector4.hpp"

enum eAlignment : unsigned int 
{
	TOP_LEFT,
	TOP,
	TOP_RIGHT,
	LEFT,
	CENTER,
	RIGHT,
	BOTTOM_LEFT,
	BOTTOM,
	BOTTOM_RIGHT
};

class AABB2D
{
public:
	Vector2 mins;
	Vector2 maxs;
	static const AABB2D ZERO_TO_ONE;

	~AABB2D();
	AABB2D();
	AABB2D(const AABB2D& copy);
	explicit AABB2D(float initialX, float initialY);
	explicit AABB2D(float minX, float minY, float maxX, float maxY);
	explicit AABB2D(const Vector2& mins, const Vector2& maxs);
	explicit AABB2D(const Vector2& center, float radiusX, float radiusY);
	void StretchToIncludePoint(const Vector2& point);
	void AddPadding(float xPaddingRadius, float yPaddingRadius);
	void Translate(const Vector2& translation);
	bool IsPointInside(const Vector2& point) const;
	const Vector2 CalcSize() const;
	const Vector2 CalcCenter() const;
	const Vector2 GetUV(const Vector2& normalizedPosition) const;
	const Vector2 CalcClosestPoint(const Vector2& point);
	const Vector2 Sample(const Vector2& point) const;
	const Vector2 GetExtends() const;
	friend const AABB2D Interpolate(const AABB2D& start, const AABB2D& end, float fractionToEnd);

	const AABB2D operator + (const Vector2& translation) const;
	const AABB2D operator - (const Vector2& antiTranslation) const;
	void operator += (const Vector2& translation);
	void operator -= (const Vector2& antiTranslation);
};
const AABB2D Interpolate(const AABB2D& start, const AABB2D& end, float fractionToEnd);
const Vector2 GetSmallestOffset(const AABB2D& obj, const AABB2D& container);
const AABB2D MoveToBestFit(const AABB2D& obj, const AABB2D& container);
// Anchor and offset are in left, bottom, right, top order
const AABB2D GetBounds(const AABB2D& parent, const Vector4& anchor, const Vector4& offset);
const AABB2D AlignToContainer(const AABB2D& bounds, const AABB2D& container, const eAlignment& alignment);