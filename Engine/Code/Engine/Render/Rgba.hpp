#pragma once



class Rgba
{
public:
	unsigned char r; 
	unsigned char g; 
	unsigned char b; 
	unsigned char a;

	Rgba::Rgba();
	Rgba::~Rgba();
	explicit Rgba::Rgba(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte = 255);
//	explicit Rgba::Rgba(float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha = 1.0f);
	void Rgba::SetAsBytes(unsigned char redByte, unsigned char greenByte, unsigned char blueByte, unsigned char alphaByte = 255);
	void Rgba::SetAsFloats(float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha = 1.0f);
	void Rgba::GetAsFloats(float& out_normalizedRed, float& out_normalizedGreen, float& out_normalizedBlue, float& out_normalizedAlpha) const;
	void Rgba::ScaleRGB(float rgbScale); 
	void Rgba::ScaleAlpha(float alphaScale); 
	unsigned char Rgba::ConvertFloatColorToByteColor(float colorToConvert);
	float Rgba::ConvertByteColorToFloatColor(unsigned char colorToConvert);
	bool operator == (const Rgba& vectorToEqual) const;
	bool Rgba::operator<(const Rgba& vectorToEqual) const;
};

Rgba Interpolate(Rgba zero, Rgba one, float value, bool effect_alpha = false);