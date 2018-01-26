#pragma once


typedef unsigned int uint;

class UintVector3
{
public:
	UintVector3();
	UintVector3(uint initialX, uint initialY, uint initialZ);

	void GetXYZ(uint& out_x, uint& out_y, uint& out_z) const;
	const uint* GetAsUintArray() const;
	uint* GetAsUintArray();
	void SetXYZ(uint newX, uint newY, uint newZ);
	bool operator == (const UintVector3& vectorToEqual) const;
	bool operator != (const UintVector3& vectorToNotEqual) const;
	const UintVector3 operator + (const UintVector3& vectorToAdd) const;
	const UintVector3 operator - (const UintVector3& vectorToSubtract) const;
	const UintVector3 operator * (uint scale) const;
	const UintVector3 operator + (uint valueToAdd) const;
	const UintVector3 operator * (const UintVector3& perAxisScaleFactors) const;
	const UintVector3 operator / (const UintVector3& perAxisInverseScaleFactors) const;
	const UintVector3 operator / (uint inverseScale) const;
	void operator *= (uint scale);
	void operator *= (const UintVector3& perAxisScaleFactors);
	void operator += (const UintVector3& vectorToAdd);
	void operator -= (const UintVector3& vectorToSubtract);
	void operator = (const UintVector3& assignedFrom);

public:
	uint x;
	uint y;
	uint z;
};