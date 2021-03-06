#include "pch.h"
#include "Render3DComponent.h"

#include "KGRenderQueue.h"
#include "ResourceContainer.h"
#include "AnimationComponent.h"
#include "CameraComponent.h"
#include "GeometryComponent.h"
#include "MaterialComponent.h"

void KG::Component::Render3DComponent::OnRender(ID3D12GraphicsCommandList* commadList)
{
}

void KG::Component::Render3DComponent::CullingProcess(const DirectX::BoundingFrustum& frustum)
{
    const auto& mat = this->transform->GetGlobalWorldMatrix();
    for (size_t i = 0; i < this->renderJobs.size(); i++)
    {
        auto* job = this->renderJobs[i];
        BoundingBox aabb;
        job->geometry->aabb.Transform(aabb, XMLoadFloat4x4(&mat));
        jobCulled[i] = frustum.Intersects(aabb);
        if (jobCulled[i]) job->AddCullObject();
    }
}

void KG::Component::Render3DComponent::OnPreRender()
{
    if (!this->isVisible) return;
	for ( size_t i = 0; i < this->renderJobs.size(); i++ )
	{
		auto* renderJob = this->renderJobs[i];
		int updateCount = renderJob->GetUpdateCount(jobCulled[i]);
		auto mat = Math::Matrix4x4::Transpose(this->transform->GetGlobalWorldMatrix());
		renderJob->objectBuffer->mappedData[updateCount].object.world = mat;
		if ( this->material )
		{
			renderJob->objectBuffer->mappedData[updateCount].object.materialIndex = this->material->GetMaterialIndex(this->jobMaterialIndexs[i]);
		}
		if ( this->boneAnimation && renderJob->animationBuffer != nullptr )
		{
			for ( size_t k = 0; k < this->boneAnimation->frameCache[i].size(); k++ )
			{
				//auto finalAnim = Math::Matrix4x4::Inverse(this->boneAnimation->frameCache[i][k]->GetTransform()->GetGlobalWorldMatrix());
				if ( this->boneAnimation->frameCache[i][k] )
				{
					//auto wo = Math::Matrix4x4::Inverse( this->transform->GetLocalWorldMatrix() );
					auto finalTransform = this->boneAnimation->frameCache[i][k]->GetTransform()->GetGlobalWorldMatrix();
					renderJob->animationBuffer->mappedData[updateCount].currentTransforms[k] = Math::Matrix4x4::Transpose(finalTransform);
				}
			}
			if ( this->boneAnimation->frameCache[i].size() != 0 )
			{
				renderJob->animationBuffer->mappedData[updateCount].currentTransforms[63] = Math::Matrix4x4::Transpose(this->gameObject->GetTransform()->GetLocalWorldMatrix());
			}
		}
		if ( this->reflectionProbe )
		{
			renderJob->objectBuffer->mappedData[updateCount].object.environmentMapIndex = this->reflectionProbe->GetRenderTexture().renderTargetResource.GetDescriptor(DescriptorType::SRV).HeapIndex;
		}
		else
		{
			assert(KG::Renderer::KGDXRenderer::GetInstance()->GetSkymapTexutreId() != 0 && "SkyBox Texture should not null! add SkyBox to scene!!");
			renderJob->objectBuffer->mappedData[updateCount].object.environmentMapIndex =
				KG::Resource::ResourceContainer::GetInstance()->LoadTexture(KG::Renderer::KGDXRenderer::GetInstance()->GetSkymapTexutreId())->index;
		}
	}
    this->cullingIndex = -1;
    this->updateIndex = -1;
}




void KG::Component::Render3DComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	IRenderComponent::OnCreate(gameObject);
	this->RegisterTransform(gameObject->GetComponent<TransformComponent>());
	this->RegisterMaterial(gameObject->GetComponent<MaterialComponent>());
	this->RegisterGeometry(gameObject->GetComponent<GeometryComponent>());
	this->RegisterBoneAnimation(gameObject->GetComponent<BoneTransformComponent>());
	auto geometryCount = this->geometry->geometrys.size();
	auto materialCount = this->material->materialIndexs.size();

	if ( materialCount != 1 && geometryCount != materialCount )
	{
		DebugErrorMessage("Material Count Not Matched Geometry");
	}
	this->ReloadRender();
	//?????? ???? ?????? ??????
}

