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
	this->renderJobsDirty = true;
	auto& inst = this->renderJobs.emplace_back( job );
	inst.bufferPool = &this->bufferPool;
	return inst;
}

KG::Renderer::KGRenderEngine::KGRenderEngine( ID3D12Device* device )
	:bufferPool( device, BufferPool<ObjectData>::defaultFixedSize, BufferPool<ObjectData>::defaultReservedSize )
{
}

KGRenderJob* KG::Renderer::KGRenderEngine::GetRenderJob( Shader* shader, Geometry* geometry )
{
	KGRenderJob target;
	target.shader = shader;
	target.geometry = geometry;

	auto [start, end] = std::equal_range( renderJobs.begin(), renderJobs.end(), target, KGRenderJob::OrderCompare );
	auto result = std::find_if( start, end, [=]( const KGRenderJob& job ) { return job.geometry == geometry; } );

	KGRenderJob* resultJob = result != end ? &*result : &CreateRenderJob( target );
	return resultJob;
}

void KG::Renderer::KGRenderEngine::Render( ID3D12GraphicsCommandList* cmdList )
{
	for ( KGRenderJob& job : this->renderJobs )
	{
		job.Render( cmdList, this->currentShader );
	}
}

void KG::Renderer::KGRenderEngine::ClearJobs()
{
	this->renderJobs.clear();
}

void KG::Renderer::KGRenderEngine::SortJobs()
{
	std::sort( renderJobs.begin(), renderJobs.end(), KGRenderJob::OrderCompare );
}

void KG::Renderer::KGRenderEngine::ClearUpdateCount()
{
	for ( KGRenderJob& job : this->renderJobs )
	{
		job.ClearCount();
	}
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

	if ( CheckBufferFull() ) GetNewBuffer();
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
	return std::make_tuple( a.shader->GetShaderType(), a.shader->GetRenderPriority(), (int)a.shader ) <
		std::make_tuple( b.shader->GetShaderType(), b.shader->GetRenderPriority(), (int)b.shader );
}
