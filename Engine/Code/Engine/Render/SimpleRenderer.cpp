#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/RHI/RHITypes.hpp"
#include "Engine/Render/Vertex.hpp"
#include "Engine//RHI/IndexBuffer.hpp"
#include "Engine/RHI/RasterState.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Math3D.hpp"
#include "Engine/RHI/Image.hpp"
#include "Engine/Render/Skeleton.hpp"
#include "Engine/RHI/StructuredBuffer.hpp"
#include "Engine/RHI/Shader.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Render/Animator3D.hpp"
#include "Engine/RHI/TextureDX.hpp"
#include <string>
#include <algorithm>
#include <sstream>

SimpleRenderer::SimpleRenderer()
	:m_currentTarget(nullptr),
	m_output(nullptr),
	m_device(nullptr),
	m_context(nullptr),
	m_vertBuffer(nullptr),
	m_indexBuffer(nullptr),
	m_matrixBuffer(nullptr),
	m_blendState(nullptr),
	m_matrixData{},
	m_defaultDepthStencil(nullptr),
	m_currentDepthStencil(nullptr),
	m_depthStencilDesc{},
	m_depthStencilState(nullptr),
	m_lightBuffer(nullptr),
	m_lightData{},
	m_lightIndex(0),
	m_textureCube(nullptr),
	m_textQuad(new Mesh()),
	m_currentPrimitive(PRIMITIVE_TRIANGLE),
	m_mostRecentIndexCount(0)
{

}

SimpleRenderer::~SimpleRenderer()
{
	Destroy();
}

void SimpleRenderer::Setup(unsigned int width, unsigned int height, const std::string& title)
{
	RHIInstance* ri = RHIInstance::GetInstance();
	m_output = ri->CreateOutput(width, height, IntVector2(50, 50));
	m_device = m_output->m_device;
	m_context = m_device->m_immediateContext;
	std::vector<Vertex3_PCT> vertices(12000);
	m_vertBuffer = m_device->CreateVertexBuffer(vertices.data(), 12000, eBufferUsage::BUFFERUSAGE_DYNAMIC);
	std::vector<unsigned int> indexes(12000);
	m_indexBuffer = m_device->CreateIndexBuffer(indexes, 12000, eBufferUsage::BUFFERUSAGE_DYNAMIC);

	SetWindowTitle(title);
	SetRenderTarget(nullptr);

	m_matrixBuffer = new ConstantBuffer(m_device, &m_matrixData, sizeof(m_matrixData));
	SetConstantBuffer(0, m_matrixBuffer);

	m_lightBuffer = new ConstantBuffer(m_device, &m_lightData, sizeof(m_lightData));
	SetConstantBuffer(2, m_lightBuffer);

	// white texture
	Image image;
	image.CreateClear(1, 1, Rgba(255, 255, 255, 255));
	m_whiteTexture = new Texture2D(m_device, image);

	// create a default normal texture
	image.CreateClear(1, 1, Rgba(128, 128, 255, 255));
	m_flatNormalTexture = new Texture2D(m_device, image);

	image.CreateClear(1, 1, Rgba(128, 128, 128, 255));
	m_greyTexture = new Texture2D(m_device, image);

	image.CreateClear(1, 1, Rgba(0, 0, 0, 255));
	m_blackTexture = new Texture2D(m_device, image);

	m_rasterState = new RasterState(m_device, SOLID, BACK);
	m_context->SetRasterizer(m_rasterState);

	m_defaultDepthStencil = new Texture2D(m_device, width, height, eImageFormat::IMAGEFORMAT_D24S8);
	m_currentDepthStencil = nullptr;

	m_depthStencilDesc.depthTestEnabled = false;
	m_depthStencilDesc.depthWritingEnabled = true;
	m_depthStencilState = new DepthStencilState(m_device, m_depthStencilDesc);
	m_context->SetDepthStencilState(m_depthStencilState);

	SetRenderTarget(nullptr, nullptr);
}

void SimpleRenderer::Destroy()
{
	if (m_output != nullptr) {
		delete m_output;
		m_output = nullptr;
	}

	if (m_indexBuffer != nullptr)
	{
		delete m_indexBuffer;
		m_indexBuffer = nullptr;
	}

	if (m_matrixBuffer != nullptr)
	{
		delete m_matrixBuffer;
		m_matrixBuffer = nullptr;
	}

	if (m_lightBuffer != nullptr)
	{
		delete m_lightBuffer;
		m_lightBuffer = nullptr;
	}

	if (m_whiteTexture != nullptr)
	{
		delete m_whiteTexture;
		m_whiteTexture = nullptr;
	}

	if (m_flatNormalTexture != nullptr)
	{
		delete m_flatNormalTexture;
		m_flatNormalTexture = nullptr;
	}

	if (m_rasterState != nullptr)
	{
		delete m_rasterState;
		m_rasterState = nullptr;
	}

	if (m_defaultDepthStencil != nullptr)
	{
		delete m_defaultDepthStencil;
		m_defaultDepthStencil = nullptr;
	}

	if (m_depthStencilState != nullptr)
	{
		delete m_depthStencilState;
		m_depthStencilState = nullptr;
	}
}

void SimpleRenderer::SetRenderTarget(Texture2D* colorTarget)
{
	if (colorTarget != nullptr) {
		m_currentTarget = colorTarget;
	}
	else
	{
		m_currentTarget = m_output->m_renderTarget;
	}

	m_context->dx_deviceContext->OMSetRenderTargets(1, &m_currentTarget->dx_renderTarget,nullptr);
}

void SimpleRenderer::SetRenderTarget(Texture2D* colorTarget, Texture2D* depthTarget)
{
	if (colorTarget != nullptr) {
		if (colorTarget->IsRenderTarget()) {
			m_currentTarget = colorTarget;
		} // else, WTH?
	}
	else {
		m_currentTarget = m_output->GetRenderTarget();
	}

	if (depthTarget == nullptr) {
		depthTarget = m_defaultDepthStencil;
	}

	m_currentDepthStencil = depthTarget;
	m_context->SetColorTarget(m_currentTarget, depthTarget);
}

void SimpleRenderer::SetWindowTitle(const std::string& title)
{
	std::wstring titleW(title.length(), L' '); 
	std::copy(title.begin(), title.end(), titleW.begin());
	m_output->m_window->SetWindowTitle(title.c_str());
}

