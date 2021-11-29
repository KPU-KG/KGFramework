#include "pch.h"
#include "KGRenderQueue.h"
#include <vector>
#include <algorithm>
#include <tuple>
#include "KGGeometry.h"
#include "KGShader.h"
#include "RootParameterIndex.h"
#include "KGDXRenderer.h"
#include "PostProcess.h"
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
	this->group.resize(ShaderGroup::MaxShaderCount);
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
        KGDXRenderer::GetInstance()->AddNewRenderJob(resultJob);
	}
	else
	{
		resultJob = *result;
	}
	if ( shader->GetGroup() == KG::Renderer::ShaderGroup::ParticleAdd
		|| shader->GetGroup() == KG::Renderer::ShaderGroup::ParticleTransparent )
	{
		resultJob->objectSize = 10000;
	}
	//KGRenderJob* resultJob = result != end ? &*result : &CreateRenderJob( target );
	return resultJob;
}

void KG::Renderer::KGRenderEngine::Render( ShaderGroup group, ShaderGeometryType geoType, ShaderPixelType pixType, ID3D12GraphicsCommandList* cmdList, bool culled)
{
	this->Render( group, geoType, pixType, ShaderTesselation::NormalMesh, cmdList, culled );
}

void KG::Renderer::KGRenderEngine::Render( ShaderGroup group, ShaderGeometryType geoType, ShaderPixelType pixType, ShaderTesselation tessel, ID3D12GraphicsCommandList* cmdList, bool culled)
{
	for ( KGRenderJob* job : this->group[group] )
	{
        if (culled) job->TurnOnCulledRenderOnce();
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
	if ( shader->GetGroup() == KG::Renderer::ShaderGroup::ParticleAdd
		|| shader->GetGroup() == KG::Renderer::ShaderGroup::ParticleTransparent )
	{
		this->objectSize = 10000;
	}

	if ( this->objectBuffer )
		this->objectBuffer->isUsing = false;
	this->objectBuffer = this->objectBufferPool->GetNewBuffer( this->objectSize );
    this->matrixs.resize(this->objectBuffer->GetSize());

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
	if ( shader->GetGroup() == KG::Renderer::ShaderGroup::ParticleAdd
		|| shader->GetGroup() == KG::Renderer::ShaderGroup::ParticleTransparent )
	{
		this->objectSize = 10000;
		this->visibleSize = 10000;
	}
}

void KG::Renderer::KGRenderJob::OnObjectRemove(bool isVisible)
{
	this->objectSize -= 1;
	if ( isVisible ) OnVisibleRemove();
	if ( shader->GetGroup() == KG::Renderer::ShaderGroup::ParticleAdd
		|| shader->GetGroup() == KG::Renderer::ShaderGroup::ParticleTransparent )
	{
		this->objectSize = 10000;
		this->visibleSize = 10000;
	}
}

void KG::Renderer::KGRenderJob::SetVisibleSize(int count)
{
	this->visibleSize = count;
}

void KG::Renderer::KGRenderJob::SetAnimationCount(int count)
{
	this->animationCount = count;
}

void KG::Renderer::KGRenderJob::OnVisibleAdd()
{
	this->visibleSize += 1;
}

void KG::Renderer::KGRenderJob::OnVisibleRemove()
{
	this->visibleSize -= 1;
}

void KG::Renderer::KGRenderJob::AddCullObject()
{
    this->culledMax += 1;
    this->notCullIndexStart = culledMax;
}

void KG::Renderer::KGRenderJob::SetObjectSize(int count)
{
	this->objectSize = count;
}

void KG::Renderer::KGRenderJob::SetUpdateCount(int count, bool isCulled)
{
    if (CheckBufferFull()) GetNewBuffer();
    this->updateCount = count;
    if (!isCulled)
    {
        this->notCullIndexStart = this->updateCount;
    }
}

int KG::Renderer::KGRenderJob::GetUpdateCount(bool isCulled)
{
	if ( CheckBufferFull() ) GetNewBuffer();
    int result = 0;
    if (isCulled)
    {
        result = this->updateCount;
        this->updateCount++;
    }
    else 
    {
        result = this->notCullIndexStart;
        this->notCullIndexStart++;
    }
	return result;
}


void KG::Renderer::KGRenderJob::ClearCount()
{
	this->updateCount = 0;
    this->culledMax = 0;
    this->notCullIndexStart = 0;
}

void KG::Renderer::KGRenderJob::TurnOnCulledRenderOnce()
{
    this->isCulling = true;
}

void KG::Renderer::KGRenderJob::BuildAnimatedBLAS(ID3D12Device5* device, ID3D12GraphicsCommandList4* commandList)
{
	auto updateCount = this->animationCount;
	if (updateCount <= 0) return;
    auto [vCount, iCount] = this->geometry->GetCounts();
    auto [preResultSize, preScratchSize] = this->geometry->GetPrebuildInfo();

    while(this->animatedVertex.size() < updateCount)
        this->animatedVertex.push_back(::CreateBufferResource(device, commandList, vCount * sizeof(NormalVertex)));

    while (this->blasResults.size() < updateCount)
        this->blasResults.push_back(::CreateASBufferResource(device, commandList, preResultSize, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE));

    while (this->blasScratchs.size() < updateCount)
        this->blasScratchs.push_back(::CreateASBufferResource(device, commandList, preScratchSize, D3D12_RESOURCE_STATE_COMMON));


	for (size_t i = 0; i < updateCount; i++)
	{
		//XMMatrixTranspose
		auto mat = XMMatrixTranspose(XMMatrixInverse(0, XMLoadFloat3x4(&this->matrixs[i])));
		XMFLOAT4X4 world;
		XMStoreFloat4x4(&world, mat);
		world._44 = 1;
		KGDXRenderer::GetInstance()->GetPostProcess()->CalculateAnimation(
			commandList,
			this->animatedVertex[i]->GetGPUVirtualAddress(),
			this->geometry->GetVertexBufferGPUAddress(),
			this->geometry->GetBoneOffsetGPUAddress(),
			this->animationBuffer->buffer.resource->GetGPUVirtualAddress(),
			vCount,
			i,
			world
		);
	}

     // 후처리로 스키닝 연산

    for (size_t i = 0; i < updateCount; i++)
    {
        D3D12_RAYTRACING_GEOMETRY_DESC rtgDesc;
        ZeroDesc(rtgDesc);
        rtgDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE::D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        rtgDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAGS::D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
        //Vertex
        rtgDesc.Triangles.VertexBuffer.StartAddress = this->animatedVertex[i]->GetGPUVirtualAddress();
        rtgDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(NormalVertex);
        rtgDesc.Triangles.VertexCount = vCount;
        rtgDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        //Index
        rtgDesc.Triangles.IndexBuffer = this->geometry->GetIndexBufferGPUAddress();
        rtgDesc.Triangles.IndexCount = iCount;
        rtgDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
        //Transform
        rtgDesc.Triangles.Transform3x4 = 0;

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs;
        ZeroDesc(inputs);
        inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
        inputs.NumDescs = 1;
        inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        inputs.pGeometryDescs = &rtgDesc;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild;
        device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild);

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc;
        ZeroDesc(buildDesc);
        buildDesc.Inputs = inputs;
        buildDesc.DestAccelerationStructureData = this->blasResults[i]->GetGPUVirtualAddress();
        buildDesc.ScratchAccelerationStructureData = this->blasScratchs[i]->GetGPUVirtualAddress();
        {
            D3D12_RESOURCE_BARRIER barrier[2] = {
                CD3DX12_RESOURCE_BARRIER::Transition(this->animatedVertex[i], D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
                CD3DX12_RESOURCE_BARRIER::Transition(this->geometry->GetIndexBuffer(), D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
            };
            commandList->ResourceBarrier(2, barrier);
        }
        commandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);
        {
            D3D12_RESOURCE_BARRIER barrier[2] = {
                CD3DX12_RESOURCE_BARRIER::Transition(this->animatedVertex[i], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
                CD3DX12_RESOURCE_BARRIER::Transition(this->geometry->GetIndexBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_INDEX_BUFFER),
            };
            commandList->ResourceBarrier(2, barrier);
        }
		{
			D3D12_RESOURCE_BARRIER barrier[2] = {
				CD3DX12_RESOURCE_BARRIER::UAV(this->blasResults[i]),
				CD3DX12_RESOURCE_BARRIER::UAV(this->blasScratchs[i])
			};
			commandList->ResourceBarrier(2, barrier);
		}
    }
}

D3D12_GPU_VIRTUAL_ADDRESS KG::Renderer::KGRenderJob::GetBLAS(int animationIndex) const
{
	return animationIndex == -1 ? this->geometry->GetBLAS() : this->blasResults[animationIndex]->GetGPUVirtualAddress();
}

D3D12_GPU_VIRTUAL_ADDRESS KG::Renderer::KGRenderJob::GetVertexBufferGPUAddress(int animationIndex) const
{
	return animationIndex == -1 ? this->geometry->GetVertexBufferGPUAddress() : this->animatedVertex[animationIndex]->GetGPUVirtualAddress();
}

D3D12_GPU_VIRTUAL_ADDRESS KG::Renderer::KGRenderJob::GetIndexBufferGPUAddress() const
{
    return this->geometry->GetIndexBufferGPUAddress();
}

D3D12_GPU_VIRTUAL_ADDRESS KG::Renderer::KGRenderJob::GetObjectBufferGPUAddress() const
{
    return this->objectBuffer->buffer.resource->GetGPUVirtualAddress();
}

D3D12_GPU_VIRTUAL_ADDRESS KG::Renderer::KGRenderJob::GetMaterialBufferGPUAddress() const
{
    return this->shader->GetMaterialBufferGPUAddress();
}

void KG::Renderer::KGRenderJob::Render( ShaderGeometryType geoType, ShaderPixelType pixType, ID3D12GraphicsCommandList* cmdList, Shader*& prevShader )
{
	this->Render( geoType, pixType, ShaderTesselation::NormalMesh, cmdList, prevShader );
}

void KG::Renderer::KGRenderJob::Render( ShaderGeometryType geoType, ShaderPixelType pixType, ShaderTesselation tessel, ID3D12GraphicsCommandList* cmdList, Shader*& prevShader )
{
    bool cull = this->isCulling;
    this->isCulling = false;
	if ( this->visibleSize == 0 ) return;
	if ( prevShader != this->shader )
	{
		prevShader = this->shader;
	}
	this->shader->Set( cmdList, this->meshType, pixType, geoType, tessel );
	auto addr = this->objectBuffer->buffer.resource->GetGPUVirtualAddress();
	cmdList->SetGraphicsRootShaderResourceView( GraphicRootParameterIndex::InstanceData, addr );

	if ( this->meshType == ShaderMeshType::SkinnedMesh )
	{
		auto animAddr = this->animationBuffer->buffer.resource->GetGPUVirtualAddress();
		cmdList->SetGraphicsRootShaderResourceView( GraphicRootParameterIndex::AnimationTransformData, animAddr );
	}

	if ( shader->GetGroup() == KG::Renderer::ShaderGroup::AmbientLight ||
		shader->GetGroup() == KG::Renderer::ShaderGroup::MeshVolumeLight ||
		shader->GetGroup() == KG::Renderer::ShaderGroup::DirectionalLight ||
		shader->GetGroup() == KG::Renderer::ShaderGroup::Transparent )
	{
		auto shadowAddr = this->shadowLightBuffer->buffer.resource->GetGPUVirtualAddress();
		cmdList->SetGraphicsRootShaderResourceView( GraphicRootParameterIndex::LightData, shadowAddr );
	}

	this->geometry->Render( cmdList, cull ? this->updateCount : this->notCullIndexStart);
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

