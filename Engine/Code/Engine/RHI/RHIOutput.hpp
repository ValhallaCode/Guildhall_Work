#pragma once
#include "Engine/RHI/Texture2D.hpp"
#include "Engine/RHI/RHIDevice.hpp"

class Window;

class RHIOutput
{
public:
	RHIOutput(RHIDevice* owner, Window* window, IDXGISwapChain* dx_swapChain/*[OPTIONAL]*/ /*, eRHIOutputMode const mode = RENDEROUTPUT_WINDOWED*/);
	~RHIOutput();

	// Whatever is currently being rendered
	// that is what we're working with.
	void Present();
	void Close();
	void Open();
	Texture2D* GetRenderTarget();
	float GetWidth() const;
	float GetHeight() const;
	float GetAspectRatio() const;
	// [OPTIONAL]
	//bool set_display_mode(eRHIOutputMode const mode, float const width = 0.0f, float const height = 0.0f);

	RHIDevice* m_device;
	Window* m_window;
	IDXGISwapChain* m_swapChain;
	Texture2D* m_renderTarget;
};