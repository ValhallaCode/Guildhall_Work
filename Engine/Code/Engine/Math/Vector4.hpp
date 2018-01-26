#pragma once

class Vector4
{
	friend float CalcDistance(const Vector4& positionA, const Vector4& positionB);
	friend float CalcDistanceSquared(const Vector4& posA, const Vector4& posB);
	friend const Vector4 operator * (float scale, const Vector4& vectorToScale);
	friend float DotProduct(const Vector4& a, const Vector4& b);
	friend const Vector4 Interpolate(const Vector4& start, const Vector4& end, float fractionToEnd);

public:
	Vector4();
	Vector4(float initialX, float initialY, float initialZ, float initialW);

	void GetXYZW(float& out_x, float& out_y, float& out_z, float& out_w) const;
	const float* GetAsFloatArray() const;
	float* GetAsFloatArray();
	float CalcLength() const;
	float SumOfAllParts() const;
	float CalcLengthSquared() const;
	void SetXYZW(float newX, float newY, float newZ, float newW);
	float Normalize3D();
	float Normalize4D();
	void SetLength(float newLength);
	void ScaleUniform(float scale);
	void ScaleNonUniform(const Vector4& perAxisScaleFactors);
	void InverseScaleNonUniform(const Vector4& perAxisDivisors);
	bool operator == (const Vector4& vectorToEqual) const;
	bool operator != (const Vector4& vectorToNotEqual) const;
	const Vector4 operator + (const Vector4& vectorToAdd) const;
	const Vector4 operator - (const Vector4& vectorToSubtract) const;
	const Vector4 operator * (float scale) const;
	const Vector4 operator + (float valueToAdd) const;
	const Vector4 operator * (const Vector4& perAxisScaleFactors) const;
	const Vector4 operator / (const Vector4& perAxisInverseScaleFactors) const;
	const Vector4 operator / (float inverseScale) const;
	void operator *= (float scale);
	void operator *= (const Vector4& perAxisScaleFactors);
	void operator += (const Vector4& vectorToAdd);
	void operator -= (const Vector4& vectorToSubtract);
	void operator = (const Vector4& assignedFrom);

public:
	float x;
	float y;
	float z;
	float w;
};

const Vector4 Interpolate(const Vector4& start, const Vector4& end, float fractionToEnd);