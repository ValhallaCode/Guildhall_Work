#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/Quaternion.hpp"
#include "Engine/Render/Vertex.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Engine/Math/UintVector4.hpp"
#include <string>

typedef unsigned int uint;
typedef unsigned char byte_t;

// My stream [can be broken into InputStream & OutputStream variants if you prefer]
// Anything can be a stream [as we'll see in networking [SD6], we'll create a 
// NetworkBinaryStream for TCP/IP connections/pulling resources offline/from other users]

// Our basic interface looks like this - note, given either a singular or multiple version
// of writing/reading - you can implement the other.  

static constexpr uint ENDIAN_CHECK = 0x01020304;

enum eEndianness
{
	LITTLE_ENDIAN,
	BIG_ENDIAN,
};

class BinaryStream
{
public:
	// read/write a single byte.  Returns success.
	virtual bool read_byte(byte_t *out) = 0;      // = 0 -> Means this is a pure virtual
	virtual bool write_byte(byte_t const &value) = 0;

	// read/write 'count' bytes.  Returns number of bytes actually read. 
	// will return 0 on failure. 
	virtual uint read_bytes(void *out_buffer, uint const count) = 0;
	virtual uint write_bytes(void const* buffer, uint const size) = 0;

	eEndianness GetHostOrder() const;
	bool IsBigEndian();
	void FlipBytes(void *buffer, uint const size);

	template <typename T>
	bool write(T const &v)
	{
		if (should_flip()) {
			T copy = v; // again, advanced types will cause issues - as 
						// this calls the copy constructor.
			FlipBytes(&copy, sizeof(copy));
			return write_bytes(&copy, sizeof(T)) == 1;
		}
		else {
			return write_bytes(&v, sizeof(T)) == 1;
		}
	}

	template <typename T>
	bool read(T* v)
	{
		if (should_flip()) {
			bool result = read_bytes(v, sizeof(T)) == 1;
			FlipBytes(v, sizeof(*v));
			return result;
		}
		else {
			return read_bytes(v, sizeof(T)) == 1;
		}
	}

	// Write Size in INT First
	template <>
	bool BinaryStream::write(std::string const &str)
	{
		return write_bytes(str.c_str(), (uint)str.size()) == 1;
	}

	template <>
	bool BinaryStream::write(const Vector3& vector)
	{
		bool writeX = write(vector.x);
		bool writeY = write(vector.y);
		bool writeZ = write(vector.z);
		return writeX && writeY && writeZ;
	}

	template <>
	bool BinaryStream::write(const Vector2& vector)
	{
		bool writeX = write(vector.x);
		bool writeY = write(vector.y);
		return writeX && writeY;

	}

	template <>
	bool BinaryStream::write(const Vector4& vector)
	{
		bool writeX = write(vector.x);
		bool writeY = write(vector.y);
		bool writeZ = write(vector.z);
		bool writeW = write(vector.w);
		return writeX && writeY && writeZ && writeW;
	}

	template <>
	bool BinaryStream::write(const Matrix4& matrix)
	{
		Vector4 iBasis = matrix.GetIBasis();
		Vector4 jBasis = matrix.GetJBasis();
		Vector4 kBasis = matrix.GetKBasis();
		Vector4 tBasis = matrix.GetTBasis();

		bool writeI = write(iBasis);
		bool writeJ = write(jBasis);
		bool writeK = write(kBasis);
		bool writeT = write(tBasis);
		return writeI && writeJ && writeK && writeT;
	}

	template <>
	bool BinaryStream::write(const Vertex3_PCT& vertex)
	{
		bool write1 = write(vertex.m_position);
		bool write2 = write(vertex.m_color);
		bool write3 = write(vertex.m_texCoords);
		bool write4 = write(vertex.m_normal);
		bool write5 = write(vertex.m_bitangent);
		bool write6 = write(vertex.m_tangent);
		bool write7 = write(vertex.m_boneIndices);
		bool write8 = write(vertex.m_boneWeights);
		return write1 && write2 && write3 && write4 && write5 && write6 &&write7 && write8;
	}

