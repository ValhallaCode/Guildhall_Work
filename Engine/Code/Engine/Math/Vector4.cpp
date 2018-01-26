#include "Engine/Math/Vector4.hpp"
#include <math.h>

float CalcDistance(const Vector4& positionA, const Vector4& positionB)
{
	float differenceX = positionA.x - positionB.x;
	float differenceY = positionA.y - positionB.y;
	float differenceZ = positionA.z - positionB.z;
	float differenceW = positionA.w - positionB.w;
	return (float)sqrt((differenceX * differenceX) + (differenceY * differenceY) + (differenceZ * differenceZ) + (differenceW * differenceW));
}

float CalcDistanceSquared(const Vector4& posA, const Vector4& posB)
{
	float differenceX = posA.x - posB.x;
	float differenceY = posA.y - posB.y;
	float differenceZ = posA.z - posB.z;
	float differenceW = posA.w - posB.w;
	return (differenceX * differenceX) + (differenceY * differenceY) + (differenceZ * differenceZ) + (differenceW * differenceW);
}

float DotProduct(const Vector4& a, const Vector4& b)
{
	return ((a.x*b.x) + (a.y*b.y) + (a.z*b.z) + (a.w*b.w));
}

const Vector4 operator * (float scale, const Vector4& vectorToScale)
{
	Vector4 currentVector;
	currentVector.x = vectorToScale.x * scale;
	currentVector.y = vectorToScale.y * scale;
	currentVector.z = vectorToScale.z * scale;
	currentVector.w = vectorToScale.w * scale;
	return currentVector;
}

Vector4::Vector4()
	:x(0.f)
	, y(0.f)
	, z(0.f)
	, w(0.f)
{
}

Vector4::Vector4(float initialX, float initialY, float initialZ, float initialW)
	:x(initialX)
	, y(initialY)
	, z(initialZ)
	, w(initialW)
{
}

void Vector4::GetXYZW(float& out_x, float& out_y, float& out_z, float& out_w) const
{
	out_x = x;
	out_y = y;
	out_z = z;
	out_w = w;
}

const float* Vector4::GetAsFloatArray() const
{
	return &x;
}

float* Vector4::GetAsFloatArray()
{
	return &x;
}

float Vector4::CalcLength() const
{
	return (float)sqrt((x * x) + (y * y) + (z * z) + (w * w));
}

float Vector4::SumOfAllParts() const
{
	return x + y + z + w;
}

float Vector4::CalcLengthSquared() const
{
	return ((x * x) + (y * y) + (z * z) + (w * w));
}

void Vector4::SetXYZW(float newX, float newY, float newZ, float newW)
{
	x = newX;
	y = newY;
	z = newZ;
	w = newW;
}

float Vector4::Normalize3D()
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

float Vector4::Normalize4D()
{
	float length = (float)sqrt((x*x) + (y*y) + (z*z) + (w*w));
	if (length > 0.f)
	{
		float invLength = (1.0f / length);
		x *= invLength;
		y *= invLength;
		z *= invLength;
		w *= invLength;
	}
	return length;
}

void Vector4::SetLength(float newLength)
{
	Normalize4D();
	x *= newLength;
	y *= newLength;
	z *= newLength;
	w *= newLength;
}

void Vector4::ScaleUniform(float scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
	w *= scale;
}

void Vector4::ScaleNonUniform(const Vector4& perAxisScaleFactors)
{
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
	z *= perAxisScaleFactors.z;
	w *= perAxisScaleFactors.w;
}

void Vector4::InverseScaleNonUniform(const Vector4& perAxisDivisors)
{
	x /= perAxisDivisors.x;
	y /= perAxisDivisors.y;
	z /= perAxisDivisors.z;
	w /= perAxisDivisors.w;
}

void Vector4::operator=(const Vector4& assignedFrom)
{
	x = assignedFrom.x;
	y = assignedFrom.y;
	z = assignedFrom.z;
	w = assignedFrom.w;
}

void Vector4::operator-=(const Vector4& vectorToSubtract)
{
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
	z -= vectorToSubtract.z;
	w -= vectorToSubtract.w;
}

