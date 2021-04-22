#include "fbxImpoter.h"
#define KFBX_DLLINFO
#include "MathHelper.h"
#include <iomanip>
#include <fbxsdk.h>
#include <DirectXMathConvert.inl>
#include <DirectXMath.h>
#include <DirectXMathMisc.inl>
#include <mutex>



static inline bool exists(const std::string& name)
{
	FILE* file = nullptr;
	fopen_s(&file, name.c_str(), "r");
	if ( file )
	{
		fclose(file);
		return true;
	}
	else
	{
		return false;
	}
}

void KG::Utill::ModelNode::AddChild(ModelNode* node)
{
	if ( this->child != nullptr )
	{
		this->child->AddSibling(node);
	}
	else
	{
		this->child = node;
	}
}

void KG::Utill::ModelNode::AddSibling(ModelNode* node)
{
	if ( this->sibling != nullptr )
	{
		this->sibling->AddSibling(node);
	}
	else
	{
		this->sibling = node;
	}
}


FbxScene* pFbxScene;

static FbxAMatrix GetGeometricOffsetTransform(FbxNode* pfbxNode)
{
	const FbxVector4 T = pfbxNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 R = pfbxNode->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 S = pfbxNode->GetGeometricScaling(FbxNode::eSourcePivot);

	return(FbxAMatrix(T, R, S));
}

static void fbxamatrix_to_xmfloat4x4(const FbxAMatrix& fbxamatrix, DirectX::XMFLOAT4X4& xmfloat4x4)
{
	for ( int row = 0; row < 4; row++ )
	{
		for ( int column = 0; column < 4; column++ )
		{
			xmfloat4x4.m[row][column] = static_cast<float>(fbxamatrix[row][column]);
		}
	}
}


//-------------------------------------------------------------------------------------
// DirectXMeshTangentFrame.cpp
//
// DirectX Mesh Geometry Library - Normals, Tangents, and Bi-Tangents Computation
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkID=324981
//-------------------------------------------------------------------------------------
static void CalculateTangentBinormal(KG::Utill::MeshData& data, UINT i0, UINT i1, UINT i2)
{
	using namespace DirectX;

	static constexpr float EPSILON = 0.0001f;
	static const XMVECTORF32 s_flips = { { { 1.f, -1.f, -1.f, 1.f } } };

	XMVECTOR t0 = XMLoadFloat2(&data.uvs[0][i0]);
	XMVECTOR t1 = XMLoadFloat2(&data.uvs[0][i1]);
	XMVECTOR t2 = XMLoadFloat2(&data.uvs[0][i2]);

	XMVECTOR tangent1[3] = { {},{},{} };
	XMVECTOR tangent2[3] = { {},{},{} };

	XMVECTOR s = XMVectorMergeXY(XMVectorSubtract(t1, t0), XMVectorSubtract(t2, t0));

	XMFLOAT4A tmp;
	XMStoreFloat4A(&tmp, s);
	float d = tmp.x * tmp.w - tmp.z * tmp.y;
	d = (fabsf(d) <= EPSILON) ? 1.f : (1.f / d);
	s = XMVectorScale(s, d);
	s = XMVectorMultiply(s, s_flips);

	XMMATRIX m0;
	m0.r[0] = XMVectorPermute<3, 2, 6, 7>(s, g_XMZero);
	m0.r[1] = XMVectorPermute<1, 0, 4, 5>(s, g_XMZero);
	m0.r[2] = m0.r[3] = g_XMZero;

	XMVECTOR p0 = XMLoadFloat3(&data.positions[i0]);
	XMVECTOR p1 = XMLoadFloat3(&data.positions[i1]);
	XMVECTOR p2 = XMLoadFloat3(&data.positions[i2]);

	XMMATRIX m1;
	m1.r[0] = XMVectorSubtract(p1, p0);
	m1.r[1] = XMVectorSubtract(p2, p0);
	m1.r[2] = m1.r[3] = g_XMZero;

	XMMATRIX uv = XMMatrixMultiply(m0, m1);

	tangent1[0] = XMVectorAdd(tangent1[0], uv.r[0]);
	tangent1[1] = XMVectorAdd(tangent1[1], uv.r[0]);
	tangent1[2] = XMVectorAdd(tangent1[2], uv.r[0]);

	tangent2[0] = XMVectorAdd(tangent2[0], uv.r[1]);
	tangent2[1] = XMVectorAdd(tangent2[1], uv.r[1]);
	tangent2[2] = XMVectorAdd(tangent2[2], uv.r[1]);

	for ( size_t j = 0; j < 3; ++j )
	{
		UINT index[3] = { i0, i1, i2 };
		// Gram-Schmidt orthonormalization
		XMVECTOR b0 = XMLoadFloat3(&data.normals[index[j]]);
		b0 = XMVector3Normalize(b0);

		XMVECTOR tan1 = tangent1[j];
		XMVECTOR b1 = XMVectorSubtract(tan1, XMVectorMultiply(XMVector3Dot(b0, tan1), b0));
		b1 = XMVector3Normalize(b1);

		XMVECTOR tan2 = tangent2[j];
		XMVECTOR b2 = XMVectorSubtract(XMVectorSubtract(tan2, XMVectorMultiply(XMVector3Dot(b0, tan2), b0)), XMVectorMultiply(XMVector3Dot(b1, tan2), b1));
		b2 = XMVector3Normalize(b2);

		// handle degenerate vectors
		float len1 = XMVectorGetX(XMVector3Length(b1));
		float len2 = XMVectorGetY(XMVector3Length(b2));

		if ( (len1 <= EPSILON) || (len2 <= EPSILON) )
		{
			if ( len1 > 0.5f )
			{
				// Reset bi-tangent from tangent and normal
				b2 = XMVector3Cross(b0, b1);
			}
			else if ( len2 > 0.5f )
			{
				// Reset tangent from bi-tangent and normal
				b1 = XMVector3Cross(b2, b0);
			}
			else
			{
				// Reset both tangent and bi-tangent from normal
				XMVECTOR axis;

				float d0 = fabsf(XMVectorGetX(XMVector3Dot(g_XMIdentityR0, b0)));
				float d1 = fabsf(XMVectorGetX(XMVector3Dot(g_XMIdentityR1, b0)));
				float d2 = fabsf(XMVectorGetX(XMVector3Dot(g_XMIdentityR2, b0)));
				if ( d0 < d1 )
				{
					axis = (d0 < d2) ? g_XMIdentityR0 : g_XMIdentityR2;
				}
				else if ( d1 < d2 )
				{
					axis = g_XMIdentityR1;
				}
				else
				{
					axis = g_XMIdentityR2;
				}

				b1 = XMVector3Cross(b0, axis);
				b2 = XMVector3Cross(b0, b1);
			}
		}
		XMStoreFloat3(&data.tangent[index[j]], b1);
		XMStoreFloat3(&data.biTangent[index[j]], b2);
	}
}

