#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/AABB2D.hpp"

class Disc2D
{
public:
	Vector2 m_center;
	float m_radius;
	static const Disc2D UNIT_CIRCLE;

	~Disc2D();
	Disc2D();
	Disc2D(const Disc2D& copy);
	explicit Disc2D(float initialX, float initialY, float initialRadius);
	explicit Disc2D(const Vector2& initialCenter, float initialRadius);
	void StretchToIncludePoint(const Vector2& point);
	void AddPadding(float paddingRadius);
	void Translate(const Vector2& translation);
	bool IsPointInside(const Vector2& point) const;
	friend bool DoDiscsOverlap(Disc2D& DiscA, Disc2D& DiscB);
	friend bool DoesTheDiscOverlapWithAnAABB2D(Disc2D& disc, AABB2D& aabb);
	friend void BounceBothDiscs2D(Disc2D& discA, Disc2D& discB, Vector2& velocityA, Vector2& velocityB, float elasticity);
	friend void BounceFirstDiscOffTheSecond(Disc2D& discA, Disc2D& discB, Vector2& velocityOfDiscA, float elasticity);
	friend void BounceDiscOffAABB2D(Disc2D& disc, AABB2D& aabb, Vector2& velocityOfDisc, float massOfDisc, float elasticity);
	friend const Disc2D Interpolate(const Disc2D& start, const Disc2D& end, float fractionToEnd);

	const Disc2D operator + (const Vector2& translation) const;
	const Disc2D operator - (const Vector2& antiTranslation) const;
	void operator += (const Vector2& translation);
	void operator -= (const Vector2& antiTranslation);
	bool operator == (const Disc2D& discToEqual);
};
bool DoDiscsOverlap(Disc2D& DiscA, Disc2D& DiscB);
bool DoesTheDiscOverlapWithAnAABB2D(Disc2D& disc, AABB2D& aabb);
void BounceFirstDiscOffTheSecond(Disc2D& discA, Disc2D& discB, Vector2& velocityOfDiscA, float elasticity);
void BounceBothDiscs2D(Disc2D& discA, Disc2D& discB, Vector2& velocityA, Vector2& velocityB, float elasticity);
void BounceDiscOffAABB2D(Disc2D& disc, AABB2D& aabb, Vector2& velocityOfDisc, float massOfDisc, float elasticity);
const Disc2D Interpolate(const Disc2D& start, const Disc2D& end, float fractionToEnd);