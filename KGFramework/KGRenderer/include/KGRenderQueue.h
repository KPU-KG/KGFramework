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

		//static �Ҷ� ó�� �߰� �ʿ� // �� ������ ���ε��� �ʿ䰡 ����.
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
		std::vector<KGRenderJob> renderJobs; // �ϴ� ���� �˰����� ���� / �ѹ� ������ �����̸� 0�� ����� �ٽ� ���� ���ɼ��� �ִٰ� ����
		BufferPool<ObjectData> bufferPool; // �̰� ���Ŀ� ���� ��������� ���� �ɷ� �ٲ�� ��

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
