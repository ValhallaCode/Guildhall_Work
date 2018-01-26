#pragma once
#include "Engine/RHI/RHIOutput.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "dxguid.lib" )

class RHIInstance
{
public:
	// Constructor is private:  Singleton Class
	~RHIInstance();
	static RHIInstance* GetInstance();
	RHIOutput* RHIInstance::CreateOutput(UINT const px_width, UINT const px_height, const IntVector2& position/*OPTIONAL*/ /*, eRHIOutputMode const initial_mode = RENDEROUTPUT_WINDOWED*/);
	static RHIInstance* m_instance;

private:
	RHIInstance();
	static void SetDebugName(ID3D11DeviceChild *d3d_obj, char const *name);
	static void ReportLiveObjects();
};