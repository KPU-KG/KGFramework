#pragma once
#include <d3d12.h>
#include <vector>
#include <deque>
#include <queue>
#include <set>
#include <tuple>
#include <functional>
#include "KGGraphicBuffer.h"
#include "RenderTexture.h"
#include "BoneData.h"
#include "KGShader.h"

#include "LightComponent.h"

namespace KG::Renderer
{
	class Geometry;

	struct KGRenderJob
	{
		Shader* shader;
		Geometry* geometry;
		int objectSize = 0;
		int visibleSize = 0;
		int updateCount = 0;
        int culledMax = 0;
        int notCullIndexStart = 0;
        bool isCulling = false;
		KG::Renderer::ShaderMeshType meshType;
        std::vector<XMFLOAT3X4> matrixs;
		BufferPool<ObjectData>* objectBufferPool = nullptr;
		PooledBuffer<ObjectData>* objectBuffer = nullptr;

		BufferPool<KG::Resource::AnimationData>* animationBufferPool = nullptr;
		PooledBuffer<KG::Resource::AnimationData>* animationBuffer = nullptr;
		
		BufferPool<ShadowLightData>* shadowLightBufferPool = nullptr;
		PooledBuffer<ShadowLightData>* shadowLightBuffer = nullptr;

		bool CheckBufferFull() const;
		void GetNewBuffer();

		//static 할때 처리 추가 필요 // 매 프레임 업로드할 필요가 없다.
		void OnObjectAdd( bool isVisible );
		void OnObjectRemove( bool isVisible );
		void SetVisibleSize(int count);
		void OnVisibleAdd();
		void OnVisibleRemove();
        void AddCullObject();
		void SetObjectSize(int count);
        void SetUpdateCount(int count, bool isCulled = false);
        int GetUpdateCount(bool isCulled = false);
		void ClearCount();

        void TurnOnCulledRenderOnce();

        D3D12_GPU_VIRTUAL_ADDRESS GetVertexBufferGPUAddress() const;
        D3D12_GPU_VIRTUAL_ADDRESS GetIndexBufferGPUAddress() const;
        D3D12_GPU_VIRTUAL_ADDRESS GetObjectBufferGPUAddress() const;
        D3D12_GPU_VIRTUAL_ADDRESS GetMaterialBufferGPUAddress() const;

		void Render( ShaderGeometryType geoType, ShaderPixelType pixType, ID3D12GraphicsCommandList* cmdList, Shader*& prevShader );
		void Render( ShaderGeometryType geoType, ShaderPixelType pixType, ShaderTesselation tessel, ID3D12GraphicsCommandList* cmdList, Shader*& prevShader );

		static bool ShaderCompare( const KGRenderJob& a, const KGRenderJob& b );
		static bool GeometryCompare( const KGRenderJob& a, const KGRenderJob& b );
		static bool OrderCompare( const KGRenderJob& a, const KGRenderJob& b );
	};

	struct KGRenderJobOrderComparer
	{
		bool operator()( const KGRenderJob* a, const KGRenderJob* b ) const
		{
			return KGRenderJob::OrderCompare( *a, *b );
		}
	};

	class KGRenderEngine
	{
		Shader* currentShader = nullptr;
		Geometry* currentGeometry = nullptr;

		using GroupJobs = std::set<KGRenderJob*, KGRenderJobOrderComparer>;
		std::deque<KGRenderJob> pool;
		std::vector<GroupJobs> group;
		BufferPool<ObjectData> bufferPool; // 이거 추후에 버디 얼로케이터 같은 걸로 바꿔야 함
		BufferPool<KG::Resource::AnimationData> animationBufferPool; // 이거 추후에 버디 얼로케이터 같은 걸로 바꿔야 함
		BufferPool<ShadowLightData> shadowLightBufferPool; // 이거 추후에 버디 얼로케이터 같은 걸로 바꿔야 함

		KGRenderJob& CreateRenderJob( const KGRenderJob& job );
	public:
		KGRenderEngine( ID3D12Device* device );
		KGRenderJob* GetRenderJob( Shader* shader, Geometry* geometry );
		void Render( ShaderGroup group, ShaderGeometryType geoType, ShaderPixelType pixType, ID3D12GraphicsCommandList* cmdList, bool culled = false);
		void Render( ShaderGroup group, ShaderGeometryType geoType, ShaderPixelType pixType, ShaderTesselation tessel, ID3D12GraphicsCommandList* cmdList, bool culled = false);
		void ClearJobs();
		void ClearUpdateCount();
		bool hasRenderJobs() const
		{
			for ( auto& i : this->group )
			{
				if ( i.size() )
					return true;
			}
			return false;
		}
	};
}