void SimpleRenderer::SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
	// Also, set which region of the screen we're rendering to, in this case, all of it 
	D3D11_VIEWPORT viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.TopLeftX = (FLOAT)x;
	viewport.TopLeftY = (FLOAT)y;
	viewport.Width = (FLOAT)width;
	viewport.Height = (FLOAT)height;
	viewport.MinDepth = 0.0f;        // must be between 0 and 1 (defualt is 0);
	viewport.MaxDepth = 1.0f;        // must be between 0 and 1 (default is 1)

	m_context->dx_deviceContext->RSSetViewports(1, &viewport);
}

void SimpleRenderer::ClearColor(const Rgba& color)
{
	m_context->ClearColorTarget(m_currentTarget, color);
}

void SimpleRenderer::ClearDepth(float depth, uint8_t stencil)
{
	m_context->ClearDepthTarget(m_currentDepthStencil, depth, stencil);
}

void SimpleRenderer::ClearTargetColor(Texture2D* target, const Rgba& color)
{
	m_context->ClearColorTarget(target, color);
}

void SimpleRenderer::Present()
{
	//DrawIndexed(m_currentPrimitive, m_vertBuffer, m_indexBuffer, m_mostRecentIndexCount);
	m_output->Present();
}

void SimpleRenderer::ProcessMessages()
{
	m_output->m_window->ProcessMessages();
}

void SimpleRenderer::IsClosed()
{
	m_output->m_window->IsClosed();
}

void SimpleRenderer::SetConstantBuffer(unsigned int bufferIndex, const ConstantBuffer* buffer)
{
	m_context->SetConstantBuffer(bufferIndex, buffer);
}

void SimpleRenderer::SetShaderProgram(ShaderProgram* shader)
{
	m_context->SetShaderProgram(shader);
}

void SimpleRenderer::SetTexture(unsigned int textureIndex, TextureDX* texture)
{
	m_context->SetTexture(textureIndex, texture);
}

void SimpleRenderer::SetSampler(unsigned int samplerIndex, Sampler* sampler)
{
	m_context->SetSampler(samplerIndex, sampler);
}

void SimpleRenderer::ResizeBuffers(unsigned int vertCount, unsigned int indexCount)
{
	if (m_vertBuffer->m_vertCount < vertCount)
	{
		std::vector<Vertex3_PCT> vertices(vertCount);
		m_vertBuffer = m_device->CreateVertexBuffer(vertices.data(), vertCount, eBufferUsage::BUFFERUSAGE_DYNAMIC);
	}

	if (m_indexBuffer->m_indexCount < indexCount)
	{
		std::vector<unsigned int> indexes(indexCount);
		m_indexBuffer = m_device->CreateIndexBuffer(indexes, indexCount, eBufferUsage::BUFFERUSAGE_DYNAMIC);
	}
}

void SimpleRenderer::UpdateBuffersAndDraw(Vertex3_PCT* vertices, const std::vector<unsigned int>& indexes, unsigned int vertCount, unsigned int indexCount, ePrimitiveType primitive /*= PRIMITIVE_TRIANGLE*/)
{
	ResizeBuffers(vertCount, indexCount);
	m_vertBuffer->Update(m_context, *vertices, vertCount);
	m_indexBuffer->Update(m_context, indexes, indexCount);
	//m_mostRecentIndexCount += indexCount;
	//if(m_currentPrimitive != primitive)
	//{
		DrawIndexed(primitive, m_vertBuffer, m_indexBuffer, indexCount);
		//m_currentPrimitive = primitive;
		//m_mostRecentIndexCount = 0;
	//}
}

void SimpleRenderer::DrawLine(const Vector3& startPoint, const Vector3& endPoint, const Rgba& color /*= Rgba(255,255,255,255)*/)
{
	Vertex3_PCT vertices[2] = {
		Vertex3_PCT(startPoint, color, Vector2(0.0f,0.0f)), /*0*/
		Vertex3_PCT(endPoint, color, Vector2(1.0f,1.0f)) /*1*/
	};

	std::vector<unsigned int> indexes = { 0,1 };

	UpdateBuffersAndDraw(vertices, indexes, 2, 2, ePrimitiveType::PRIMITIVE_LINE);
}

void SimpleRenderer::DrawPoint(const Vector3& position, const Rgba& color /*= Rgba(255,255,255,255)*/)
{
	Vector3 r = Vector3(0.2f, 0.0f, 0.0f);
	Vector3 u = Vector3(0.0f, 0.2f, 0.0f);
	Vector3 f = Vector3(0.0f, 0.0f, 0.2f);

	Vertex3_PCT vertices[6] = {
		Vertex3_PCT(position - r, color, Vector2(0.0f,0.0f)),
		Vertex3_PCT(position + r, color, Vector2(0.0f,0.0f)),
		Vertex3_PCT(position - u, color, Vector2(0.0f,0.0f)),
		Vertex3_PCT(position + u, color, Vector2(0.0f,0.0f)),
		Vertex3_PCT(position - f, color, Vector2(0.0f,0.0f)),
		Vertex3_PCT(position + f, color, Vector2(0.0f,0.0f))
	};

	std::vector<unsigned int> indexes = { 0,1,2,3,4,5 };

	UpdateBuffersAndDraw(vertices, indexes, 6, 6, ePrimitiveType::PRIMITIVE_LINE);
}

void SimpleRenderer::DrawPoint(const Vector2& position, const Rgba& color /*= Rgba(255,255,255,255)*/, float radius /*= 10.0f*/)
{
	Vector2 r = Vector2(radius, 0.0f);
	Vector2 u = Vector2(0.0f, radius);

	Vector2 outer_point1 = position - r;
	Vector2 outer_point2 = position + r;
	Vector2 outer_point3 = position - u;
	Vector2 outer_point4 = position + u;

	Vertex3_PCT vertices[4] = {
		Vertex3_PCT(Vector3(outer_point1.x, outer_point1.y, 0.0f), color, Vector2(0.0f,0.0f)),
		Vertex3_PCT(Vector3(outer_point2.x, outer_point2.y, 0.0f), color, Vector2(0.0f,0.0f)),
		Vertex3_PCT(Vector3(outer_point3.x, outer_point3.y, 0.0f), color, Vector2(0.0f,0.0f)),
		Vertex3_PCT(Vector3(outer_point4.x, outer_point4.y, 0.0f), color, Vector2(0.0f,0.0f))
	};

	std::vector<unsigned int> indexes = { 0,1,2,3 };

	UpdateBuffersAndDraw(vertices, indexes, 4, 4, ePrimitiveType::PRIMITIVE_LINE);
}

