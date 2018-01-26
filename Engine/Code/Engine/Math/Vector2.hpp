#pragma once
// Code help from Squirrel Eiserloh
class Vector2
{
	friend float CalcDistance(const Vector2& positionA, const Vector2& positionB);
	friend float CalcDistanceSquared(const Vector2& posA, const Vector2& posB);
	friend const Vector2 operator * (float scale, const Vector2& vectorToScale);
	friend float DotProduct(const Vector2& a, const Vector2& b);
	friend const Vector2 Interpolate(const Vector2& start, const Vector2& end, float fractionToEnd);

public:
	Vector2();
	Vector2(float initialX, float initialY);

	void GetXY(float& out_x, float& out_y) const;
	const float* GetAsFloatArray() const;
	float* GetAsFloatArray();
	float CalcLength() const;
	float CalcLengthSquared() const;
	float CalcHeadingDegrees() const;
	float CalcHeadingRadians() const;
	void SetXY(float newX, float newY);
	void Rotate90Degrees();
	void RotateNegative90Degrees();
	void RotateDegrees(float degrees);
	void RotateRadians(float radians);
	float Normalize();
	void SetLength(float newLength);
	void ScaleUniform(float scale);
	void ScaleNonUniform(const Vector2& perAxisScaleFactors);
	void InverseScaleNonUniform(const Vector2& perAxisDivisors);
	void SetUnitLengthAndYawDegrees(float yawDegrees);
	void SetUnitLengthAndYawRadians(float yawRadians);
	void SetLengthAndYawDegrees(float newLength, float yawDegrees);
	void SetLengthAndYawRadians(float newLength, float yawRadians);
	bool operator == (const Vector2& vectorToEqual) const;
	bool operator != (const Vector2& vectorToNotEqual) const;
	const Vector2 operator + (const Vector2& vectorToAdd) const;
	const Vector2 operator - (const Vector2& vectorToSubtract) const;
	const Vector2 operator * (float scale) const;
	const Vector2 operator + (float valueToAdd) const;
	const Vector2 operator * (const Vector2& perAxisScaleFactors) const;
	const Vector2 operator / (const Vector2& perAxisInverseScaleFactors) const;
	const Vector2 operator / (float inverseScale) const;
	void operator *= (float scale);
	void operator *= (const Vector2& perAxisScaleFactors);
	void operator += (const Vector2& vectorToAdd);
	void operator -= (const Vector2& vectorToSubtract);
	void operator = (const Vector2& assignedFrom);


public:
	float x;
	float y;
	
};

const Vector2 Interpolate(const Vector2& start, const Vector2& end, float fractionToEnd);

