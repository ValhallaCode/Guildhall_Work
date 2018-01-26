#include "Engine/Math/Sphere3D.hpp"
#include "Engine/Math/Math3D.hpp"
#include <math.h>

Sphere3D::~Sphere3D()
{
}

Sphere3D::Sphere3D()
	:m_center(0.f, 0.f, 0.f)
	, m_radius(0.f)
{
}

Sphere3D::Sphere3D(const Sphere3D& copy)
	: m_center(copy.m_center.x, copy.m_center.y, copy.m_center.z)
	, m_radius(copy.m_radius)
{
}

Sphere3D::Sphere3D(float initialX, float initialY, float initialZ, float initialRadius)
	:m_center(initialX, initialY, initialZ)
	,m_radius(initialRadius)
{
}

Sphere3D::Sphere3D(const Vector3& initialCenter, float initialRadius)
	:m_center(initialCenter.x, initialCenter.y, initialCenter.z)
	, m_radius(initialRadius)
{
}

void Sphere3D::StretchToIncludePoint(const Vector3& point)
{
	float differenceInX = point.x - m_center.x;
	float differenceInY = point.y - m_center.y;
	float differenceInZ = point.z - m_center.z;
	m_radius = (float)sqrt((differenceInX * differenceInX) + (differenceInY * differenceInY) + (differenceInZ * differenceInZ));
}

void Sphere3D::AddPadding(float paddingRadius)
{
	m_radius += paddingRadius;
}

void Sphere3D::Translate(const Vector3& translation)
{
	m_center += translation;
}

bool Sphere3D::IsPointInside(const Vector3& point) const
{
	float distanceToPoint = CalcDistance(point, m_center);
	if (m_radius >= distanceToPoint)
		return true;
	else
		return false;
}

bool Sphere3D::operator==(const Sphere3D& sphereToEqual)
{
	return (m_center == sphereToEqual.m_center && m_radius == sphereToEqual.m_radius);
}

void Sphere3D::operator-=(const Vector3& antiTranslation)
{
	m_center.x -= antiTranslation.x;
	m_center.y -= antiTranslation.y;
	m_center.z -= antiTranslation.z;
}

void Sphere3D::operator+=(const Vector3& translation)
{
	m_center.x += translation.x;
	m_center.y += translation.y;
	m_center.z += translation.z;
}

const Sphere3D Sphere3D::operator-(const Vector3& antiTranslation) const
{
	Sphere3D sphereToOperateOn;
	sphereToOperateOn.m_center.x = this->m_center.x - antiTranslation.x;
	sphereToOperateOn.m_center.y = this->m_center.y - antiTranslation.y;
	sphereToOperateOn.m_center.z = this->m_center.z - antiTranslation.z;
	return sphereToOperateOn;
}

const Sphere3D Sphere3D::operator+(const Vector3& translation) const
{
	Sphere3D sphereToOperateOn;
	sphereToOperateOn.m_center.x = this->m_center.x + translation.x;
	sphereToOperateOn.m_center.y = this->m_center.y + translation.y;
	sphereToOperateOn.m_center.z = this->m_center.z + translation.z;
	return sphereToOperateOn;
}

void BounceFirstSphereOffTheSecond(Sphere3D& movingSphere, Sphere3D& fixedSphere, Vector3& velocityOfMovingSphere, float elasticity)
{
	if (!DoSpheresOverlap(movingSphere, fixedSphere))
		return;

	Vector3 displacementAToB = fixedSphere.m_center - movingSphere.m_center;
	float distanceToCenters = displacementAToB.CalcLength();
	Vector3 directionAtoB = displacementAToB;
	directionAtoB.Normalize();
	float overlapDistance = (movingSphere.m_radius + fixedSphere.m_radius) - distanceToCenters;
	Vector3 aPositionCorrection = (-1.f * directionAtoB) * overlapDistance;
	movingSphere += aPositionCorrection;

	float speedOfMovingDiscTowardFixed = DotProduct(velocityOfMovingSphere, directionAtoB);
	if (speedOfMovingDiscTowardFixed > 0.f)
	{
		Vector3 parallelVelocityA = directionAtoB * speedOfMovingDiscTowardFixed;
		Vector3 perpindicularVelocityA = velocityOfMovingSphere - parallelVelocityA;
		velocityOfMovingSphere = perpindicularVelocityA - ((elasticity * parallelVelocityA));
	}
}

const Sphere3D Interpolate(const Sphere3D& start, const Sphere3D& end, float fractionToEnd)
{
	float fractionOfStart = 1.f - fractionToEnd;
	Sphere3D blended;
	blended.m_center = (fractionOfStart * start.m_center) + (fractionToEnd * end.m_center);
	blended.m_radius = (fractionOfStart * start.m_radius) + (fractionToEnd * end.m_radius);
	return blended;
}

void BounceBothSpheres3D(Sphere3D& sphereA, Sphere3D& sphereB, Vector3& velocityA, Vector3& velocityB, float elasticity)
{
	if (!DoSpheresOverlap(sphereA, sphereB))
		return;

	Vector3 displacementAToB = sphereB.m_center - sphereA.m_center;
	float distanceToCenters = displacementAToB.CalcLength();
	Vector3 directionAtoB = displacementAToB;
	directionAtoB.Normalize();
	float overlapDistance = (sphereA.m_radius + sphereB.m_radius) - distanceToCenters;
	Vector3 bPositionCorrection = directionAtoB * (overlapDistance * .5f);
	sphereB += bPositionCorrection;
	sphereA -= bPositionCorrection;

	float speedOfAToB = DotProduct(velocityA, directionAtoB);
	float speedOfBFromA = DotProduct(velocityB, directionAtoB);
	if (speedOfAToB > speedOfBFromA)
	{
		Vector3 parallelVelocityA = directionAtoB * speedOfAToB;
		Vector3 perpindicularVelocityA = velocityA - parallelVelocityA;
		Vector3 parallelVelocityB = directionAtoB * speedOfBFromA;
		Vector3 perpindicularVelocityB = velocityB - parallelVelocityB;

		velocityA = perpindicularVelocityA + ((elasticity * parallelVelocityB));
		velocityB = perpindicularVelocityB + ((elasticity * parallelVelocityA));
	}
}

