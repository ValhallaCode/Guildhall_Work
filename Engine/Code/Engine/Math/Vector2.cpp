#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <math.h>
// Code help from Squirrel Eiserloh

Vector2::Vector2()
	: x(0.f)
	, y(0.f)
{
}

Vector2::Vector2(float initialX, float initialY)
	: x(initialX)
	, y(initialY)
{
}

void Vector2::GetXY(float& out_x, float& out_y) const
{
	out_x = x;
	out_y = y;
}

const float* Vector2::GetAsFloatArray() const
{
	return &x;
}

float* Vector2::GetAsFloatArray()
{
	return &x;
}

float Vector2::CalcLength() const
{
	return (float) sqrt((x * x) + (y *y));
}

float Vector2::CalcLengthSquared() const
{
	return ((x * x) + (y * y));
}

float Vector2::CalcHeadingDegrees() const
{
	return (float) atan2(y, x) * (180.f / PI);
}

float Vector2::CalcHeadingRadians() const
{
	return (float) atan2(y, x);
}

void Vector2::SetXY(float newX, float newY)
{
	x = newX;
	y = newY;
}

void Vector2::Rotate90Degrees()
{
	float negativeY = -y;
	y = x;
	x = negativeY;
}

void Vector2::RotateNegative90Degrees()
{
	float negativeX = -x;
	x = y;
	y = negativeX;
}

void Vector2::RotateDegrees(float degrees)
{
	float radius = (float) sqrt((x*x) + (y*y));
	float myDegrees = atan2InDegrees(y, x);
	myDegrees += degrees;
	x = radius * CosInDegrees(myDegrees);
	y = radius * SinInDegrees(myDegrees);
}

void Vector2::RotateRadians(float radians)
{
	float radius = (float) sqrt((x*x) + (y*y));
	float myRadians = (float) atan2(y, x);
	myRadians += radians;
	x = radius * CosInDegrees(myRadians);
	y = radius * SinInDegrees(myRadians);
}

float Vector2::Normalize()
{
	float length = (float) sqrt((x*x) + (y*y));
	if (length > 0.f)
	{
		float invLength = (1.0f / length);
		x *= invLength;
		y *= invLength;
	}
	return length;
}

void Vector2::SetLength(float newLength)
{
	Normalize();
	x *= newLength;
	y *= newLength;
}

void Vector2::ScaleUniform(float scale)
{
	x *= scale;
	y *= scale;
}

void Vector2::ScaleNonUniform(const Vector2& perAxisScaleFactors)
{
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
}

void Vector2::InverseScaleNonUniform(const Vector2& perAxisDivisors)
{
	x /= perAxisDivisors.x;
	y /= perAxisDivisors.y;
}

void Vector2::SetUnitLengthAndYawDegrees(float yawDegrees)
{
	Normalize();
	x = CosInDegrees(yawDegrees);
	y = SinInDegrees(yawDegrees);
}

void Vector2::SetUnitLengthAndYawRadians(float yawRadians)
{
	Normalize();
	x = (float) cos(yawRadians);
	y = (float) sin(yawRadians);
}

void Vector2::SetLengthAndYawDegrees(float newLength, float yawDegrees)
{
	x = newLength * CosInDegrees(yawDegrees);
	y = newLength * SinInDegrees(yawDegrees);
}

void Vector2::SetLengthAndYawRadians(float newLength, float yawRadians)
{
	x = newLength * (float) cos(yawRadians);
	y = newLength * (float) sin(yawRadians);
}

const Vector2 Vector2::operator+(float valueToAdd) const
{
	Vector2 result(0.f, 0.f);
	result.x = x + valueToAdd;
	result.y = y + valueToAdd;
	return result;
}

void Vector2::operator=(const Vector2& assignedFrom)
{
	x = assignedFrom.x;
	y = assignedFrom.y;
}

const Vector2 Vector2::operator/(const Vector2& perAxisInverseScaleFactors) const
{
	Vector2 result(0.f, 0.f);
	result.x = this->x / perAxisInverseScaleFactors.x;
	result.y = this->y / perAxisInverseScaleFactors.y;
	return result;
}

void Vector2::operator-=(const Vector2& vectorToSubtract)
{
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
}

void Vector2::operator+=(const Vector2& vectorToAdd)
{
	x += vectorToAdd.x;
	y += vectorToAdd.y;
}

void Vector2::operator*=(const Vector2& perAxisScaleFactors)
{
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
}

void Vector2::operator*=(float scale)
{
	x *= scale;
	y *= scale;
}

const Vector2 Vector2::operator/(float inverseScale) const
{
	Vector2 currentVector;
	currentVector.x = x / inverseScale;
	currentVector.y = y / inverseScale;
	return currentVector;
}

const Vector2 Vector2::operator*(const Vector2& perAxisScaleFactors) const
{
	Vector2 currentVector;
	currentVector.x = this->x * perAxisScaleFactors.x;
	currentVector.y = this->y * perAxisScaleFactors.y;
	return currentVector;
}

const Vector2 Vector2::operator*(float scale) const
{
	Vector2 currentVector;
	currentVector.x = this->x * scale;
	currentVector.y = this->y * scale;
	return currentVector;
}

const Vector2 Vector2::operator-(const Vector2& vectorToSubtract) const
{
	Vector2 currentVector;
	currentVector.x = this->x - vectorToSubtract.x;
	currentVector.y = this->y - vectorToSubtract.y;
	return currentVector;
}

const Vector2 Vector2::operator+(const Vector2& vectorToAdd) const
{
	Vector2 currentVector;
	currentVector.x = this->x + vectorToAdd.x;
	currentVector.y = this->y + vectorToAdd.y;
	return currentVector;
}

bool Vector2::operator==(const Vector2& vectorToEqual) const
{
	return (x == vectorToEqual.x && y == vectorToEqual.y);
}

bool Vector2::operator!=(const Vector2 & vectorToNotEqual) const
{
	return (x != vectorToNotEqual.x || y != vectorToNotEqual.y);
}

float CalcDistance(const Vector2& positionA, const Vector2& positionB)
{
	float differenceX = positionA.x - positionB.x;
	float differenceY = positionA.y - positionB.y;
	return (float)sqrt((differenceX * differenceX) + (differenceY * differenceY));
}

float CalcDistanceSquared(const Vector2& posA, const Vector2& posB)
{
	float differenceX = posA.x - posB.x;
	float differenceY = posA.y - posB.y;
	return (differenceX * differenceX) + (differenceY * differenceY);
}

const Vector2 operator * (float scale, const Vector2& vectorToScale)
{
	Vector2 currentVector;
	currentVector.x = vectorToScale.x * scale;
	currentVector.y = vectorToScale.y * scale;
	return currentVector;
}

float DotProduct(const Vector2& a, const Vector2& b)
{
	return ((a.x*b.x) + (a.y*b.y));
}

const Vector2 Interpolate(const Vector2& start, const Vector2& end, float fractionToEnd)
{
	float fractionOfStart = 1.f - fractionToEnd;
	Vector2 blended;
	blended.x = (fractionOfStart * start.x) + (fractionToEnd * end.x);
	blended.y = (fractionOfStart * start.y) + (fractionToEnd * end.y);
	return blended;
}
