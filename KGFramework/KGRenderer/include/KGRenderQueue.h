#pragma once
#include <d3d12.h>
#include <vector>
#include <tuple>
#include "KGGraphicBuffer.h"
namespace KG::Renderer
{
	class Shader;
	class Geometry;

	struct KGRenderJob
	{
		Shader* shader;
		Geometry* geometry;
		int objectSize = 0;
		int visibleSize = 0;
		int updateCount = 0;
		BufferPool<ObjectData>* bufferPool;
		PooledBuffer<ObjectData>* objectBuffer = nullptr;
		
		bool CheckBufferFull() const;
		void GetNewBuffer();

		//static 할때 처리 추가 필요 // 매 프레임 업로드할 필요가 없다.
		void OnObjectAdd( bool isVisible );
		void OnObjectRemove( bool isVisible );
		void OnVisibleAdd();
		void OnVisibleRemove();
		int GetUpdateCount();
		void ClearCount();

		void Render( ID3D12GraphicsCommandList* cmdList, Shader*& prevShader );
		static bool ShaderCompare( const KGRenderJob& a, const KGRenderJob& b );
		static bool GeometryCompare( const KGRenderJob& a, const KGRenderJob& b );
		static bool OrderCompare( const KGRenderJob& a, const KGRenderJob& b );
	};

	class KGRenderEngine
	{
		Shader* currentShader = nullptr;
		Geometry* currentGeometry = nullptr;
		bool renderJobsDirty = false;
		std::vector<KGRenderJob> renderJobs; // 일단 제거 알고리즘은 없음 / 한번 생성된 조합이면 0이 됬더라도 다시 생길 가능성이 있다고 생각
		BufferPool<ObjectData> bufferPool; // 이거 추후에 버디 얼로케이터 같은 걸로 바꿔야 함

		KGRenderJob& CreateRenderJob( const KGRenderJob& job );
	public:
		KGRenderEngine( ID3D12Device* device );
		KGRenderJob* GetRenderJob( Shader* shader, Geometry* geometry );
		void Render( ID3D12GraphicsCommandList* cmdList );
		void ClearJobs();
		void SortJobs();
		void ClearUpdateCount();
	};
}
