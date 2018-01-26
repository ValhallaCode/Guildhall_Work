#include "Engine/Math/Disc2D.hpp"
#include <math.h>

Disc2D::~Disc2D()
{}

Disc2D::Disc2D()
	:m_center(0.f,0.f)
	, m_radius(0.f)
{}

Disc2D::Disc2D(const Disc2D& copy)
	:m_center(copy.m_center)
	, m_radius(copy.m_radius)
{}

Disc2D::Disc2D(float initialX, float initialY, float initialRadius)
	:m_center(initialX, initialY)
	, m_radius(initialRadius)
{}

Disc2D::Disc2D(const Vector2& initialCenter, float initialRadius)
	:m_center(initialCenter)
	, m_radius(initialRadius)
{}

void Disc2D::StretchToIncludePoint(const Vector2& point)
{
	float differenceInX = point.x - m_center.x;
	float differenceInY = point.y - m_center.y;
	m_radius = (float)sqrt((differenceInX * differenceInX) + (differenceInY * differenceInY));
}

void Disc2D::AddPadding(float paddingRadius)
{
	m_radius += paddingRadius;
}

void Disc2D::Translate(const Vector2& translation)
{
	m_center += translation;
}

bool Disc2D::IsPointInside(const Vector2& point) const
{
	float distanceToPoint = CalcDistance(point, m_center);
	if (m_radius >= distanceToPoint)
		return true;
	else
		return false;
}

bool DoDiscsOverlap(Disc2D& DiscA, Disc2D& DiscB)
{
	float distanceBetweenDiscs = CalcDistance(DiscA.m_center, DiscB.m_center);
	float sumOfRadii = DiscA.m_radius + DiscB.m_radius;

	if (distanceBetweenDiscs < sumOfRadii)
		return true;
	else
		return false;
}

bool DoesTheDiscOverlapWithAnAABB2D(Disc2D& disc, AABB2D& aabb)
{
	float rightOfDisc = disc.m_center.x + disc.m_radius;
	float leftOfDisc = disc.m_center.x - disc.m_radius;
	float topOfDisc = disc.m_center.y + disc.m_radius;
	float bottomOfDisc = disc.m_center.y - disc.m_radius;

	if (rightOfDisc > aabb.mins.x)
		return true;

	if (leftOfDisc < aabb.maxs.x)
		return true;

	if (topOfDisc > aabb.mins.y)
		return true;

	if (bottomOfDisc < aabb.maxs.y)
		return true;

	return false;
}

bool Disc2D::operator==(const Disc2D& discToEqual)
{
	return (m_center == discToEqual.m_center && m_radius == discToEqual.m_radius);
}

void Disc2D::operator-=(const Vector2& antiTranslation)
{

	m_center.x -= antiTranslation.x;
	m_center.y -= antiTranslation.y;
}

void Disc2D::operator+=(const Vector2& translation)
{
	m_center.x += translation.x;
	m_center.y += translation.y;
}

const Disc2D Disc2D::operator-(const Vector2& antiTranslation) const
{
	Disc2D discToOperateOn;
	discToOperateOn.m_center.x = this->m_center.x - antiTranslation.x;
	discToOperateOn.m_center.y = this->m_center.y - antiTranslation.y;
	return discToOperateOn;
}

const Disc2D Disc2D::operator+(const Vector2& translation) const
{
	Disc2D discToOperateOn;
	discToOperateOn.m_center.x = this->m_center.x + translation.x;
	discToOperateOn.m_center.y = this->m_center.y + translation.y;
	return discToOperateOn;
}