void SimpleRenderer::DrawIndexed(ePrimitiveType topology, VertexBuffer* vbo, IndexBuffer* ibo, const unsigned int indexCount)
{
	D3D11_PRIMITIVE_TOPOLOGY dx_primative;
	switch (topology) {
	case PRIMITIVE_TRIANGLE:
		dx_primative = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		break;
	case PRIMITIVE_LINE:
		dx_primative = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		break;
	default:
		dx_primative = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}

	m_context->dx_deviceContext->IASetPrimitiveTopology(dx_primative);
	unsigned int stride = sizeof(Vertex3_PCT);
	unsigned int offsets = 0;
	m_context->dx_deviceContext->IASetVertexBuffers(0, 1, &vbo->dx_buffer, &stride, &offsets);
	m_context->dx_deviceContext->IASetIndexBuffer(ibo->dx_buffer, DXGI_FORMAT_R32_UINT, offsets);

	m_context->dx_deviceContext->DrawIndexed(indexCount, 0, 0);
}

void SimpleRenderer::SetProjectionMatrix(const Matrix4& proj)
{
	m_matrixData.projection = proj.GetTranspose();
	m_matrixBuffer->Update(m_context, &m_matrixData);
}

void SimpleRenderer::SetModelMatrix(const Matrix4& transform)
{
	m_matrixData.model = transform;
	m_matrixData.model.Transpose();
	m_matrixBuffer->Update(m_context, &m_matrixData);
}

void SimpleRenderer::MakeViewMatrixIdentity()
{
	m_matrixData.view.MakeIdentity();
	m_matrixBuffer->Update(m_context, &m_matrixData);
}

void SimpleRenderer::MakeModelMatrixIdentity()
{
	m_matrixData.model.MakeIdentity();
	m_matrixBuffer->Update(m_context, &m_matrixData);
}

void SimpleRenderer::SetViewMatrix(const Vector3& rotationInDegrees, const Vector3& position)
{
	SetEyePosition(position);

	float cosPitch = CosInDegrees(rotationInDegrees.x);
	float sinPitch = SinInDegrees(rotationInDegrees.x);
	float cosYaw = CosInDegrees(rotationInDegrees.y);
	float sinYaw = SinInDegrees(rotationInDegrees.y);

	Vector3 xAxis = Vector3(cosYaw, 0, -sinYaw);
	Vector3 yAxis = Vector3(sinYaw * sinPitch, cosPitch, cosYaw * sinPitch);
	Vector3 zAxis = Vector3(sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw);

	Matrix4 viewMatrix =
	{
		Vector4(xAxis.x,            yAxis.x,            zAxis.x,      0),
		Vector4(xAxis.y,            yAxis.y,            zAxis.y,      0),
		Vector4(xAxis.z,            yAxis.z,            zAxis.z,      0),
		Vector4(-DotProduct(xAxis, position), -DotProduct(yAxis, position), -DotProduct(zAxis, position), 1)
	};

	m_matrixData.view = viewMatrix.GetTranspose();
	m_matrixBuffer->Update(m_context, &m_matrixData);
}

void SimpleRenderer::SetOrthoProjection(const Vector2& bottom_left, const Vector2& top_right)
{
	Matrix4 proj = MatrixMakeOrthoProjection(bottom_left.x, top_right.x, bottom_left.y, top_right.y, 0.0f, 1.0f);
	SetProjectionMatrix(proj);
}

void SimpleRenderer::SetPerspectiveProjection(float fieldOfViewInDegrees, float aspectRatio, float nearZ, float farZ)
{
	Matrix4 proj = MatrixMakePerspectiveProjection(fieldOfViewInDegrees, aspectRatio, nearZ, farZ);
	SetProjectionMatrix(proj);
}

Matrix4 SimpleRenderer::MatrixMakeOrthoProjection(float nx, float fx, float ny, float fy, float nz /*= 0.0f*/, float fz /*= 1.0f*/)
{
	Matrix4 mat;

	float sx = 1.0f / (fx - nx);
	float sy = 1.0f / (fy - ny);
	float sz = 1.0f / (fz - nz);

	mat.SetIJKTBases(
		Vector4(2.0f * sx, 0.0f, 0.0f, 0.0f),
		Vector4(0.0f, 2.0f * sy, 0.0f, 0.0f),
		Vector4(0.0f, 0.0f, sz, 0.0f),
		Vector4(-(fx + nx) * sx, -(fy + ny) * sy, -nz * sz, 1.0f)
	);

	return mat;
}

Matrix4 SimpleRenderer::MatrixMakePerspectiveProjection(float fieldOfViewInDegrees, float aspectRatio, float nearZ, float farZ)
{
	Matrix4 mat;

	float FOVRads = ConvertDegreesToRadians(fieldOfViewInDegrees);
	float invTAN = 1.0f / tanf(FOVRads * 0.5f);
	float invAspect = 1.0f / aspectRatio;
	float zDiff = farZ - nearZ;
	float zProd = farZ * nearZ;
	float invZDiff = 1.0f / zDiff;

	mat.SetIJKTBases(
		Vector4(invTAN * invAspect, 0.0f, 0.0f, 0.0f),
		Vector4(0.0f, invTAN, 0.0f, 0.0f),
		Vector4(0.0f, 0.0f, farZ * invZDiff, 1.0f),
		Vector4(0.0f, 0.0f, -zProd * invZDiff, 0.0f)
	);

	return mat;
}

void SimpleRenderer::EnableBlend(eBlendFactor sourceColor, eBlendFactor destinationColor)
{
	if (m_blendState != nullptr)
	{
		if (m_blendStruct.enabled == true && m_blendStruct.sourceFactor == sourceColor && m_blendStruct.destFactor == destinationColor)
		{
			return;
		}
	}

	if (m_blendState != nullptr)
	{
		delete m_blendState;
		m_blendState = nullptr;
	}

	BlendState* state = new BlendState(m_device, true, sourceColor, destinationColor);
	m_context->SetBlendState(state);

	m_blendStruct.enabled = true;
	m_blendStruct.sourceFactor = sourceColor;
	m_blendStruct.destFactor = destinationColor;
	m_blendStruct.sourceAlpha = BLEND_ONE;
	m_blendStruct.destAlpha = BLEND_ONE;

	m_blendState = state;
}

