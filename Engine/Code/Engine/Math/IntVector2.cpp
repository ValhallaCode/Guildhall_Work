#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <math.h>
// Code help from Squirrel Eiserloh

IntVector2::IntVector2()
	: x(0)
	, y(0)
{
}

IntVector2::IntVector2(int initialX, int initialY)
	: x(initialX)
	, y(initialY)
{
}

void IntVector2::GetXY(int& out_x, int& out_y) const
{
	out_x = x;
	out_y = y;
}

const int* IntVector2::GetAsIntArray() const
{
	return &x;
}

int* IntVector2::GetAsIntArray()
{
	return &x;
}

int IntVector2::CalcLength() const
{
	return (int)sqrt((x * x) + (y *y));
}

int IntVector2::CalcLengthSquared() const
{
	return (int)((x * x) + (y * y));
}

float IntVector2::CalcHeadingDegrees() const
{
	return (float)atan2(y, x) * (180.f / PI);
}

float IntVector2::CalcHeadingRadians() const
{
	return (float)atan2(y, x);
}

void IntVector2::SetXY(int newX, int newY)
{
	x = newX;
	y = newY;
}

void IntVector2::Rotate90Degrees()
{
	int negativeY = -y;
	y = x;
	x = negativeY;
}

void IntVector2::RotateNegative90Degrees()
{
	int negativeX = -x;
	x = y;
	y = negativeX;
}

void IntVector2::RotateDegrees(int degrees)
{
	int radius = (int)sqrt((x*x) + (y*y));
	float myDegrees = atan2InDegrees((float)y, (float)x);
	myDegrees += degrees;
	x = radius * (int)CosInDegrees(myDegrees);
	y = radius * (int)SinInDegrees(myDegrees);
}

void IntVector2::RotateRadians(int radians)
{
	int radius = (int)sqrt((x*x) + (y*y));
	float myRadians = (float)atan2(y, x);
	myRadians += radians;
	x = radius * (int)CosInDegrees(myRadians);
	y = radius * (int)SinInDegrees(myRadians);
}

int IntVector2::Normalize()
{
	int length = (int)sqrt((x*x) + (y*y));
	if (length > 0.f)
	{
		int invLength = (1 / length);
		x *= invLength;
		y *= invLength;
	}
	return length;
}

void IntVector2::SetLength(int newLength)
{
	Normalize();
	x *= newLength;
	y *= newLength;
}

void IntVector2::ScaleUniform(int scale)
{
	x *= scale;
	y *= scale;
}

void IntVector2::ScaleNonUniform(const IntVector2& perAxisScaleFactors)
{
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
}

void IntVector2::InverseScaleNonUniform(const IntVector2& perAxisDivisors)
{
	x /= perAxisDivisors.x;
	y /= perAxisDivisors.y;
}

void IntVector2::SetUnitLengthAndYawDegrees(float yawDegrees)
{
	Normalize();
	x = (int)CosInDegrees(yawDegrees);
	y = (int)SinInDegrees(yawDegrees);
}

void IntVector2::SetUnitLengthAndYawRadians(float yawRadians)
{
	Normalize();
	x = (int)cos(yawRadians);
	y = (int)sin(yawRadians);
}

void IntVector2::SetLengthAndYawDegrees(int newLength, float yawDegrees)
{
	x = newLength * (int)CosInDegrees(yawDegrees);
	y = newLength * (int)SinInDegrees(yawDegrees);
}

void IntVector2::SetLengthAndYawRadians(int newLength, float yawRadians)
{
	x = newLength * (int)cos(yawRadians);
	y = newLength * (int)sin(yawRadians);
}

const IntVector2 IntVector2::operator+(int valueToAdd) const
{
	IntVector2 result(0, 0);
	result.x = x + valueToAdd;
	result.y = y + valueToAdd;
	return result;
}

void IntVector2::operator=(const IntVector2& assignedFrom)
{
	x = assignedFrom.x;
	y = assignedFrom.y;
}

const IntVector2 IntVector2::operator/(const IntVector2& perAxisInverseScaleFactors) const
{
	IntVector2 result(0, 0);
	result.x = this->x / perAxisInverseScaleFactors.x;
	result.y = this->y / perAxisInverseScaleFactors.y;
	return result;
}

void IntVector2::operator-=(const IntVector2& vectorToSubtract)
{
	x -= vectorToSubtract.x;
	y -= vectorToSubtract.y;
}

void IntVector2::operator+=(const IntVector2& vectorToAdd)
{
	x += vectorToAdd.x;
	y += vectorToAdd.y;
}

void IntVector2::operator*=(const IntVector2& perAxisScaleFactors)
{
	x *= perAxisScaleFactors.x;
	y *= perAxisScaleFactors.y;
}

void IntVector2::operator*=(int scale)
{
	x *= scale;
	y *= scale;
}

const IntVector2 IntVector2::operator/(int inverseScale) const
{
	IntVector2 currentVector;
	currentVector.x = x / inverseScale;
	currentVector.y = y / inverseScale;
	return currentVector;
}

const IntVector2 IntVector2::operator*(const IntVector2& perAxisScaleFactors) const
{
	IntVector2 currentVector;
	currentVector.x = this->x * perAxisScaleFactors.x;
	currentVector.y = this->y * perAxisScaleFactors.y;
	return currentVector;
}

const IntVector2 IntVector2::operator*(int scale) const
{
	IntVector2 currentVector;
	currentVector.x = this->x * scale;
	currentVector.y = this->y * scale;
	return currentVector;
}

const IntVector2 IntVector2::operator-(const IntVector2& vectorToSubtract) const
{
	IntVector2 currentVector;
	currentVector.x = this->x - vectorToSubtract.x;
	currentVector.y = this->y - vectorToSubtract.y;
	return currentVector;
}

const IntVector2 IntVector2::operator+(const IntVector2& vectorToAdd) const
{
	IntVector2 currentVector;
	currentVector.x = this->x + vectorToAdd.x;
	currentVector.y = this->y + vectorToAdd.y;
	return currentVector;
}

bool IntVector2::operator==(const IntVector2& vectorToEqual) const
{
	return (x == vectorToEqual.x && y == vectorToEqual.y);
}

bool IntVector2::operator!=(const IntVector2 & vectorToNotEqual) const
{
	return (x != vectorToNotEqual.x || y != vectorToNotEqual.y);
}

bool operator < (const IntVector2& lhs, const IntVector2& rhs) 
{
	if (lhs.y < rhs.y)
		return true;
	if (lhs.y > rhs.y)
		return false;
	return lhs.x < rhs.x;
}

int CalcDistance(const IntVector2& positionA, const IntVector2& positionB)
{
	int differenceX = positionA.x - positionB.x;
	int differenceY = positionA.y - positionB.y;
	return (int)sqrt((differenceX * differenceX) + (differenceY * differenceY));
}

int CalcDistanceSquared(const IntVector2& posA, const IntVector2& posB)
{
	int differenceX = posA.x - posB.x;
	int differenceY = posA.y - posB.y;
	return (differenceX * differenceX) + (differenceY * differenceY);
}

const IntVector2 operator * (int scale, const IntVector2& vectorToScale)
{
	IntVector2 currentVector;
	currentVector.x = vectorToScale.x * scale;
	currentVector.y = vectorToScale.y * scale;
	return currentVector;
}

float DotProduct(const IntVector2& a, const IntVector2& b)
{
	return (((float)a.x*(float)b.x) + ((float)a.y*(float)b.y));
}