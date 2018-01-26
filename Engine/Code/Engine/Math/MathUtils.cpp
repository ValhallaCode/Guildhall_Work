#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector2.hpp"
#include <cmath>
#include <cstdlib>

float ConvertRadiansToDegrees(float radians)
{
	return radians * (180.f / PI);
}

float ConvertDegreesToRadians(float degrees)
{
	return degrees * (PI / 180.f);
}

float CosInDegrees(float degrees)
{
	float radians = ConvertDegreesToRadians(degrees);
	return (float)cos(radians);
}

float SinInDegrees(float degrees)
{
	float radians = ConvertDegreesToRadians(degrees);
	return (float)sin(radians);
}

float atan2InDegrees(float y, float x)
{
	return (float) atan2(y, x) * ((float) 180 / PI);
}

float ClampNormalizedFloat(float input)
{
	if (input < 0.f)
		return 0.f;
	if (input > 1.f)
		return 1.f;
	return input;
}

float ClampWithin(float input, float maxValue, float minValue)
{
	if (input < minValue)
		return minValue;
	if (input > maxValue)
		return maxValue;
	return input;
}

int ClampWithin(int input, int maxValue, int minValue)
{
	if (input < minValue)
		return minValue;
	if (input > maxValue)
		return maxValue;
	return input;
}

float RangeMapFloat(float startMin, float startMax, float endMin, float endMax, float startValue)
{
	if ((startMax - startMin) != 0.f)
	{
		float normalizedRange = (startValue - startMin) / (startMax - startMin);
		return (normalizedRange * (endMax - endMin)) + endMin;
	}
	ASSERT_OR_DIE((startMax - startMin == 0), "Difference in startMin and startMax results in 0\n");
	return false;
}

unsigned char RangeMapUnsignedChar(unsigned char startMin, unsigned char startMax, unsigned char endMin, unsigned char endMax, unsigned char startValue)
{
	if ((startMax - startMin) != 0.f)
	{
		float normalizedRange = ((float)startValue - (float)startMin) / ((float)startMax - (float)startMin);
		return (unsigned char)((normalizedRange * ((float)endMax - (float)endMin)) + (float)endMin);
	}
	ASSERT_OR_DIE((startMax - startMin == 0), "Difference in startMin and startMax results in 0\n");
	return false;
}

int GetRandomIntLessThan(int maxValueNotInclusive)
{
	return rand() % maxValueNotInclusive;
}

int GetRandomIntInRange(int minValueInclusive, int maxValueInclusive)
{
	return minValueInclusive + (rand() % (int)(maxValueInclusive - minValueInclusive + 1));
}

float GetRandomFloatZeroToOne()
{
	return (float)rand() / (float)RAND_MAX;
}

float GetRandomFloatInRange(float minInclusive, float maxInclusive)
{
	float randomZeroToOne = GetRandomFloatZeroToOne();
	return minInclusive + ((maxInclusive - minInclusive) * randomZeroToOne);
}

float CalculateShortestAngularDistance(float startDegrees, float endDegrees)
{
	float angularDistance = endDegrees - startDegrees;

	while (angularDistance > 180.f)
	{
		angularDistance -= 360.f;
	}

	while (angularDistance < -180.f)
	{
		angularDistance += 360.f;
	}

	return angularDistance;
}

bool IsEquivalent(float a, float b, float epsilon /*= 0.0001f*/)
{
	return std::abs(a - b) < epsilon;
}

bool IsEquivalent(const Vector3& a, const Vector3& b, float epsilon /*= 0.0001f*/)
{
	return (std::abs(a.x - b.x) < epsilon) && (std::abs(a.y - b.y) < epsilon) && (std::abs(a.z - b.z) < epsilon);
}

float FastFloor(float floorPoint)						// faster replacement for floor()
{
	int value = (int)(floorPoint + 32768.f) - 32768;
	return (float)value;
}

float SmoothStep(float inputZeroToOne)					// 3t^2 - 2t^3
{
	return (3.f * (inputZeroToOne * inputZeroToOne) - 2.f * (inputZeroToOne * inputZeroToOne * inputZeroToOne));
}

float SmoothStep5(float inputZeroToOne)					// 6t^5 - 15t^4 + 10t^3
{
	float firstComponent = 6.f * (inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne);
	float secondComponent = 15.f * (inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne);
	float thirdComponent = 10.f * (inputZeroToOne * inputZeroToOne * inputZeroToOne);
	return firstComponent - secondComponent + thirdComponent;
}

float SmoothStop(float inputZeroToOne)
{
	return 1.f - ((1.f -inputZeroToOne) * (1.f - inputZeroToOne));
}

float CalculateMatrix3Determinant(float m00, float m01, float m02,
	float m10, float m11, float m12,
	float m20, float m21, float m22) {
	float a = m00;
	float b = m01;
	float c = m02;
	float det_not_a = CalculateMatrix2Determinant(m11, m12, m21, m22);
	float det_not_b = CalculateMatrix2Determinant(m10, m12, m20, m22);
	float det_not_c = CalculateMatrix2Determinant(m10, m11, m20, m21);

	return a * det_not_a - b * det_not_b + c * det_not_c;
}

float CalculateMatrix2Determinant(float m00, float m01,
	float m10, float m11) {
	return m00 * m11 - m01 * m10;
}

int CalculateManhattanDistance(const IntVector2& start, const IntVector2& end)
{
	int xDif = end.x - start.x;
	int yDif = end.y - start.y;
	return std::abs(xDif) + std::abs(yDif);
}

float LERP(float start, float end, float fraction)
{
	return start + fraction * (end - start);
}