void SimpleRenderer::DisableBlend()
{
	if (!m_blendStruct.enabled)
	{
		return;
	}

	if (m_blendState != nullptr)
	{
		delete m_blendState;
		m_blendState = nullptr;
	}

	BlendState* state = new BlendState(m_device, false);
	m_context->SetBlendState(state);

	m_blendStruct.enabled = false;

	m_blendState = state;
}

void SimpleRenderer::DrawTextWithFont(KerningFont* font, float sx, float sy, const std::string& text, const Rgba& color, float scale /*= 1.0f*/, float aspect /*= 1.0f*/)
{
	Vector2 cursor = Vector2((float)sx, (float)sy);

	char const *c = text.c_str();
	int prevCharCode = -1;
	
	//assume one texture
	Texture2D* texture = CreateOrGetTexture2D(font->m_name, this, "Data/Fonts/" + font->m_pageMap[0]);
	SetTexture(texture);

	while (*c != NULL)
	{
		Glyph valid = font->GetGlyph(*c);
		const Glyph* glyph = &(valid);

		if (glyph == nullptr)
		{
			Glyph invalid = font->GetInvalidGlyph();
			glyph = &(invalid);
			//or skip
		}

		Vector2 bottomLeft;
		Vector2 topRight;

		Vector2 topLeft = cursor + Vector2(((float)glyph->xoffset * scale) * aspect, ((float)-glyph->yoffset * scale) * aspect);//
		bottomLeft = topLeft + Vector2(0, ((float)-glyph->height  * scale) * aspect);//
		topRight = topLeft + Vector2(((float)glyph->width * scale) * aspect, 0);//

		Vector2 uvTopLeft = Vector2((float)glyph->x / (float)font->m_scaleW, (float)glyph->y / (float)font->m_scaleH);
		Vector2 uvBottomLeft = uvTopLeft + Vector2(0, (float)glyph->height / (float)font->m_scaleH);
		Vector2 uvTopRight = uvTopLeft + Vector2((float)glyph->width / (float)font->m_scaleW, 0);

		m_textQuad->CreateOneSidedQuad(bottomLeft, topRight, uvBottomLeft, uvTopRight, color);
		DrawMesh(*m_textQuad);

		cursor += Vector2(((float)glyph->xadvance  * scale) * aspect, 0);//
		prevCharCode = *c;
		++c;

		int kerning = font->GetKerning((int)prevCharCode, (int)*c);
		cursor += Vector2(((float)kerning  * scale) * aspect, 0);//
	}
}

void SimpleRenderer::DrawTextCenteredOnPosition2D(KerningFont* font, const Vector2& position, const std::string& text, const Rgba& color, float scale /*= 1.0f*/, float aspectRatio /*= 1.0f*/)
{
	DrawTextWithFont(font, position.x - (static_cast<float>(font->GetTextWidth(text, scale, aspectRatio)) * 0.5f), position.y + (static_cast<float>(font->GetTextHeight(text, scale)) * 0.5f), text, color, scale, aspectRatio);
}

void SimpleRenderer::DrawTextInBox(KerningFont* font, const AABB2D& box, const std::string& text, const Rgba& color, float scaleX /*= 1.0f*/, float scaleY /*= 1.0f*/)
{
	Vector2 boxDimensions = box.CalcSize();
	float textWidth = static_cast<float>(font->GetTextWidth(text)) * scaleX;
	if (boxDimensions.x < textWidth)
	{
		scaleX *= boxDimensions.x / textWidth;
	}

	float textHeight = static_cast<float>(font->m_lineHeight) * scaleY;
	if (boxDimensions.y < textHeight)
	{
		scaleY *= boxDimensions.y / textHeight;
	}

	//Vector2 boxDimensions = box.CalcSize();
	//float textWidth = static_cast<float>(font->GetTextWidth(font, text)) * scaleX;
	//if (boxDimensions.x < textWidth)
	//{
	//	scaleX *= boxDimensions.x / textWidth;
	//}
	//
	//float textHeight = static_cast<float>(font->m_lineHeight) * scaleY;
	//if (boxDimensions.y < textHeight)
	//{
	//	scaleY *= boxDimensions.y / textHeight;
	//}

	DrawTextCenteredOnPosition2D(font, box.CalcCenter(), text, color, scaleX, scaleX/scaleY);
}

void SimpleRenderer::EnableDepthTest(bool enable)
{
	if (m_depthStencilDesc.depthTestEnabled != enable) {
		m_depthStencilDesc.depthTestEnabled = enable;
		if (nullptr != (m_depthStencilState)) { delete m_depthStencilState; m_depthStencilState = nullptr; }
		m_depthStencilState = new DepthStencilState(m_device, m_depthStencilDesc);
		m_context->SetDepthStencilState(m_depthStencilState);
	}
}

void SimpleRenderer::EnableDepthWrite(bool enable)
{
	if (m_depthStencilDesc.depthWritingEnabled != enable) {
		m_depthStencilDesc.depthWritingEnabled = enable;
		if (nullptr != (m_depthStencilState)) { delete m_depthStencilState; m_depthStencilState = nullptr; }
		m_depthStencilState = new DepthStencilState(m_device, m_depthStencilDesc);
		m_context->SetDepthStencilState(m_depthStencilState);
	}
}

bool SimpleRenderer::IsLightArrayFull()
{
	return m_lightIndex >= (sizeof(m_lightData.lightsArray) / sizeof(*m_lightData.lightsArray)) - 1;
}

void SimpleRenderer::SetAmbientLight(float intensity, const Rgba& color /*= Rgba(255,255,255,255)*/)
{
	color.GetAsFloats(m_lightData.ambientLightColor[0], m_lightData.ambientLightColor[1], m_lightData.ambientLightColor[2], m_lightData.ambientLightColor[3]);
	m_lightData.ambientLightColor[3] = intensity;

	m_lightBuffer->Update(m_context, &m_lightData);
}

