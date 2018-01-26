#pragma once


typedef unsigned int uint;

class UintVector4
{
public:
	UintVector4();
	UintVector4(uint initialX, uint initialY, uint initialZ, uint initialW);

	void GetXYZW(uint& out_x, uint& out_y, uint& out_z, uint& out_w) const;
	const uint* GetAsUintArray() const;
	uint* GetAsUintArray();
	void SetXYZW(uint newX, uint newY, uint newZ, uint newW);
	bool operator == (const UintVector4& vectorToEqual) const;
	bool operator != (const UintVector4& vectorToNotEqual) const;
	const UintVector4 operator + (const UintVector4& vectorToAdd) const;
	const UintVector4 operator - (const UintVector4& vectorToSubtract) const;
	const UintVector4 operator * (uint scale) const;
	const UintVector4 operator + (uint valueToAdd) const;
	const UintVector4 operator * (const UintVector4& perAxisScaleFactors) const;
	const UintVector4 operator / (const UintVector4& perAxisInverseScaleFactors) const;
	const UintVector4 operator / (uint inverseScale) const;
	void operator *= (uint scale);
	void operator *= (const UintVector4& perAxisScaleFactors);
	void operator += (const UintVector4& vectorToAdd);
	void operator -= (const UintVector4& vectorToSubtract);
	void operator = (const UintVector4& assignedFrom);

public:
	uint x;
	uint y;
	uint z;
	uint w;
};