	template <>
	bool BinaryStream::write(const Rgba& color)
	{
		bool writeR = write(color.r);
		bool writeG = write(color.g);
		bool writeB = write(color.b);
		bool writeA = write(color.a);
		return writeR && writeG && writeB && writeA;
	}

	template <>
	bool BinaryStream::write(const UintVector4& vector)
	{
		bool writeX = write(vector.x);
		bool writeY = write(vector.y);
		bool writeZ = write(vector.z);
		bool writeW = write(vector.w);
		return writeX && writeY && writeZ && writeW;
	}

	// Set Size in INT First
	template <>
	bool BinaryStream::read(std::string* str) // not so sure
	{
		// write the string data, and a terminating NULL to signify I'm done
		std::string temp = *str;
		bool readIt = read_bytes(&temp[0], (uint)str->size()) == 1;
		*str = temp;
		return readIt;
	}

	template <>
	bool BinaryStream::write(const Quaternion& q)
	{
		bool writeW = write(q.w);
		bool writeA = write(q.axis);
		return writeW && writeA;
	}

	template <>
	bool BinaryStream::read(Vector3* vector)
	{
		bool readX = read(&vector->x);
		bool readY = read(&vector->y);
		bool readZ = read(&vector->z);
		return readX && readY && readZ;
	}

	template <>
	bool BinaryStream::read(Vector2* vector)
	{
		bool readX = read(&vector->x);
		bool readY = read(&vector->y);
		return readX && readY;
	}

	template <>
	bool BinaryStream::read(Vector4* vector)
	{
		bool readX = read(&vector->x);
		bool readY = read(&vector->y);
		bool readZ = read(&vector->z);
		bool readW = read(&vector->w);
		return readX && readY && readZ && readW;
	}

	template <>
	bool BinaryStream::read(Matrix4* matrix)
	{
		Vector4 iBasis;
		Vector4 jBasis;
		Vector4 kBasis;
		Vector4 tBasis;

		bool readI = read(&iBasis);
		bool readJ = read(&jBasis);
		bool readK = read(&kBasis);
		bool readT = read(&tBasis);

		matrix->SetIJKTBases(iBasis, jBasis, kBasis, tBasis);
		return readI && readJ && readK && readT;
	}

	template <>
	bool BinaryStream::read(Quaternion* q)
	{
		bool readW = read(&q->w);
		bool readA = read(&q->axis);
		return readA && readW;
	}

	template <>
	bool BinaryStream::read(Vertex3_PCT* vertex)
	{
		bool read1 = read(&vertex->m_position);
		bool read2 = read(&vertex->m_color);
		bool read3 = read(&vertex->m_texCoords);
		bool read4 = read(&vertex->m_normal);
		bool read5 = read(&vertex->m_bitangent);
		bool read6 = read(&vertex->m_tangent);
		bool read7 = read(&vertex->m_boneIndices);
		bool read8 = read(&vertex->m_boneWeights);
		return read1 && read2 && read3 && read4 && read5 && read6 && read7 && read8;
	}

	template <>
	bool BinaryStream::read(Rgba* color)
	{
		bool readR = read(&color->r);
		bool readG = read(&color->g);
		bool readB = read(&color->b);
		bool readA = read(&color->a);
		return readR && readG && readB && readA;
	}

	template <>
	bool BinaryStream::read(UintVector4* vector)
	{
		bool readX = read(&vector->x);
		bool readY = read(&vector->y);
		bool readZ = read(&vector->z);
		bool readW = read(&vector->w);
		return readX && readY && readZ && readW;
	}
	// read update left as an exercize.
	inline bool should_flip() const { return stream_order != GetHostOrder(); }

public:
	eEndianness stream_order;
};

