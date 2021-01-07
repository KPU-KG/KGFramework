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

class aiMesh;
class aiScene;
class aiNode;


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
		KG::Utill::HashString nodeId;
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
		std::vector<std::array<VertexBoneData, 4>> vertexBone;

		std::vector<BoneData> bones;
	};

	struct ModelNode
	{
		KG::Utill::HashString nodeId;

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
		std::vector<MeshData> meshs;
		std::deque<ModelNode> nodes;
		ModelNode* root = nullptr;
		void LoadFromPathAssimp( const std::string& path );
		ModelNode* ProcessNode( const aiNode* node, const aiScene* scene );
	};
}

/*
Copyright (c) 2006-2015 assimp team
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:


Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of the assimp team nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/