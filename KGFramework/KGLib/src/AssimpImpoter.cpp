#include "pch.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include "AssimpImpoter.h"
using namespace KG::Utill;

static MeshData ConvertMesh(const aiMesh* mesh)
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
	data.biTangent.reserve( mesh->mNumVertices );
	data.tangent.reserve( mesh->mNumVertices );

	auto textureCount = std::count_if(std::begin(mesh->mTextureCoords), std::end(mesh->mTextureCoords), [](const auto ptr) {return ptr != nullptr; });
	data.uvs.resize(textureCount);

	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		data.positions.push_back(XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
		data.normals.push_back( XMFLOAT3( mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z ) );
		data.tangent.push_back( XMFLOAT3( mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z ) );
		data.biTangent.push_back( XMFLOAT3( mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z ) );

		for (size_t j = 0; j < textureCount; j++)
		{
			data.uvs[j].push_back(XMFLOAT2(mesh->mTextureCoords[j][i].x, mesh->mTextureCoords[j][i].y));
		}
	}
	data.vertexBone.resize( mesh->mNumVertices );

	for ( int i = 0; i < mesh->mNumBones; i++ )
	{
		for ( int j = 0; j < mesh->mBones[i]->mNumWeights; j++ )
		{
			unsigned int vertexId = mesh->mBones[i]->mWeights[j].mVertexId;
			float weight = mesh->mBones[i]->mWeights[j].mWeight;

			for ( int k = 0; k < 4; k++ )
			{
				// push_back 효과를 구현
				if ( data.vertexBone[vertexId][k].boneWeight == 0.0f )
				{
					data.vertexBone[vertexId][k].bondId = i;
					data.vertexBone[vertexId][k].boneWeight = weight;
					break;
				}
			}
		}
		BoneData boneData;
		boneData.nodeId = KG::Utill::HashString(mesh->mBones[i]->mName.C_Str());
		auto& mat = mesh->mBones[i]->mOffsetMatrix;
		boneData.offsetMatrix = XMFLOAT4X4(
			mat.a1, mat.b1, mat.c1, mat.d1,
			mat.a2, mat.b2, mat.c2, mat.d2,
			mat.a3, mat.b3, mat.c3, mat.d3,
			mat.a4, mat.b4, mat.c4, mat.d4
		);
		data.bones.push_back( boneData );
	}

	// sort bone Weight

	for ( auto& bone : data.vertexBone )
	{
		std::sort( bone.begin(), bone.end(), []( auto& a, auto& b ) {return a.boneWeight > b.boneWeight; } );
	}

	return data;
}

void KG::Utill::ModelNode::AddChild( ModelNode* node )
{
	if ( this->child != nullptr )
	{
		this->child->AddSibling( node );
	}
	else
	{
		this->child = node;
	}
}

void KG::Utill::ModelNode::AddSibling( ModelNode* node )
{
	if ( this->sibling != nullptr )
	{
		this->sibling->AddSibling( node );
	}
	else
	{
		this->sibling = node;
	}
}


void KG::Utill::ImportData::LoadFromPathAssimp( const std::string& path )
{
	Assimp::Importer importer;
	importer.SetPropertyBool( AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false );
	const aiScene* scene = importer.ReadFile( path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_MaxQuality );

	if ( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode )
	{
		std::wstringstream ss;
		ss << "ERROR : ASSIMP : " << importer.GetErrorString() << std::endl;
		OutputDebugString( ss.str().c_str() );
	}
	else 
	{
		for ( size_t i = 0; i < scene->mNumMeshes; i++ )
		{
			this->meshs.push_back( ConvertMesh( scene->mMeshes[i] ) );
		}
		this->root = this->ProcessNode( scene->mRootNode, scene );
		//this->root = this->ProcessNode( scene->mRootNode->mChildren[0], scene );
	}
}

ModelNode* KG::Utill::ImportData::ProcessNode( const aiNode* node, const aiScene* scene )
{
	auto& importNode = this->nodes.emplace_back();
	importNode.name = node->mName.C_Str();
	importNode.nodeId = KG::Utill::HashString(node->mName.C_Str());
	aiVector3D position;
	aiQuaternion rotation;
	aiVector3D scale;
	node->mTransformation.Decompose( scale, rotation, position );

	importNode.position = XMFLOAT3( position.x, position.y, position.z );
	importNode.rotation = XMFLOAT4( rotation.x, rotation.y, rotation.z, rotation.w );
	importNode.scale = XMFLOAT3( scale.x, scale.y, scale.z );

	for ( size_t i = 0; i < node->mNumMeshes; i++ )
	{
		importNode.meshs.push_back( node->mMeshes[i] );
	}
	
	for ( size_t i = 0; i < node->mNumChildren; i++ )
	{
		importNode.AddChild( this->ProcessNode( node->mChildren[i], scene ) );
	}
	return &importNode;
}