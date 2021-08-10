#pragma once
namespace KG::Renderer
{
	struct GraphicRootParameterIndex
	{
		inline static constexpr size_t InstanceData = 0;
		inline static constexpr size_t AnimationTransformData = 1;
		inline static constexpr size_t BoneOffsetData = 2;
		inline static constexpr size_t MaterialData = 3;
		inline static constexpr size_t LightData = 4;
		inline static constexpr size_t CameraData = 5;
		inline static constexpr size_t Texture = 6;
        inline static constexpr size_t TextureArray = 7;
        inline static constexpr size_t TextureCube = 8;
        inline static constexpr size_t GBufferHeap = 9;
	};

    struct ComputeRootParameterIndex
    {
        inline static constexpr size_t Result = 0;
        inline static constexpr size_t PrevResult = 1;
        inline static constexpr size_t Source = 2;
        inline static constexpr size_t GBufferStart = 3;
        inline static constexpr size_t UAVBuffers = 4;
        inline static constexpr size_t SRVBuffers = 5;
        inline static constexpr size_t MaterialData = 6;
        inline static constexpr size_t MaterialIndex = 7;
        inline static constexpr size_t CameraData = 8;
        inline static constexpr size_t FrameData = 9;
        inline static constexpr size_t Texture1Heap = 10;
        inline static constexpr size_t Texture2Heap = 11;
    };
}