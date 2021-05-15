#pragma once
#include <string_view>
#include <sstream>
#include <fstream>
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

        void SaveToKGG(std::ofstream& out);
        void LoadToKGG(std::ifstream& in);
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

        void SaveToKGG(std::ofstream& out, const std::deque<ModelNode>& nodes);
        void LoadToKGG(std::ifstream& in, std::deque<ModelNode>& nodes);
	};

	struct KeyData
	{
		float keyTime = 0.0f;
		float value = 0.0f;
		bool operator<( const KeyData& other) const noexcept
		{
			return this->keyTime < other.keyTime;
		}
	};

	struct Vector3Data
	{
		std::vector<KeyData> x;
		std::vector<KeyData> y;
		std::vector<KeyData> z;

        void SaveToKGG(std::ofstream& out);
        void LoadToKGG(std::ifstream& in);
	};

	struct NodeAnimation
	{
		KG::Utill::HashString nodeId;
		XMFLOAT4 preRotation;
		Vector3Data protation;
		Vector3Data translation;
		Vector3Data rotation;
		Vector3Data scale;
        void SaveToKGG(std::ofstream& out);
        void LoadToKGG(std::ifstream& in);
	};

	struct AnimationLayer
	{
		std::vector<NodeAnimation> nodeAnimations;
        void SaveToKGG(std::ofstream& out);
        void LoadToKGG(std::ifstream& in);
	};

	struct AnimationSet
	{
		KG::Utill::HashString animationId;
		std::vector<AnimationLayer> layers;
        void SaveToKGG(std::ofstream& out);
        void LoadToKGG(std::ifstream& in);
	};

	struct ImportData
	{
	public:
		ImportData() = default;
		ImportData(const ImportData&) = delete;
		ImportData(ImportData&& other) noexcept;

		ImportData& operator=(const ImportData&) = delete;
		ImportData& operator=(ImportData&& other) noexcept;

		std::vector<MeshData> meshs;
		std::deque<ModelNode> nodes;
		std::vector<AnimationSet> animations;
		ModelNode* root = nullptr;
		void LoadFromPathFBX( const std::string& path );
        bool LoadToKGG(const std::string& path);
        void SaveToKGG(const std::string& path);
	};
	extern DirectX::XMFLOAT4 ChangeEulerToDxQuat( float x, float y, float z );
}
