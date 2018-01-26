#pragma once


class IntVector3
{
	friend int CalcDistance(const IntVector3& positionA, const IntVector3& positionB);
	friend int CalcDistanceSquared(const IntVector3& posA, const IntVector3& posB);
	friend const IntVector3 operator * (int scale, const IntVector3& vectorToScale);
	friend int DotProduct(const IntVector3& a, const IntVector3& b);

public:
	int x;
	int y;
	int z;

	IntVector3();
	IntVector3(int initialX, int initialY, int initialZ);

	void GetXYZ(int& out_x, int& out_y, int& out_z) const;
	const int* GetAsIntArray() const;
	int* GetAsIntArray();
	int CalcLength() const;
	int CalcLengthSquared() const;
	void SetXYZ(int newX, int newY, int newZ);
	int Normalize();
	void SetLength(int newLength);
	void ScaleUniform(int scale);
	void ScaleNonUniform(const IntVector3& perAxisScaleFactors);
	void InverseScaleNonUniform(const IntVector3& perAxisDivisors);
	bool operator == (const IntVector3& vectorToEqual) const;
	bool operator != (const IntVector3& vectorToNotEqual) const;
	const IntVector3 operator + (const IntVector3& vectorToAdd) const;
	const IntVector3 operator - (const IntVector3& vectorToSubtract) const;
	const IntVector3 operator * (int scale) const;
	const IntVector3 operator + (int valueToAdd) const;
	const IntVector3 operator * (const IntVector3& perAxisScaleFactors) const;
	const IntVector3 operator / (const IntVector3& perAxisInverseScaleFactors) const;
	const IntVector3 operator / (int inverseScale) const;
	void operator *= (int scale);
	void operator *= (const IntVector3& perAxisScaleFactors);
	void operator += (const IntVector3& vectorToAdd);
	void operator -= (const IntVector3& vectorToSubtract);
	void operator = (const IntVector3& assignedFrom);
};