void SimpleRenderer::SetPointLight(const Vector3& pos, const Rgba& color, float intensity /*= 1.0f*/, const Vector3& attenuation /*= Vector3(0.0f, 0.0f, 1.0f)*/, const Vector3& specularAttenuation /*= Vector3(0.0f, 0.0f, 1.0f)*/)
{
	if (IsLightArrayFull())
		return;

	m_lightData.lightsArray[m_lightIndex].lightPosition = Vector4(pos.x, pos.y, pos.z, 1.0f);
	color.GetAsFloats(m_lightData.lightsArray[m_lightIndex].pointLightColor[0], m_lightData.lightsArray[m_lightIndex].pointLightColor[1], m_lightData.lightsArray[m_lightIndex].pointLightColor[2], m_lightData.lightsArray[m_lightIndex].pointLightColor[3]);
	m_lightData.lightsArray[m_lightIndex].pointLightColor[3] = intensity;
	m_lightData.lightsArray[m_lightIndex].attenuation = Vector4(attenuation.x, attenuation.y, attenuation.z, 1.0f);
	m_lightData.lightsArray[m_lightIndex].specularAttenuation = Vector4(specularAttenuation.x, specularAttenuation.y, specularAttenuation.z, 1.0f);

	++m_lightIndex;

	m_lightBuffer->Update(m_context, &m_lightData);
}

void SimpleRenderer::ResetLightingIndex()
{
	m_lightIndex = 0;
}

void SimpleRenderer::SetSpecularConstraints(const float specPower, const float specFactor /*= 1.0f*/)
{
	m_lightData.specularFactor = specFactor;
	m_lightData.specularPower = specPower;

	m_lightBuffer->Update(m_context, &m_lightData);
}

void SimpleRenderer::DisablePointLight()
{
	SetPointLight(Vector3(0.0f, 0.0f, 0.0f), Rgba(255,255,255,255), 0.0f, Vector3(1.0f, 0.0f, 0.0f));
}

void SimpleRenderer::SetEyePosition(const Vector3& eyePosition)
{
	m_matrixData.eyePosition = Vector4(eyePosition.x, eyePosition.y, eyePosition.z, 1.0f);
	m_matrixBuffer->Update(m_context, &m_matrixData);
}

void SimpleRenderer::SetDirectionalLight(const Vector4& directionOfView, const Rgba& color)
{
	m_lightData.diretionOfView = directionOfView;
	color.GetAsFloats(m_lightData.directionalColor[0], m_lightData.directionalColor[1], m_lightData.directionalColor[2], m_lightData.directionalColor[3]);

	m_lightBuffer->Update(m_context, &m_lightData);
}

void SimpleRenderer::DrawMesh(Mesh& mesh)
{
	UpdateBuffersAndDraw(mesh.m_vertices.data(), mesh.m_indices, static_cast<unsigned int>(mesh.m_vertices.size()), static_cast<unsigned int>(mesh.m_indices.size()));
}

void SimpleRenderer::DrawOneSidedQuad(const Vector3& position, const Vector3& extension, const Rgba& color /*= Rgba(255,255,255,255)*/)
{
	Vertex3_PCT vertices[4] = {
		Vertex3_PCT(position + Vector3(-extension.x, -extension.y, extension.z), color, Vector2(0.0f,1.0f)),/*0*/
		Vertex3_PCT(position + Vector3(-extension.x, extension.y, extension.z), color, Vector2(0.0f,0.0f)),/*1*/
		Vertex3_PCT(position + Vector3(extension.x, extension.y, extension.z), color, Vector2(1.0f,0.0f)),/*2*/
		Vertex3_PCT(position + Vector3(extension.x, -extension.y, extension.z), color, Vector2(1.0f,1.0f))/*3*/
	};

	std::vector<unsigned int> indices = { 0,3,2,0,2,1 };

	UpdateBuffersAndDraw(vertices, indices, 4, 6);
}

void SimpleRenderer::SetMaterial(Material& material)
{
	ClearDepth();
	EnableBlendOnShader(*material.m_shader);
	m_context->SetMaterial(&material);
	//DisableBlendOnShader(*material.m_shader);
}

void SimpleRenderer::SetRasterStateOnShader(Shader& shader)
{
	if (shader.m_wasRasterSet)
		return;

	m_context->SetRasterizer(shader.GetRasterState());
	shader.m_wasRasterSet = true;
}

void SimpleRenderer::SetDepthStencilOnShader(Shader& shader)
{
	if (shader.m_depthDesc.depthTestEnabled != shader.m_depthTestSetting) {
		shader.m_depthDesc.depthTestEnabled = shader.m_depthTestSetting;
		shader.SetDepthStencil(new DepthStencilState(m_device, shader.m_depthDesc));
		m_context->SetDepthStencilState(shader.GetDepthStencilState());
	}
	if (shader.m_depthDesc.depthWritingEnabled != shader.m_depthWriteSetting) {
		shader.m_depthDesc.depthWritingEnabled = shader.m_depthWriteSetting;
		shader.SetDepthStencil(new DepthStencilState(m_device, shader.m_depthDesc));
		m_context->SetDepthStencilState(shader.GetDepthStencilState());
	}
}

void SimpleRenderer::EnableBlendOnShader(Shader& shader)
{

	if (shader.m_blendStruct == nullptr)
	{
		shader.m_blendStruct = new BlendState_T();
	}

	if (shader.GetBlendState() != nullptr)
	{
		if (shader.m_blendStruct->enabled == true && shader.m_blendStruct->sourceFactor == shader.m_sourceColor && shader.m_blendStruct->destFactor == shader.m_destinationColor)
		{
			return;
		}
	}

	shader.SetBlendState(new BlendState(m_device, true, shader.m_sourceColor, shader.m_destinationColor));
	m_context->SetBlendState(shader.GetBlendState());

	shader.m_blendStruct->enabled = true;
	shader.m_blendStruct->sourceFactor = shader.m_sourceColor;
	shader.m_blendStruct->destFactor = shader.m_destinationColor;
	shader.m_blendStruct->sourceAlpha = BLEND_ONE;
	shader.m_blendStruct->destAlpha = BLEND_ONE;
}

void SimpleRenderer::DisableBlendOnShader(Shader& shader)
{
	if (!shader.m_blendStruct->enabled)
	{
		return;
	}

	shader.SetBlendState(new BlendState(m_device, false));

	m_context->SetBlendState(shader.GetBlendState());

	shader.m_blendStruct->enabled = false;
}

void SimpleRenderer::DrawSkyBox(Vector3& position, float zDepth, std::string& filePath)
{
	if (m_textureCube == nullptr)
	{
		m_textureCube = new TextureCube(m_device, filePath);
	}

	SetTexture(m_textureCube->m_texture);
	m_textureCube->m_cube->CreateInverseCube(position, Vector3(zDepth - 25.0f, zDepth - 25.0f, zDepth - 25.0f));
	DrawMesh(*m_textureCube->m_cube);
}

