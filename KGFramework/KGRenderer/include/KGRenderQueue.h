#pragma once
#include <map>
#include <vector>
#include <list>
#include <tuple>
#include <algorithm>
#include "ResourceContainer.h"
#include "KGShader.h"
#include "KGGeometry.h"
#include "ShaderData.h"
#include "KGGraphicBuffer.h"
namespace KG::Renderer
{
	struct KGRenderJob
	{
		Shader* shader;
		Geometry* geometry;
		int objectSize = 0;
		int visibleSize = 0;
		int updateCount = 0;
		BufferPool<ObjectData>* bufferPool;
		PooledBuffer<ObjectData>* objectBuffer = nullptr;
		
		bool CheckBufferFull() const
		{
			return this->objectBuffer == nullptr ||  this->objectBuffer->GetSize() <= this->objectSize;
		}

		void GetNewBuffer()
		{
			if( this->objectBuffer )
				this->objectBuffer->isUsing = false;
			this->objectBuffer = this->bufferPool->GetNewBuffer( this->objectSize );
		}

		//static 할때 처리 추가 필요 // 매 프레임 업로드할 필요가 없다.
		void OnObjectAdd( bool isVisible )
		{
			this->objectSize += 1;

			if ( CheckBufferFull() ) GetNewBuffer();
			if ( isVisible ) OnVisibleAdd();
		}
		void OnObjectRemove( bool isVisible )
		{
			this->objectSize -= 1;
			if ( isVisible ) OnVisibleRemove();
		}
		void OnVisibleAdd()
		{
			this->visibleSize += 1;
		}
		void OnVisibleRemove()
		{
			this->visibleSize -= 1;
		}
		int GetUpdateCount()
		{
			auto result = this->updateCount;
			this->updateCount++;
			return result;
		}
		void ClearCount()
		{
			this->updateCount = 0;
		}

		void Render( ID3D12GraphicsCommandList* cmdList, Shader*& prevShader )
		{
			if ( this->visibleSize == 0 ) return;
			if ( prevShader != this->shader )
			{
				prevShader = this->shader;
			}
			this->shader->Set( cmdList );
			auto addr = this->objectBuffer->buffer.resource->GetGPUVirtualAddress();
			cmdList->SetGraphicsRootShaderResourceView( 2, addr );
			this->geometry->Render( cmdList, this->visibleSize );
		}

		static bool ShaderCompare( const KGRenderJob& a, const KGRenderJob& b )
		{
			return a.shader < b.shader;
		}

		static bool GeometryCompare( const KGRenderJob& a, const KGRenderJob& b )
		{
			return a.geometry < b.geometry;
		}
		static bool OrderCompare( const KGRenderJob& a, const KGRenderJob& b)
		{
			return std::make_tuple(a.shader->GetShaderType(), a.shader->GetRenderPriority(), (int)a.shader ) <
				std::make_tuple(b.shader->GetShaderType(), b.shader->GetRenderPriority(), (int)b.shader );
		}
	};

	class KGRenderEngine
	{
		Shader* currentShader = nullptr;
		Geometry* currentGeometry = nullptr;
		bool renderJobsDirty = false;
		std::vector<KGRenderJob> renderJobs; // 일단 제거 알고리즘은 없음 / 한번 생성된 조합이면 0이 됬더라도 다시 생길 가능성이 있다고 생각
		BufferPool<ObjectData> bufferPool; // 이거 추후에 버디 얼로케이터 같은 걸로 바꿔야 함
		KGRenderJob& CreateRenderJob( const KGRenderJob& job )
		{
			this->renderJobsDirty = true;
			auto& inst = this->renderJobs.emplace_back( job );
			inst.bufferPool = &this->bufferPool;
			return inst;
		}
	public:
		KGRenderEngine(ID3D12Device* device)
			:bufferPool(device, BufferPool<ObjectData>::defaultFixedSize, BufferPool<ObjectData>::defaultReservedSize)
		{
		}
		KGRenderJob* GetRenderJob( Shader* shader, Geometry* geometry )
		{
			KGRenderJob target;
			target.shader = shader;
			target.geometry = geometry;

			auto [start, end] = std::equal_range( renderJobs.begin(), renderJobs.end(), target, KGRenderJob::OrderCompare );
			auto result = std::find_if( start, end, [=]( const KGRenderJob& job ) { return job.geometry == geometry; } );

			KGRenderJob* resultJob = result != end ? &*result : &CreateRenderJob( target );
			return resultJob;
		}

		void Render( ID3D12GraphicsCommandList* cmdList )
		{
			for ( KGRenderJob& job : this->renderJobs )
			{
				job.Render( cmdList, this->currentShader );
			}
		}

		void ClearJobs()
		{
			this->renderJobs.clear();
		}

		void SortJobs()
		{
			std::sort( renderJobs.begin(), renderJobs.end(), KGRenderJob::OrderCompare );
		}
	};
}