static DirectX::XMFLOAT3 ReadTangent(FbxMesh* mesh, int controlPointIndex, int vertexCounter) //vec3 ReadNormal //vec3 RadeBinormal
{
	if ( mesh->GetElementTangentCount() < 1 )
	{
		DebugErrorMessage(mesh->GetName() << " : Invalid Tangent Number");
	}
	FbxGeometryElementTangent* vertexTangent = mesh->GetElementTangent(0);

	DirectX::XMFLOAT3 result;

	switch ( vertexTangent->GetMappingMode() )
	{
		case FbxGeometryElement::eByControlPoint:
			switch ( vertexTangent->GetReferenceMode() )
			{
				case FbxGeometryElement::eDirect:
				{
					result.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(controlPointIndex).mData[0]);
					result.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(controlPointIndex).mData[1]);
					result.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(controlPointIndex).mData[2]);
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					int index = vertexTangent->GetIndexArray().GetAt(controlPointIndex);
					result.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
					result.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
					result.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
				}
				break;
				default:
					DebugErrorMessage("Error: Invalid vertex reference mode!");
			}
			break;
		case FbxGeometryElement::eByPolygonVertex:
			switch ( vertexTangent->GetReferenceMode() )
			{
				case FbxGeometryElement::eDirect:
				{
					result.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(vertexCounter).mData[0]);
					result.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(vertexCounter).mData[1]);
					result.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(vertexCounter).mData[2]);
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					int index = vertexTangent->GetIndexArray().GetAt(vertexCounter);
					result.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
					result.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
					result.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
				}
				break;
				default: DebugErrorMessage("Error: Invalid vertex reference mode!");
			}
			break;
	}
	return result;
}

