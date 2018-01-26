#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Math3D.hpp"
#include "Engine/Math/Quaternion.hpp"
#include <math.h>

Matrix4::Matrix4()
{
	m_values[0] = 1.f;
	m_values[1] = 0.f;
	m_values[2] = 0.f;
	m_values[3] = 0.f;
	m_values[4] = 0.f;
	m_values[5] = 1.f;
	m_values[6] = 0.f;
	m_values[7] = 0.f;
	m_values[8] = 0.f;
	m_values[9] = 0.f;
	m_values[10] = 1.f;
	m_values[11] = 0.f;
	m_values[12] = 0.f;
	m_values[13] = 0.f;
	m_values[14] = 0.f;
	m_values[15] = 1.f;
}

Matrix4::Matrix4(const Matrix4& copy)
{
	for (int index = 0; index < 16; index++)
	{
		m_values[index] = copy.m_values[index];
	}
}

Matrix4::Matrix4(const float* arrayOfFloats)
{
	for (int index = 0; index < 16; index++)
	{
		m_values[index] = arrayOfFloats[index];
	};
}

Matrix4::Matrix4(const Quaternion& q)
{
	Quaternion q_norm = q;
	q_norm.Normalize();

	float wx, wy, wz, xx, yy, yz, xy, xz, zz;

	xx = q.axis.x * (q.axis.x + q.axis.x);	xy = q.axis.x * (q.axis.y + q.axis.y);	xz = q.axis.x * (q.axis.z + q.axis.z);
	yy = q.axis.y * (q.axis.y + q.axis.y);	yz = q.axis.y * (q.axis.z + q.axis.z);	zz = q.axis.z * (q.axis.z + q.axis.z);
	wx = q.w * (q.axis.x + q.axis.x);			wy = q.w * (q.axis.y + q.axis.y);			wz = q.w * (q.axis.z + q.axis.z);

	Matrix4 result(
		Vector4(1.0f - (yy + zz),	xy + wz,			xz - wy,			0.0f),
		Vector4(xy - wz,			1.0f - (xx + zz),	yz + wx,			0.0f),
		Vector4(xz + wy,			yz - wx,			1.0f - (xx + yy),	0.0f),
		Vector4(0.0f,				0.0f,				0.0f,				1.0f)
	);

	*this = result;
}

Matrix4::Matrix4(const Vector2& iBasis, const Vector2& jBasis, const Vector2& translation)
{
	m_values[0] = iBasis.x;
	m_values[1] = iBasis.y;
	m_values[2] = 0.f;
	m_values[3] = 0.f;
	m_values[4] = jBasis.x;
	m_values[5] = jBasis.y;
	m_values[6] = 0.f;
	m_values[7] = 0.f;
	m_values[8] = 0.f;
	m_values[9] = 0.f;
	m_values[10] = 1.f;
	m_values[11] = 0.f;
	m_values[12] = translation.x;
	m_values[13] = translation.y;
	m_values[14] = 0.f;
	m_values[15] = 1.f;
}

Matrix4::Matrix4(const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis, const Vector3& translation)
{
	m_values[0] = iBasis.x;
	m_values[1] = iBasis.y;
	m_values[2] = iBasis.z;
	m_values[3] = 0.f;
	m_values[4] = jBasis.x;
	m_values[5] = jBasis.y;
	m_values[6] = jBasis.z;
	m_values[7] = 0.f;
	m_values[8] = kBasis.x;
	m_values[9] = kBasis.y;
	m_values[10] = kBasis.z;
	m_values[11] = 0.f;
	m_values[12] = translation.x;
	m_values[13] = translation.y;
	m_values[14] = translation.z;
	m_values[15] = 1.f;
}

Matrix4::Matrix4(const Vector4& iBasis, const Vector4& jBasis, const Vector4& kBasis, const Vector4& translation)
{
	m_values[0] = iBasis.x;
	m_values[1] = iBasis.y;
	m_values[2] = iBasis.z;
	m_values[3] = iBasis.w;
	m_values[4] = jBasis.x;
	m_values[5] = jBasis.y;
	m_values[6] = jBasis.z;
	m_values[7] = jBasis.w;
	m_values[8] = kBasis.x;
	m_values[9] = kBasis.y;
	m_values[10] = kBasis.z;
	m_values[11] = kBasis.w;
	m_values[12] = translation.x;
	m_values[13] = translation.y;
	m_values[14] = translation.z;
	m_values[15] = translation.w;
}

