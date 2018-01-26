#pragma once
#include "Engine/Math/Vector3.hpp"

class Sphere3D
{
public:
	Vector3 m_center;
	float m_radius;
	static const Sphere3D UNIT_CIRCLE;

	~Sphere3D();
	Sphere3D();
	Sphere3D(const Sphere3D& copy);
	explicit Sphere3D(float initialX, float initialY, float initialZ, float initialRadius);
	explicit Sphere3D(const Vector3& initialCenter, float initialRadius);
	void StretchToIncludePoint(const Vector3& point);
	void AddPadding(float paddingRadius);
	void Translate(const Vector3& translation);
	bool IsPointInside(const Vector3& point) const;
	friend void BounceBothSpheres3D(Sphere3D& sphereA, Sphere3D& sphereB, Vector3& velocityA, Vector3& velocityB, float elasticity);
	friend void BounceFirstSphereOffTheSecond(Sphere3D& movingSphere, Sphere3D& fixedSphere, Vector3& velocityOfMovingSphere, float elasticity);
	friend const Sphere3D Interpolate(const Sphere3D& start, const Sphere3D& end, float fractionToEnd);

	const Sphere3D operator + (const Vector3& translation) const;
	const Sphere3D operator - (const Vector3& antiTranslation) const;
	void operator += (const Vector3& translation);
	void operator -= (const Vector3& antiTranslation);
	bool operator == (const Sphere3D& sphereToEqual);
};
void BounceFirstSphereOffTheSecond(Sphere3D& movingSphere, Sphere3D& fixedSphere, Vector3& velocityOfMovingSphere, float elasticity);
void BounceBothSpheres3D(Sphere3D& sphereA, Sphere3D& sphereB, Vector3& velocityA, Vector3& velocityB, float elasticity);
const Sphere3D Interpolate(const Sphere3D& start, const Sphere3D& end, float fractionToEnd);