static DirectX::XMFLOAT3 ReadBinormal(FbxMesh* mesh, int controlPointIndex, int vertexCounter) //vec3 ReadNormal //vec3 RadeBinormal
{
	if ( mesh->GetElementBinormalCount() < 1 )
	{
		DebugErrorMessage(mesh->GetName() << " : Invalid Binormal Number");
	}
	FbxGeometryElementBinormal* vertexBinormal = mesh->GetElementBinormal(0);

	DirectX::XMFLOAT3 result;

	switch ( vertexBinormal->GetMappingMode() )
	{
		case FbxGeometryElement::eByControlPoint:
			switch ( vertexBinormal->GetReferenceMode() )
			{
				case FbxGeometryElement::eDirect:
				{
					result.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(controlPointIndex).mData[0]);
					result.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(controlPointIndex).mData[1]);
					result.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(controlPointIndex).mData[2]);
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					int index = vertexBinormal->GetIndexArray().GetAt(controlPointIndex);
					result.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
					result.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
					result.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
				}
				break;
				default:
					DebugErrorMessage("Error: Invalid vertex reference mode!");
			}
			break;
		case FbxGeometryElement::eByPolygonVertex:
			switch ( vertexBinormal->GetReferenceMode() )
			{
				case FbxGeometryElement::eDirect:
				{
					result.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(vertexCounter).mData[0]);
					result.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(vertexCounter).mData[1]);
					result.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(vertexCounter).mData[2]);
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					int index = vertexBinormal->GetIndexArray().GetAt(vertexCounter);
					result.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
					result.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
					result.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
				}
				break;
				default: DebugErrorMessage("Error: Invalid vertex reference mode!");
			}
			break;
	}
	return result;
}

static DirectX::XMFLOAT3 ReadNormal(FbxMesh* mesh, int controlPointIndex, int vertexCounter) //vec3 ReadNormal //vec3 RadeBinormal
{
	if ( mesh->GetElementNormalCount() < 1 )
	{
		DebugErrorMessage(mesh->GetName() << " : Invalid Normal Number");
	}
	FbxGeometryElementNormal* vertexNormal = mesh->GetElementNormal(0);

	DirectX::XMFLOAT3 result;

	switch ( vertexNormal->GetMappingMode() )
	{
		case FbxGeometryElement::eByControlPoint:
			switch ( vertexNormal->GetReferenceMode() )
			{
				case FbxGeometryElement::eDirect:
				{
					result.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[0]);
					result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[1]);
					result.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[2]);
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					int index = vertexNormal->GetIndexArray().GetAt(controlPointIndex);
					result.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
					result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
					result.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
				}
				break;
				default:
					DebugErrorMessage("Error: Invalid vertex reference mode!");
			}
			break;
		case FbxGeometryElement::eByPolygonVertex:
			switch ( vertexNormal->GetReferenceMode() )
			{
				case FbxGeometryElement::eDirect:
				{
					result.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[0]);
					result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[1]);
					result.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(vertexCounter).mData[2]);
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					int index = vertexNormal->GetIndexArray().GetAt(vertexCounter);
					result.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
					result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
					result.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
				}
				break;
				default: DebugErrorMessage("Error: Invalid vertex reference mode!");
			}
			break;
	}
	return result;
}

static DirectX::XMFLOAT2 ReadUV(FbxMesh* mesh, int controlPointIndex, int i, int j)
{
	controlPointIndex = 0;
	bool doneRead = false;
	DirectX::XMFLOAT2 result = { 0, 0 };

	FbxGeometryElementUV* pfbxElementUV = mesh->GetElementUV(0, FbxLayerElement::eUnknown);
	switch ( pfbxElementUV->GetMappingMode() )
	{
		case FbxGeometryElement::eByControlPoint:
			switch ( pfbxElementUV->GetReferenceMode() )
			{
				case FbxGeometryElement::eDirect:
				{
					auto ref = (pfbxElementUV->GetDirectArray().GetAt(controlPointIndex));
					result.x = static_cast<float>(ref.mData[0]);
					result.y = static_cast<float>(ref.mData[1]);
					doneRead = true;
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					auto ref = (pfbxElementUV->GetDirectArray().GetAt(pfbxElementUV->GetIndexArray().GetAt(controlPointIndex)));
					result.x = static_cast<float>(ref.mData[0]);
					result.y = static_cast<float>(ref.mData[1]);
					doneRead = true;
				}
				break;
				default:
					break;
			}
			break;
		case FbxGeometryElement::eByPolygonVertex:
		{
			int nTextureUVIndex = mesh->GetTextureUVIndex(i, j);
			switch ( pfbxElementUV->GetReferenceMode() )
			{
				case FbxGeometryElement::eDirect:
				case FbxGeometryElement::eIndexToDirect:
				{
					auto ref = (pfbxElementUV->GetDirectArray().GetAt(nTextureUVIndex));
					result.x = static_cast<float>(ref.mData[0]);
					result.y = static_cast<float>(ref.mData[1]);
					doneRead = true;
				}
				break;
				default:
					break;
			}
		}
		break;
		case FbxGeometryElement::eByPolygon:
		case FbxGeometryElement::eAllSame:
		case FbxGeometryElement::eNone:
			break;
		default:
			break;
	}
	if ( !doneRead )
	{
		DebugErrorMessage("UV CAN'T READ");
	}
	result.y = 1 - result.y;
	return result;
}



