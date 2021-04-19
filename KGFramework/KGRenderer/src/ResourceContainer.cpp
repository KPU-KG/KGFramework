#include "pch.h"
#include <map>
#include <mutex>
#include <algorithm>
#include <stack>
#include "Debug.h"
#include "Scene.h"
#include "ResourceContainer.h"
#include "KGShader.h"
#include "KGGeometry.h"
#include "KGResourceLoader.h"
#include "KGDXRenderer.h"
#include "KGModel.h"
#include "Transform.h"
#include "ImguiHelper.h"

std::unique_ptr<KG::Resource::ResourceContainer> KG::Resource::ResourceContainer::instance = nullptr;

KG::Resource::ResourceContainer::ResourceContainer()
{
	DebugNormalMessage(TOSTRING(ResourceContainer) << " : Initialize");
}

KG::Resource::ResourceContainer::~ResourceContainer()
{
	DebugNormalMessage(TOSTRING(ResourceContainer) << " : Removed");
}

KG::Renderer::Shader* KG::Resource::ResourceContainer::LoadShader(const KG::Utill::HashString& id)
{
	if ( this->shaders.count(id) )
	{
		return &this->shaders.at(id);
	}
	else
	{
		auto metaData = ResourceLoader::LoadShaderSetFromFile("Resource/ShaderCode.xml", id);
		return &this->shaders.emplace(id, metaData).first->second;
	}
}

KG::Resource::FrameModel* KG::Resource::ResourceContainer::LoadModel(const KG::Utill::HashString& id)
{
	if ( this->models.count(id) )
	{
		return &this->models.at(id);
	}
	else
	{
		auto metaData = ResourceLoader::LoadGeometrySetFromFile("Resource/GeometrySet.xml", id);
		auto* model = &this->models.emplace(id, metaData).first->second;
		auto& frame = model->data;
		for ( size_t i = 0; i < frame.meshs.size(); i++ )
		{
			this->CreateGeometry(id, i, frame.meshs[i]);
		}
		return model;
	}
}

KG::Renderer::Geometry* KG::Resource::ResourceContainer::LoadGeometry(const KG::Utill::HashString& id, UINT geometryIndex)
{
	auto handle = std::make_pair(id, geometryIndex);
	if ( this->geometrys.count(handle) )
	{
		return &this->geometrys.at(handle);
	}
	else
	{
		auto metaData = ResourceLoader::LoadGeometrySetFromFile("Resource/GeometrySet.xml", id);
		if ( metaData.isRawMesh )
		{
			return this->LoadRawModel(id);
		}
		else
		{
			this->LoadModel(id);
			return this->LoadGeometry(id, geometryIndex);
		}
	}
}

KG::Renderer::Geometry* KG::Resource::ResourceContainer::LoadRawModel(const KG::Utill::HashString& id)
{
	auto* model = this->LoadModel(id);
	return &this->geometrys.emplace(std::make_pair(id, 0), model->data.meshs[0]).first->second;
}

KG::Renderer::Geometry* KG::Resource::ResourceContainer::CreateGeometry(const KG::Utill::HashString& id, UINT geometryIndex, KG::Utill::MeshData& mesh)
{
	auto handle = std::make_pair(id, geometryIndex);
	if ( this->geometrys.count(handle) )
	{
		return &this->geometrys.at(handle);
		DebugErrorMessage("Try Load Already Loaded model's Geometry")
	}
	else
	{
		return &this->geometrys.emplace(std::make_pair(handle, mesh)).first->second;
	}
}

KG::Renderer::Geometry* KG::Resource::ResourceContainer::CreateFakeGeometry(D3D12_PRIMITIVE_TOPOLOGY topology, int vertexCount)
{
	auto handle = std::make_pair(topology, vertexCount);
	if ( this->fakeGeometrys.count(handle) )
	{
		return &this->fakeGeometrys.at(handle);
		DebugErrorMessage("Try Load Already Loaded model's Geometry")
	}
	else
	{
		return &this->fakeGeometrys.emplace(std::make_pair(handle, handle)).first->second;
	}
}

KG::Resource::Texture* KG::Resource::ResourceContainer::LoadTexture(const KG::Utill::HashString& id)
{
	if ( id == 0 )
	{
		std::cout << "Texture id 0 Load\n";
	}
	if ( this->textures.count(id) )
	{
		return &this->textures.at(id);
	}
	else
	{
		auto metaData = ResourceLoader::LoadTextureFromFile("Resource/TextureSet.xml", id);
		return &this->textures.emplace(id, metaData).first->second;
	}
}

KG::Resource::Texture* KG::Resource::ResourceContainer::CreateTexture(const KG::Utill::HashString& id, ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc)
{
	if ( this->textures.count(id) )
	{
		DebugErrorMessage(id.value << L"는 이미 존재하는 텍스처입니다.");
		return &this->textures.at(id);
	}
	else
	{
		return &this->textures.emplace(id, KG::Resource::Texture(resource, srvDesc)).first->second;
	}
}

