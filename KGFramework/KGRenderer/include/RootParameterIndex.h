#pragma once
namespace KG::Renderer
{
	struct RootParameterIndex
	{
		inline static constexpr size_t InstanceData = 0;
		inline static constexpr size_t AnimationTransformData = 1;
		inline static constexpr size_t BoneOffsetData = 2;
		inline static constexpr size_t MaterialData = 3;
		inline static constexpr size_t CameraData = 4;
		inline static constexpr size_t PassData = 5;
		inline static constexpr size_t Texture1Heap = 6;
		inline static constexpr size_t Texture2Heap = 7;
		inline static constexpr size_t GBuffer0 = 8;
		inline static constexpr size_t GBuffer1 = 9;
		inline static constexpr size_t GBuffer2 = 10;
		inline static constexpr size_t GBuffer3 = 11;
		inline static constexpr size_t GBuffer4 = 12;
		inline static constexpr size_t DepthStencilBuffer = 12;
		inline static constexpr size_t GBufferHeap = 8;

	};
}