static int AddMesh(FbxNodeAttribute* attr, std::vector<FbxMesh*>& meshs)
{
	if ( !(attr->GetAttributeType() == FbxNodeAttribute::eMesh) )
	{
		return -1;
	}
	auto* mesh = reinterpret_cast<FbxMesh*>(attr);
	DebugNormalMessage("Mesh Find : " << mesh->GetName() << " : PolyCount : " << mesh->GetPolygonCount());
	auto it = std::find(meshs.begin(), meshs.end(), mesh);
	if ( it == meshs.end() )
	{
		meshs.push_back(mesh);
		return meshs.size() - 1;
	}
	else
	{
		return std::distance(meshs.begin(), it);
	}
}


static KG::Utill::MeshData ConvertMesh(FbxMesh* mesh)
{
	KG::Utill::MeshData data;
	std::vector<UINT> controlPoints;
	std::vector<std::vector<KG::Utill::VertexBoneData>> boneControl;
	int vertexCount = 0;
	int controlpointCount = mesh->GetControlPointsCount();

	auto textureCount = mesh->GetUVLayerCount();
	data.uvs.resize(1);
	//for ( size_t i = 0; i < data.uvs.size(); i++ )
	//{
	//	data.uvs[i].resize( controlpointCount );
	//}
	//data.positions.resize( controlpointCount );
	//data.normals.resize( controlpointCount );
	//data.biTangent.resize( controlpointCount );
	//data.tangent.resize( controlpointCount );

	std::vector<UINT> vtx;
	vtx.reserve(3);
	for ( unsigned int i = 0; i < mesh->GetPolygonCount(); i++ )
	{
		vtx.clear();
		for ( unsigned int j = 0; j < 3; j++ )
		{
			UINT index = mesh->GetPolygonVertex(i, j);
			controlPoints.push_back(index);
			data.positions.push_back(DirectX::XMFLOAT3(mesh->GetControlPointAt(index)[0], mesh->GetControlPointAt(index)[1], mesh->GetControlPointAt(index)[2]));
			data.normals.push_back(ReadNormal(mesh, index, vertexCount));
			data.uvs[0].push_back(ReadUV(mesh, index, i, j));
			if ( mesh->GetElementBinormalCount() < 1 || mesh->GetElementTangentCount() < 1 )
			{
				vtx.push_back(vertexCount);
			}
			else
			{
				data.tangent.push_back(ReadTangent(mesh, index, vertexCount));
				data.biTangent.push_back(ReadBinormal(mesh, index, vertexCount));
			}
			data.indices.push_back(vertexCount);
			vertexCount++;
		}
		if ( !vtx.empty() )
		{
			data.tangent.push_back(DirectX::XMFLOAT3(0, 0, 0));
			data.tangent.push_back(DirectX::XMFLOAT3(0, 0, 0));
			data.tangent.push_back(DirectX::XMFLOAT3(0, 0, 0));
			data.biTangent.push_back(DirectX::XMFLOAT3(0, 0, 0));
			data.biTangent.push_back(DirectX::XMFLOAT3(0, 0, 0));
			data.biTangent.push_back(DirectX::XMFLOAT3(0, 0, 0));
			CalculateTangentBinormal(data, vtx[0], vtx[1], vtx[2]);
		}
	}
	FbxSkin* pfbxSkinDeformer = (FbxSkin*)mesh->GetDeformer(0, FbxDeformer::eSkin);
	if ( pfbxSkinDeformer )
	{
		int nClusters = pfbxSkinDeformer->GetClusterCount();

		FbxAMatrix fbxmtxGeometryOffset = GetGeometricOffsetTransform(mesh->GetNode());

		for ( int j = 0; j < nClusters; j++ )
		{
			FbxCluster* pfbxCluster = pfbxSkinDeformer->GetCluster(j);

			FbxNode* pfbxClusterLinkNode = pfbxCluster->GetLink();

			FbxAMatrix fbxmtxBindPoseMeshToRoot; //Cluster Transform
			pfbxCluster->GetTransformMatrix(fbxmtxBindPoseMeshToRoot);
			FbxAMatrix fbxmtxBindPoseBoneToRoot; //Cluster Link Transform
			pfbxCluster->GetTransformLinkMatrix(fbxmtxBindPoseBoneToRoot);

			FbxAMatrix fbxmtxVertextToLinkNode = fbxmtxBindPoseBoneToRoot.Inverse() * fbxmtxBindPoseMeshToRoot * fbxmtxGeometryOffset;
			//FbxAMatrix fbxmtxVertextToLinkNode = fbxmtxBindPoseBoneToRoot.Inverse() * fbxmtxBindPoseMeshToRoot;

			KG::Utill::BoneData bone;
			bone.nodeId = KG::Utill::HashString(pfbxClusterLinkNode->GetName());
			fbxamatrix_to_xmfloat4x4(fbxmtxVertextToLinkNode, bone.offsetMatrix);
			data.bones.push_back(bone);
		}

		boneControl.resize(controlpointCount);
		for ( int j = 0; j < nClusters; j++ )
		{
			FbxCluster* pfbxCluster = pfbxSkinDeformer->GetCluster(j);

			int* pnControlPointIndices = pfbxCluster->GetControlPointIndices();
			double* pfControlPointWeights = pfbxCluster->GetControlPointWeights();
			int nControlPointIndices = pfbxCluster->GetControlPointIndicesCount();
			for ( int k = 0; k < nControlPointIndices; k++ )
			{
				int nVertex = pnControlPointIndices[k];
				KG::Utill::VertexBoneData vbd;
				vbd.bondId = j;
				vbd.boneWeight = (float)pfControlPointWeights[k];
				boneControl[nVertex].push_back(vbd);
			}
		}

		for ( auto& vertexBone : boneControl )
		{
			//Weight 순으로 정렬
			std::sort(vertexBone.begin(), vertexBone.end(),
				[](const KG::Utill::VertexBoneData& a, const KG::Utill::VertexBoneData& b)
				{
					return a.boneWeight > b.boneWeight;
				}
			);
			//Bone 4개 안되면 4개까지 채우기
			while ( vertexBone.size() < 4 )
			{
				KG::Utill::VertexBoneData emptyData;
				emptyData.bondId = 0;
				emptyData.boneWeight = 0.0f;
				vertexBone.push_back(emptyData);
			}
			//4개로 Weight 제한
			vertexBone.resize(4);
			//4개 Weight 정규화
			float fullWeight = vertexBone[0].boneWeight + vertexBone[1].boneWeight + vertexBone[2].boneWeight + vertexBone[3].boneWeight;
			for ( size_t i = 0; i < 4; i++ )
			{
				vertexBone[i].boneWeight /= fullWeight;
			}
		}
	}
	data.vertexBone.resize(data.positions.size());
	if ( !boneControl.empty() )
	{
		for ( size_t i = 0; i < data.vertexBone.size(); i++ )
		{
			data.vertexBone[i] = boneControl[controlPoints[i]];
		}
	}
	return data;
}