Matrix4::~Matrix4()
{
}

void Matrix4::MakeIdentity()
{
	m_values[0] = 1.f;
	m_values[1] = 0.f;
	m_values[2] = 0.f;
	m_values[3] = 0.f;
	m_values[4] = 0.f;
	m_values[5] = 1.f;
	m_values[6] = 0.f;
	m_values[7] = 0.f;
	m_values[8] = 0.f;
	m_values[9] = 0.f;
	m_values[10] = 1.f;
	m_values[11] = 0.f;
	m_values[12] = 0.f;
	m_values[13] = 0.f;
	m_values[14] = 0.f;
	m_values[15] = 1.f;
}

void Matrix4::ConcatenateTranform(const Matrix4& matrixToConcatenate)
{
	Vector4 myI = this->GetIBasis();
	Vector4 myJ = this->GetJBasis();
	Vector4 myK = this->GetKBasis();
	Vector4 myT = this->GetTBasis();
	Vector4 myX = this->GetXComponents();
	Vector4 myY = this->GetYComponents();
	Vector4 myZ = this->GetZComponents();
	Vector4 myW = this->GetWComponents();

	Vector4 rhsI = matrixToConcatenate.GetIBasis();
	Vector4 rhsJ = matrixToConcatenate.GetJBasis();
	Vector4 rhsK = matrixToConcatenate.GetKBasis();
	Vector4 rhsT = matrixToConcatenate.GetTBasis();
	Vector4 rhsX = matrixToConcatenate.GetXComponents();
	Vector4 rhsY = matrixToConcatenate.GetYComponents();
	Vector4 rhsZ = matrixToConcatenate.GetZComponents();
	Vector4 rhsW = matrixToConcatenate.GetWComponents();

	float m00 = DotProduct(myI, rhsX);  float m01 = DotProduct(myI, rhsY); float m02 = DotProduct(myI, rhsZ);  float m03 = DotProduct(myI, rhsW);
	float m10 = DotProduct(myJ, rhsX);  float m11 = DotProduct(myJ, rhsY); float m12 = DotProduct(myJ, rhsZ);; float m13 = DotProduct(myJ, rhsW);;
	float m20 = DotProduct(myK, rhsX);  float m21 = DotProduct(myK, rhsY); float m22 = DotProduct(myK, rhsZ);  float m23 = DotProduct(myK, rhsW);
	float m30 = DotProduct(myT, rhsX);  float m31 = DotProduct(myT, rhsY); float m32 = DotProduct(myT, rhsZ);  float m33 = DotProduct(myT, rhsW);

	m_values[0] = m00;
	m_values[1] = m01;
	m_values[2] = m02;
	m_values[3] = m03;
	m_values[4] = m10;
	m_values[5] = m11;
	m_values[6] = m12;
	m_values[7] = m13;
	m_values[8] = m20;
	m_values[9] = m21;
	m_values[10] = m22;
	m_values[11] = m23;
	m_values[12] = m30;
	m_values[13] = m31;
	m_values[14] = m32;
	m_values[15] = m33;
}

Matrix4 Matrix4::GetTransformed(const Matrix4& matrixToTransform)
{
	Matrix4 transformedMatrix;
	//iBasis
	for (int index = 0; index < 4; index++)
	{
		float firstValue = this->m_values[index] * matrixToTransform.m_values[0];
		float secondValue = this->m_values[index + 4] * matrixToTransform.m_values[1];
		float thirdValue = this->m_values[index + 8] * matrixToTransform.m_values[2];
		float fourthValue = this->m_values[index + 12] * matrixToTransform.m_values[3];
		transformedMatrix.m_values[index] = (firstValue)+(secondValue)+(thirdValue)+(fourthValue);
	}
	//jBasis
	for (int index = 0; index < 4; index++)
	{
		float firstValue = this->m_values[index] * matrixToTransform.m_values[4];
		float secondValue = this->m_values[index + 4] * matrixToTransform.m_values[5];
		float thirdValue = this->m_values[index + 8] * matrixToTransform.m_values[6];
		float fourthValue = this->m_values[index + 12] * matrixToTransform.m_values[7];
		transformedMatrix.m_values[index + 4] = (firstValue)+(secondValue)+(thirdValue)+(fourthValue);
	}
	//kBasis
	for (int index = 0; index < 4; index++)
	{
		float firstValue = this->m_values[index] * matrixToTransform.m_values[8];
		float secondValue = this->m_values[index + 4] * matrixToTransform.m_values[9];
		float thirdValue = this->m_values[index + 8] * matrixToTransform.m_values[10];
		float fourthValue = this->m_values[index + 12] * matrixToTransform.m_values[11];
		transformedMatrix.m_values[index + 8] = (firstValue)+(secondValue)+(thirdValue)+(fourthValue);
	}
	//tBasis
	for (int index = 0; index < 4; index++)
	{
		float firstValue = this->m_values[index] * matrixToTransform.m_values[12];
		float secondValue = this->m_values[index + 4] * matrixToTransform.m_values[13];
		float thirdValue = this->m_values[index + 8] * matrixToTransform.m_values[14];
		float fourthValue = this->m_values[index + 12] * matrixToTransform.m_values[15];
		transformedMatrix.m_values[index + 12] = (firstValue)+(secondValue)+(thirdValue)+(fourthValue);
	}
	return transformedMatrix;
}

