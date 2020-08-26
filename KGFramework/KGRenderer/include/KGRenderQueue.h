#pragma once
#include <d3d12.h>
#include <vector>
#include <deque>
#include <queue>
#include <set>
#include <tuple>
#include <functional>
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
		BufferPool<ObjectData>* bufferPool = nullptr;
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

		using PassJobs = std::set<KGRenderJob*, KGRenderJobOrderComparer>;
		std::deque<KGRenderJob> pool;
		std::vector<PassJobs> pass;

		using PassEnterFunction = std::function<void( ID3D12GraphicsCommandList*, ID3D12Resource*, D3D12_CPU_DESCRIPTOR_HANDLE )>;
		PassEnterFunction OnPassEnterEvent[4];
		PassEnterFunction OnRenderEndEvent;

		BufferPool<ObjectData> bufferPool; // �̰� ���Ŀ� ���� ��������� ���� �ɷ� �ٲ�� ��

		KGRenderJob& CreateRenderJob( const KGRenderJob& job );
	public:
		KGRenderEngine( ID3D12Device* device );
		KGRenderJob* GetRenderJob( Shader* shader, Geometry* geometry );
		void Render( ID3D12GraphicsCommandList* cmdList, ID3D12Resource* rt, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle );
		void ClearJobs();
		void ClearUpdateCount();
		const PassEnterFunction& GetPassEnterEventFunction( size_t pass )
		{
			return this->OnPassEnterEvent[pass];
		}

		void SetPassEnterEventFunction( size_t pass, const PassEnterFunction& function )
		{
			this->OnPassEnterEvent[pass] = function;
		}

		void SetRenderEndEventFunction( const PassEnterFunction& function )
		{
			OnRenderEndEvent = function;
		}
	};
}