static auto& operator<<(std::ostream& os, const fbxsdk::FbxDouble3 vec)
{
	return os << std::fixed << std::setprecision(8) << vec.mData[0] << ", " << vec.mData[1] << ", " << vec.mData[2];
}

static auto& operator<<(std::ostream& os, const fbxsdk::FbxDouble4 vec)
{
	return os << std::fixed << std::setprecision(8) << vec.mData[0] << ", " << vec.mData[1] << ", " << vec.mData[2] << "," << vec.mData[3];
}

static auto& operator<<(std::ostream& os, const fbxsdk::FbxVector4 vec)
{
	return os << std::fixed << std::setprecision(8) << vec.mData[0] << ", " << vec.mData[1] << ", " << vec.mData[2] << ", " << vec.mData[3];
}

static DirectX::XMVECTOR FORCEINLINE _XMQuaternionRotationXYZ(float x, float y, float z)
{
	using namespace DirectX;

	auto _0 = XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), x);
	auto _1 = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), y);
	auto _2 = XMQuaternionRotationAxis(XMVectorSet(0, 0, 1, 0), z);

	return XMQuaternionMultiply(XMQuaternionMultiply(_0, _1), _2);
}

static KG::Utill::ModelNode* ProcessNode(KG::Utill::ImportData* importData, FbxNode* pFbxNode, std::vector<FbxMesh*>& meshs)
{
	auto& importNode = importData->nodes.emplace_back();
	const char* nodeName = pFbxNode->GetName();
	importNode.name = nodeName;
	importNode.nodeId = KG::Utill::HashString(nodeName);

	DebugNormalMessage("ProcessNode : " << nodeName << " : Processed");

	{
		auto s_t = pFbxNode->LclTranslation.Get();
		auto s_r = pFbxNode->LclRotation.Get();
		auto s_s = pFbxNode->LclScaling.Get();
		auto s_pr = pFbxNode->PreRotation.Get();

		using namespace DirectX;
		XMVECTOR t = XMVectorSet(s_t.mData[0], s_t.mData[1], s_t.mData[2], 1.0f);
		XMVECTOR pr = _XMQuaternionRotationXYZ(
			XMConvertToRadians(s_pr.mData[0]),
			XMConvertToRadians(s_pr.mData[1]),
			XMConvertToRadians(s_pr.mData[2])
		);
		XMVECTOR r = _XMQuaternionRotationXYZ(
			XMConvertToRadians(s_r.mData[0]),
			XMConvertToRadians(s_r.mData[1]),
			XMConvertToRadians(s_r.mData[2])
		);
		XMVECTOR s = XMVectorSet(s_s.mData[0], s_s.mData[1], s_s.mData[2], 1.0f);

		XMStoreFloat3(&importNode.position, t);
		XMStoreFloat4(&importNode.rotation, XMQuaternionMultiply(r, pr));
		XMStoreFloat3(&importNode.scale, s);
	}

	for ( int i = 0; i < pFbxNode->GetNodeAttributeCount(); ++i )
	{
		int id = AddMesh(pFbxNode->GetNodeAttributeByIndex(i), meshs);
		if ( id != -1 )
		{
			importNode.meshs.push_back(id);
		}
	}

	int count = pFbxNode->GetChildCount();
	for ( int i = 0; i < count; ++i )
	{
		importNode.AddChild(ProcessNode(importData, pFbxNode->GetChild(i), meshs));
	}
	return &importNode;
}


