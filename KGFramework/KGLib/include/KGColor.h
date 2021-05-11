#pragma once
#include <DirectXMath.h>
namespace KG::Utill
{
	struct Color
	{
		union
		{
			struct
			{
				float m[4];
			};
			struct
			{
				float r;
				float g;
				float b;
				float a;
			};
		};

		Color()
		{
			this->SetByFloat(1, 1, 1, 1);
		}
		Color(float r, float g, float b, float a = 1.0f)
		{
			this->SetByFloat(r,g,b,a);
		}

		void SetByFloat(float r, float g, float b, float a = 1.0f)
		{
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		}
		void SetByRGB(int r, int g, int b, float a = 1.0f)
		{
			this->r = (float)r / 255.0f;
			this->g = (float)g / 255.0f;
			this->b = (float)b / 255.0f;
			this->a = a;
		}
		void SetByHEX(int hex, float a = 1.0f)
		{
			this->r = ((hex >> 16) & 0xFF) / 255.0; // Extract the RR byte
			this->g = ((hex >> 8) & 0xFF) / 255.0; // Extract the GG byte
			this->b = ((hex) & 0xFF) / 255.0; // Extract the BB byte
			this->a = a;
		}

        DirectX::XMFLOAT3 ToFloat3() const
        {
            return DirectX::XMFLOAT3(r, g, b);
        };
        DirectX::XMFLOAT4 ToFloat4() const
        {
            return DirectX::XMFLOAT4(r, g, b, a);
        };

		operator DirectX::XMFLOAT3()
		{
            return ToFloat3();
        }
		operator DirectX::XMFLOAT4()
		{
            return ToFloat4();
        }
	};
}