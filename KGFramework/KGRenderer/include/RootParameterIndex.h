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

    struct DXRRootParameterIndex
    {
        inline static constexpr size_t InstanceIndexData = 0;
        inline static constexpr size_t LightData = 1;
        inline static constexpr size_t CameraData = 2;
        inline static constexpr size_t RWTexture = 3;
        inline static constexpr size_t AccelerationStructure = 4;
        inline static constexpr size_t Texture = 5;
        inline static constexpr size_t TextureArray = 6;
        inline static constexpr size_t TextureCube = 7;
        inline static constexpr size_t Ambient = 8;
        inline static constexpr size_t GBufferHeap = 9;
        inline static constexpr size_t RWHeaps = 10;
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

    struct AnimationRootParameterIndex
    {
        inline static constexpr size_t Result = 0;
        inline static constexpr size_t Source = 1;
        inline static constexpr size_t Bone = 2;
        inline static constexpr size_t Animation = 3;
		inline static constexpr size_t InstanceID = 4;
		inline static constexpr size_t World = 5;
    };
}