#include "pch.h"
#include "ResourceContainer.h"
#include "KGShader.h"
#include "KGGeometry.h"

using namespace KG::Renderer;
void KG::Renderer::ResourceContainer::AddResource(KG::Utill::_ID id, Geometry* ptr)
{
	this->geometry.insert(std::make_pair(id, std::unique_ptr<Geometry>(ptr)));
}

void KG::Renderer::ResourceContainer::AddResource(KG::Utill::_ID id, IShader* ptr)
{
	this->shader.insert(std::make_pair(id, std::unique_ptr<IShader>(ptr)));
}

void KG::Renderer::ResourceContainer::AddResource(KG::Utill::_ID id, std::unique_ptr<Geometry>&& ptr)
{
	this->geometry.insert(std::make_pair(id, std::move(ptr)));
}

void KG::Renderer::ResourceContainer::AddResource(KG::Utill::_ID id, std::unique_ptr<IShader>&& ptr)
{
	this->shader.insert(std::make_pair(id, std::move(ptr)));
}

IShader* KG::Renderer::ResourceContainer::GetShader(KG::Utill::_ID id)
{
	return this->shader.at(id).get();
}

Geometry* KG::Renderer::ResourceContainer::GetGeometry(KG::Utill::_ID id)
{
	return this->geometry.at(id).get();
}

