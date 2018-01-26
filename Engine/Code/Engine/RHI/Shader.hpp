#pragma once
#include "Engine/RHI/BlendState.hpp"
#include "Engine/RHI/RasterState.hpp"
#include "Engine/RHI/DepthStencilState.hpp"
#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include <string>

class SimpleRenderer;
struct BlendState_T;

class Shader 
{
public:
	Shader(SimpleRenderer* renderer);
	Shader(tinyxml2::XMLElement& element, SimpleRenderer* renderer);
	~Shader();
	DepthStencilState* GetDepthStencilState();
	BlendState* GetBlendState();
	ShaderProgram* GetShaderProgram();
	RasterState* GetRasterState();
	void SetRasterState(const eFillMode& fillMode, const eCullMode& cullMode);
	void SetBlendState(bool enabled, const eBlendFactor& source, const eBlendFactor& destination);
	void SetBlendState(BlendState* newBlend);
	void SetShaderProgram(const std::string& name, const std::string& filePath);
	void SetDepthStencil(bool enableTest, bool enableWrite);
	void SetDepthStencil(DepthStencilState* newDepth);
	void EnableDepthTest(bool enable);
	void EnableDepthWrite(bool enable);
private:
	SimpleRenderer* m_currentRenderer;
public:
	std::string m_name;
	ShaderProgram* m_shaderProgram;
	RasterState* m_raster;
	BlendState* m_blend;
	DepthStencilState* m_depthStencil;
	DepthStencilDesc_T m_depthDesc;
	bool m_depthTestSetting;
	bool m_depthWriteSetting;
	BlendState_T* m_blendStruct;
	eBlendFactor m_sourceColor;
	eBlendFactor m_destinationColor;
	bool m_wasRasterSet;
};