void SimpleRenderer::DrawRenderMesh(RenderMesh& renderObject)
{
	for (auto meshIter = renderObject.m_meshs.begin(); meshIter != renderObject.m_meshs.end(); ++meshIter)
	{
		auto matIter = renderObject.m_materials.find(meshIter->first);
		if (matIter != renderObject.m_materials.end())
			SetMaterial(matIter->second);
		else
			SetTexture(m_whiteTexture);

		DrawMesh(meshIter->second);
	}
}

void SimpleRenderer::DrawTexturedSprite(const AABB2D& worldBounds, Texture2D& texture, const AABB2D& texBounds, const Rgba& tint, eBlendFactor sourceColor, eBlendFactor destinationColor)
{
	Vertex3_PCT vertexes[4];
	vertexes[0].m_position = Vector3(worldBounds.mins.x, worldBounds.mins.y, 0.f);
	vertexes[0].m_color = tint;
	vertexes[0].m_texCoords = Vector2(texBounds.mins.x, texBounds.maxs.y);
	vertexes[1].m_position = Vector3(worldBounds.maxs.x, worldBounds.mins.y, 0.f);
	vertexes[1].m_color = tint;
	vertexes[1].m_texCoords = Vector2(texBounds.maxs.x, texBounds.maxs.y);
	vertexes[2].m_position = Vector3(worldBounds.maxs.x, worldBounds.maxs.y, 0.f);
	vertexes[2].m_color = tint;
	vertexes[2].m_texCoords = Vector2(texBounds.maxs.x, texBounds.mins.y);
	vertexes[3].m_position = Vector3(worldBounds.mins.x, worldBounds.maxs.y, 0.f);
	vertexes[3].m_color = tint;
	vertexes[3].m_texCoords = Vector2(texBounds.mins.x, texBounds.mins.y);

	std::vector<unsigned int> indices = { 0,2,3,0,1,2 };

	EnableBlend(sourceColor, destinationColor);
	SetTexture(&texture);
	UpdateBuffersAndDraw(vertexes, indices, 4, 6, PRIMITIVE_TRIANGLE);
	DisableBlend();
}

void SimpleRenderer::Draw2DDebugQuad(Vector2& position, Vector2& extension, Rgba& innerColor, Rgba& outerColor, Texture2D& texture, float thickness /*= 0.0f*/)
{
	Vertex3_PCT vertexes[8] = {
		Vertex3_PCT(Vector3(position.x, position.y, 0.0f) + Vector3(-extension.x, -extension.y, 0.0f), innerColor, Vector2(0.0f,1.0f)),/*0*/
		Vertex3_PCT(Vector3(position.x, position.y, 0.0f) + Vector3(-extension.x, extension.y, 0.0f), innerColor, Vector2(0.0f,0.0f)),/*1*/
		Vertex3_PCT(Vector3(position.x, position.y, 0.0f) + Vector3(extension.x, extension.y, 0.0f), innerColor, Vector2(1.0f,0.0f)),/*2*/
		Vertex3_PCT(Vector3(position.x, position.y, 0.0f) + Vector3(extension.x, -extension.y, 0.0f), innerColor, Vector2(1.0f,1.0f)),/*3*/

		Vertex3_PCT(Vector3(position.x, position.y, 0.0f) + Vector3(-extension.x, -extension.y, 0.0f) + Vector3(-thickness, -thickness,0.0f), outerColor, Vector2(0.0f,1.0f)),/*4*/
		Vertex3_PCT(Vector3(position.x, position.y, 0.0f) + Vector3(-extension.x, extension.y, 0.0f) + Vector3(-thickness, thickness,0.0f), outerColor, Vector2(0.0f,0.0f)),/*5*/
		Vertex3_PCT(Vector3(position.x, position.y, 0.0f) + Vector3(extension.x, extension.y, 0.0f)	+ Vector3(thickness, thickness,0.0f), outerColor, Vector2(1.0f,0.0f)),/*6*/
		Vertex3_PCT(Vector3(position.x, position.y, 0.0f) + Vector3(extension.x, -extension.y, 0.0f) + Vector3(thickness, -thickness,0.0f), outerColor, Vector2(1.0f,1.0f))/*7*/
	};

	std::vector<unsigned int> indices = { 4,1,5,4,0,1, 5,2,6,5,1,2, 6,3,7,6,2,3, 7,0,4,7,3,0 };

	SetTexture(&texture);
	UpdateBuffersAndDraw(vertexes, indices, 8, 24, PRIMITIVE_TRIANGLE);
}

void SimpleRenderer::Draw2DDebugLine(Vector2& start, Vector2& end, Rgba& startColor, Rgba& endColor, Texture2D& texture, float thickness /*= 0.0f*/)
{
	Vector2 unit = end - start;
	unit.Normalize();

	Vertex3_PCT vertexes[4] = {
		Vertex3_PCT(Vector3(start.x, start.y, 0.0f) + Vector3(-thickness * unit.y, thickness * unit.x, 0.0f), startColor, Vector2(0.0f,1.0f)),/*0*/
		Vertex3_PCT(Vector3(end.x, end.y, 0.0f) + Vector3(-thickness * unit.y, thickness * unit.x, 0.0f), endColor, Vector2(0.0f,0.0f)),/*1*/
		Vertex3_PCT(Vector3(end.x, end.y, 0.0f) + Vector3(thickness * unit.y, -thickness * unit.x, 0.0f), endColor, Vector2(1.0f,0.0f)),/*2*/
		Vertex3_PCT(Vector3(start.x, start.y, 0.0f) + Vector3(thickness * unit.y, -thickness * unit.x, 0.0f), startColor, Vector2(1.0f,1.0f)),/*3*/
	};

	std::vector<unsigned int> indices = { 0,2,1,0,3,2 };

	SetTexture(&texture);
	UpdateBuffersAndDraw(vertexes, indices, 4, 6, PRIMITIVE_TRIANGLE);
}

