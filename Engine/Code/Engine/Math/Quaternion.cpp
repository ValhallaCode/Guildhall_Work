#include "Engine/Math/Quaternion.hpp"

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/Math3D.hpp"

#include <cmath>

Quaternion::Quaternion()
	: w(1.0f)
	, axis(Vector3(0.0f, 0.0f, 0.0f)) {
	/* DO NOTHING */
}
Quaternion::Quaternion(const Matrix4& mat)
	: w(1.0f)
	, axis(Vector3(0.0f, 0.0f, 0.0f))
{
	//From http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/

	Matrix4 qMat = mat.GetTranspose();

	Vector4 diag = qMat.GetDiagonal();
	float trace = diag.x + diag.y + diag.z;

	Vector4 row_zero = qMat.GetIBasis();
	Vector4 row_one = qMat.GetJBasis();
	Vector4 row_two = qMat.GetKBasis();

	if (trace > 0.0f) {

		float S = std::sqrt(1.0f + trace) * 2.0f;

		w = 0.25f * S;

		axis.x = (row_two.y - row_one.z) / S;
		axis.y = (row_zero.z - row_two.x) / S;
		axis.z = (row_one.x - row_zero.y) / S;

	}
	else if ((diag.x > diag.y) && (diag.x > diag.z)) {
		float S = std::sqrt(1.0f + diag.x - diag.y - diag.z) * 2.0f;
		w = (row_two.y - row_one.z) / S;
		axis.x = 0.25f * S;
		axis.y = (row_zero.y + row_one.x) / S;
		axis.z = (row_zero.z + row_two.x) / S;
	}
	else if (diag.y > diag.z) {
		float S = std::sqrt(1.0f + diag.y - diag.x - diag.z) * 2.0f;
		w = (row_zero.z - row_two.x) / S;
		axis.x = (row_zero.y + row_one.x) / S;
		axis.y = 0.25f * S;
		axis.z = (row_one.z + row_two.y) / S;
	}
	else {
		float S = std::sqrt(1.0f + diag.z - diag.x - diag.y) * 2.0f;
		w = (row_one.x - row_zero.y) / S;
		axis.x = (row_zero.z + row_two.x) / S;
		axis.y = (row_one.z + row_two.y) / S;
		axis.z = 0.25f * S;
	}

	Normalize();
}

Quaternion::Quaternion(float initialScalar, const Vector3& initialAxis)
	: w(initialScalar)
	, axis(initialAxis)
{
	/* DO NOTHING */
}

Quaternion::Quaternion(float initialW, float initialX, float initialY, float initialZ)
	: w(initialW)
	, axis(initialX, initialY, initialZ)
{
	/* DO NOTHING */
}

Quaternion::Quaternion(const Vector3& rotations)
	: w(0.0f)
	, axis(rotations)
{
	/* DO NOTHING */
}
Quaternion::~Quaternion() {
	/* DO NOTHING */
}

Quaternion  Quaternion::operator+(const Quaternion& rhs) {
	return Quaternion(this->w + rhs.w, this->axis + rhs.axis);
}
Quaternion& Quaternion::operator+=(const Quaternion& rhs) {
	this->w += rhs.w;
	this->axis += rhs.axis;
	return *this;
}
Quaternion  Quaternion::operator-(const Quaternion& rhs) {
	return Quaternion(this->w - rhs.w, this->axis - rhs.axis);
}

Quaternion& Quaternion::operator-=(const Quaternion& rhs) {
	this->w -= rhs.w;
	this->axis -= rhs.axis;
	return *this;
}

Quaternion Quaternion::operator*(const Quaternion& rhs) {
	return Quaternion(this->w * rhs.w - DotProduct(this->axis, rhs.axis),
		this->w * rhs.axis + rhs.w * this->axis + CrossProduct3D(this->axis, rhs.axis));
}
Quaternion& Quaternion::operator*=(const Quaternion& rhs) {
	this->w = this->w * rhs.w - DotProduct(this->axis, rhs.axis);
	this->axis = this->w * rhs.axis + rhs.w * this->axis + CrossProduct3D(this->axis, rhs.axis);
	return *this;
}

Quaternion Quaternion::operator*(float scalar) {
	return Quaternion(this->w * scalar, this->axis * scalar);
}
Quaternion& Quaternion::operator*=(float scalar) {
	this->w *= scalar;
	this->axis *= axis;
	return *this;
}

Quaternion Quaternion::operator*(const Vector3& rhs) {
	return *this * Quaternion(rhs);
}
Quaternion& Quaternion::operator*=(const Vector3& rhs) {
	return this->operator*=(Quaternion(rhs));
}
Quaternion Quaternion::operator-() {
	return Quaternion(-this->w, -this->axis);
}


bool Quaternion::operator==(const Quaternion& rhs) {
	return IsEquivalent(this->w, rhs.w) && IsEquivalent(this->axis, rhs.axis);
}
bool Quaternion::operator!=(const Quaternion& rhs) {
	return !(*this == rhs);
}


