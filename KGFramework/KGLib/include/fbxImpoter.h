#pragma once
#include <string_view>
#include <sstream>
#include <Windows.h>
#include <vector>
#include <deque>
#include <array>
#include <map>
#include <DirectXMath.h>
#include <algorithm>

#include "hash.h"


namespace KG::Utill
{
	using namespace DirectX;

	struct VertexBoneData
	{
		UINT bondId;
		float boneWeight;
	};

	struct BoneData
	{
		KG::Utill::HashString nodeId = KG::Utill::HashString( 0 );
		XMFLOAT4X4 offsetMatrix;
	};

	struct MeshData
	{
		std::vector<UINT> indices;
		std::vector<XMFLOAT3> positions;
		std::vector<XMFLOAT3> normals;
		std::vector<XMFLOAT3> tangent;
		std::vector<XMFLOAT3> biTangent;
		std::vector<std::vector<XMFLOAT2>> uvs;
		std::vector<std::vector<VertexBoneData>> vertexBone;

		std::vector<BoneData> bones;
	};

	struct ModelNode
	{
		KG::Utill::HashString nodeId = KG::Utill::HashString( 0 );
		std::string name;
		ModelNode* child = nullptr;
		ModelNode* sibling = nullptr;

		XMFLOAT3 position;
		XMFLOAT4 rotation;
		XMFLOAT3 scale;

		std::vector<UINT> meshs;

		void AddChild( ModelNode* node );
		void AddSibling( ModelNode* node );
	};

	struct ImportData
	{
	public:
		std::vector<MeshData> meshs;
		std::deque<ModelNode> nodes;
		ModelNode* root = nullptr;
		void LoadFromPathFBX( const std::string& path );
	};
}