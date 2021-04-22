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
		KG::Renderer::ShaderMeshType meshType;
		BufferPool<ObjectData>* objectBufferPool = nullptr;
		PooledBuffer<ObjectData>* objectBuffer = nullptr;

		BufferPool<KG::Resource::AnimationData>* animationBufferPool = nullptr;
		PooledBuffer<KG::Resource::AnimationData>* animationBuffer = nullptr;
		
		BufferPool<ShadowLightData>* shadowLightBufferPool = nullptr;
		PooledBuffer<ShadowLightData>* shadowLightBuffer = nullptr;


		bool CheckBufferFull() const;
		void GetNewBuffer();

		//static �Ҷ� ó�� �߰� �ʿ� // �� ������ ���ε��� �ʿ䰡 ����.
		void OnObjectAdd( bool isVisible );
		void OnObjectRemove( bool isVisible );
		void OnVisibleAdd();
		void OnVisibleRemove();
		int GetUpdateCount();
		void ClearCount();

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
		BufferPool<ObjectData> bufferPool; // �̰� ���Ŀ� ���� ��������� ���� �ɷ� �ٲ�� ��
		BufferPool<KG::Resource::AnimationData> animationBufferPool; // �̰� ���Ŀ� ���� ��������� ���� �ɷ� �ٲ�� ��
		BufferPool<ShadowLightData> shadowLightBufferPool; // �̰� ���Ŀ� ���� ��������� ���� �ɷ� �ٲ�� ��

		KGRenderJob& CreateRenderJob( const KGRenderJob& job );
	public:
		KGRenderEngine( ID3D12Device* device );
		KGRenderJob* GetRenderJob( Shader* shader, Geometry* geometry );
		void Render( ShaderGroup group, ShaderGeometryType geoType, ShaderPixelType pixType, ID3D12GraphicsCommandList* cmdList );
		void Render( ShaderGroup group, ShaderGeometryType geoType, ShaderPixelType pixType, ShaderTesselation tessel, ID3D12GraphicsCommandList* cmdList );
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
