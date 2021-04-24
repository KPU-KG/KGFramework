#pragma once
#include <iostream>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

inline DirectX::XMFLOAT3 operator*( const DirectX::XMFLOAT3& vector, float scalar )
{
	using namespace DirectX;
	auto vec = XMLoadFloat3( &vector );
	auto result = XMVectorScale( vec, scalar );
	XMFLOAT3 resultVector;
	XMStoreFloat3( &resultVector, result );
	return resultVector;
}
inline DirectX::XMFLOAT3 operator*( float scalar, const DirectX::XMFLOAT3& vector)
{
	return vector * scalar;
}

inline DirectX::XMFLOAT4 operator*( const DirectX::XMFLOAT4& vector, float scalar )
{
	using namespace DirectX;
	auto vec = XMLoadFloat4( &vector );
	auto result = XMVectorScale( vec, scalar );
	XMFLOAT4 resultVector;
	XMStoreFloat4( &resultVector, result );
	return resultVector;
}
inline DirectX::XMFLOAT4 operator*( float scalar, const DirectX::XMFLOAT4& vector )
{
	return vector * scalar;
}


namespace KG::Math
{
	constexpr float  PI = 3.14159265358979f;
	constexpr double PI_D = 3.141592653589793238463;
	constexpr DirectX::XMFLOAT3 look = DirectX::XMFLOAT3(0, 0, 1);
	constexpr DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0, 1, 0);
	constexpr DirectX::XMFLOAT3 right = DirectX::XMFLOAT3(1, 0, 0);


	namespace Vector3
	{
		using namespace DirectX;
		inline XMFLOAT3 XMVectorToFloat3(const XMVECTOR& xmvVector)
		{
			XMFLOAT3 xmf3Result;
			XMStoreFloat3(&xmf3Result, xmvVector);
			return xmf3Result;
		}
		inline XMFLOAT3 ScalarProduct(const XMFLOAT3& xmf3Vector, float fScalar, bool bNormalize = true)
		{
			XMFLOAT3 xmf3Result;
			if (bNormalize)
			{
				XMStoreFloat3(&xmf3Result, XMVector3Normalize(XMLoadFloat3(&xmf3Vector)) * fScalar);
			}
			else
			{
				XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector) * fScalar);
			}
			return xmf3Result;
		}
		inline XMFLOAT3 Add(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2)
		{
			XMFLOAT3 xmf3Result;
			XMStoreFloat3(&xmf3Result, XMVectorAdd(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2)));
			return xmf3Result;
		}
		inline XMFLOAT3 Add(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2, float fScalar)
		{
			XMFLOAT3 xmf3Result;
			XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) + (XMLoadFloat3(&xmf3Vector2) * fScalar));
			return(xmf3Result);
		}
		inline XMFLOAT3 Multiply( const XMFLOAT3& xmf4Vector1, const XMFLOAT3& xmf4Vector2 )
		{
			XMFLOAT3 xmf4Result;
			XMStoreFloat3( &xmf4Result, XMLoadFloat3( &xmf4Vector1 ) *
				XMLoadFloat3( &xmf4Vector2 ) );
			return(xmf4Result);
		}
		inline XMFLOAT3 Multiply( float fScalar, const XMFLOAT3& xmf4Vector )
		{
			XMFLOAT3 xmf4Result;
			XMStoreFloat3( &xmf4Result, fScalar * XMLoadFloat3( &xmf4Vector ) );
			return(xmf4Result);
		}
		inline XMFLOAT3 Subtract(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2)
		{
			XMFLOAT3 xmf3Result;
			XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) -
				XMLoadFloat3(&xmf3Vector2));
			return(xmf3Result);
		}
		inline float DotProduct(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2)
		{
			XMFLOAT3 xmf3Result;
			XMStoreFloat3(&xmf3Result, XMVector3Dot(XMLoadFloat3(&xmf3Vector1),
				XMLoadFloat3(&xmf3Vector2)));
			return(xmf3Result.x);
		}
		inline XMFLOAT3 CrossProduct(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2, bool
			bNormalize = true)
		{
			XMFLOAT3 xmf3Result;
			if (bNormalize)
				XMStoreFloat3(&xmf3Result,
					XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&xmf3Vector1),
						XMLoadFloat3(&xmf3Vector2))));
			else
				XMStoreFloat3(&xmf3Result, XMVector3Cross(XMLoadFloat3(&xmf3Vector1),
					XMLoadFloat3(&xmf3Vector2)));
			return(xmf3Result);
		}
		inline XMFLOAT3 Normalize(const XMFLOAT3& xmf3Vector)
		{
			XMFLOAT3 m_xmf3Normal;
			XMStoreFloat3(&m_xmf3Normal, XMVector3Normalize(XMLoadFloat3(&xmf3Vector)));
			return(m_xmf3Normal);
		}
		inline float Length(const XMFLOAT3& xmf3Vector)
		{
			XMFLOAT3 xmf3Result;
			XMStoreFloat3(&xmf3Result, XMVector3Length(XMLoadFloat3(&xmf3Vector)));
			return(xmf3Result.x);
		}
		inline float Angle(const XMVECTOR& xmvVector1, const XMVECTOR& xmvVector2)
		{
			XMVECTOR xmvAngle = XMVector3AngleBetweenNormals(xmvVector1, xmvVector2);
			return(XMConvertToDegrees(acosf(XMVectorGetX(xmvAngle))));
		}
		inline float Angle(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2)
		{
			return(Angle(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2)));
		}
		inline XMFLOAT3 TransformNormal(const XMFLOAT3& xmf3Vector, const XMMATRIX& xmmtxTransform)
		{
			XMFLOAT3 xmf3Result;
			XMStoreFloat3(&xmf3Result, XMVector3TransformNormal(XMLoadFloat3(&xmf3Vector),
				xmmtxTransform));
			return(xmf3Result);
		}
		inline XMFLOAT3 TransformCoord(const XMFLOAT3& xmf3Vector, const XMMATRIX& xmmtxTransform)
		{
			XMFLOAT3 xmf3Result;
			XMStoreFloat3(&xmf3Result, XMVector3TransformCoord(XMLoadFloat3(&xmf3Vector),
				xmmtxTransform));
			return(xmf3Result);
		}
		inline XMFLOAT3 TransformCoord(const XMFLOAT3& xmf3Vector, const XMFLOAT4X4& xmmtx4x4Matrix)
		{
			return(TransformCoord(xmf3Vector, XMLoadFloat4x4(&xmmtx4x4Matrix)));
		}
		inline XMFLOAT3 RandomAxis()
		{
			XMFLOAT3 result{};
			do
			{
				float x = rand() % 360;
				float y = rand() % 360;
				float z = rand() % 360;
				XMVECTOR dir;
				dir = XMVectorSet(x - 180.0f, y - 180.0f, z - 180.0f, 0.0);
				dir = XMVector3Normalize(dir);
				XMStoreFloat3(&result, dir);
			} while (!(result.x != 0.0f || result.y != 0.0f || result.z != 0.0f));
			return result;
		}

		inline XMFLOAT3 RandomDirection()
		{
			float x = rand() % 360;
			float y = rand() % 360;
			float z = rand() % 360;
			XMVECTOR dir = XMVectorSet(x - 180.0f, y - 180.0f, z - 180.0f, 0.0);
			dir = XMVector3Normalize(dir);
			XMFLOAT3 result{};
			XMStoreFloat3(&result, dir);
			return result;
		}
	}

	namespace Vector4
	{
		using namespace DirectX;
		inline XMFLOAT4 Add(const XMFLOAT4& xmf4Vector1, const XMFLOAT4& xmf4Vector2)
		{
			XMFLOAT4 xmf4Result;
			XMStoreFloat4(&xmf4Result, XMLoadFloat4(&xmf4Vector1) +
				XMLoadFloat4(&xmf4Vector2));
			return(xmf4Result);
		}
		inline XMFLOAT4 Multiply(const XMFLOAT4& xmf4Vector1, const XMFLOAT4& xmf4Vector2)
		{
			XMFLOAT4 xmf4Result;
			XMStoreFloat4(&xmf4Result, XMLoadFloat4(&xmf4Vector1) *
				XMLoadFloat4(&xmf4Vector2));
			return(xmf4Result);
		}
		inline XMFLOAT4 Multiply(float fScalar, const XMFLOAT4& xmf4Vector)
		{
			XMFLOAT4 xmf4Result;
			XMStoreFloat4(&xmf4Result, fScalar * XMLoadFloat4(&xmf4Vector));
			return(xmf4Result);
		}
		inline XMFLOAT4 Subtract( const XMFLOAT4& xmf4Vector1, const XMFLOAT4& xmf4Vector2 )
		{
			XMFLOAT4 xmf4Result;
			XMStoreFloat4( &xmf4Result, XMLoadFloat4( &xmf4Vector1 ) -
				XMLoadFloat4( &xmf4Vector2 ) );
			return(xmf4Result);
		}
		inline XMFLOAT4 Normalize(const XMFLOAT4& xmf3Vector1)
		{
			XMFLOAT4 xmf3Result;
			XMStoreFloat4(&xmf3Result, XMVector4Normalize(XMLoadFloat4(&xmf3Vector1)));
			return xmf3Result;
		}
	}

	namespace Matrix4x4
	{
		using namespace DirectX;
		inline XMFLOAT4X4 Identity()
		{
			XMFLOAT4X4 xmmtx4x4Result;
			XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixIdentity());
			return xmmtx4x4Result;
		}
		inline XMFLOAT4X4 Multiply(const XMFLOAT4X4& xmmtx4x4Matrix1, const XMFLOAT4X4& xmmtx4x4Matrix2)
		{
			XMFLOAT4X4 xmmtx4x4Result;
			XMStoreFloat4x4(&xmmtx4x4Result, XMLoadFloat4x4(&xmmtx4x4Matrix1) *
				XMLoadFloat4x4(&xmmtx4x4Matrix2));
			return(xmmtx4x4Result);
		}
		inline XMFLOAT4X4 Multiply(const XMFLOAT4X4& xmmtx4x4Matrix1, const XMMATRIX& xmmtxMatrix2)
		{
			XMFLOAT4X4 xmmtx4x4Result;
			XMStoreFloat4x4(&xmmtx4x4Result, XMLoadFloat4x4(&xmmtx4x4Matrix1) * xmmtxMatrix2);
			return(xmmtx4x4Result);
		}
		inline XMFLOAT4X4 Multiply(const XMMATRIX& xmmtxMatrix1, const XMFLOAT4X4& xmmtx4x4Matrix2)
		{
			XMFLOAT4X4 xmmtx4x4Result;
			XMStoreFloat4x4(&xmmtx4x4Result, xmmtxMatrix1 * XMLoadFloat4x4(&xmmtx4x4Matrix2));
			return(xmmtx4x4Result);
		}
		inline XMFLOAT4X4 Inverse(const XMFLOAT4X4& xmmtx4x4Matrix)
		{
			XMFLOAT4X4 xmmtx4x4Result;
			XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixInverse(NULL,
				XMLoadFloat4x4(&xmmtx4x4Matrix)));
			return(xmmtx4x4Result);
		}
		inline XMFLOAT4X4 Transpose(const XMFLOAT4X4& xmmtx4x4Matrix)
		{
			XMFLOAT4X4 xmmtx4x4Result;
			XMStoreFloat4x4(&xmmtx4x4Result,
				XMMatrixTranspose(XMLoadFloat4x4(&xmmtx4x4Matrix)));
			return(xmmtx4x4Result);
		}
		inline XMFLOAT4X4 PerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ, float FarZ)
		{
			XMFLOAT4X4 xmmtx4x4Result;
			XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio, NearZ, FarZ));
			return(xmmtx4x4Result);
		}
		inline XMFLOAT4X4 LookAtLH(const XMFLOAT3& xmf3EyePosition, const XMFLOAT3& xmf3LookAtPosition, const XMFLOAT3& xmf3UpDirection)
		{
			XMFLOAT4X4 xmmtx4x4Result;
			XMStoreFloat4x4(&xmmtx4x4Result,
				XMMatrixLookAtLH(
					XMLoadFloat3(&xmf3EyePosition),
					XMLoadFloat3(&xmf3LookAtPosition),
					XMLoadFloat3(&xmf3UpDirection)
				)
			);
			return(xmmtx4x4Result);
		}
	}

	inline DirectX::XMVECTOR XMQuaternionRotationXYZ( float x, float y, float z )
	{
		using namespace DirectX;

		auto _0 = XMQuaternionRotationAxis( XMVectorSet( 1, 0, 0, 0 ), x );
		auto _1 = XMQuaternionRotationAxis( XMVectorSet( 0, 1, 0, 0 ), y );
		auto _2 = XMQuaternionRotationAxis( XMVectorSet( 0, 0, 1, 0 ), z );

		return XMQuaternionMultiply( XMQuaternionMultiply( _0, _1 ), _2 );
	}

	namespace Quaternion
	{
		using namespace DirectX;

		inline XMFLOAT4 Identity()
		{
			XMFLOAT4 result;
			XMStoreFloat4( &result, XMQuaternionIdentity() );
			return result;
		};

		inline XMFLOAT4 FromEuler( const XMFLOAT3& euler )
		{
			XMFLOAT4 result;
			XMStoreFloat4( &result, XMQuaternionRotationRollPitchYaw( euler.x, euler.y, euler.z ) );
			return result;
		}

		inline XMFLOAT4 FromXYZEuler( const XMFLOAT3& euler )
		{
			XMFLOAT4 result;
			XMStoreFloat4( &result, KG::Math::XMQuaternionRotationXYZ( euler.x, euler.y, euler.z ) );
			return result;
		}

		inline XMFLOAT4 FromXYZEuler( float x, float y, float z )
		{
			XMFLOAT4 result;
			XMStoreFloat4( &result, KG::Math::XMQuaternionRotationXYZ( x, y, z ) );
			return result;
		}

		inline XMFLOAT4 Multiply( const XMFLOAT4& a, const XMFLOAT4& b )
		{
			XMFLOAT4 result;
			XMStoreFloat4( &result, XMQuaternionMultiply( XMLoadFloat4(&a), XMLoadFloat4( &b ) ) );
			return result;
		}

		inline XMFLOAT3 ToEuler( const XMFLOAT4& q1, bool isRadian = true )
		{
			XMFLOAT3 euler = {0,0,0};
			double test = q1.x * q1.y + q1.z * q1.w;
			if ( test > 0.499f )
			{ // singularity at north pole  
				euler.y = 2 * atan2f(q1.x, q1.w);
				euler.z = PI / 2;
				euler.x = 0;
			}
			else if ( test < -0.499f )
			{ // singularity at south pole  
				euler.y = -2 * atan2(q1.x, q1.w);
				euler.z = -PI / 2;
				euler.x = 0;
			}
			else
			{
				double sqx = q1.x * q1.x;
				double sqy = q1.y * q1.y;
				double sqz = q1.z * q1.z;
				euler.y = atan2f(2 * q1.y * q1.w - 2 * q1.x * q1.z, 1 - 2 * sqy - 2 * sqz);
				euler.z = asinf(2 * test);
				euler.x = atan2f(2 * q1.x * q1.w - 2 * q1.y * q1.z, 1 - 2 * sqx - 2 * sqz);
			}
			return euler;
		}

	};
	
	namespace Literal
	{

		inline DirectX::XMFLOAT2 operator+(const DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2& b)
		{
			return DirectX::XMFLOAT2(a.x + b.x, a.y + b.y);
		}

		inline DirectX::XMFLOAT2 operator-(const DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2& b)
		{
			return DirectX::XMFLOAT2(a.x - b.x, a.y - b.y);
		}

		inline DirectX::XMFLOAT2 operator*(const DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2& b)
		{
			return DirectX::XMFLOAT2(a.x * b.x, a.y * b.y);
		}
		inline DirectX::XMFLOAT2 operator*(float scalar, const DirectX::XMFLOAT2& a)
		{
			return DirectX::XMFLOAT2(scalar * a.x, scalar * a.y);
		}
		inline DirectX::XMFLOAT2 operator*(const DirectX::XMFLOAT2& a, float scalar)
		{
			return DirectX::XMFLOAT2(scalar * a.x, scalar * a.y);
		}



		inline DirectX::XMFLOAT3 operator+( const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b )
		{
			return Vector3::Add( a, b );
		}

		inline DirectX::XMFLOAT3 operator-( const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b )
		{
			return Vector3::Subtract( a, b );
		}

		inline DirectX::XMFLOAT3 operator*( const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b )
		{
			return Vector3::Multiply( a, b );
		}
		inline DirectX::XMFLOAT3 operator*( float scalar, const DirectX::XMFLOAT3& a )
		{
			return Vector3::Multiply( scalar, a );
		}
		inline DirectX::XMFLOAT3 operator*( const DirectX::XMFLOAT3& a, float scalar )
		{
			return Vector3::Multiply( scalar, a );
		}

		inline DirectX::XMFLOAT4 operator+( const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b )
		{
			return Vector4::Add( a, b );
		}

		inline DirectX::XMFLOAT4 operator-( const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b )
		{
			return Vector4::Subtract( a, b );
		}

		inline DirectX::XMFLOAT4 operator*( const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b )
		{
			return Vector4::Multiply( a, b );
		}
		inline DirectX::XMFLOAT4 operator*( float scalar, const DirectX::XMFLOAT4& a )
		{
			return Vector4::Multiply( scalar, a );
		}
		inline DirectX::XMFLOAT4 operator*( const DirectX::XMFLOAT4& a, float scalar )
		{
			return Vector4::Multiply( scalar, a );
		}

	};

	inline DirectX::XMFLOAT4 RandomColor()
	{
		return DirectX::XMFLOAT4(rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX), 1.0f);
	}
	inline float RandomFloat()
	{
		return rand() / float(RAND_MAX);
	}

	inline float RandomFloat(float value)
	{
		return RandomFloat() * value;
	}
	inline float RandomFloat(float min, float max)
	{
		return min + RandomFloat(max - min);
	}
	inline float RandomAbsRangeFloat(float absRange)
	{
		return RandomFloat(-absRange, absRange);
	}

	inline int RandomInt()
	{
		return rand();
	}

	inline int RandomInt(int value)
	{
		return RandomInt() % value;
	}
	inline int RandomInt(int min, int max)
	{
		return min + RandomInt(max - min);
	}
	inline int RandomAbsRangeInt(int absRange)
	{
		return RandomInt(-absRange, absRange);
	}




	inline DirectX::XMFLOAT2 RandomVector2(const DirectX::XMFLOAT2& min, const DirectX::XMFLOAT2& max)
	{
		return DirectX::XMFLOAT2(
			RandomFloat(min.x, max.x),
			RandomFloat(min.y, max.y)
		);
	}

	inline DirectX::XMFLOAT3 RandomVector3(const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max)
	{
		return DirectX::XMFLOAT3(
			RandomFloat(min.x, max.x),
			RandomFloat(min.y, max.y),
			RandomFloat(min.z, max.z)
		);
	}

	inline DirectX::XMFLOAT4 RandomVector4(const DirectX::XMFLOAT4& min, const DirectX::XMFLOAT4& max)
	{
		return DirectX::XMFLOAT4(
			RandomFloat(min.x, max.x),
			RandomFloat(min.y, max.y),
			RandomFloat(min.z, max.z),
			RandomFloat(min.w, max.w)
			);
	}


	inline DirectX::XMFLOAT4 XM_CALLCONV GetColor(const DirectX::FXMVECTOR& color)
	{
		using namespace DirectX;
		XMFLOAT4 result;
		XMStoreFloat4(&result, color);
		return result;
	}
	inline DirectX::XMVECTOR XM_CALLCONV XMVectorConvertToRadian(const DirectX::FXMVECTOR& vector)
	{
		using namespace DirectX;
		return DirectX::XMVectorScale(vector, DirectX::XM_PI / 180.0f);
	}

	inline std::ostream& operator<<(std::ostream& out, const DirectX::XMFLOAT3& position)
	{
		using namespace DirectX;
		return out << position.x << ", " << position.y << ", " << position.z;

	}
	inline std::ostream& operator<<(std::ostream& out, const DirectX::XMFLOAT4& position)
	{
		using namespace DirectX;
		return out << position.x << ", " << position.y << ", " << position.z << ", " << position.w;
	}
	
	template<typename Ty>
	inline Ty Lerp( const Ty& a, const Ty& b, float t )
	{
		using namespace Math::Literal;
		return ((1 - t) * a ) + (t * b);
	}

	template <typename Ty>
	inline bool InCycle(const Ty& value, const Ty& min, const Ty& max)
	{
		return (value <= max && value >= min);
	}

	template<typename Ty>
	inline Ty CycleValue(Ty value, const Ty& min, const Ty& max)
	{
		using namespace Math::Literal;
		while ( !InCycle(value, min, max) )
		{
			if ( value > max )
			{
				value -= max - min;
			}
			else if ( value < min )
			{
				value += max - min;
			}
		}
		return value;
	}

	template<typename Ty>
	inline Ty Clamp(Ty value, const Ty& _min, const Ty& _max)
	{
		using namespace Math::Literal;
		return (std::max)((std::min)(value, _max), _min);
	}

}