void SimpleRenderer::DrawOrientedDebugCircle(const Vector2& position, float radius, Rgba color /*= Rgba(255, 255, 255, 255)*/)
{
	std::vector<Vertex3_PCT> vertexes;

	const int NUM_SIDES = 20;
	const float DEGREES_PER_SIDE = 18.0f;
	Vertex3_PCT newVert;

	newVert.m_color = color;
	newVert.m_texCoords = Vector2(0.5f, 0.5f);
	newVert.m_position = Vector3(position.x, position.y, 0.0f);
	vertexes.push_back(newVert);

	for (int sideIndex = 0; sideIndex < NUM_SIDES; ++sideIndex)
	{
		float angleDegrees = DEGREES_PER_SIDE * (float)sideIndex;
		float x = CosInDegrees(angleDegrees) * radius;
		float y = SinInDegrees(angleDegrees) * radius;
		newVert.m_texCoords = Vector2(CosInDegrees(angleDegrees), SinInDegrees(angleDegrees));
		newVert.m_position = Vector3(x, y, 0.f) + Vector3(position.x, position.y, 0.0f);
		vertexes.push_back(newVert);
	}

	std::vector<unsigned int> indices;
	indices.push_back(0);
	indices.push_back(1);

	// for loop for edge
	for (uint side_index = 1; side_index < vertexes.size() - 1; ++side_index)
	{
		indices.push_back(side_index);
		indices.push_back(side_index + 1);
	}

	indices.push_back(vertexes.size() - 1);
	indices.push_back(1);

	SetTexture(m_whiteTexture);
	UpdateBuffersAndDraw(vertexes.data(), indices, vertexes.size(), indices.size(), PRIMITIVE_LINE);
}

void SimpleRenderer::Draw2DDebugCircle(Vector2& position, float radius, Rgba& innerColor, Rgba& outerColor, Texture2D& texture, float thickness /*= 0.0f*/)
{
	std::vector<Vertex3_PCT> vertexes;

	const int NUM_SIDES = 20;
	const float DEGREES_PER_SIDE = 18.0f;
	Vertex3_PCT newVert;

	for (int sideIndex = 0; sideIndex < NUM_SIDES; ++sideIndex)
	{
		newVert.m_color = innerColor;

		float angleDegrees = DEGREES_PER_SIDE * (float)sideIndex;
		float x = CosInDegrees(angleDegrees) * radius;
		float y = SinInDegrees(angleDegrees) * radius;
		newVert.m_texCoords = Vector2(CosInDegrees(angleDegrees), SinInDegrees(angleDegrees));
		newVert.m_position = Vector3(x, y, 0.f) + Vector3(position.x, position.y, 0.0f);
		vertexes.push_back(newVert);
	}

	for (int sideIndex = 0; sideIndex <= NUM_SIDES; ++sideIndex)
	{
		newVert.m_color = outerColor;

		float angleDegrees = DEGREES_PER_SIDE * (float)sideIndex;
		float x = CosInDegrees(angleDegrees) * radius;
		float y = SinInDegrees(angleDegrees) * radius;
		newVert.m_texCoords = Vector2(CosInDegrees(angleDegrees), SinInDegrees(angleDegrees));
		newVert.m_position = Vector3(x + (CosInDegrees(angleDegrees) * thickness), y + (SinInDegrees(angleDegrees) * thickness), 0.f) + Vector3(position.x, position.y, 0.0f);
		vertexes.push_back(newVert);
	}

	std::vector<unsigned int> indices;
	for (int sideIndex = 0; sideIndex < NUM_SIDES; ++sideIndex)
	{
		indices.push_back(sideIndex + NUM_SIDES);
		indices.push_back(sideIndex + NUM_SIDES + 1);
		indices.push_back(sideIndex + 1);
		indices.push_back(sideIndex + NUM_SIDES);
		indices.push_back(sideIndex + 1);
		indices.push_back(sideIndex);
	}

	indices.push_back((NUM_SIDES * 2));
	indices.push_back(0);
	indices.push_back((NUM_SIDES - 1));

	SetTexture(&texture);
	UpdateBuffersAndDraw(vertexes.data(), indices, vertexes.size(), indices.size(), PRIMITIVE_TRIANGLE);
}

void SimpleRenderer::DrawCharInBox(KerningFont* font, const AABB2D& bounds, const std::string& text, const Rgba& color, float scaleX /*= 1.0f*/, float scaleY /*= 1.0f*/)
{
	Vector2 cursor = bounds.CalcCenter();

	char const *c = text.c_str();
	int prevCharCode = -1;

	//assume one texture
	Texture2D texture(m_device, "Data/Fonts/" + font->m_pageMap[0]);
	SetTexture(&texture);

	while (*c != NULL)
	{
		Glyph valid = font->GetGlyph(*c);
		const Glyph* glyph = &(valid);

		if (glyph == nullptr)
		{
			Glyph invalid = font->GetInvalidGlyph();
			glyph = &(invalid);
			//or skip
		}

		Vector2 uvTopLeft = Vector2((float)glyph->x / (float)font->m_scaleW, (float)glyph->y / (float)font->m_scaleH);
		Vector2 uvBottomLeft = uvTopLeft + Vector2(0, (float)glyph->height / (float)font->m_scaleH);
		Vector2 uvTopRight = uvTopLeft + Vector2((float)glyph->width / (float)font->m_scaleW, 0);

		m_textQuad->CreateOneSidedQuad(bounds.mins + Vector2((1.0f - scaleX) * 0.5f, (1.0f - scaleY) * 0.5f), bounds.maxs - Vector2((1.0f - scaleX) * 0.5f, (1.0f - scaleY) * 0.5f), uvBottomLeft, uvTopRight, color);
		DrawMesh(*m_textQuad);

		cursor += Vector2((float)glyph->xadvance * scaleX, 0);
		prevCharCode = *c;
		++c;

		int kerning = font->GetKerning((int)prevCharCode, (int)*c);
		cursor += Vector2((float)kerning * scaleX, 0);
	}
}