void Vector4::operator+=(const Vector4& vectorToAdd)
{
	x += vectorToAdd.x;
	y += vectorToAdd.y;
	z += vectorToAdd.z;
	w += vectorToAdd.w;
}

void Vector4::operator*=(const Vector4& perAxisScaleFactors)
{
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
	z *= perAxisScaleFactors.z;
	w *= perAxisScaleFactors.w;
}

void Vector4::operator*=(float scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
	w *= scale;
}

const Vector4 Vector4::operator/(float inverseScale) const
{
	Vector4 currentVector;
	currentVector.x = x / inverseScale;
	currentVector.y = y / inverseScale;
	currentVector.z = z / inverseScale;
	currentVector.w = w / inverseScale;
	return currentVector;
}

const Vector4 Vector4::operator/(const Vector4& perAxisInverseScaleFactors) const
{
	Vector4 result(0.f, 0.f, 0.f, 0.f);
	result.x = this->x / perAxisInverseScaleFactors.x;
	result.y = this->y / perAxisInverseScaleFactors.y;
	result.z = this->z / perAxisInverseScaleFactors.z;
	result.w = this->w / perAxisInverseScaleFactors.w;
	return result;
}

const Vector4 Vector4::operator*(const Vector4& perAxisScaleFactors) const
{
	Vector4 currentVector;
	currentVector.x = this->x * perAxisScaleFactors.x;
	currentVector.y = this->y * perAxisScaleFactors.y;
	currentVector.z = this->z * perAxisScaleFactors.z;
	currentVector.w = this->w * perAxisScaleFactors.w;
	return currentVector;
}

const Vector4 Vector4::operator+(float valueToAdd) const
{
	Vector4 result(0.f, 0.f, 0.f, 0.f);
	result.x = this->x + valueToAdd;
	result.y = this->y + valueToAdd;
	result.z = this->z + valueToAdd;
	result.w = this->w + valueToAdd;
	return result;
}

const Vector4 Vector4::operator*(float scale) const
{
	Vector4 result(0.f, 0.f, 0.f, 0.f);
	result.x = this->x * scale;
	result.y = this->y * scale;
	result.z = this->z * scale;
	result.w = this->w * scale;
	return result;
}

const Vector4 Vector4::operator-(const Vector4& vectorToSubtract) const
{
	Vector4 result(0.f, 0.f, 0.f, 0.f);
	result.x = this->x - vectorToSubtract.x;
	result.y = this->y - vectorToSubtract.y;
	result.z = this->z - vectorToSubtract.z;
	result.w = this->w - vectorToSubtract.w;
	return result;
}

const Vector4 Vector4::operator+(const Vector4& vectorToAdd) const
{
	Vector4 result(0.f, 0.f, 0.f, 0.f);
	result.x = this->x + vectorToAdd.x;
	result.y = this->y + vectorToAdd.y;
	result.z = this->z + vectorToAdd.z;
	result.w = this->w + vectorToAdd.w;
	return result;
}

bool Vector4::operator!=(const Vector4& vectorToNotEqual) const
{
	return (x != vectorToNotEqual.x || y != vectorToNotEqual.y || z != vectorToNotEqual.z || w != vectorToNotEqual.w);
}

bool Vector4::operator==(const Vector4& vectorToEqual) const
{
	return (x == vectorToEqual.x && y == vectorToEqual.y && z == vectorToEqual.z && w == vectorToEqual.w);
}

const Vector4 Interpolate(const Vector4& start, const Vector4& end, float fractionToEnd)
{
	float fractionOfStart = 1.f - fractionToEnd;
	Vector4 blended;
	blended.x = (fractionOfStart * start.x) + (fractionToEnd * end.x);
	blended.y = (fractionOfStart * start.y) + (fractionToEnd * end.y);
	blended.z = (fractionOfStart * start.z) + (fractionToEnd * end.z);
	blended.w = (fractionOfStart * start.w) + (fractionToEnd * end.w);
	return blended;
}
