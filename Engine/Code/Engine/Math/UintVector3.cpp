#include "Engine/Math/UintVector3.hpp"

UintVector3::UintVector3()
	:x(0)
	, y(0)
	, z(0)
{

}

UintVector3::UintVector3(uint initialX, uint initialY, uint initialZ)
	:x(initialX)
	, y(initialY)
	, z(initialZ)
{

}

void UintVector3::GetXYZ(uint& out_x, uint& out_y, uint& out_z) const
{
	out_x = x;
	out_y = y;
	out_z = z;
}

const uint* UintVector3::GetAsUintArray() const
{
	return &x;
}

uint* UintVector3::GetAsUintArray()
{
	return &x;
}

void UintVector3::SetXYZ(uint newX, uint newY, uint newZ)
{
	x = newX;
	y = newY;
	z = newZ;
}

void UintVector3::operator=(const UintVector3& assignedFrom)
{
	x = assignedFrom.x;
	y = assignedFrom.y;
	z = assignedFrom.z;
}

void UintVector3::operator-=(const UintVector3& vectorToSubtract)
{
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
	z -= vectorToSubtract.z;
}

void UintVector3::operator+=(const UintVector3& vectorToAdd)
{
	x += vectorToAdd.x;
	y += vectorToAdd.y;
	z += vectorToAdd.z;
}

void UintVector3::operator*=(const UintVector3& perAxisScaleFactors)
{
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
	z *= perAxisScaleFactors.z;
}

void UintVector3::operator*=(uint scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
}

const UintVector3 UintVector3::operator/(uint inverseScale) const
{
	UintVector3 currentVector;
	currentVector.x = x / inverseScale;
	currentVector.y = y / inverseScale;
	currentVector.z = z / inverseScale;
	return currentVector;
}

const UintVector3 UintVector3::operator/(const UintVector3& perAxisInverseScaleFactors) const
{
	UintVector3 result;
	result.x = this->x / perAxisInverseScaleFactors.x;
	result.y = this->y / perAxisInverseScaleFactors.y;
	result.z = this->z / perAxisInverseScaleFactors.z;
	return result;
}

const UintVector3 UintVector3::operator*(const UintVector3& perAxisScaleFactors) const
{
	UintVector3 currentVector;
	currentVector.x = this->x * perAxisScaleFactors.x;
	currentVector.y = this->y * perAxisScaleFactors.y;
	currentVector.z = this->z * perAxisScaleFactors.z;
	return currentVector;
}

const UintVector3 UintVector3::operator+(uint valueToAdd) const
{
	UintVector3 result;
	result.x = this->x + valueToAdd;
	result.y = this->y + valueToAdd;
	result.z = this->z + valueToAdd;
	return result;
}

const UintVector3 UintVector3::operator*(uint scale) const
{
	UintVector3 result;
	result.x = this->x * scale;
	result.y = this->y * scale;
	result.z = this->z * scale;
	return result;
}

const UintVector3 UintVector3::operator-(const UintVector3& vectorToSubtract) const
{
	UintVector3 result;
	result.x = this->x - vectorToSubtract.x;
	result.y = this->y - vectorToSubtract.y;
	result.z = this->z - vectorToSubtract.z;
	return result;
}

const UintVector3 UintVector3::operator+(const UintVector3& vectorToAdd) const
{
	UintVector3 result;
	result.x = this->x + vectorToAdd.x;
	result.y = this->y + vectorToAdd.y;
	result.z = this->z + vectorToAdd.z;
	return result;
}

bool UintVector3::operator!=(const UintVector3& vectorToNotEqual) const
{
	return (x != vectorToNotEqual.x || y != vectorToNotEqual.y || z != vectorToNotEqual.z);
}

bool UintVector3::operator==(const UintVector3& vectorToEqual) const
{
	return (x == vectorToEqual.x && y == vectorToEqual.y && z == vectorToEqual.z);
}
