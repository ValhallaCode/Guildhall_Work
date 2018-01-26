#include "Engine/Math/IntVector3.hpp"
#include <math.h>

IntVector3::IntVector3()
	: x(0)
	, y(0)
	, z(0)
{
}

IntVector3::IntVector3(int initialX, int initialY, int initialZ)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
{
}

void IntVector3::GetXYZ(int& out_x, int& out_y, int& out_z) const
{
	out_x = x;
	out_y = y;
	out_z = z;
}

const int* IntVector3::GetAsIntArray() const
{
	return &x;
}

int* IntVector3::GetAsIntArray()
{
	return &x;
}

int IntVector3::CalcLength() const
{
	return (int)sqrt((x * x) + (y * y) + (z * z));
}

int IntVector3::CalcLengthSquared() const
{
	return (int)((x * x) + (y * y) + (z * z));
} 

void IntVector3::SetXYZ(int newX, int newY, int newZ)
{
	x = newX;
	y = newY;
	z = newZ;
}

int IntVector3::Normalize()
{
	int length = (int)sqrt((x*x) + (y*y) + (z*z));
	if (length > 0.f)
	{
		int invLength = (1 / length);
		x *= invLength;
		y *= invLength;
		z *= invLength;
	}
	return length;
}

void IntVector3::SetLength(int newLength)
{
	Normalize();
	x *= newLength;
	y *= newLength;
	z *= newLength;
}

void IntVector3::ScaleUniform(int scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
}

void IntVector3::ScaleNonUniform(const IntVector3& perAxisScaleFactors)
{
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
	z *= perAxisScaleFactors.z;
}

void IntVector3::InverseScaleNonUniform(const IntVector3& perAxisDivisors)
{
	x /= perAxisDivisors.x;
	y /= perAxisDivisors.y;
	z /= perAxisDivisors.z;
}

const IntVector3 IntVector3::operator+(int valueToAdd) const
{
	IntVector3 result(0, 0, 0);
	result.x = x + valueToAdd;
	result.y = y + valueToAdd;
	result.z = z + valueToAdd;
	return result;
}

void IntVector3::operator=(const IntVector3& assignedFrom)
{
	x = assignedFrom.x;
	y = assignedFrom.y;
	z = assignedFrom.z;
}

const IntVector3 IntVector3::operator/(const IntVector3& perAxisInverseScaleFactors) const
{
	IntVector3 result(0, 0, 0);
	result.x = this->x / perAxisInverseScaleFactors.x;
	result.y = this->y / perAxisInverseScaleFactors.y;
	result.z = this->y / perAxisInverseScaleFactors.z;
	return result;
}

void IntVector3::operator-=(const IntVector3& vectorToSubtract)
{
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
	z -= vectorToSubtract.z;
}

void IntVector3::operator+=(const IntVector3& vectorToAdd)
{
	x += vectorToAdd.x;
	y += vectorToAdd.y;
	z += vectorToAdd.z;
}

void IntVector3::operator*=(const IntVector3& perAxisScaleFactors)
{
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
	z *= perAxisScaleFactors.z;
}

void IntVector3::operator*=(int scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
}

const IntVector3 IntVector3::operator/(int inverseScale) const
{
	IntVector3 currentVector;
	currentVector.x = x / inverseScale;
	currentVector.y = y / inverseScale;
	currentVector.z = z / inverseScale;
	return currentVector;
}

const IntVector3 IntVector3::operator*(const IntVector3& perAxisScaleFactors) const
{
	IntVector3 currentVector;
	currentVector.x = this->x * perAxisScaleFactors.x;
	currentVector.y = this->y * perAxisScaleFactors.y;
	currentVector.z = this->z * perAxisScaleFactors.z;
	return currentVector;
}

const IntVector3 IntVector3::operator*(int scale) const
{
	IntVector3 currentVector;
	currentVector.x = this->x * scale;
	currentVector.y = this->y * scale;
	currentVector.z = this->z * scale;
	return currentVector;
}

const IntVector3 IntVector3::operator-(const IntVector3& vectorToSubtract) const
{
	IntVector3 currentVector;
	currentVector.x = this->x - vectorToSubtract.x;
	currentVector.y = this->y - vectorToSubtract.y;
	currentVector.z = this->z - vectorToSubtract.z;
	return currentVector;
}

const IntVector3 IntVector3::operator+(const IntVector3& vectorToAdd) const
{
	IntVector3 currentVector;
	currentVector.x = this->x + vectorToAdd.x;
	currentVector.y = this->y + vectorToAdd.y;
	currentVector.z = this->z + vectorToAdd.z;
	return currentVector;
}

bool IntVector3::operator==(const IntVector3& vectorToEqual) const
{
	return (x == vectorToEqual.x && y == vectorToEqual.y && z == vectorToEqual.z);
}

bool IntVector3::operator!=(const IntVector3 & vectorToNotEqual) const
{
	return (x != vectorToNotEqual.x || y != vectorToNotEqual.y || z != vectorToNotEqual.z);
}

int CalcDistance(const IntVector3& positionA, const IntVector3& positionB)
{
	int differenceX = positionA.x - positionB.x;
	int differenceY = positionA.y - positionB.y;
	int differenceZ = positionA.z - positionB.z;
	return (int)sqrt((differenceX * differenceX) + (differenceY * differenceY) + (differenceZ * differenceZ));
}

int CalcDistanceSquared(const IntVector3& posA, const IntVector3& posB)
{
	int differenceX = posA.x - posB.x;
	int differenceY = posA.y - posB.y;
	int differenceZ = posA.z - posB.z;
	return (differenceX * differenceX) + (differenceY * differenceY) + (differenceZ * differenceZ);
}

const IntVector3 operator * (int scale, const IntVector3& vectorToScale)
{
	IntVector3 currentVector;
	currentVector.x = vectorToScale.x * scale;
	currentVector.y = vectorToScale.y * scale;
	currentVector.z = vectorToScale.z * scale;
	return currentVector;
}

int DotProduct(const IntVector3& a, const IntVector3& b)
{
	return ((a.x*b.x) + (a.y*b.y) + (a.z*b.z));
}