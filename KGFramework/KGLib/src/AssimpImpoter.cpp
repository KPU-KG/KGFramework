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
				// 이거 아마 이렇게 하면 큰일날 것임!
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

#define SOLDIER_ANIMATION_NUMBER 3
const char* animPath = "Resource/Geometry/animation/Soldier@";
const char* animName[SOLDIER_ANIMATION_NUMBER]{
	"WalkForward",
	"WalkRight",
	"WalkLeft"
};

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

		// 애니메이션 로드 코드
		// 위에 디파인 값들이 xml로 들어가야됨
		// + 그냥 애니메이션 채로 대입연산자를 사용하면 ( animation = scene->mAnimation[i]; )
		// 씬 포인터가 사용하는 메모리가 같아서 
		// 포문 돌릴때 그냥 이전 데이터를 덮어써버림
		if (scene->HasAnimations()) {
			for (int anim = 0; anim < SOLDIER_ANIMATION_NUMBER; ++anim) {
				std::string p;
				p.append(animPath);
				p.append(animName[anim]);
				p.append(".fbx");
				Assimp::Importer importer;
				const aiScene* animScene = importer.ReadFile(p.c_str(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_MaxQuality);
				for (int i = 0; i < animScene->mNumAnimations; ++i) {
					std::string s;
					s.append(animName[anim]);
					s.append("\0");

					aiAnimation* pAni = new aiAnimation();
					pAni->mDuration = animScene->mAnimations[i]->mDuration;
					pAni->mNumChannels = animScene->mAnimations[i]->mNumChannels;
					pAni->mChannels = new aiNodeAnim * [pAni->mNumChannels];
					for (int ch = 0; ch < pAni->mNumChannels; ++ch) {
						pAni->mChannels[ch] = new aiNodeAnim;
						pAni->mChannels[ch]->mNodeName = animScene->mAnimations[i]->mChannels[ch]->mNodeName;
						pAni->mChannels[ch]->mNumPositionKeys = animScene->mAnimations[i]->mChannels[ch]->mNumPositionKeys;
						pAni->mChannels[ch]->mNumRotationKeys = animScene->mAnimations[i]->mChannels[ch]->mNumRotationKeys;
						pAni->mChannels[ch]->mNumScalingKeys = animScene->mAnimations[i]->mChannels[ch]->mNumScalingKeys;
						
						pAni->mChannels[ch]->mPositionKeys = new aiVectorKey[pAni->mChannels[ch]->mNumPositionKeys];
						for (int pos = 0; pos < pAni->mChannels[ch]->mNumPositionKeys; ++pos) {
							pAni->mChannels[ch]->mPositionKeys[pos].mTime = animScene->mAnimations[i]->mChannels[ch]->mPositionKeys[pos].mTime;
							pAni->mChannels[ch]->mPositionKeys[pos].mValue = animScene->mAnimations[i]->mChannels[ch]->mPositionKeys[pos].mValue;
						}
						
						pAni->mChannels[ch]->mRotationKeys = new aiQuatKey[pAni->mChannels[ch]->mNumRotationKeys];
						for (int rot = 0; rot < pAni->mChannels[ch]->mNumRotationKeys; ++rot) {
							pAni->mChannels[ch]->mRotationKeys[rot].mTime = animScene->mAnimations[i]->mChannels[ch]->mRotationKeys[rot].mTime;
							pAni->mChannels[ch]->mRotationKeys[rot].mValue = animScene->mAnimations[i]->mChannels[ch]->mRotationKeys[rot].mValue;
						}

						pAni->mChannels[ch]->mScalingKeys = new aiVectorKey[pAni->mChannels[ch]->mNumScalingKeys];
						for (int scale = 0; scale < pAni->mChannels[ch]->mNumScalingKeys; ++scale) {
							pAni->mChannels[ch]->mScalingKeys[scale].mTime = animScene->mAnimations[i]->mChannels[ch]->mScalingKeys[scale].mTime;
							pAni->mChannels[ch]->mScalingKeys[scale].mValue = animScene->mAnimations[i]->mChannels[ch]->mScalingKeys[scale].mValue;
						}
					}
					pAni->mName.Set(s);
					pAni->mTicksPerSecond = animScene->mAnimations[i]->mTicksPerSecond;
						
					//  = animScene->mAnimations[i];
					// pAni->mName.Set(s);
					this->root->animations.emplace_back(pAni);
				}
			}



		}
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