void KG::Component::Render3DComponent::SetVisible(bool visible)
{
	for ( auto* renderJob : this->renderJobs )
	{
		if ( this->isVisible == visible )
			return;
		this->isVisible = visible;
		if ( this->isVisible )
		{
			renderJob->OnVisibleAdd();
		}
		else
		{
			renderJob->OnVisibleRemove();
		}
	}
}

void KG::Component::Render3DComponent::SetReflectionProbe(CubeCameraComponent* probe)
{
	this->reflectionProbe = probe;
}

void KG::Component::Render3DComponent::RemoveJobs()
{
	for ( size_t i = 0; i < this->renderJobs.size(); i++ )
	{
		this->renderJobs[i]->OnObjectRemove(this->isVisible);
		this->jobMaterialIndexs[i] = 0;
	}
	this->renderJobs.clear();
	this->jobMaterialIndexs.clear();
}

void KG::Component::Render3DComponent::ReloadRender()
{
	auto geometryCount = this->geometry->geometrys.size();
	auto materialCount = this->material->materialIndexs.size();
	for ( size_t i = 0; i < geometryCount; i++ )
	{
		auto materialIndex = materialCount == 1 ? 0 : i;
		auto job = KG::Renderer::KGDXRenderer::GetInstance()->GetRenderEngine()->GetRenderJob(this->material->shaders[materialIndex], this->geometry->geometrys[i]);
		this->AddRenderJob(job, materialIndex);
	}
}

void KG::Component::Render3DComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::Render3DComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::Render3DComponent);
}

bool KG::Component::Render3DComponent::OnDrawGUI()
{
	if ( ImGui::ComponentHeader< KG::Component::Render3DComponent>() )
	{
        ImGui::Checkbox("IsVisible", &this->isVisible);
        ImGui::BulletText("Geometry Info");
        ImGui::Indent();
        for (size_t i = 0; i < this->geometry->geometrys.size(); i++)
        {
 
            auto counts = this->geometry->geometrys[i]->GetCounts();
            ImGui::BulletText("%d. v : %d / i : %d / p : %d", i, counts.first, counts.second, counts.second / 3);
        }
        ImGui::Unindent();
		ImGui::BulletText("RenderJobs Count : %d", this->renderJobs.size());
		for ( size_t i = 0; i < this->renderJobs.size(); i++ )
		{
			ImGui::BulletText("RenderJob Ptr : %d", (int)this->renderJobs[i]);
		}
		if ( ImGui::Button("Reload RenderJob") )
		{
			this->RemoveJobs();
			this->material->ReloadMaterial();
			this->ReloadRender();
		}
	}
	return false;
}

bool KG::Component::Render3DComponent::GetVisible() const
{
    return this->isVisible;
}

void KG::Component::Render3DComponent::SetReflectionProbe(ICubeCameraComponent* probe)
{
}

void KG::Component::Render3DComponent::AddRenderJob(KG::Renderer::KGRenderJob* renderJob, UINT materialIndex)
{
	this->renderJobs.push_back(renderJob);
    this->jobMaterialIndexs.push_back(materialIndex);
    this->jobCulled.push_back(false);
    renderJob->OnObjectAdd(this->isVisible);
}

void KG::Component::Render3DComponent::RegisterTransform(TransformComponent* transform)
{
	this->transform = transform;
}

void KG::Component::Render3DComponent::RegisterMaterial(MaterialComponent* material)
{
	this->material = material;
}

void KG::Component::Render3DComponent::RegisterGeometry(GeometryComponent* geometry)
{
	this->geometry = geometry;
}

void KG::Component::Render3DComponent::RegisterBoneAnimation(BoneTransformComponent* anim)
{
	this->boneAnimation = anim;
}