const float* Matrix4::GetAsFloatArray() const
{
	return &m_values[0];
}

float* Matrix4::GetAsFloatArray()
{
	return &m_values[0];
}

Vector2 Matrix4::TransformPosition(const Vector2& position2D) const
{
	float xValue = (m_values[0] * position2D.x) + (m_values[4] * position2D.y) + (m_values[8] * 0.f) + (m_values[12] * 1.f);
	float yValue = (m_values[1] * position2D.x) + (m_values[5] * position2D.y) + (m_values[9] * 0.f) + (m_values[13] * 1.f);
	return Vector2(xValue, yValue);
}

Vector3 Matrix4::TransformPosition(const Vector3& position3D) const
{
	float xValue = (m_values[0] * position3D.x) + (m_values[4] * position3D.y) + (m_values[8] * position3D.z) + (m_values[12] * 1.f);
	float yValue = (m_values[1] * position3D.x) + (m_values[5] * position3D.y) + (m_values[9] * position3D.z) + (m_values[13] * 1.f);
	float zValue = (m_values[2] * position3D.x) + (m_values[6] * position3D.y) + (m_values[10] * position3D.z) + (m_values[14] * 1.f);
	return Vector3(xValue, yValue, zValue);
}

Vector2 Matrix4::TransformDirection(const Vector2& direction2D) const 
{
	float xValue = (m_values[0] * direction2D.x) + (m_values[4] * direction2D.y) + (m_values[8] * 0.f) + (m_values[12] * 0.f);
	float yValue = (m_values[1] * direction2D.x) + (m_values[5] * direction2D.y) + (m_values[9] * 0.f) + (m_values[13] * 0.f);
	return Vector2(xValue, yValue);
}

Vector3 Matrix4::TransformDirection(const Vector3& direction3D) const 
{
	float xValue = (m_values[0] * direction3D.x) + (m_values[4] * direction3D.y) + (m_values[8] * direction3D.z) + (m_values[12] * 0.f);
	float yValue = (m_values[1] * direction3D.x) + (m_values[5] * direction3D.y) + (m_values[9] * direction3D.z) + (m_values[13] * 0.f);
	float zValue = (m_values[2] * direction3D.x) + (m_values[6] * direction3D.y) + (m_values[10] * direction3D.z) + (m_values[14] * 0.f);
	return Vector3(xValue, yValue, zValue);
}

Vector4 Matrix4::TransformVector(const Vector4& homogeneousVector) const
{
	float xValue = (m_values[0] * homogeneousVector.x) + (m_values[4] * homogeneousVector.y) + (m_values[8] * homogeneousVector.z) + (m_values[12] * homogeneousVector.w);
	float yValue = (m_values[1] * homogeneousVector.x) + (m_values[5] * homogeneousVector.y) + (m_values[9] * homogeneousVector.z) + (m_values[13] * homogeneousVector.w);
	float zValue = (m_values[2] * homogeneousVector.x) + (m_values[6] * homogeneousVector.y) + (m_values[10] * homogeneousVector.z) + (m_values[14] * homogeneousVector.w);
	float wValue = (m_values[3] * homogeneousVector.x) + (m_values[7] * homogeneousVector.y) + (m_values[11] * homogeneousVector.z) + (m_values[15] * homogeneousVector.w);
	return Vector4(xValue, yValue, zValue, wValue);
}

