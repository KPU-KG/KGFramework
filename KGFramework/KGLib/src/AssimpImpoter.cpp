#include "pch.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include "AssimpImpoter.h"
using namespace KG::Utill;

MeshData KG::Utill::ModelData::processMesh(aiMesh* mesh, const aiScene* scene)
{
	MeshData data;
	for ( unsigned int i = 0; i < mesh->mNumFaces; i++ )
	{
		aiFace face = mesh->mFaces[i];
		for ( unsigned int j = 0; j < face.mNumIndices; j++ )
			data.indices.push_back( face.mIndices[j] );
	}
	data.positions.reserve(mesh->mNumVertices);
	data.normals.reserve(mesh->mNumVertices);
	auto textureCount = std::count_if(std::begin(mesh->mTextureCoords), std::end(mesh->mTextureCoords), [](const auto ptr) {return ptr != nullptr; });
	data.uvs.resize(textureCount);

	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		data.positions.push_back(XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
		data.normals.push_back(XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));

		for (size_t j = 0; j < textureCount; j++)
		{
			data.uvs[j].push_back(XMFLOAT2(mesh->mTextureCoords[j][i].x, mesh->mTextureCoords[j][i].y));
		}
	}
	return data;
}

void KG::Utill::ModelData::processNode(aiNode* node, const aiScene* scene)
{
	// 노드의 모든 mesh들을 처리(만약 있다면)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		this->meshes.push_back(processMesh(mesh, scene));
	}
	// 그런 다음 각 자식들에게도 동일하게 적용
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

void KG::Utill::ModelData::LoadModel(const std::string& path)
{
	Assimp::Importer importer;
	//const aiScene* scene = importer.ReadFile( path, aiProcess_Triangulate | aiProcess_FlipUVs );
	const aiScene* scene = importer.ReadFile( path, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_MaxQuality );
	//flag = process http://assimp.sourceforge.net/lib_html/postprocess_8h.html

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::wstringstream ss;
		ss << "ERROR : ASSIMP : " << importer.GetErrorString() << std::endl;
		OutputDebugString(ss.str().c_str());
	}
	std::string directory = path.substr(0, path.find_last_of('/'));
	this->processNode( scene->mRootNode , scene );
}