void SimpleRenderer::DrawBitmapFont(const BitmapFont* font, const Vector2& startBottomLeft, const std::string& asciiText, float cellHeight /*= 1.0f*/, const Rgba& tint /*= Rgba(255,255,255,255)*/, float cellAspectScale /*= 1.0f*/)
{
	float cellWidth = cellHeight * cellAspectScale;
	cellHeight *= cellAspectScale;
	Vector2 startOfText(startBottomLeft);

	//loop through text
	for (int textIndex = 0; textIndex < (int)asciiText.size(); textIndex++)
	{
		AABB2D fontTexCoords = font->GetTexCoordsForGlyph((int)asciiText[textIndex]);
		AABB2D worldBounds(startOfText + Vector2(cellWidth * 0.5f, cellHeight * 0.5f), Vector2(startOfText.x + cellWidth, startOfText.y + cellHeight));
		Texture2D* texture = font->GetDirectXFontTexture();
		DrawTexturedSprite(worldBounds, *texture, fontTexCoords, tint, BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
		startOfText.x += cellWidth;
	}
}

void SimpleRenderer::DrawSkeleton(Skeleton* skeleton)
{
	for (unsigned int index = 0; index < skeleton->GetJointCount(); ++index)
	{
		if(skeleton->DoesJointHaveParent(index))
		{
			Vector3 start = skeleton->m_globalTransform[skeleton->GetJointParent(index)].GetPosition();
			Vector3 end = skeleton->m_globalTransform[index].GetPosition();
			DrawLine(start, end, Rgba(255, 255, 255, 255));
		}
	}
}

void SimpleRenderer::DrawSkeletonWithMotion(const Motion* motion, Skeleton* skeleton, float time, ePlayMode playMode)
{
	Pose* poseToDraw = new Pose();
	motion->Evaluate(poseToDraw, time, playMode);
	DrawSkeletonWithPose(poseToDraw, skeleton);
	delete poseToDraw;
	poseToDraw = nullptr;
}

void SimpleRenderer::DrawSkeletonWithPose(Pose* pose, Skeleton* skeleton)
{
	for (unsigned int index = 0; index < skeleton->GetJointCount(); ++index)
	{
		if(skeleton->DoesJointHaveParent(index))
		{
			Matrix4 start = pose->GetGlobalTransformForLocalIndex(skeleton, index);

			unsigned int parentIndex = skeleton->GetJointParent(index);
			Matrix4 end = pose->GetGlobalTransformForLocalIndex(skeleton, parentIndex);

			Vector3 me = start.GetPosition();
			Vector3 parent = end.GetPosition();
			DrawLine(me, parent, Rgba(255, 128, 0, 255));
		}
	}
}

void SimpleRenderer::DrawAnimatedMesh(Mesh* mesh, const Motion* motion, Skeleton* skeleton, ShaderProgram* shader, float time, ePlayMode playMode)
{
	Pose* poseToDraw = new Pose();
	motion->Evaluate(poseToDraw, time, playMode);

	skeleton->CalculateSkinMatrix(m_context, poseToDraw);

	SetStructuredBuffer(skeleton->m_skinTransforms);
	SetShaderProgram(shader);
	DrawMesh(*mesh);

	delete poseToDraw;
	poseToDraw = nullptr;
}

void SimpleRenderer::DrawAnimatedMesh(Animator3D* animator)
{
	animator->m_skeleton->CalculateSkinMatrix(m_context, animator->m_currentPose);
	SetStructuredBuffer(animator->m_skeleton->m_skinTransforms);
	DrawRenderMesh(*animator->m_mesh);
}

void SimpleRenderer::SetStructuredBuffer(unsigned int structureIndex, StructuredBuffer* structure)
{
	m_context->SetStructuredBuffer(structureIndex, structure);
}

void SimpleRenderer::DrawArrow2D(Vector2& start, Vector2& end, Rgba& startColor, Rgba& endColor, float thickness /*= 0.1f*/, Texture2D* texture /*= nullptr*/)
{
	float radius = thickness * 0.5f;
	Vector2 displacement = end - start;
	Vector2 forwardDir = displacement;
	forwardDir.Normalize();
	Vector2 leftDir(-forwardDir.y, forwardDir.x);
	Vector2 sternum = end - (3.0f * radius * forwardDir);

	// 7 Vertex points
	Vector2 leftHeel = start + (radius * leftDir);
	Vector2 rightHeel = start - (radius * leftDir);
	Vector2 endTip = end;
	Vector2 leftArmPit = sternum + (radius * leftDir);
	Vector2 rightArmPit = sternum - (radius * leftDir);
	Vector2 leftWingTip = sternum + (3.0f * radius * leftDir);
	Vector2 rightWingTip = sternum - (3.0f * radius * leftDir);

	Vertex3_PCT vertices[7] = {
		Vertex3_PCT(Vector3(endTip.x, endTip.y, 0.0f), endColor, Vector2(1.0f, 0.5f)),
		Vertex3_PCT(Vector3(leftWingTip.x, leftWingTip.y, 0.0f), endColor, Vector2(0.5f, 0.0f)),
		Vertex3_PCT(Vector3(leftArmPit.x, leftArmPit.y, 0.0f), endColor, Vector2(0.5f, 0.25f)),
		Vertex3_PCT(Vector3(leftHeel.x, leftHeel.y, 0.0f), startColor, Vector2(0.0f, 0.25f)),
		Vertex3_PCT(Vector3(rightHeel.x, rightHeel.y, 0.0f), startColor, Vector2(0.0f, 0.75f)),
		Vertex3_PCT(Vector3(rightArmPit.x, rightArmPit.y, 0.0f), endColor, Vector2(0.5f, 0.75f)),
		Vertex3_PCT(Vector3(rightWingTip.x, rightWingTip.y, 0.0f), endColor, Vector2(0.5f, 1.0f))
	};

	std::vector<unsigned int> indices = { 0,1,2, 0,2,3, 0,3,4, 0,4,5, 0,5,6};

	if (texture == nullptr)
		SetTexture(m_whiteTexture);
	else
		SetTexture(texture);

	UpdateBuffersAndDraw(vertices, indices, 7, 15);
}

void SimpleRenderer::Draw2DQuad(const Vector2& position, const Vector2& extension, Texture2D* texture /*= nullptr*/, const Rgba& color /*= Rgba(255,255,255,255)*/)
{
	Vector2 point1 = position + Vector2(-extension.x, -extension.y);
	Vector2 point2 = position + Vector2(-extension.x, extension.y);
	Vector2 point3 = position + Vector2(extension.x, extension.y);
	Vector2 point4 = position + Vector2(extension.x, -extension.y);

	Vertex3_PCT vertices[4] = {
		Vertex3_PCT(Vector3(point1.x, point1.y, 0.0f), color, Vector2(0.0f,1.0f)),/*0*/
		Vertex3_PCT(Vector3(point2.x, point2.y, 0.0f), color, Vector2(0.0f,0.0f)),/*1*/
		Vertex3_PCT(Vector3(point3.x, point3.y, 0.0f), color, Vector2(1.0f,0.0f)),/*2*/
		Vertex3_PCT(Vector3(point4.x, point4.y, 0.0f), color, Vector2(1.0f,1.0f))/*3*/
	};

	std::vector<unsigned int> indices = { 0,3,2,0,2,1 };

	if (texture == nullptr)
		SetTexture(m_whiteTexture);
	else
		SetTexture(texture);

	UpdateBuffersAndDraw(vertices, indices, 4, 6);
}