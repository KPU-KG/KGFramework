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
	this->group[job.shader->GetGroup()].emplace( &inst );
	inst.meshType = job.geometry->HasBone() ? ShaderMeshType::SkinnedMesh : ShaderMeshType::StaticMesh;
	inst.objectBufferPool = &this->bufferPool;
	inst.animationBufferPool = &this->animationBufferPool;
	inst.shadowLightBufferPool = &this->shadowLightBufferPool;
	return inst;
}

KG::Renderer::KGRenderEngine::KGRenderEngine( ID3D12Device* device )
	:bufferPool( device, BufferPool<ObjectData>::defaultFixedSize, BufferPool<ObjectData>::defaultReservedSize ),
	animationBufferPool( device, BufferPool<KG::Resource::AnimationData>::defaultFixedSize, BufferPool<KG::Resource::AnimationData>::defaultReservedSize ),
	shadowLightBufferPool( device, BufferPool<ShadowLightData>::defaultFixedSize, BufferPool<ShadowLightData>::defaultReservedSize )
{
	this->group.resize( 6 );
}

KGRenderJob* KG::Renderer::KGRenderEngine::GetRenderJob( Shader* shader, Geometry* geometry )
{
	KGRenderJob target;
	target.shader = shader;
	target.geometry = geometry;

	//정렬을 Dirty플래그 써서 한번만 하기로 되있는데
	//여기서 
	auto& renderJobs = this->group[target.shader->GetGroup()];
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

void KG::Renderer::KGRenderEngine::Render( ShaderGroup group, ShaderGeometryType geoType, ShaderPixelType pixType,
	ID3D12GraphicsCommandList* cmdList)
{
	this->Render( group, geoType, pixType, ShaderTesselation::NormalMesh, cmdList );
}

void KG::Renderer::KGRenderEngine::Render( ShaderGroup group, ShaderGeometryType geoType, ShaderPixelType pixType, ShaderTesselation tessel, ID3D12GraphicsCommandList* cmdList )
{
	for ( KGRenderJob* job : this->group[group] )
	{
		job->Render( geoType, pixType, tessel, cmdList, this->currentShader );
	}
}

void KG::Renderer::KGRenderEngine::ClearJobs()
{
	for ( auto& renderJobs : this->group )
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
}


//static 할때 처리 추가 필요 // 매 프레임 업로드할 필요가 없다.

bool KG::Renderer::KGRenderJob::CheckBufferFull() const
{
	return (this->objectBuffer == nullptr || this->objectBuffer->GetSize() < this->objectSize);
}

void KG::Renderer::KGRenderJob::GetNewBuffer()
{
	if ( shader->GetGroup() == KG::Renderer::ShaderGroup::Particle )
	{
		this->objectSize = 100000;
	}

	if ( this->objectBuffer )
		this->objectBuffer->isUsing = false;
	this->objectBuffer = this->objectBufferPool->GetNewBuffer( this->objectSize );

	if ( meshType == ShaderMeshType::SkinnedMesh )
	{
		if ( this->animationBuffer )
			this->animationBuffer->isUsing = false;
		this->animationBuffer = this->animationBufferPool->GetNewBuffer( this->objectSize );
	}

	if ( shader->GetGroup() == KG::Renderer::ShaderGroup::AmbientLight ||
		shader->GetGroup() == KG::Renderer::ShaderGroup::MeshVolumeLight ||
		shader->GetGroup() == KG::Renderer::ShaderGroup::DirectionalLight || 
		shader->GetGroup() == KG::Renderer::ShaderGroup::Transparent )
	{
		if ( this->shadowLightBuffer )
			this->shadowLightBuffer->isUsing = false;
		this->shadowLightBuffer = this->shadowLightBufferPool->GetNewBuffer( this->objectSize );
	}
}

void KG::Renderer::KGRenderJob::OnObjectAdd( bool isVisible )
{
	this->objectSize += 1;
	if ( isVisible ) OnVisibleAdd();
	if ( shader->GetGroup() == KG::Renderer::ShaderGroup::Particle )
	{
		this->objectSize = 100000;
		this->visibleSize = 100000;
	}
}

void KG::Renderer::KGRenderJob::OnObjectRemove(bool isVisible)
{
	this->objectSize -= 1;
	if ( isVisible ) OnVisibleRemove();
	if ( shader->GetGroup() == KG::Renderer::ShaderGroup::Particle )
	{
		this->objectSize = 100000;
		this->visibleSize = 100000;
	}
}

void KG::Renderer::KGRenderJob::SetVisibleSize(int count)
{
	this->visibleSize = count;
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

void KG::Renderer::KGRenderJob::Render( ShaderGeometryType geoType, ShaderPixelType pixType, ID3D12GraphicsCommandList* cmdList, Shader*& prevShader )
{
	this->Render( geoType, pixType, ShaderTesselation::NormalMesh, cmdList, prevShader );
}

void KG::Renderer::KGRenderJob::Render( ShaderGeometryType geoType, ShaderPixelType pixType, ShaderTesselation tessel, ID3D12GraphicsCommandList* cmdList, Shader*& prevShader )
{
	if ( this->visibleSize == 0 ) return;
	if ( prevShader != this->shader )
	{
		prevShader = this->shader;
	}
	this->shader->Set( cmdList, this->meshType, pixType, geoType, tessel );
	auto addr = this->objectBuffer->buffer.resource->GetGPUVirtualAddress();
	cmdList->SetGraphicsRootShaderResourceView( RootParameterIndex::InstanceData, addr );

	if ( this->meshType == ShaderMeshType::SkinnedMesh )
	{
		auto animAddr = this->animationBuffer->buffer.resource->GetGPUVirtualAddress();
		cmdList->SetGraphicsRootShaderResourceView( RootParameterIndex::AnimationTransformData, animAddr );
	}

	if ( shader->GetGroup() == KG::Renderer::ShaderGroup::AmbientLight ||
		shader->GetGroup() == KG::Renderer::ShaderGroup::MeshVolumeLight ||
		shader->GetGroup() == KG::Renderer::ShaderGroup::DirectionalLight ||
		shader->GetGroup() == KG::Renderer::ShaderGroup::Transparent )
	{
		auto shadowAddr = this->shadowLightBuffer->buffer.resource->GetGPUVirtualAddress();
		cmdList->SetGraphicsRootShaderResourceView( RootParameterIndex::LightData, shadowAddr );
	}

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