void Matrix4::Translate(const Vector2& translation2D)
{
	m_values[12] += translation2D.x;
	m_values[13] += translation2D.y;
}

void Matrix4::Translate(const Vector3& translation3D)
{
	m_values[12] += translation3D.x;
	m_values[13] += translation3D.y;
	m_values[14] += translation3D.z;
}

void Matrix4::SetTranslate(const Vector3& translation3D)
{
	m_values[12] = translation3D.x;
	m_values[13] = translation3D.y;
	m_values[14] = translation3D.z;
}

void Matrix4::Scale(float uniformScale)
{
	for (int index = 0; index < 15; index++)
	{
		m_values[index] *= uniformScale;
	}
}

void Matrix4::Scale(const Vector2& nonUniformScale2D)
{
	for (int index = 0; index < 16; index += 4)
	{
		m_values[index] *= nonUniformScale2D.x;
	}

	for (int index = 1; index < 16; index += 4)
	{
		m_values[index] *= nonUniformScale2D.y;
	}
}

void Matrix4::Scale(const Vector3& nonUniformScale3D)
{
	for (int index = 0; index < 16; index += 4)
	{
		m_values[index] *= nonUniformScale3D.x;
	}

	for (int index = 1; index < 16; index += 4)
	{
		m_values[index] *= nonUniformScale3D.y;
	}

	for (int index = 2; index < 16; index += 4)
	{
		m_values[index] *= nonUniformScale3D.z;
	}
}

void Matrix4::RotateDegreesAboutX(float degrees)
{
	Matrix4 rotationAboutX;
	rotationAboutX.m_values[5] = CosInDegrees(degrees);
	rotationAboutX.m_values[6] = SinInDegrees(degrees);
	rotationAboutX.m_values[9] = -SinInDegrees(degrees);
	rotationAboutX.m_values[10] = CosInDegrees(degrees);
	ConcatenateTranform(rotationAboutX);
}

void Matrix4::RotateDegreesAboutY(float degrees)
{
	Matrix4 rotationAboutY;
	rotationAboutY.m_values[0] = CosInDegrees(degrees);
	rotationAboutY.m_values[2] = -SinInDegrees(degrees);
	rotationAboutY.m_values[8] = SinInDegrees(degrees);
	rotationAboutY.m_values[10] = CosInDegrees(degrees);
	ConcatenateTranform(rotationAboutY);
}

void Matrix4::RotateDegreesAboutYLeftHand(float degrees)
{
	Matrix4 rotationAboutY;
	rotationAboutY.m_values[0] = CosInDegrees(degrees);
	rotationAboutY.m_values[2] = SinInDegrees(degrees);
	rotationAboutY.m_values[8] = -SinInDegrees(degrees);
	rotationAboutY.m_values[10] = CosInDegrees(degrees);
	ConcatenateTranform(rotationAboutY);
}

void Matrix4::RotateDegreesAboutZ(float degrees)
{
	Matrix4 rotationAboutZ;
	rotationAboutZ.m_values[0] = CosInDegrees(degrees);
	rotationAboutZ.m_values[1] = SinInDegrees(degrees);
	rotationAboutZ.m_values[4] = -SinInDegrees(degrees);
	rotationAboutZ.m_values[5] = CosInDegrees(degrees);
	ConcatenateTranform(rotationAboutZ);
}

void Matrix4::RotateRadiansAboutX(float radians)
{
	Matrix4 rotationAboutX;
	rotationAboutX.m_values[5] = (float)cos(radians);
	rotationAboutX.m_values[6] = (float)sin(radians);
	rotationAboutX.m_values[9] = (float)-sin(radians);
	rotationAboutX.m_values[10] = (float)cos(radians);
	ConcatenateTranform(rotationAboutX);
}

void Matrix4::RotateRadiansAboutY(float radians)
{
	Matrix4 rotationAboutY;
	rotationAboutY.m_values[0] = (float)cos(radians);
	rotationAboutY.m_values[2] = (float)-sin(radians);
	rotationAboutY.m_values[8] = (float)sin(radians);
	rotationAboutY.m_values[10] = (float)cos(radians);
	ConcatenateTranform(rotationAboutY);
}

void Matrix4::RotateRadiansAboutZ(float radians)
{
	Matrix4 rotationAboutZ;
	m_values[0] = (float)cos(radians);
	m_values[1] = (float)sin(radians);
	m_values[4] = (float)-sin(radians);
	m_values[5] = (float)cos(radians);
	ConcatenateTranform(rotationAboutZ);
}

