#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"

class Quaternion;

class Matrix4
{
public:
	float m_values[16];

	Matrix4();
	Matrix4(const Matrix4& copy);
	Matrix4(const float* arrayOfFloats);
	Matrix4(const Vector2& iBasis, const Vector2& jBasis, const Vector2& translation = Vector2(0.f, 0.f));
	Matrix4(const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis, const Vector3& translation = Vector3(0.f, 0.f, 0.f));
	Matrix4(const Vector4& iBasis, const Vector4& jBasis, const Vector4& kBasis, const Vector4& translation = Vector4(0.f, 0.f, 0.f, 1.f));
	Matrix4(const Quaternion& q);
	~Matrix4();

	void operator = (const Matrix4& assignedFrom);
	void MakeIdentity();
	void ConcatenateTranform(const Matrix4& matrixToConcatenate);
	Matrix4 GetTransformed(const Matrix4& matrixToTransform);
	const float* GetAsFloatArray() const;
	float* GetAsFloatArray();
	Vector2 TransformPosition(const Vector2& position2D) const; // Assumes z=0, w=1
	Vector3 TransformPosition(const Vector3& position3D) const; // Assumes w=1
	Vector2 TransformDirection(const Vector2& direction2D) const; // Assumes z=0, w=0
	Vector3 TransformDirection(const Vector3& direction3D) const; // Assumes w=0
	Vector4 TransformVector(const Vector4& homogeneousVector) const; // w is provided
	void Translate(const Vector2& translation2D); // z translation assumed to be 0
	void Translate(const Vector3& translation3D);
	void SetTranslate(const Vector3& translation3D);
	void Scale(float uniformScale);
	void Scale(const Vector2& nonUniformScale2D); // z scale assumed to be 1
	void Scale(const Vector3& nonUniformScale3D);
	void RotateDegreesAboutX(float degrees);
	void RotateDegreesAboutY(float degrees);
	void RotateDegreesAboutYLeftHand(float degrees);
	void RotateDegreesAboutZ(float degrees); // Possibly also offered as RotateDegrees2D
	void RotateRadiansAboutX(float radians);
	void RotateRadiansAboutY(float radians);
	void RotateRadiansAboutZ(float radians); // Possibly also offered as RotateRadians2D
	static Matrix4 CreateTranslation(const Vector2& translation2D); // z translation assumed to be 0
	static Matrix4 CreateTranslation(const Vector3& translation3D);
	static Matrix4 CreateScale(float uniformScale);
	static Matrix4 CreateScale(const Vector2& nonUniformScale2D); // z scale assumed to be 1
	static Matrix4 CreateScale(const Vector3& nonUniformScale3D);
	Vector3 GetScale();
	Vector3 GetPosition();
	static Matrix4 CreateRotationDegreesAboutX(float degrees);
	static Matrix4 CreateRotationDegreesAboutY(float degrees);
	Matrix4 CreateRotationDegreesAboutYLeftHand(float degrees);
	static Matrix4 CreateRotationDegreesAboutZ(float degrees); // a.k.a. CreateRotationDegrees2D
	static Matrix4 CreateRotationRadiansAboutX(float radians);
	static Matrix4 CreateRotationRadiansAboutY(float radians);
	static Matrix4 CreateRotationRadiansAboutZ(float radians); // a.k.a. CreateRotationRadians2D
	Vector4 GetIBasis() const;
	void SetIBasis(const Vector4& vector);
	Vector4 GetJBasis() const;
	Vector4 GetKBasis() const;
	Vector4 GetTBasis() const;
	Vector4 GetXComponents() const;
	Vector4 GetYComponents() const;
	Vector4 GetZComponents() const;
	Vector4 GetWComponents() const;
	Vector3 MultiplyByVector(const Vector3& vector);
	Vector4 MultiplyByVector(const Vector4& vector);
	Vector4 GetDiagonal() const;
	Matrix4 GetInverse();
	Matrix4 MultiplyByFloat(float value);
	float CalculateDeterminant();
	void OrthoNormalize();
	void OrthoNormalizeLeftHand();
	void Transpose();
	Matrix4 GetTranspose() const;
	void SetIJKTBases(const Vector4& i, const Vector4& j, const Vector4& k, const Vector4& t);
	Matrix4 GetOrthoNormalizedMatrix() const;
	void SetIJKBases(const Vector3& iBasis, const Vector3& jBasis, const Vector3& kBasis);
	friend Matrix4 MatrixMultiplicationRowMajorAB(const Matrix4& A, const Matrix4& B);
};
Matrix4 MatrixMultiplicationRowMajorAB(const Matrix4& A, const Matrix4& B);