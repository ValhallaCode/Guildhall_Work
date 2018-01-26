#include "Engine/Render/Rgba.hpp"



Rgba::Rgba()
	:r((unsigned char)255)
	, g((unsigned char)255)
	, b((unsigned char)255)
	, a((unsigned char)255)
{}

Rgba::~Rgba()
{}

Rgba::Rgba(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte)
	:r(redByte)
	, g(greenByte)
	, b(blueByte)
	, a(alphaByte)
{}

void Rgba::SetAsBytes(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte)
{
	r = redByte;
	g = greenByte;
	b = blueByte;
	a = alphaByte;
}

void Rgba::SetAsFloats(float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha)
{
	r = ConvertFloatColorToByteColor(normalizedRed);
	g = ConvertFloatColorToByteColor(normalizedGreen);
	b = ConvertFloatColorToByteColor(normalizedBlue);
	a = ConvertFloatColorToByteColor(normalizedAlpha);
}

void Rgba::GetAsFloats(float& out_normalizedRed, float& out_normalizedGreen, float& out_normalizedBlue, float& out_normalizedAlpha) const
{
	out_normalizedRed = (float) r / 255.f;
	out_normalizedGreen = (float) g / 255.f;
	out_normalizedBlue = (float) b / 255.f;
	out_normalizedAlpha = (float) a / 255.f;
}

void Rgba::ScaleRGB(float rgbScale)
{
	if(r > (unsigned char)0)
		r -= ConvertFloatColorToByteColor(rgbScale);
	if (g > (unsigned char)0)
		g -= ConvertFloatColorToByteColor(rgbScale);
	if (b > (unsigned char)0)
		b -= ConvertFloatColorToByteColor(rgbScale);

	if (r < (unsigned char)0)
		r = (unsigned char)0;
	if (g < (unsigned char)0)
		g = (unsigned char)0;
	if (b < (unsigned char)0)
		b = (unsigned char)0;

	if (r > (unsigned char)255)
		r = (unsigned char)255;
	if (g > (unsigned char)255)
		g = (unsigned char)255;
	if (b > (unsigned char)255)
		b = (unsigned char)255;
}

void Rgba::ScaleAlpha(float alphaScale)
{
	if (a >(unsigned char)0)
		a -= ConvertFloatColorToByteColor(alphaScale);
	if (a < (unsigned char)0)
		a = (unsigned char)0;
	if (a > (unsigned char)255)
		a = (unsigned char)255;
}

unsigned char Rgba::ConvertFloatColorToByteColor(float colorToConvert)
{
	float colorInBytes = colorToConvert * 255.f;
	if (colorInBytes > 255.f)
		colorInBytes = 255.f;
	if (colorInBytes < 0.f)
		colorInBytes = 0.f;
	return (unsigned char)colorInBytes;
}

float Rgba::ConvertByteColorToFloatColor(unsigned char colorToConvert)
{
	float colorInFloat = (float)colorToConvert / 255.f;
	return colorInFloat;
}

bool Rgba::operator==(const Rgba& vectorToEqual) const
{
	return (r == vectorToEqual.r && g == vectorToEqual.g && b == vectorToEqual.b && a == vectorToEqual.a);
}


Rgba Interpolate(Rgba zero, Rgba one, float value, bool effect_alpha /*= false*/)
{
	float zero_r = zero.r;
	float zero_g = zero.g;
	float zero_b = zero.b;
	float zero_a = zero.a;

	float one_r = one.r;
	float one_g = one.g;
	float one_b = one.b;
	float one_a = one.a;

	unsigned char rR = static_cast<unsigned char>(((1.0f - value) * zero_r) + (value * one_r));
	unsigned char gR = static_cast<unsigned char>(((1.0f - value) * zero_g) + (value * one_g));
	unsigned char bR = static_cast<unsigned char>(((1.0f - value) * zero_b) + (value * one_b));
	unsigned char aR = 255;
	if(effect_alpha)
		aR = static_cast<unsigned char>(((1.0f - value) * zero_a) + (value * one_a));

	return Rgba(rR, gR, bR, aR);
}