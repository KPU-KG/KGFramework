#include "pch.h"
#include "KGRenderQueue.h"
#include <vector>
#include <algorithm>
#include <tuple>
#include "KGGeometry.h"
#include "KGShader.h"
#include "RootParameterIndex.h"
using namespace KG::Renderer;
// 이거 추후에 버디 얼로케이터 같은 걸로 바꿔야 함

KGRenderJob& KG::Renderer::KGRenderEngine::CreateRenderJob( const KGRenderJob& job )
{
	auto& inst = this->pool.emplace_back( job );
	this->pass[job.shader->GetShaderType()].emplace( &inst );
	inst.bufferPool = &this->bufferPool;
	return inst;
}

KG::Renderer::KGRenderEngine::KGRenderEngine( ID3D12Device* device )
	:bufferPool( device, BufferPool<ObjectData>::defaultFixedSize, BufferPool<ObjectData>::defaultReservedSize )
{
	this->pass.resize( 4 );
}

KGRenderJob* KG::Renderer::KGRenderEngine::GetRenderJob( Shader* shader, Geometry* geometry )
{
	KGRenderJob target;
	target.shader = shader;
	target.geometry = geometry;

	//정렬을 Dirty플래그 써서 한번만 하기로 되있는데
	//여기서 
	auto& renderJobs = this->pass[target.shader->GetShaderType()];
	auto result = renderJobs.find( &target );
	//auto result = std::find_if( start, end, [=]( const KGRenderJob& job ) { return job.geometry == geometry; } );

	KGRenderJob* resultJob = nullptr;
	if ( result == renderJobs.end() )
	{
		resultJob = &CreateRenderJob( target );
	}
	else
	{
		resultJob = *result;
	}

	//KGRenderJob* resultJob = result != end ? &*result : &CreateRenderJob( target );
	return resultJob;
}

void KG::Renderer::KGRenderEngine::Render( ID3D12GraphicsCommandList* cmdList, RenderTexture& renderTexture )
{
	for ( size_t i = 0; i < this->pass.size(); ++i )
	{
		if ( this->OnPassEnterEvent[i] )
			this->OnPassEnterEvent[i]( cmdList, renderTexture );

		for ( KGRenderJob* job : pass[i] )
		{
			if ( this->OnPassPreRenderEvent[i] )
				this->OnPassPreRenderEvent[i]( cmdList, renderTexture );

			job->Render( cmdList, this->currentShader );

			if ( this->OnPassEndRenderEvent[i] )
				this->OnPassEndRenderEvent[i]( cmdList, renderTexture );
		}
	}
	this->OnPassEndEvent( cmdList, renderTexture );
}

void KG::Renderer::KGRenderEngine::ClearJobs()
{
	for ( auto& renderJobs : this->pass )
	{
		renderJobs.clear();
	}
}

void KG::Renderer::KGRenderEngine::ClearUpdateCount()
{
	for ( KGRenderJob& job : this->pool )
	{
		job.ClearCount();
	}
	//for ( auto& renderJobs : pass )
	//{
	//	for ( KGRenderJob* job : renderJobs )
	//	{
	//		job.ClearCount();
	//	}
	//}
}


//static 할때 처리 추가 필요 // 매 프레임 업로드할 필요가 없다.

bool KG::Renderer::KGRenderJob::CheckBufferFull() const
{
	return this->objectBuffer == nullptr || this->objectBuffer->GetSize() <= this->objectSize;
}

void KG::Renderer::KGRenderJob::GetNewBuffer()
{
	if ( this->objectBuffer )
		this->objectBuffer->isUsing = false;
	this->objectBuffer = this->bufferPool->GetNewBuffer( this->objectSize );
}

void KG::Renderer::KGRenderJob::OnObjectAdd( bool isVisible )
{
	this->objectSize += 1;

	if ( isVisible ) OnVisibleAdd();
}

void KG::Renderer::KGRenderJob::OnObjectRemove( bool isVisible )
{
	this->objectSize -= 1;
	if ( isVisible ) OnVisibleRemove();
}

void KG::Renderer::KGRenderJob::OnVisibleAdd()
{
	this->visibleSize += 1;
}

void KG::Renderer::KGRenderJob::OnVisibleRemove()
{
	this->visibleSize -= 1;
}

int KG::Renderer::KGRenderJob::GetUpdateCount()
{
	if ( CheckBufferFull() ) GetNewBuffer();
	auto result = this->updateCount;
	this->updateCount++;
	return result;
}

void KG::Renderer::KGRenderJob::ClearCount()
{
	this->updateCount = 0;
}

void KG::Renderer::KGRenderJob::Render( ID3D12GraphicsCommandList* cmdList, Shader*& prevShader )
{
	if ( this->visibleSize == 0 ) return;
	if ( prevShader != this->shader )
	{
		prevShader = this->shader;
	}
	this->shader->Set( cmdList );
	auto addr = this->objectBuffer->buffer.resource->GetGPUVirtualAddress();
	cmdList->SetGraphicsRootShaderResourceView( RootParameterIndex::InstanceData, addr );
	this->geometry->Render( cmdList, this->visibleSize );
}

bool KG::Renderer::KGRenderJob::ShaderCompare( const KGRenderJob& a, const KGRenderJob& b )
{
	return a.shader < b.shader;
}

bool KG::Renderer::KGRenderJob::GeometryCompare( const KGRenderJob& a, const KGRenderJob& b )
{
	return a.geometry < b.geometry;
}

bool KG::Renderer::KGRenderJob::OrderCompare( const KGRenderJob& a, const KGRenderJob& b )
{
	return std::make_tuple( a.shader->GetRenderPriority(), (int)a.shader, (int)a.geometry ) <
		std::make_tuple( b.shader->GetRenderPriority(), (int)b.shader, (int)b.geometry );
}

