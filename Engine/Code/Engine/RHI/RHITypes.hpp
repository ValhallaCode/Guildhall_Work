#pragma once
#include "Engine/RHI/DX11.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

enum eBufferUsage : unsigned int
{
	BUFFERUSAGE_GPU,     // GPU Read/Write, CPU Full Writes        (RenderTargets?)
	BUFFERUSAGE_STATIC,  // GPU Read, Written only at creation     (TEXTURES, STATIC MESHES)
	BUFFERUSAGE_DYNAMIC, // GPU Read, CPU Write (Partial or Full)  (DYNAMIC MESHES/DYNAMIC TEXTURES)
	BUFFERUSAGE_STAGING, // GPU Read/Write, CPU Read/Write         Slowest - image composition/manipulation
};

enum ePrimitiveType : unsigned int
{
	PRIMITIVE_NONE,
	PRIMITIVE_POINT,
	PRIMITIVE_LINE,
	PRIMITIVE_TRIANGLE,
};