void BounceBothDiscs2D(Disc2D& discA, Disc2D& discB, Vector2& velocityA, Vector2& velocityB, float elasticity)
{
	if (!DoDiscsOverlap(discA, discB))
		return;
	
	Vector2 displacementAToB = discB.m_center - discA.m_center;
	float distanceToCenters = displacementAToB.CalcLength();
	Vector2 directionAtoB = displacementAToB;
	directionAtoB.Normalize();
	float overlapDistance = (discA.m_radius + discB.m_radius) - distanceToCenters;
	Vector2 bPositionCorrection = directionAtoB * (overlapDistance * .5f);
	discB += bPositionCorrection;
	discA -= bPositionCorrection;

	float speedOfAToB = DotProduct(velocityA, directionAtoB);
	float speedOfBFromA = DotProduct(velocityB, directionAtoB);
	if (speedOfAToB > speedOfBFromA)
	{
		Vector2 parallelVelocityA = directionAtoB * speedOfAToB;
		Vector2 perpindicularVelocityA = velocityA - parallelVelocityA;
		Vector2 parallelVelocityB = directionAtoB * speedOfBFromA;
		Vector2 perpindicularVelocityB = velocityB - parallelVelocityB;

		velocityA = perpindicularVelocityA + ((elasticity * parallelVelocityB));	//TODO Add in masses
		velocityB = perpindicularVelocityB + ((elasticity * parallelVelocityA));
	}
}

void BounceFirstDiscOffTheSecond(Disc2D& movingDisc, Disc2D& fixedDisc, Vector2& velocityOfDiscA, float elasticity)
{
	if (!DoDiscsOverlap(movingDisc, fixedDisc))
		return;

	Vector2 displacementAToB = fixedDisc.m_center - movingDisc.m_center;
	float distanceToCenters = displacementAToB.CalcLength();
	Vector2 directionAtoB = displacementAToB;
	directionAtoB.Normalize();
	float overlapDistance = (movingDisc.m_radius + fixedDisc.m_radius) - distanceToCenters;
	Vector2 aPositionCorrection = (-1.f * directionAtoB) * overlapDistance;
	movingDisc += aPositionCorrection;

	float speedOfMovingDiscTowardFixed = DotProduct(velocityOfDiscA, directionAtoB);
	if (speedOfMovingDiscTowardFixed > 0.f)
	{
		Vector2 parallelVelocityA = directionAtoB * speedOfMovingDiscTowardFixed;
		Vector2 perpindicularVelocityA = velocityOfDiscA - parallelVelocityA;
		velocityOfDiscA = perpindicularVelocityA - ((elasticity * parallelVelocityA));
	}
}

void BounceDiscOffAABB2D(Disc2D& disc, AABB2D& aabb, Vector2& velocityOfDisc, float massOfDisc, float elasticity)
{
	float rightOfDisc = disc.m_center.x + disc.m_radius;
	float leftOfDisc = disc.m_center.x - disc.m_radius;
	float topOfDisc = disc.m_center.y + disc.m_radius;
	float bottomOfDisc = disc.m_center.y - disc.m_radius;

	if (rightOfDisc > aabb.mins.x)
	{
		float overlap = rightOfDisc - aabb.mins.x;
		disc.m_center.x -= overlap;

		velocityOfDisc.x = (-1 * elasticity * velocityOfDisc.x) / massOfDisc;
	}

	if (leftOfDisc < aabb.maxs.x)
	{
		float overlap = aabb.maxs.x - leftOfDisc;
		disc.m_center.x += overlap;

		velocityOfDisc.x = (-1 * elasticity * velocityOfDisc.x) / massOfDisc;
	}

	if (topOfDisc > aabb.mins.y)
	{
		float overlap = topOfDisc - aabb.mins.y;
		disc.m_center.y -= overlap;

		velocityOfDisc.y = (-1 * elasticity * velocityOfDisc.y) / massOfDisc;
	}

	if (bottomOfDisc < aabb.maxs.y)
	{
		float overlap = aabb.maxs.y - bottomOfDisc;
		disc.m_center.y += overlap;

		velocityOfDisc.y = (-1 * elasticity * velocityOfDisc.y) / massOfDisc;
	}
}

const Disc2D Interpolate(const Disc2D& start, const Disc2D& end, float fractionToEnd)
{
	float fractionOfStart = 1.f - fractionToEnd;
	Disc2D blended;
	blended.m_center = (fractionOfStart * start.m_center) + (fractionToEnd * end.m_center);
	blended.m_radius = (fractionOfStart * start.m_radius) + (fractionToEnd * end.m_radius);
	return blended;
}
