#include "Engine/Math/UintVector4.hpp"

UintVector4::UintVector4()
	:x(0)
	, y(0)
	, z(0)
	, w(0)
{

}

UintVector4::UintVector4(uint initialX, uint initialY, uint initialZ, uint initialW)
	:x(initialX)
	, y(initialY)
	, z(initialZ)
	, w(initialW)
{
	
}

void UintVector4::GetXYZW(uint& out_x, uint& out_y, uint& out_z, uint& out_w) const
{
	out_x = x;
	out_y = y;
	out_z = z;
	out_w = w;
}

const uint* UintVector4::GetAsUintArray() const
{
	return &x;
}

uint* UintVector4::GetAsUintArray()
{
	return &x;
}

void UintVector4::SetXYZW(uint newX, uint newY, uint newZ, uint newW)
{
	x = newX;
	y = newY;
	z = newZ;
	w = newW;
}

void UintVector4::operator=(const UintVector4& assignedFrom)
{
	x = assignedFrom.x;
	y = assignedFrom.y;
	z = assignedFrom.z;
	w = assignedFrom.w;
}

void UintVector4::operator-=(const UintVector4& vectorToSubtract)
{
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
	z -= vectorToSubtract.z;
	w -= vectorToSubtract.w;
}

void UintVector4::operator+=(const UintVector4& vectorToAdd)
{
	x += vectorToAdd.x;
	y += vectorToAdd.y;
	z += vectorToAdd.z;
	w += vectorToAdd.w;
}

void UintVector4::operator*=(const UintVector4& perAxisScaleFactors)
{
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
	z *= perAxisScaleFactors.z;
	w *= perAxisScaleFactors.w;
}

void UintVector4::operator*=(uint scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
	w *= scale;
}

const UintVector4 UintVector4::operator/(uint inverseScale) const
{
	UintVector4 currentVector;
	currentVector.x = x / inverseScale;
	currentVector.y = y / inverseScale;
	currentVector.z = z / inverseScale;
	currentVector.w = w / inverseScale;
	return currentVector;
}

const UintVector4 UintVector4::operator/(const UintVector4& perAxisInverseScaleFactors) const
{
	UintVector4 result;
	result.x = this->x / perAxisInverseScaleFactors.x;
	result.y = this->y / perAxisInverseScaleFactors.y;
	result.z = this->z / perAxisInverseScaleFactors.z;
	result.w = this->w / perAxisInverseScaleFactors.w;
	return result;
}

const UintVector4 UintVector4::operator*(const UintVector4& perAxisScaleFactors) const
{
	UintVector4 currentVector;
	currentVector.x = this->x * perAxisScaleFactors.x;
	currentVector.y = this->y * perAxisScaleFactors.y;
	currentVector.z = this->z * perAxisScaleFactors.z;
	currentVector.w = this->w * perAxisScaleFactors.w;
	return currentVector;
}

const UintVector4 UintVector4::operator+(uint valueToAdd) const
{
	UintVector4 result;
	result.x = this->x + valueToAdd;
	result.y = this->y + valueToAdd;
	result.z = this->z + valueToAdd;
	result.w = this->w + valueToAdd;
	return result;
}

const UintVector4 UintVector4::operator*(uint scale) const
{
	UintVector4 result;
	result.x = this->x * scale;
	result.y = this->y * scale;
	result.z = this->z * scale;
	result.w = this->w * scale;
	return result;
}

const UintVector4 UintVector4::operator-(const UintVector4& vectorToSubtract) const
{
	UintVector4 result;
	result.x = this->x - vectorToSubtract.x;
	result.y = this->y - vectorToSubtract.y;
	result.z = this->z - vectorToSubtract.z;
	result.w = this->w - vectorToSubtract.w;
	return result;
}

const UintVector4 UintVector4::operator+(const UintVector4& vectorToAdd) const
{
	UintVector4 result;
	result.x = this->x + vectorToAdd.x;
	result.y = this->y + vectorToAdd.y;
	result.z = this->z + vectorToAdd.z;
	result.w = this->w + vectorToAdd.w;
	return result;
}

bool UintVector4::operator!=(const UintVector4& vectorToNotEqual) const
{
	return (x != vectorToNotEqual.x || y != vectorToNotEqual.y || z != vectorToNotEqual.z || w != vectorToNotEqual.w);
}

bool UintVector4::operator==(const UintVector4& vectorToEqual) const
{
	return (x == vectorToEqual.x && y == vectorToEqual.y && z == vectorToEqual.z && w == vectorToEqual.w);
}
