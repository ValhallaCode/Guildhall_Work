#pragma once


class IntVector2
{
	friend int CalcDistance(const IntVector2& positionA, const IntVector2& positionB);
	friend int CalcDistanceSquared(const IntVector2& posA, const IntVector2& posB);
	friend const IntVector2 operator * (int scale, const IntVector2& vectorToScale);
	friend float DotProduct(const IntVector2& a, const IntVector2& b);

public:
	int x;
	int y;

	IntVector2();
	IntVector2(int initialX, int initialY);

	void GetXY(int& out_x, int& out_y) const;
	const int* GetAsIntArray() const;
	int* GetAsIntArray();
	int CalcLength() const;
	int CalcLengthSquared() const;
	float CalcHeadingDegrees() const;
	float CalcHeadingRadians() const;
	void SetXY(int newX, int newY);
	void Rotate90Degrees();
	void RotateNegative90Degrees();
	void RotateDegrees(int degrees);
	void RotateRadians(int radians);
	int Normalize();
	void SetLength(int newLength);
	void ScaleUniform(int scale);
	void ScaleNonUniform(const IntVector2& perAxisScaleFactors);
	void InverseScaleNonUniform(const IntVector2& perAxisDivisors);
	void SetUnitLengthAndYawDegrees(float yawDegrees);
	void SetUnitLengthAndYawRadians(float yawRadians);
	void SetLengthAndYawDegrees(int newLength, float yawDegrees);
	void SetLengthAndYawRadians(int newLength, float yawRadians);
	bool operator == (const IntVector2& vectorToEqual) const;
	bool operator != (const IntVector2& vectorToNotEqual) const;
	const IntVector2 operator + (const IntVector2& vectorToAdd) const;
	const IntVector2 operator - (const IntVector2& vectorToSubtract) const;
	const IntVector2 operator * (int scale) const;
	const IntVector2 operator + (int valueToAdd) const;
	const IntVector2 operator * (const IntVector2& perAxisScaleFactors) const;
	const IntVector2 operator / (const IntVector2& perAxisInverseScaleFactors) const;
	const IntVector2 operator / (int inverseScale) const;
	friend bool operator < (const IntVector2& lhs, const IntVector2& rhs);
	void operator *= (int scale);
	void operator *= (const IntVector2& perAxisScaleFactors);
	void operator += (const IntVector2& vectorToAdd);
	void operator -= (const IntVector2& vectorToSubtract);
	void operator = (const IntVector2& assignedFrom);
};

bool operator < (const IntVector2& lhs, const IntVector2& rhs);