float Quaternion::CalcLength() const {
	return std::sqrt(CalcLengthSquared());
}

float Quaternion::CalcLengthSquared() const {
	float scalar = this->w * this->w;
	float imaginary = this->axis.CalcLengthSquared();
	return scalar + imaginary;
}

void Quaternion::Normalize() {
	float lengthSq = CalcLengthSquared();
	if (!IsEquivalent(lengthSq, 0.0f)) {
		float invLength = 1.0f / std::sqrt(lengthSq);
		this->w *= invLength;
		this->axis *= invLength;
	}
}

void Quaternion::Conjugate() {
	this->axis = -1.0f * this->axis;
}

void Quaternion::Inverse() {
	float lengthSq = this->CalcLengthSquared();
	if (!IsEquivalent(lengthSq, 0.0f)) {
		Quaternion q_conj = *this;
		q_conj.Conjugate();
		float invLengthSq = 1.0f / lengthSq;
		Quaternion result = q_conj * invLengthSq;
		this->w = result.w;
		this->axis = result.axis;
	}
}


float DotProduct(const Quaternion& a, const Quaternion& b)
{
	return (a.w * b.w) + DotProduct(a.axis, b.axis);
}

Quaternion Quaternion::CreateRealQuaternion(float scalar) {
	return Quaternion(scalar, Vector3(0.0f, 0.0f, 0.0f));
}
Quaternion Quaternion::CreatePureQuaternion(const Vector3& v) {
	return Quaternion(0.0f, v);
}

Quaternion Quaternion::CreateFromAxisAngle(const Vector3& axis, float degreesAngle) {
	float angle = ConvertDegreesToRadians(degreesAngle);
	float factor = std::sin(angle * 0.5f);

	Vector3 factoredAxis = axis * factor;
	float w = std::cos(angle * 0.5f);

	Quaternion result(w, factoredAxis);
	result.Normalize();
	return result;
}

Quaternion Quaternion::CreateFromEulerAnglesDegrees(float yaw, float pitch, float roll) {
	return CreateFromEulerAngles(yaw, pitch, roll, true);
}

Quaternion Quaternion::CreateFromEulerAnglesRadians(float yaw, float pitch, float roll) {
	return CreateFromEulerAngles(yaw, pitch, roll, false);
}

Quaternion Quaternion::CreateFromEulerAngles(float yaw, float pitch, float roll, bool degrees) {

	if (degrees) {
		yaw = ConvertDegreesToRadians(yaw);
		pitch = ConvertDegreesToRadians(pitch);
		roll = ConvertDegreesToRadians(roll);
	}
	//float c1 = std::cos(yaw / 2);
	//float s1 = std::sin(yaw / 2);
	//float c2 = std::cos(roll / 2);
	//float s2 = std::sin(roll / 2);
	//float c3 = std::cos(pitch / 2);
	//float s3 = std::sin(pitch / 2);

	float c1 = std::cos(pitch / 2);
	float s1 = std::sin(pitch / 2);
	float c2 = std::cos(roll / 2);
	float s2 = std::sin(roll / 2);
	float c3 = std::cos(yaw / 2);
	float s3 = std::sin(yaw / 2);

	float c1c2 = c1*c2;
	float s1s2 = s1*s2;
	Quaternion result;
	result.w = c1c2*c3 - s1s2*s3;
	result.axis.x = c1c2*s3 + s1s2*c3;
	result.axis.y = s1*c2*c3 + c1*s2*s3;
	result.axis.z = c1*s2*c3 - s1*c2*s3;
	return result;
}

Quaternion Quaternion::GetIdentity() {
	return Quaternion(1.0f, Vector3(0.0f, 0.0f, 0.0f));
}
Quaternion Conjugate(const Quaternion& q) {
	return Quaternion(q.w, -1.0f * q.axis);
}

Quaternion Inverse(const Quaternion& q) {
	float lengthSq = q.CalcLengthSquared();
	if (!IsEquivalent(lengthSq, 0.0f)) {
		Quaternion q_conj = q;
		q_conj.Conjugate();
		float invLengthSq = 1.0f / lengthSq;
		Quaternion result = q_conj * invLengthSq;
		return result;
	}
	return q;
}

Quaternion operator*(float scalar, const Quaternion& rhs) {
	return Quaternion(scalar * rhs.w, scalar * rhs.axis);
}
Quaternion& operator*=(float scalar, Quaternion& rhs) {
	rhs.w *= scalar;
	rhs.axis *= scalar;
	return rhs;
}

Quaternion operator*(const Vector3& lhs, const Quaternion& rhs) {
	return Quaternion(Quaternion(lhs) * rhs);
}

Quaternion& operator*=(const Vector3& lhs, Quaternion& rhs) {
	rhs = Quaternion(lhs) * rhs;
	return rhs;
}