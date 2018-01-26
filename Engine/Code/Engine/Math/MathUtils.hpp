#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"

class Vector3;
class IntVector2;

const float PI = 3.1415926535897932384626433832795f;
const float TWO_PI = PI * 2.f;

float ConvertRadiansToDegrees(float radians);
float ConvertDegreesToRadians(float degrees);
int GetRandomIntLessThan(int maxValueNotInclusive);
int GetRandomIntInRange(int minValueInclusive, int maxValueInclusive);
float GetRandomFloatZeroToOne();
float GetRandomFloatInRange(float minimumInclusive, float maximumInclusive);
float CalculateShortestAngularDistance(float startDegrees, float endDegrees);
bool IsEquivalent(float a, float b, float epsilon = 0.0001f);
float FastFloor(float floorPoint);
float SmoothStep(float inputZeroToOne);
float SmoothStep5(float inputZeroToOne);
float SmoothStop(float inputZeroToOne);
float CosInDegrees(float degrees);
float SinInDegrees(float degrees);
float atan2InDegrees(float y, float x);
float ClampNormalizedFloat(float input);
float ClampWithin(float input, float maxValue, float minValue);
int ClampWithin(int input, int maxValue, int minValue);
float RangeMapFloat(float startMin, float startMax, float endMin, float endMax, float startValue);
unsigned char RangeMapUnsignedChar(unsigned char startMin, unsigned char startMax, unsigned char endMin, unsigned char endMax, unsigned char startValue);
bool IsEquivalent(const Vector3& a, const Vector3& b, float epsilon = 0.0001f);
float CalculateMatrix3Determinant(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22);
float CalculateMatrix2Determinant(float m00, float m01, float m10, float m11);
int CalculateManhattanDistance(const IntVector2& start, const IntVector2& end);
float LERP(float start, float end, float fraction);

template <typename T>
inline void Swap(T &a, T &b)
{
	T temp = a;
	a = b;
	b = temp;
};

template<typename T>
T SLERP(const T& a, const T& b, float t) {
	ASSERT_OR_DIE(t >= 0.0f && t <= 1.0f, "Time to SLERP is outside 0 to 1!");
	float dot = DotProduct(a, b);

	T start = a;
	if (dot < 0.0f) {
		// If it's negative - it means it's going the long way
		// flip it.
		start = -start;
		dot = -dot;
	}

	dot = ClampWithin(dot, 1.0f, -1.0f);

	float theta = std::acos(dot);
	float s_t = std::sin(theta);
	float inv_s_t = 1.0f / s_t;
	float s_t_f = std::sin((1.0f - t) * theta);
	float s_tt = std::sin(t * theta);

	float Scale0;
	float Scale1;
	if (abs(dot) < 0.9999f)
	{
		Scale1 = s_tt * inv_s_t;
		Scale0 = (s_t_f)* inv_s_t;
	}
	else
	{
		Scale1 = t;
		Scale0 = 1.0f - t;
	}

	if (dot < 0.0f) {
		Scale1 = -Scale1;
	}
	
	return (Scale0 * start) + (Scale1 * b);
}

// Scale1 = s_tt * inv_s_t
// Scale0 = (s_t_f) * inv_s_t

// if in lock
// Scale1 = t
// Scale0 = 1 - t