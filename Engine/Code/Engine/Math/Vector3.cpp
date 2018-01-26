#include "Engine/Math/Vector3.hpp"
#include <math.h>


Vector3::Vector3()
	:x(0.f)
	,y(0.f)
	,z(0.f)
{
}

Vector3::Vector3(float initialX, float initialY, float initialZ)
	:x(initialX)
	,y(initialY)
	,z(initialZ)
{
}

Vector3::Vector3(const Vector4& copy) 
	: x(copy.x)
	, y(copy.y)
	, z(copy.z)
{ 
}

void Vector3::GetXYZ(float& out_x, float& out_y, float& out_z) const
{
	out_x = x;
	out_y = y;
	out_z = z;
}

const float* Vector3::GetAsFloatArray() const
{
	return &x;
}

float* Vector3::GetAsFloatArray()
{
	return &x;
}

float GetNormalize(Vector3& vectorToNormalize)
{
	return vectorToNormalize.Normalize();
}

float Vector3::CalcLength() const
{
	return (float)sqrt((x * x) + (y * y) + (z * z));
}

float Vector3::CalcLengthSquared() const
{
	return (x * x) + (y * y) + (z * z);
}

void Vector3::SetXYZ(float newX, float newY, float newZ)
{
	x = newX;
	y = newY;
	z = newZ;
}

float Vector3::Normalize()
{
	float length = (float)sqrt((x*x) + (y*y) + (z*z));
	if (length > 0.f)
	{
		float invLength = (1.0f / length);
		x *= invLength;
		y *= invLength;
		z *= invLength;
	}
	return length;
}

void Vector3::SetLength(float newLength)
{
	Normalize();
	x *= newLength;
	y *= newLength;
	z *= newLength;
}

void Vector3::ScaleUniform(float scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
}

void Vector3::ScaleNonUniform(const Vector3& perAxisScaleFactors)
{
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
	z *= perAxisScaleFactors.z;
}

void Vector3::InverseScaleNonUniform(const Vector3& perAxisDivisors)
{
	x /= perAxisDivisors.x;
	y /= perAxisDivisors.y;
	z /= perAxisDivisors.z;
}

void Vector3::operator=(const Vector3& assignedFrom)
{
	x = assignedFrom.x;
	y = assignedFrom.y;
	z = assignedFrom.z;
}

void Vector3::operator-=(const Vector3& vectorToSubtract)
{
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
	z -= vectorToSubtract.z;
}

void Vector3::operator+=(const Vector3& vectorToAdd)
{
	x += vectorToAdd.x;
	y += vectorToAdd.y;
	z += vectorToAdd.z;
}

void Vector3::operator*=(const Vector3& perAxisScaleFactors)
{
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
	z *= perAxisScaleFactors.z;
}

void Vector3::operator*=(float scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
}

const Vector3 Vector3::operator/(float inverseScale) const
{
	Vector3 currentVector;
	currentVector.x = x / inverseScale;
	currentVector.y = y / inverseScale;
	currentVector.z = z / inverseScale;
	return currentVector;
}

const Vector3 Vector3::operator/(const Vector3& perAxisInverseScaleFactors) const
{
	Vector3 result(0.f, 0.f, 0.f);
	result.x = this->x / perAxisInverseScaleFactors.x;
	result.y = this->y / perAxisInverseScaleFactors.y;
	result.z = this->z / perAxisInverseScaleFactors.z;
	return result;
}

const Vector3 Vector3::operator*(const Vector3& perAxisScaleFactors) const
{
	Vector3 currentVector;
	currentVector.x = this->x * perAxisScaleFactors.x;
	currentVector.y = this->y * perAxisScaleFactors.y;
	currentVector.z = this->z * perAxisScaleFactors.z;
	return currentVector;
}

const Vector3 Vector3::operator+(float valueToAdd) const
{
	Vector3 result(0.f, 0.f, 0.f);
	result.x = x + valueToAdd;
	result.y = y + valueToAdd;
	result.z = z + valueToAdd;
	return result;
}

const Vector3 Vector3::operator*(float scale) const
{
	Vector3 currentVector;
	currentVector.x = this->x * scale;
	currentVector.y = this->y * scale;
	currentVector.z = this->z * scale;
	return currentVector;
}

const Vector3 Vector3::operator-(const Vector3& vectorToSubtract) const
{
	Vector3 currentVector;
	currentVector.x = this->x - vectorToSubtract.x;
	currentVector.y = this->y - vectorToSubtract.y;
	currentVector.z = this->z - vectorToSubtract.z;
	return currentVector;
}

const Vector3 Vector3::operator-() const
{
	Vector3 currentVector;
	currentVector.x = -this->x;
	currentVector.y = -this->y;
	currentVector.z = -this->z;
	return currentVector;
}

const Vector3 Vector3::operator+(const Vector3& vectorToAdd) const
{
	Vector3 currentVector;
	currentVector.x = this->x + vectorToAdd.x;
	currentVector.y = this->y + vectorToAdd.y;
	currentVector.z = this->z + vectorToAdd.z;
	return currentVector;
}

bool Vector3::operator!=(const Vector3& vectorToNotEqual) const
{
	return (x != vectorToNotEqual.x || y != vectorToNotEqual.y || z != vectorToNotEqual.z);
}

bool Vector3::operator==(const Vector3& vectorToEqual) const
{
	return (x == vectorToEqual.x && y == vectorToEqual.y && z == vectorToEqual.z);
}

const Vector3 operator * (float scale, const Vector3& vectorToScale)
{
	Vector3 currentVector;
	currentVector.x = vectorToScale.x * scale;
	currentVector.y = vectorToScale.y * scale;
	currentVector.z = vectorToScale.z * scale;
	return currentVector;
}

float CalcDistance(const Vector3& positionA, const Vector3& positionB)
{
	float differenceX = positionA.x - positionB.x;
	float differenceY = positionA.y - positionB.y;
	float differenceZ = positionA.z - positionB.z;
	return (float)sqrt((differenceX * differenceX) + (differenceY * differenceY) + (differenceZ * differenceZ));
}

float CalcDistanceSquared(const Vector3& posA, const Vector3& posB)
{
	float differenceX = posA.x - posB.x;
	float differenceY = posA.y - posB.y;
	float differenceZ = posA.z - posB.z;
	return (differenceX * differenceX) + (differenceY * differenceY) + (differenceZ * differenceZ);
}

float DotProduct(const Vector3& a, const Vector3& b)
{
	return ((a.x*b.x) + (a.y*b.y) + (a.z*b.z));
}

const Vector3 Interpolate(const Vector3& start, const Vector3& end, float fractionToEnd)
{
	float fractionOfStart = 1.f - fractionToEnd;
	Vector3 blended;
	blended.x = (fractionOfStart * start.x) + (fractionToEnd * end.x);
	blended.y = (fractionOfStart * start.y) + (fractionToEnd * end.y);
	blended.z = (fractionOfStart * start.z) + (fractionToEnd * end.z);
	return blended;
}

Vector3 Vector3::operator=(const Vector4& assignedFrom)
{ 
	return Vector3(assignedFrom.x, assignedFrom.y, assignedFrom.z);
}