KG::Utill::AnimationSet* KG::Resource::ResourceContainer::LoadAnimation(const KG::Utill::HashString& id, UINT animationIndex)
{
	auto handle = std::make_pair(id, animationIndex);
	if ( this->animations.count(handle) )
	{
		return &this->animations.at(handle);
	}
	else
	{
		auto* model = this->LoadModel(id);
		return &this->animations.emplace(std::make_pair(id, animationIndex), std::move(model->data.animations[animationIndex])).first->second;
	}
}

std::pair<size_t, KG::Utill::HashString> KG::Resource::ResourceContainer::LoadMaterial(const KG::Utill::HashString& id)
{
	if ( this->materials.count(id) )
	{
		return this->materials.at(id);
	}
	else
	{
		auto result = KG::Resource::ResourceLoader::LoadMaterialFromFile("Resource/MaterialSet.xml", id);
		return this->materials.emplace(id, result).first->second;
	}
}

//void KG::Resource::ResourceContainer::AddAnimation( const KG::Utill::HashString& id, UINT animationIndex, const KG::Utill::AnimationSet& animation )
//{
//	this->animations.emplace( std::make_pair( id, animationIndex ), animation );
//}
//
//void KG::Resource::ResourceContainer::AddAnimation( const KG::Utill::HashString& id, UINT animationIndex, KG::Utill::AnimationSet&& animation )
//{
//	this->animations.emplace( std::make_pair( id, animationIndex ), std::move(animation) );
//}

void KG::Resource::ResourceContainer::ConvertNodeToObject(const KG::Utill::HashString& id, KG::Core::GameObject* object, KG::Utill::ModelNode* node, const MaterialMatch& materials, KG::Core::GameObject* rootObject)
{

	object->tag = KG::Utill::HashString(node->name);
	auto* renderer = KG::Renderer::KGDXRenderer::GetInstance();

	auto* tran = object->GetComponent<KG::Component::TransformComponent>();
	tran->SetScale(node->scale);
	tran->SetRotation(node->rotation);
	tran->SetPosition(node->position);
	if ( node->meshs.size() != 0 )
	{
		auto* geo = renderer->GetNewGeomteryComponent();
		auto* mat = renderer->GetNewMaterialComponent();

		auto& materialSet = materials.GetMaterial(object->tag);
		for ( size_t i = 0; i < node->meshs.size(); i++ )
		{
			auto index = node->meshs[i];
			geo->AddGeometry(id, index, i);
		}
		for ( size_t i = 0; i < materialSet.size(); i++ )
		{
			mat->PostMaterial(materialSet[i], i);
		}
		object->AddComponent(geo);
		object->AddComponent(mat);
		
		if ( geo->HasBone() )
		{
			this->postModelLoadFunctions.emplace_back(
				[renderer, rootObject, object]()
				{
					auto* avatar = renderer->GetNewBoneTransformComponent();
					avatar->SetRootNode(rootObject);
					object->AddComponent(avatar);
					object->AddComponent(renderer->GetNewRenderComponent());
				}
			);
		}
		else
		{
			object->AddComponent(renderer->GetNewRenderComponent());
		}
	}
	//Bone Debug
	//else 
	//{
	//	object->AddComponent( renderer->GetNewGeomteryComponent() );
	//	object->AddComponent( renderer->GetNewMaterialComponent() );
	//	auto* geo = object->GetComponent<KG::Component::GeometryComponent>();
	//	auto* mat = object->GetComponent<KG::Component::MaterialComponent>();
	//	geo->InitializeGeometry( KG::Utill::HashString("sphere"_id) );
	//	mat->InitializeMaterial( KG::Utill::HashString("PBRStone"_id) );
	//	object->AddComponent( renderer->GetNewRenderComponent() );
	//}

}

