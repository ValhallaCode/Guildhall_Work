#pragma once
#include "Engine/Math/Vector4.hpp"

class Vector3
{
	friend float CalcDistance(const Vector3& positionA, const Vector3& positionB);
	friend float CalcDistanceSquared(const Vector3& posA, const Vector3& posB);
	friend const Vector3 operator * (float scale, const Vector3& vectorToScale);
	friend float DotProduct(const Vector3& a, const Vector3& b);
	friend const Vector3 Interpolate(const Vector3& start, const Vector3& end, float fractionToEnd);

public:
	Vector3();
	Vector3(float initialX, float initialY, float initialZ);
	Vector3(const Vector4& copy);
	void GetXYZ(float& out_x, float& out_y, float& out_z) const;
	const float* GetAsFloatArray() const;
	float* GetAsFloatArray();
	friend float GetNormalize(Vector3& vectorToNormalize);
	float CalcLength() const;
	float CalcLengthSquared() const;
	void SetXYZ(float newX, float newY, float newZ);
	float Normalize();
	void SetLength(float newLength);
	void ScaleUniform(float scale);
	void ScaleNonUniform(const Vector3& perAxisScaleFactors);
	void InverseScaleNonUniform(const Vector3& perAxisDivisors);
	const Vector3 Vector3::operator-() const;
	bool operator == (const Vector3& vectorToEqual) const;
	bool operator != (const Vector3& vectorToNotEqual) const;
	const Vector3 operator + (const Vector3& vectorToAdd) const;
	const Vector3 operator - (const Vector3& vectorToSubtract) const;
	const Vector3 operator * (float scale) const;
	const Vector3 operator + (float valueToAdd) const;
	const Vector3 operator * (const Vector3& perAxisScaleFactors) const;
	const Vector3 operator / (const Vector3& perAxisInverseScaleFactors) const;
	const Vector3 operator / (float inverseScale) const;
	void operator *= (float scale);
	void operator *= (const Vector3& perAxisScaleFactors);
	void operator += (const Vector3& vectorToAdd);
	void operator -= (const Vector3& vectorToSubtract);
	void operator = (const Vector3& assignedFrom);
	Vector3 Vector3::operator=(const Vector4& assignedFrom);
public:
	float x;
	float y;
	float z;
};

const Vector3 Interpolate(const Vector3& start, const Vector3& end, float fractionToEnd);
float GetNormalize(Vector3& vectorToNormalize);