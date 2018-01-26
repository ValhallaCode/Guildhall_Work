#pragma once
#include "Engine/Math/AABB2D.hpp"
#include "Engine/Math/Disc2D.hpp"

bool DoAABBsOverlap(const AABB2D& first, const AABB2D& second);
bool DoDiscsOverlap(const Disc2D& first, const Disc2D& second);