Matrix4 Matrix4::CreateTranslation(const Vector2& translation2D)
{
	Matrix4 translationMatrix;
	translationMatrix.m_values[12] = translation2D.x;
	translationMatrix.m_values[13] = translation2D.y;
	return translationMatrix;
}

Matrix4 Matrix4::CreateTranslation(const Vector3& translation3D)
{
	Matrix4 translationMatrix;
	translationMatrix.m_values[12] = translation3D.x;
	translationMatrix.m_values[13] = translation3D.y;
	translationMatrix.m_values[14] = translation3D.z;
	return translationMatrix;
}

Matrix4 Matrix4::CreateScale(float uniformScale)
{
	Matrix4 scaleMatrix;
	scaleMatrix.m_values[0] = uniformScale;
	scaleMatrix.m_values[5] = uniformScale;
	scaleMatrix.m_values[10] = uniformScale;
	return scaleMatrix;
}

Matrix4 Matrix4::CreateScale(const Vector2& nonUniformScale2D)
{
	Matrix4 scaleMatrix;
	scaleMatrix.m_values[0] = nonUniformScale2D.x;
	scaleMatrix.m_values[5] = nonUniformScale2D.y;
	return scaleMatrix;
}

Matrix4 Matrix4::CreateScale(const Vector3& nonUniformScale3D)
{
	Matrix4 scaleMatrix;
	scaleMatrix.m_values[0] = nonUniformScale3D.x;
	scaleMatrix.m_values[5] = nonUniformScale3D.y;
	scaleMatrix.m_values[10] = nonUniformScale3D.z;
	return scaleMatrix;
}

Vector3 Matrix4::GetScale()
{
	Vector3 i = GetIBasis();
	float xLength = i.CalcLength();

	Vector3 j = GetJBasis();
	float yLength = j.CalcLength();

	Vector3 k = GetKBasis();
	float zLength = k.CalcLength();

	return Vector3(xLength, yLength, zLength);
}

Vector3 Matrix4::GetPosition()
{
	return Vector3(m_values[12], m_values[13], m_values[14]);
}

Matrix4 Matrix4::CreateRotationDegreesAboutX(float degrees)
{
	Matrix4 rotationMatrix;
	rotationMatrix.m_values[5] = CosInDegrees(degrees);
	rotationMatrix.m_values[6] = SinInDegrees(degrees);
	rotationMatrix.m_values[9] = -SinInDegrees(degrees);
	rotationMatrix.m_values[10] = CosInDegrees(degrees);
	return rotationMatrix;
}

Matrix4 Matrix4::CreateRotationDegreesAboutY(float degrees)
{
	Matrix4 rotationMatrix;
	rotationMatrix.m_values[0] = CosInDegrees(degrees);
	rotationMatrix.m_values[2] = -SinInDegrees(degrees);
	rotationMatrix.m_values[8] = SinInDegrees(degrees);
	rotationMatrix.m_values[10] = CosInDegrees(degrees);
	return rotationMatrix;
}

Matrix4 Matrix4::CreateRotationDegreesAboutYLeftHand(float degrees)
{
	Matrix4 rotationMatrix;
	rotationMatrix.m_values[0] = CosInDegrees(degrees);
	rotationMatrix.m_values[2] = SinInDegrees(degrees);
	rotationMatrix.m_values[8] = -SinInDegrees(degrees);
	rotationMatrix.m_values[10] = CosInDegrees(degrees);
	return rotationMatrix;
}

Matrix4 Matrix4::CreateRotationDegreesAboutZ(float degrees)
{
	Matrix4 rotationMatrix;
	rotationMatrix.m_values[0] = CosInDegrees(degrees);
	rotationMatrix.m_values[1] = SinInDegrees(degrees);
	rotationMatrix.m_values[4] = -SinInDegrees(degrees);
	rotationMatrix.m_values[5] = CosInDegrees(degrees);
	return rotationMatrix;
}

Matrix4 Matrix4::CreateRotationRadiansAboutX(float radians)
{
	Matrix4 rotationMatrix;
	rotationMatrix.m_values[5] = (float)cos(radians);
	rotationMatrix.m_values[6] = (float)sin(radians);
	rotationMatrix.m_values[9] = (float)-sin(radians);
	rotationMatrix.m_values[10] = (float)cos(radians);
	return rotationMatrix;
}

