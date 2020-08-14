#pragma once
namespace KG::Renderer
{
	struct RootParameterIndex
	{
		inline static constexpr size_t InstanceData = 0;
		inline static constexpr size_t MaterialData = 1;
		inline static constexpr size_t CameraData = 2;
		inline static constexpr size_t PassData = 3;
		inline static constexpr size_t Texture1Heap = 4;
		inline static constexpr size_t Texture2Heap = 5;
		inline static constexpr size_t GBuffer0 = 6;
		inline static constexpr size_t GBuffer1 = 7;
		inline static constexpr size_t GBuffer2 = 8;
		inline static constexpr size_t GBuffer3 = 9;
		inline static constexpr size_t GBuffer4 = 10;
		inline static constexpr size_t DepthStencilBuffer = 10;
	};
}