static void AddKeyData(std::vector<KG::Utill::KeyData>& keyDataVector, FbxAnimCurve* curve)
{
	if ( curve )
	{
		int keyCount = curve->KeyGetCount();
		for ( size_t i = 0; i < keyCount; i++ )
		{
			FbxTime fbxKeyTime = curve->KeyGetTime(i);
			float fkeyTime = static_cast<float>(fbxKeyTime.GetSecondDouble());
			float fKeyValue = curve->Evaluate(fbxKeyTime);
			//float fKeyValue = curve->Evaluate( fbxKeyTime );
			KG::Utill::KeyData keyData;
			keyData.keyTime = fkeyTime;
			keyData.value = fKeyValue;
			keyDataVector.push_back(keyData);
		}
	}
}

static void ReadTranslationCurve(KG::Utill::NodeAnimation& anim, FbxNode* node, FbxAnimLayer* fbxLayer)
{
	FbxAnimCurve* curve = nullptr;
	//Read X
	curve = node->LclTranslation.GetCurve(fbxLayer, FBXSDK_CURVENODE_COMPONENT_X);
	AddKeyData(anim.translation.x, curve);

	//Read Y
	curve = node->LclTranslation.GetCurve(fbxLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	AddKeyData(anim.translation.y, curve);

	//Read Z
	curve = node->LclTranslation.GetCurve(fbxLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	AddKeyData(anim.translation.z, curve);
}

static void ReadPreRotaionCurve(KG::Utill::NodeAnimation& anim, FbxNode* node, FbxAnimLayer* fbxLayer)
{
	FbxAnimCurve* curve = nullptr;
	//Read X
	curve = node->PreRotation.GetCurve(fbxLayer, FBXSDK_CURVENODE_COMPONENT_X);
	AddKeyData(anim.protation.x, curve);

	//Read Y
	curve = node->PreRotation.GetCurve(fbxLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	AddKeyData(anim.protation.y, curve);

	//Read Z
	curve = node->PreRotation.GetCurve(fbxLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	AddKeyData(anim.protation.z, curve);
}

static void ReadRotaionCurve(KG::Utill::NodeAnimation& anim, FbxNode* node, FbxAnimLayer* fbxLayer)
{
	FbxAnimCurve* curve = nullptr;
	//Read X
	curve = node->LclRotation.GetCurve(fbxLayer, FBXSDK_CURVENODE_COMPONENT_X);
	AddKeyData(anim.rotation.x, curve);

	//Read Y
	curve = node->LclRotation.GetCurve(fbxLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	AddKeyData(anim.rotation.y, curve);

	//Read Z
	curve = node->LclRotation.GetCurve(fbxLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	AddKeyData(anim.rotation.z, curve);
}

static void ReadScaleCurve(KG::Utill::NodeAnimation& anim, FbxNode* node, FbxAnimLayer* fbxLayer)
{
	FbxAnimCurve* curve = nullptr;
	//Read X
	curve = node->LclScaling.GetCurve(fbxLayer, FBXSDK_CURVENODE_COMPONENT_X);
	AddKeyData(anim.scale.x, curve);

	//Read Y
	curve = node->LclScaling.GetCurve(fbxLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	AddKeyData(anim.scale.y, curve);

	//Read Z
	curve = node->LclScaling.GetCurve(fbxLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	AddKeyData(anim.scale.z, curve);
}

static void ReadAnimationCurve(KG::Utill::AnimationLayer& result, FbxNode* node, FbxAnimLayer* fbxLayer)
{
	FbxAnimCurve* curve = nullptr;
	auto& nodeAnim = result.nodeAnimations.emplace_back();
	nodeAnim.nodeId = KG::Utill::HashString(node->GetName());
	auto preRot = node->PreRotation.Get();
	nodeAnim.preRotation = KG::Math::Quaternion::FromXYZEuler(
		DirectX::XMConvertToRadians(preRot.mData[0]),
		DirectX::XMConvertToRadians(preRot.mData[1]),
		DirectX::XMConvertToRadians(preRot.mData[2])
	);
	ReadTranslationCurve(nodeAnim, node, fbxLayer);
	ReadRotaionCurve(nodeAnim, node, fbxLayer);
	ReadScaleCurve(nodeAnim, node, fbxLayer);
	ReadPreRotaionCurve(nodeAnim, node, fbxLayer);
}

static void ProcessAnimationNode(KG::Utill::AnimationLayer& result, FbxNode* pFbxNode, FbxAnimLayer* fbxLayer)
{
	ReadAnimationCurve(result, pFbxNode, fbxLayer);
	for ( size_t i = 0; i < pFbxNode->GetChildCount(); i++ )
	{
		ProcessAnimationNode(result, pFbxNode->GetChild(i), fbxLayer);
	}
}

FbxManager* pFbxManager = nullptr;

void KG::Utill::ImportData::LoadFromPathFBX(const std::string& path)
{
	std::string prePath = (path.substr(0, path.size() - 4) + "_pre.fbx");
	bool isPreProcessed = exists(prePath);
	std::string currentPath = isPreProcessed ? prePath : path;
	DebugNormalMessage("Load FBX From " << currentPath.c_str());
	if( !pFbxManager )  pFbxManager = FbxManager::Create();
	FbxIOSettings* pFbxIOSettings = FbxIOSettings::Create(pFbxManager, IOSROOT);
	pFbxIOSettings->SetBoolProp(IMP_FBX_TEXTURE, false);
	pFbxIOSettings->SetBoolProp(IMP_CAMERA, false);
	pFbxIOSettings->SetBoolProp(IMP_AUDIO, false);
	pFbxIOSettings->SetBoolProp(IMP_LIGHT, false);
	pFbxIOSettings->SetBoolProp(IMP_FBX_CHARACTER, false);
	pFbxIOSettings->SetBoolProp(IMP_FBX_CONSTRAINT, true);
	pFbxIOSettings->SetBoolProp(IMP_FBX_AUDIO, false);

	//FbxIOSettings* pFbxIOSettings = FbxIOSettings::Create( pFbxManager, IOSROOT );
	pFbxManager->SetIOSettings(pFbxIOSettings);

	FbxImporter* pFbxImporter = FbxImporter::Create(pFbxManager, "");
	pFbxImporter->Initialize(currentPath.c_str(), -1, pFbxManager->GetIOSettings());

	pFbxScene = FbxScene::Create(pFbxManager, "SceneName");
	pFbxImporter->Import(pFbxScene);
	pFbxImporter->Destroy();

	if ( !isPreProcessed )
	{
		FbxGeometryConverter conv(pFbxManager);
		conv.RemoveBadPolygonsFromMeshes(pFbxScene, NULL);
		conv.Triangulate(pFbxScene, true);
		conv.SplitMeshesPerMaterial(pFbxScene, true);
		FbxAxisSystem currentAxis = pFbxScene->GetGlobalSettings().GetAxisSystem();
		FbxAxisSystem directXAxis(FbxAxisSystem::eDirectX);
		if ( currentAxis != directXAxis )
		{
			directXAxis.DeepConvertScene(pFbxScene);
		}

		fbxsdk::FbxSystemUnit fbxSceneSystemUnit = pFbxScene->GetGlobalSettings().GetSystemUnit();
		if ( fbxSceneSystemUnit.GetScaleFactor() != 1.0f )
		{
			fbxsdk::FbxSystemUnit::cm.ConvertScene(pFbxScene);
		}
	}

	FbxNode* pFbxRootNode = pFbxScene->GetRootNode();

	int meshCount = pFbxRootNode->GetChildCount();

	std::vector<FbxMesh*> meshes;

	if ( pFbxRootNode )
	{
		this->root = ProcessNode(this, pFbxRootNode, meshes);

		int animStackCount = pFbxScene->GetSrcObjectCount<FbxAnimStack>();
		for ( size_t i = 0; i < animStackCount; i++ )
		{
			FbxAnimStack* animStack = pFbxScene->GetSrcObject<FbxAnimStack>(i);
			auto& animSet = this->animations.emplace_back();
			animSet.animationId = KG::Utill::HashString(animStack->GetName());

			int layerCount = animStack->GetMemberCount<FbxAnimLayer>();
			for ( size_t i = 0; i < layerCount; i++ )
			{
				FbxAnimLayer* layer = animStack->GetMember<FbxAnimLayer>(i);
				auto& result = animSet.layers.emplace_back();
				ProcessAnimationNode(result, pFbxRootNode, layer);
			}
		}
	}
	if ( !isPreProcessed )
	{
		FbxExporter* pFbxExpoter = FbxExporter::Create(pFbxManager, "");
		pFbxExpoter->Initialize(prePath.c_str());
		pFbxExpoter->Export(pFbxScene);
		pFbxExpoter->Destroy();
	}


	for ( size_t i = 0; i < meshes.size(); i++ )
	{
		DebugNormalMessage(meshes[i]->GetName() << " : Load Mesh");
		this->meshs.push_back(ConvertMesh(meshes[i]));
	}
	//pFbxManager->Destroy();

}

DirectX::XMFLOAT4 KG::Utill::ChangeEulerToDxQuat(float x, float y, float z)
{
	FbxQuaternion fbxQuat;
	FbxVector4 a;
	a.Set(x, y, z);
	fbxQuat.ComposeSphericalXYZ(a);
	XMFLOAT4 result = XMFLOAT4(fbxQuat.mData[0], fbxQuat.mData[1], fbxQuat.mData[2], fbxQuat.mData[3]);
	return result;
}

KG::Utill::ImportData::ImportData(ImportData&& other) noexcept
{
	auto otherRootIndex =  other.root - &other.nodes[0];
	this->meshs = std::move(other.meshs);
	this->nodes = std::move(other.nodes);
	this->animations = std::move(other.animations);
	this->root = &this->nodes[otherRootIndex];
}
KG::Utill::ImportData& KG::Utill::ImportData::operator=(ImportData&& other) noexcept
{
	auto otherRootIndex = other.root - &other.nodes[0];
	this->meshs = std::move(other.meshs);
	this->nodes = std::move(other.nodes);
	this->animations = std::move(other.animations);
	this->root = &this->nodes[otherRootIndex];
	return *this;
}