Matrix4 Matrix4::CreateRotationRadiansAboutY(float radians)
{
	Matrix4 rotationMatrix;
	rotationMatrix.m_values[1] = (float)cos(radians);
	rotationMatrix.m_values[3] = (float)-sin(radians);
	rotationMatrix.m_values[8] = (float)sin(radians);
	rotationMatrix.m_values[10] = (float)cos(radians);
	return rotationMatrix;
}

Matrix4 Matrix4::CreateRotationRadiansAboutZ(float radians)
{
	Matrix4 rotationMatrix;
	rotationMatrix.m_values[1] = (float)cos(radians);
	rotationMatrix.m_values[2] = (float)sin(radians);
	rotationMatrix.m_values[4] = (float)-sin(radians);
	rotationMatrix.m_values[5] = (float)cos(radians);
	return rotationMatrix;
}

void Matrix4::OrthoNormalize()
{
	Vector3 iBasis = GetIBasis();
	Vector3 jBasis = GetJBasis();
	Vector3 kBasis = GetKBasis();
	iBasis.Normalize();

	jBasis = CrossProduct3D(kBasis, iBasis);
	jBasis.Normalize();

	kBasis = CrossProduct3D(jBasis, iBasis);
	kBasis.Normalize();

	SetIJKBases(iBasis, jBasis, kBasis);
}

void Matrix4::OrthoNormalizeLeftHand()
{
	Vector3 iBasis = GetIBasis();
	Vector3 jBasis = GetJBasis();
	Vector3 kBasis = GetKBasis();
	iBasis.Normalize();

	jBasis = CrossProduct3D(kBasis, iBasis);
	jBasis.Normalize();

	kBasis = CrossProduct3D(jBasis, iBasis);
	kBasis.z = -1.0f * kBasis.z;
	kBasis.Normalize();

	SetIJKBases(iBasis, jBasis, kBasis);
}

Matrix4 Matrix4::GetOrthoNormalizedMatrix() const
{
	Matrix4 orthoNormalized(*this);
	orthoNormalized.OrthoNormalize();
	return orthoNormalized;
}

void Matrix4::SetIJKBases(const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis)
{
	m_values[0] = iBasis.x;
	m_values[1] = iBasis.y;
	m_values[2] = iBasis.z;

	m_values[4] = jBasis.x;
	m_values[5] = jBasis.y;
	m_values[6] = jBasis.z;
	
	m_values[8] = kBasis.x;
	m_values[9] = kBasis.y;
	m_values[10] = kBasis.z;
}

Matrix4 MatrixMultiplicationRowMajorAB(const Matrix4& A, const Matrix4& B)
{
	Vector4 myI = A.GetIBasis();
	Vector4 myJ = A.GetJBasis();
	Vector4 myK = A.GetKBasis();
	Vector4 myT = A.GetTBasis();
	Vector4 myX = A.GetXComponents();
	Vector4 myY = A.GetYComponents();
	Vector4 myZ = A.GetZComponents();
	Vector4 myW = A.GetWComponents();

	Vector4 rhsI = B.GetIBasis();
	Vector4 rhsJ = B.GetJBasis();
	Vector4 rhsK = B.GetKBasis();
	Vector4 rhsT = B.GetTBasis();
	Vector4 rhsX = B.GetXComponents();
	Vector4 rhsY = B.GetYComponents();
	Vector4 rhsZ = B.GetZComponents();
	Vector4 rhsW = B.GetWComponents();

	float m00 = DotProduct(myI, rhsX);  float m01 = DotProduct(myI, rhsY); float m02 = DotProduct(myI, rhsZ);  float m03 = DotProduct(myI, rhsW);
	float m10 = DotProduct(myJ, rhsX);  float m11 = DotProduct(myJ, rhsY); float m12 = DotProduct(myJ, rhsZ);; float m13 = DotProduct(myJ, rhsW);;
	float m20 = DotProduct(myK, rhsX);  float m21 = DotProduct(myK, rhsY); float m22 = DotProduct(myK, rhsZ);  float m23 = DotProduct(myK, rhsW);
	float m30 = DotProduct(myT, rhsX);  float m31 = DotProduct(myT, rhsY); float m32 = DotProduct(myT, rhsZ);  float m33 = DotProduct(myT, rhsW);

	Vector4 iBasis(m00, m01, m02, m03);
	Vector4 jBasis(m10, m11, m12, m13);
	Vector4 kBasis(m20, m21, m22, m23);
	Vector4 tBasis(m30, m31, m32, m33);

	return Matrix4(iBasis, jBasis, kBasis, tBasis);
}

