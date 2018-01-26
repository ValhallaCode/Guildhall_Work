#include "Engine/Math/Math2D.hpp"


bool DoAABBsOverlap(const AABB2D& first, const AABB2D& second)
{
	float LeftBorderOne = first.maxs.y;
	float RightBorderOne = first.mins.y;
	float TopBorderOne = first.maxs.x;
	float BottomBorderOne = first.mins.x;

	float LeftBorderTwo = second.maxs.y;
	float RightBorderTwo = second.mins.y;
	float TopBorderTwo = second.maxs.x;
	float BottomBorderTwo = second.mins.x;

	if (BottomBorderTwo > TopBorderOne || LeftBorderTwo < RightBorderOne || TopBorderTwo < BottomBorderOne || RightBorderTwo > LeftBorderOne)
		return false;
	else
		return true;
}

bool DoDiscsOverlap(const Disc2D& first, const Disc2D& second)
{
	Vector2 CenterOne = first.m_center;
	Vector2 CenterTwo = second.m_center;
	float distance = CalcDistance(CenterOne, CenterTwo);
	float RadiusOne = first.m_radius;
	float RadiusTwo = second.m_radius;
	
	if (distance > (RadiusOne + RadiusTwo))
		return false;
	else
		return true;
}
