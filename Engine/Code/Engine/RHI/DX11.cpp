#include "Engine/RHI/DX11.hpp"
#include <locale>

//--------------------------------------------------------------------------//
//							CONVERSION FUNCTIONS							//
//--------------------------------------------------------------------------//

D3D11_FILL_MODE ConvertFillMode(eFillMode fillType)
{
	switch (fillType)
	{
	case SOLID:
		return D3D11_FILL_SOLID;
	case WIREFRAME:
		return D3D11_FILL_WIREFRAME;
	default:
		ASSERT_OR_DIE(false, "Unsupported Fill Mode used to convert!");
		return D3D11_FILL_SOLID;
	}
};

D3D11_CULL_MODE ConvertCullMode(eCullMode cullType)
{
	switch (cullType)
	{
	case NONE:
		return D3D11_CULL_NONE;
	case FRONT:
		return D3D11_CULL_FRONT;
	case BACK:
		return D3D11_CULL_BACK;
	default:
		ASSERT_OR_DIE(false, "Unsupported Cull Mode used to convert!");
		return D3D11_CULL_NONE;
	}
};

eFillMode GetFillModeFromString(const std::string& string)
{
	std::locale local;
	std::string casedType = string;
	for (unsigned int i = 0; i < casedType.length(); ++i)
		casedType[i] = std::tolower(casedType[i], local);

	if (casedType == "solid")
	{
		return SOLID;
	}
	else if (casedType == "wireframe")
	{
		return WIREFRAME;
	}
	else
	{
		ASSERT_OR_DIE(false, "Unsupported Fill Mode used to convert!");
		return SOLID;
	}
}

eCullMode GetCullModeFromString(const std::string& string)
{
	std::locale local;
	std::string casedType = string;
	for (unsigned int i = 0; i < casedType.length(); ++i)
		casedType[i] = std::tolower(casedType[i], local);

	if (casedType == "none")
	{
		return NONE;
	}
	else if (casedType == "front")
	{
		return FRONT;
	}

	else if (casedType == "back")
	{
		return BACK;
	}
	else
	{
		ASSERT_OR_DIE(false, "Unsupported Cull Mode used to convert!");
		return NONE;
	}
}
