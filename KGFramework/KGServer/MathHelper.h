#pragma once
#include <iostream>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>


inline DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& vector, float scalar)
{
	using namespace DirectX;
	auto vec = XMLoadFloat3(&vector);
	auto result = XMVectorScale(vec, scalar);
	XMFLOAT3 resultVector;
	XMStoreFloat3(&resultVector, result);
	return resultVector;
}
inline DirectX::XMFLOAT3 operator*(float scalar, const DirectX::XMFLOAT3& vector)
{
	return vector * scalar;
}

inline DirectX::XMFLOAT4 operator*(const DirectX::XMFLOAT4& vector, float scalar)
{
	using namespace DirectX;
	auto vec = XMLoadFloat4(&vector);
	auto result = XMVectorScale(vec, scalar);
	XMFLOAT4 resultVector;
	XMStoreFloat4(&resultVector, result);
	return resultVector;
}
inline DirectX::XMFLOAT4 operator*(float scalar, const DirectX::XMFLOAT4& vector)
{
	return vector * scalar;
}


namespace Math
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

	namespace Quaternion
	{
		using namespace DirectX;
		inline XMFLOAT4 Identity()
		{
			XMFLOAT4 result;
			XMStoreFloat4(&result, XMQuaternionIdentity());
			return result;
		};

		inline XMFLOAT3 ToEuler(const XMFLOAT4& quaternion)
		{
			auto quatVec = XMLoadFloat4(&quaternion);
			auto sqrtQuat = XMVectorMultiply(quatVec, quatVec);
			float unit = XMVectorGetX(XMVectorSum(sqrtQuat));
			float test = quaternion.x * quaternion.w - quaternion.y * quaternion.z;
			XMFLOAT3 v;

			if (test > 0.4995f * unit)
			{ // singularity at north pole
				v.y = 2.0f * atan2(quaternion.y, quaternion.x);
				v.x = PI / 2;
				v.z = 0;
				return v;
			}
			if (test < -0.4995f * unit)
			{ // singularity at south pole
				v.y = -2.0f * atan2(quaternion.y, quaternion.x);
				v.x = -PI / 2;
				v.z = 0;
				return v;
			}
			XMFLOAT4 q = XMFLOAT4(quaternion.w, quaternion.z, quaternion.x, quaternion.y);
			v.y = atan2(2.0f * q.x * q.w + 2.0f * q.y * q.z, 1 - 2.0f * (q.z * q.z + q.w * q.w));     // Yaw
			v.x = asin(2.0f * (q.x * q.z - q.w * q.y));                             // Pitch
			v.z = atan2(2.0f * q.x * q.y + 2.0f * q.z * q.w, 1 - 2.0f * (q.y * q.y + q.z * q.z));      // Roll
			return v;
		}
	}


	inline DirectX::XMFLOAT4 RandomColor()
	{
		return DirectX::XMFLOAT4(rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX), 1.0f);
	}
	inline float RandomFloat()
	{
		return rand() / float(RAND_MAX);
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

	inline DirectX::XMFLOAT3 GetLook(DirectX::XMFLOAT4 rotation)
	{
		auto p = DirectX::XMLoadFloat3(&Math::look);
		auto q = DirectX::XMLoadFloat4(&rotation);
		auto r = DirectX::XMVector3Rotate(p, q);
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, r);
		return result;
	}
	inline DirectX::XMFLOAT3 GetUp(DirectX::XMFLOAT4 rotation)
	{
		auto p = DirectX::XMLoadFloat3(&Math::up);
		auto q = DirectX::XMLoadFloat4(&rotation);
		auto r = DirectX::XMVector3Rotate(p, q);
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, r);
		return result;
	}

	inline DirectX::XMFLOAT3 GetRight(DirectX::XMFLOAT4 rotation)
	{
		auto p = DirectX::XMLoadFloat3(&Math::right);
		auto q = DirectX::XMLoadFloat4(&rotation);
		auto r = DirectX::XMVector3Rotate(p, q);
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, r);
		return result;
	}
}