void Matrix4::operator=(const Matrix4& assignedFrom)
{
	for (int index = 0; index < 16; index++)
	{
		m_values[index] = assignedFrom.m_values[index];
	}
}

void Matrix4::Transpose()
{
	Swap(m_values[1], m_values[4]);
	Swap(m_values[2], m_values[8]);
	Swap(m_values[3], m_values[12]);
	Swap(m_values[6], m_values[9]);
	Swap(m_values[7], m_values[13]);
	Swap(m_values[11], m_values[14]);
}

Matrix4 Matrix4::GetTranspose() const
{
	Matrix4 result = *this;
	result.Transpose();
	return result;
}

void Matrix4::SetIJKTBases(const Vector4& i, const Vector4& j, const Vector4& k, const Vector4& t)
{
	m_values[0] = i.x;
	m_values[1] = i.y;
	m_values[2] = i.z;
	m_values[3] = i.w;
	m_values[4] = j.x;
	m_values[5] = j.y;
	m_values[6] = j.z;
	m_values[7] = j.w;
	m_values[8] = k.x;
	m_values[9] = k.y;
	m_values[10] = k.z;
	m_values[11] = k.w;
	m_values[12] = t.x;
	m_values[13] = t.y;
	m_values[14] = t.z;
	m_values[15] = t.w;
}

Vector4 Matrix4::GetIBasis() const
{
	return Vector4(m_values[0], m_values[1], m_values[2], m_values[3]);
}

void Matrix4::SetIBasis(const Vector4& vector)
{
	m_values[0] = vector.x;
	m_values[1] = vector.y;
	m_values[2] = vector.z;
	m_values[3] = vector.w;
}

Vector4 Matrix4::GetJBasis() const
{
	return Vector4(m_values[4], m_values[5], m_values[6], m_values[7]);
}

Vector4 Matrix4::GetKBasis() const
{
	return Vector4(m_values[8], m_values[9], m_values[10], m_values[11]);
}

Vector4 Matrix4::GetTBasis() const
{
	return Vector4(m_values[12], m_values[13], m_values[14], m_values[15]);
}

Vector4 Matrix4::GetXComponents() const
{
	return Vector4(m_values[0], m_values[4], m_values[8], m_values[12]);
}

Vector4 Matrix4::GetYComponents() const
{
	return Vector4(m_values[1], m_values[5], m_values[9], m_values[13]);
}

Vector4 Matrix4::GetZComponents() const
{
	return Vector4(m_values[2], m_values[6], m_values[10], m_values[14]);
}

Vector4 Matrix4::GetWComponents() const
{
	return Vector4(m_values[3], m_values[7], m_values[11], m_values[15]);
}

Vector3 Matrix4::MultiplyByVector(const Vector3& vector)
{
	Vector4 vecWithPos(vector.x, vector.y, vector.z, 1.0f);
	Vector4 matrixX = this->GetXComponents();
	Vector4 matrixY = this->GetYComponents();
	Vector4 matrixZ = this->GetZComponents();

	return Vector3(DotProduct(vecWithPos, matrixX), DotProduct(vecWithPos, matrixY), DotProduct(vecWithPos, matrixZ));
}

Vector4 Matrix4::MultiplyByVector(const Vector4& vector)
{
	Vector4 matrixX = this->GetXComponents();
	Vector4 matrixY = this->GetYComponents();
	Vector4 matrixZ = this->GetZComponents();
	Vector4 matrixW = this->GetWComponents();

	return Vector4(DotProduct(vector, matrixX), DotProduct(vector, matrixY), DotProduct(vector, matrixZ), DotProduct(vector, matrixW));
}

Vector4 Matrix4::GetDiagonal() const
{
	return Vector4(m_values[0], m_values[5], m_values[10], m_values[15]);
}

