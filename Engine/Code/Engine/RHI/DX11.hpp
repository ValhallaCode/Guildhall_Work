#pragma once

#include "Engine/RHI/RHITypes.hpp"
#include <string>

#include <d3d11.h>
#include <d3d10.h>
#include <DXGI.h>

#include <initguid.h>
#include <dxgidebug.h> 


#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3d10.lib" )
#pragma comment( lib, "DXGI.lib" )

#include <d3dcompiler.h>
#pragma comment( lib, "d3dcompiler.lib" )

enum eFillMode : unsigned int
{
	SOLID,
	WIREFRAME
};

enum eCullMode : unsigned int
{
	NONE,
	FRONT,
	BACK
};

D3D11_FILL_MODE ConvertFillMode(eFillMode fillType);
D3D11_CULL_MODE ConvertCullMode(eCullMode cullType);
eFillMode GetFillModeFromString(const std::string& string);
eCullMode GetCullModeFromString(const std::string& string);