KG::Core::GameObject* KG::Resource::ResourceContainer::CreateObjectFromModel(const KG::Utill::HashString& id, KG::Core::ObjectContainer& container, const MaterialMatch& materials)
{
	auto& frame = this->LoadModel(id)->data;
	std::stack<std::pair<KG::Core::GameObject*, KG::Utill::ModelNode*>> modelStack;
	auto* rootObject = container.CreateNewObject();
	auto* rootModelNode = frame.root;

	for ( size_t i = 0; i < frame.meshs.size(); i++ )
	{
		this->CreateGeometry(id, i, frame.meshs[i]);
	}

	this->ConvertNodeToObject(id, rootObject, rootModelNode, materials, rootObject);


	modelStack.push(std::make_pair(rootObject, rootModelNode));
	while ( !modelStack.empty() )
	{
		auto* object = modelStack.top().first;
		auto* node = modelStack.top().second;
		auto* transform = object->GetComponent<KG::Component::TransformComponent>();
		if ( node->child != nullptr && !transform->hasChild() )
		{
			auto* childObject = container.CreateNewObject();
			auto* childNode = node->child;
			this->ConvertNodeToObject(id, childObject, childNode, materials, rootObject);
			transform->AddChild(childObject->GetComponent<KG::Component::TransformComponent>());
			modelStack.push(std::make_pair(childObject, childNode));
		}
		else if ( node->sibling != nullptr && !transform->hasNextSibiling() )
		{
			auto* siblingObject = container.CreateNewObject();
			auto* siblingNode = node->sibling;
			this->ConvertNodeToObject(id, siblingObject, siblingNode, materials, rootObject);
			transform->AddSibiling(siblingObject->GetComponent<KG::Component::TransformComponent>());
			modelStack.push(std::make_pair(siblingObject, siblingNode));
			auto* siblingObj = object->GetSibling();
			auto* child = siblingObj->GetChild();
		}
		else
		{
			modelStack.pop();
		}
	}
	//auto rootPosition = rootObject->GetTransform()->GetPosition();
	auto rootScale = rootObject->GetTransform()->GetScale();
	float scaleFactor = 0.01f;
	//rootObject->GetTransform()->SetPosition( 0, 0, 0 );
	//rootObject->GetTransform()->SetScale( rootScale.x * scaleFactor, rootScale.y * scaleFactor, rootScale.z * scaleFactor );
	auto* parentRoot = container.CreateNewObject();
	parentRoot->GetTransform()->AddChild(rootObject->GetTransform());
	return parentRoot;
}

KG::Core::GameObject* KG::Resource::ResourceContainer::CreateObjectFromModel(const KG::Utill::HashString& id, KG::Core::Scene& scene, const MaterialMatch& materials)
{
	auto& frame = this->LoadModel(id)->data;
	std::stack<std::pair<KG::Core::GameObject*, KG::Utill::ModelNode*>> modelStack;
	auto* rootObject = scene.CreateNewTransformObject();
	auto* rootModelNode = frame.root;



	this->ConvertNodeToObject(id, rootObject, rootModelNode, materials, rootObject);


	modelStack.push(std::make_pair(rootObject, rootModelNode));
	while ( !modelStack.empty() )
	{
		auto* object = modelStack.top().first;
		auto* node = modelStack.top().second;
		auto* transform = object->GetComponent<KG::Component::TransformComponent>();
		if ( node->child != nullptr && !transform->hasChild() )
		{
			auto* childObject = scene.CreateNewTransformObject();
			auto* childNode = node->child;
			this->ConvertNodeToObject(id, childObject, childNode, materials, rootObject);
			transform->AddChild(childObject->GetComponent<KG::Component::TransformComponent>());
			modelStack.push(std::make_pair(childObject, childNode));
		}
		else if ( node->sibling != nullptr && !transform->hasNextSibiling() )
		{
			auto* siblingObject = scene.CreateNewTransformObject();
			auto* siblingNode = node->sibling;
			this->ConvertNodeToObject(id, siblingObject, siblingNode, materials, rootObject);
			transform->AddSibiling(siblingObject->GetComponent<KG::Component::TransformComponent>());
			modelStack.push(std::make_pair(siblingObject, siblingNode));
			auto* siblingObj = object->GetSibling();
			auto* child = siblingObj->GetChild();
		}
		else
		{
			modelStack.pop();
		}
	}
	//auto rootPosition = rootObject->GetTransform()->GetPosition();
	auto rootScale = rootObject->GetTransform()->GetScale();
	float scaleFactor = 0.01f;
	//rootObject->GetTransform()->SetPosition( 0, 0, 0 );
	//rootObject->GetTransform()->SetScale( rootScale.x * scaleFactor, rootScale.y * scaleFactor, rootScale.z * scaleFactor );
	auto* parentRoot = scene.CreateNewTransformObject();
	parentRoot->GetTransform()->AddChild(rootObject->GetTransform());
	for ( auto& i : this->postModelLoadFunctions )
	{
		i();
	}
	this->postModelLoadFunctions.clear();
	return parentRoot;
}

void KG::Resource::ResourceContainer::Clear()
{
	this->shaders.clear();
	this->geometrys.clear();
	this->textures.clear();
	DebugNormalMessage(TOSTRING(ResourceContainer) << "All Resource Cleared");
}

KG::Resource::ResourceContainer* KG::Resource::ResourceContainer::GetInstance()
{
	//static std::once_flag onceFlags;
	//std::call_once( onceFlags, []() {ResourceContainer::instance = std::make_unique<ResourceContainer>(); } );
	if ( ResourceContainer::instance.get() == nullptr )
	{
		ResourceContainer::instance = std::make_unique<ResourceContainer>();
	}
	return ResourceContainer::instance.get();
}

void KG::Resource::ResourceContainer::Process(ID3D12GraphicsCommandList* cmdList)
{
	for ( auto& [key, value] : this->textures )
	{
		value.Process(cmdList);
	}
}
