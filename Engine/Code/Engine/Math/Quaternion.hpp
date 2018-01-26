#pragma once

#include "Engine/Math/Vector3.hpp"

class Matrix4;

class Quaternion {

	friend float DotProduct(const Quaternion& a, const Quaternion& b);

public:
	float w;
	Vector3 axis;

	static Quaternion CreateRealQuaternion(float scalar);
	static Quaternion CreatePureQuaternion(const Vector3& v);
	static Quaternion CreateFromAxisAngle(const Vector3& axis, float degreesAngle);
	static Quaternion GetIdentity();
	static Quaternion CreateFromEulerAnglesDegrees(float yaw, float pitch, float roll);
	static Quaternion CreateFromEulerAnglesRadians(float yaw, float pitch, float roll);
	static Quaternion CreateFromEulerAngles(float yaw, float pitch, float roll, bool degrees);


	Quaternion();
	explicit Quaternion(const Matrix4& mat);
	explicit Quaternion(const Vector3& rotations);
	Quaternion(const Quaternion& other) = default;
	Quaternion& operator=(const Quaternion& rhs) = default;
	~Quaternion();

	explicit Quaternion(float initialScalar, const Vector3& initialAxis);
	explicit Quaternion(float initialW, float initialX, float initialY, float initialZ);
	Quaternion operator+(const Quaternion& rhs);
	Quaternion& operator+=(const Quaternion& rhs);
	Quaternion operator-(const Quaternion& rhs);
	Quaternion& operator-=(const Quaternion& rhs);
	Quaternion operator*(const Quaternion& rhs);
	Quaternion& operator*=(const Quaternion& rhs);

	Quaternion operator*(const Vector3& rhs);
	Quaternion& operator*=(const Vector3& rhs);

	Quaternion operator*(float scalar);
	Quaternion& operator*=(float scalar);

	Quaternion operator-();

	bool operator==(const Quaternion& rhs);
	bool operator!=(const Quaternion& rhs);

	float CalcLength() const;
	float CalcLengthSquared() const;

	void Normalize();

	void Conjugate();

	void Inverse();

protected:
private:

};

Quaternion operator*(float scalar, const Quaternion& rhs);
Quaternion& operator*=(float scalar, Quaternion& rhs);

Quaternion operator*(const Vector3& lhs, const Quaternion& rhs);
Quaternion& operator*=(const Vector3& lhs, Quaternion& rhs);

Quaternion Conjugate(const Quaternion& q);
Quaternion Inverse(const Quaternion& q);