Matrix4 Matrix4::GetInverse()
{
	//Calculate minors
	float m00 = CalculateMatrix3Determinant(m_values[5], m_values[6], m_values[7], m_values[9], m_values[10], m_values[11], m_values[13], m_values[14], m_values[15]);
	float m01 = CalculateMatrix3Determinant(m_values[4], m_values[6], m_values[7], m_values[8], m_values[10], m_values[11], m_values[12], m_values[14], m_values[15]);
	float m02 = CalculateMatrix3Determinant(m_values[4], m_values[5], m_values[7], m_values[8], m_values[9], m_values[11], m_values[12], m_values[13], m_values[15]);
	float m03 = CalculateMatrix3Determinant(m_values[4], m_values[5], m_values[6], m_values[8], m_values[9], m_values[10], m_values[12], m_values[13], m_values[14]);

	float m10 = CalculateMatrix3Determinant(m_values[1], m_values[2], m_values[3], m_values[9], m_values[10], m_values[11], m_values[13], m_values[14], m_values[15]);
	float m11 = CalculateMatrix3Determinant(m_values[0], m_values[2], m_values[3], m_values[8], m_values[10], m_values[11], m_values[12], m_values[14], m_values[15]);
	float m12 = CalculateMatrix3Determinant(m_values[0], m_values[1], m_values[3], m_values[8], m_values[9], m_values[11], m_values[12], m_values[13], m_values[15]);
	float m13 = CalculateMatrix3Determinant(m_values[0], m_values[1], m_values[2], m_values[8], m_values[9], m_values[10], m_values[12], m_values[13], m_values[14]);

	float m20 = CalculateMatrix3Determinant(m_values[1], m_values[2], m_values[3], m_values[5], m_values[6], m_values[7], m_values[13], m_values[14], m_values[15]);
	float m21 = CalculateMatrix3Determinant(m_values[0], m_values[2], m_values[3], m_values[4], m_values[6], m_values[7], m_values[12], m_values[14], m_values[15]);
	float m22 = CalculateMatrix3Determinant(m_values[0], m_values[1], m_values[3], m_values[4], m_values[5], m_values[7], m_values[12], m_values[13], m_values[15]);
	float m23 = CalculateMatrix3Determinant(m_values[0], m_values[1], m_values[2], m_values[4], m_values[5], m_values[6], m_values[12], m_values[13], m_values[14]);

	float m30 = CalculateMatrix3Determinant(m_values[1], m_values[2], m_values[3], m_values[5], m_values[6], m_values[7], m_values[9], m_values[10], m_values[11]);
	float m31 = CalculateMatrix3Determinant(m_values[0], m_values[2], m_values[3], m_values[4], m_values[6], m_values[7], m_values[8], m_values[10], m_values[11]);
	float m32 = CalculateMatrix3Determinant(m_values[0], m_values[1], m_values[3], m_values[4], m_values[5], m_values[7], m_values[8], m_values[9], m_values[11]);
	float m33 = CalculateMatrix3Determinant(m_values[0], m_values[1], m_values[2], m_values[4], m_values[5], m_values[6], m_values[8], m_values[9], m_values[10]);

	Matrix4 cofactors(Vector4(m00, -m01, m02, -m03), Vector4(-m10, m11, -m12, m13), Vector4(m20, -m21, m22, -m23), Vector4(-m30, m31, -m32, m33));

	Matrix4 adjugate(cofactors.GetTranspose());

	float det_mat = this->CalculateDeterminant();
	float inv_det = 1.0f / det_mat;

	return  adjugate.MultiplyByFloat(inv_det);
}

Matrix4 Matrix4::MultiplyByFloat(float value)
{
	for (unsigned int index = 0; index < 16; ++index)
	{
		m_values[index] *= value;
	}

	return *this;
}

float Matrix4::CalculateDeterminant()
{
	float a = m_values[0];
	float det_not_a = CalculateMatrix3Determinant(m_values[5], m_values[6], m_values[7], m_values[9], m_values[10], m_values[11], m_values[13], m_values[14], m_values[15]);

	float b = m_values[1];
	float det_not_b = CalculateMatrix3Determinant(m_values[4], m_values[6], m_values[7], m_values[8], m_values[10], m_values[11], m_values[12], m_values[14], m_values[15]);

	float c = m_values[2];
	float det_not_c = CalculateMatrix3Determinant(m_values[4], m_values[5], m_values[7], m_values[8], m_values[9], m_values[11], m_values[12], m_values[13], m_values[15]);

	float d = m_values[3];
	float det_not_d = CalculateMatrix3Determinant(m_values[4], m_values[5], m_values[6], m_values[8], m_values[9], m_values[10], m_values[12], m_values[13], m_values[14]);

	return (a * det_not_a) - (b * det_not_b) + (c * det_not_c) - (d * det_not_d);
}