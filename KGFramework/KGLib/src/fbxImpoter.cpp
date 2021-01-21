#include "fbxImpoter.h"
#include <fbxsdk.h>
#include <DirectXMathConvert.inl>
#include <DirectXMath.h>
#include <DirectXMathMisc.inl>


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


FbxScene* pFbxScene;

static FbxAMatrix GetGeometricOffsetTransform( FbxNode* pfbxNode )
{
	const FbxVector4 T = pfbxNode->GetGeometricTranslation( FbxNode::eSourcePivot );
	const FbxVector4 R = pfbxNode->GetGeometricRotation( FbxNode::eSourcePivot );
	const FbxVector4 S = pfbxNode->GetGeometricScaling( FbxNode::eSourcePivot );

	return(FbxAMatrix( T, R, S ));
}

static void fbxamatrix_to_xmfloat4x4( const FbxAMatrix& fbxamatrix, DirectX::XMFLOAT4X4& xmfloat4x4 )
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
static void CalculateTangentBinormal( KG::Utill::MeshData& data, UINT i0, UINT i1, UINT i2 )
{
	using namespace DirectX;
	
	static constexpr float EPSILON = 0.0001f;
	static const XMVECTORF32 s_flips = { { { 1.f, -1.f, -1.f, 1.f } } };

	XMVECTOR t0 = XMLoadFloat2( &data.uvs[0][i0] );
	XMVECTOR t1 = XMLoadFloat2( &data.uvs[0][i1] );
	XMVECTOR t2 = XMLoadFloat2( &data.uvs[0][i2] );

	XMVECTOR tangent1[3];
	XMVECTOR tangent2[3];

	XMVECTOR s = XMVectorMergeXY( XMVectorSubtract( t1, t0 ), XMVectorSubtract( t2, t0 ) );

	XMFLOAT4A tmp;
	XMStoreFloat4A( &tmp, s );
	float d = tmp.x * tmp.w - tmp.z * tmp.y;
	d = (fabsf( d ) <= EPSILON) ? 1.f : (1.f / d);
	s = XMVectorScale( s, d );
	s = XMVectorMultiply( s, s_flips );

	XMMATRIX m0;
	m0.r[0] = XMVectorPermute<3, 2, 6, 7>( s, g_XMZero );
	m0.r[1] = XMVectorPermute<1, 0, 4, 5>( s, g_XMZero );
	m0.r[2] = m0.r[3] = g_XMZero;

	XMVECTOR p0 = XMLoadFloat3( &data.positions[i0] );
	XMVECTOR p1 = XMLoadFloat3( &data.positions[i1] );
	XMVECTOR p2 = XMLoadFloat3( &data.positions[i2] );

	XMMATRIX m1;
	m1.r[0] = XMVectorSubtract( p1, p0 );
	m1.r[1] = XMVectorSubtract( p2, p0 );
	m1.r[2] = m1.r[3] = g_XMZero;

	XMMATRIX uv = XMMatrixMultiply( m0, m1 );

	tangent1[0] = XMVectorAdd( tangent1[0], uv.r[0] );
	tangent1[1] = XMVectorAdd( tangent1[1], uv.r[0] );
	tangent1[2] = XMVectorAdd( tangent1[2], uv.r[0] );

	tangent2[0] = XMVectorAdd( tangent2[0], uv.r[1] );
	tangent2[1] = XMVectorAdd( tangent2[1], uv.r[1] );
	tangent2[2] = XMVectorAdd( tangent2[2], uv.r[1] );

	for ( size_t j = 0; j < 3; ++j )
	{
		UINT index[3] = { i0, i1, i2 };
		// Gram-Schmidt orthonormalization
		XMVECTOR b0 = XMLoadFloat3( &data.normals[index[j]] );
		b0 = XMVector3Normalize( b0 );

		XMVECTOR tan1 = tangent1[j];
		XMVECTOR b1 = XMVectorSubtract( tan1, XMVectorMultiply( XMVector3Dot( b0, tan1 ), b0 ) );
		b1 = XMVector3Normalize( b1 );

		XMVECTOR tan2 = tangent2[j];
		XMVECTOR b2 = XMVectorSubtract( XMVectorSubtract( tan2, XMVectorMultiply( XMVector3Dot( b0, tan2 ), b0 ) ), XMVectorMultiply( XMVector3Dot( b1, tan2 ), b1 ) );
		b2 = XMVector3Normalize( b2 );

		// handle degenerate vectors
		float len1 = XMVectorGetX( XMVector3Length( b1 ) );
		float len2 = XMVectorGetY( XMVector3Length( b2 ) );

		if ( (len1 <= EPSILON) || (len2 <= EPSILON) )
		{
			if ( len1 > 0.5f )
			{
				// Reset bi-tangent from tangent and normal
				b2 = XMVector3Cross( b0, b1 );
			}
			else if ( len2 > 0.5f )
			{
				// Reset tangent from bi-tangent and normal
				b1 = XMVector3Cross( b2, b0 );
			}
			else
			{
				// Reset both tangent and bi-tangent from normal
				XMVECTOR axis;

				float d0 = fabsf( XMVectorGetX( XMVector3Dot( g_XMIdentityR0, b0 ) ) );
				float d1 = fabsf( XMVectorGetX( XMVector3Dot( g_XMIdentityR1, b0 ) ) );
				float d2 = fabsf( XMVectorGetX( XMVector3Dot( g_XMIdentityR2, b0 ) ) );
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

				b1 = XMVector3Cross( b0, axis );
				b2 = XMVector3Cross( b0, b1 );
			}
		}
		XMStoreFloat3( &data.tangent[index[j]], b1 );
		XMStoreFloat3( &data.biTangent[index[j]], b2 );
	}
}

static DirectX::XMFLOAT3 ReadTangent( FbxMesh* mesh, int controlPointIndex, int vertexCounter ) //vec3 ReadNormal //vec3 RadeBinormal
{
	if ( mesh->GetElementTangentCount() < 1 )
	{
		DebugErrorMessage( mesh->GetName() << " : Invalid Tangent Number" );
	}
	FbxGeometryElementTangent* vertexTangent = mesh->GetElementTangent( 0 );

	DirectX::XMFLOAT3 result;

	switch ( vertexTangent->GetMappingMode() )
	{
	case FbxGeometryElement::eByControlPoint:
		switch ( vertexTangent->GetReferenceMode() )
		{
		case FbxGeometryElement::eDirect:
		{
			result.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt( controlPointIndex ).mData[0]);
			result.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt( controlPointIndex ).mData[1]);
			result.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt( controlPointIndex ).mData[2]);
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexTangent->GetIndexArray().GetAt( controlPointIndex );
			result.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt( index ).mData[0]);
			result.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt( index ).mData[1]);
			result.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt( index ).mData[2]);
		}
		break;
		default:
			DebugErrorMessage( "Error: Invalid vertex reference mode!" );
		}
		break;
	case FbxGeometryElement::eByPolygonVertex:
		switch ( vertexTangent->GetReferenceMode() )
		{
		case FbxGeometryElement::eDirect:
		{
			result.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt( vertexCounter ).mData[0]);
			result.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt( vertexCounter ).mData[1]);
			result.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt( vertexCounter ).mData[2]);
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexTangent->GetIndexArray().GetAt( vertexCounter );
			result.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt( index ).mData[0]);
			result.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt( index ).mData[1]);
			result.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt( index ).mData[2]);
		}
		break;
		default: DebugErrorMessage( "Error: Invalid vertex reference mode!" );
		}
		break;
	}
	return result;
}

static DirectX::XMFLOAT3 ReadBinormal( FbxMesh* mesh, int controlPointIndex, int vertexCounter ) //vec3 ReadNormal //vec3 RadeBinormal
{
	if ( mesh->GetElementBinormalCount() < 1 )
	{
		DebugErrorMessage( mesh->GetName() << " : Invalid Binormal Number" );
	}
	FbxGeometryElementBinormal* vertexBinormal = mesh->GetElementBinormal( 0 );

	DirectX::XMFLOAT3 result;

	switch ( vertexBinormal->GetMappingMode() )
	{
	case FbxGeometryElement::eByControlPoint:
		switch ( vertexBinormal->GetReferenceMode() )
		{
		case FbxGeometryElement::eDirect:
		{
			result.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt( controlPointIndex ).mData[0]);
			result.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt( controlPointIndex ).mData[1]);
			result.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt( controlPointIndex ).mData[2]);
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt( controlPointIndex );
			result.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt( index ).mData[0]);
			result.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt( index ).mData[1]);
			result.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt( index ).mData[2]);
		}
		break;
		default:
			DebugErrorMessage( "Error: Invalid vertex reference mode!" );
		}
		break;
	case FbxGeometryElement::eByPolygonVertex:
		switch ( vertexBinormal->GetReferenceMode() )
		{
		case FbxGeometryElement::eDirect:
		{
			result.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt( vertexCounter ).mData[0]);
			result.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt( vertexCounter ).mData[1]);
			result.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt( vertexCounter ).mData[2]);
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt( vertexCounter );
			result.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt( index ).mData[0]);
			result.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt( index ).mData[1]);
			result.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt( index ).mData[2]);
		}
		break;
		default: DebugErrorMessage( "Error: Invalid vertex reference mode!" );
		}
		break;
	}
	return result;
}

static DirectX::XMFLOAT3 ReadNormal( FbxMesh* mesh, int controlPointIndex, int vertexCounter ) //vec3 ReadNormal //vec3 RadeBinormal
{
	if (mesh->GetElementNormalCount() < 1)
	{
		DebugErrorMessage( mesh->GetName() << " : Invalid Normal Number" );
	}
	FbxGeometryElementNormal* vertexNormal = mesh->GetElementNormal(0);

	DirectX::XMFLOAT3 result;

	switch (vertexNormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch ( vertexNormal->GetReferenceMode() )
		{
		case FbxGeometryElement::eDirect:
		{
			result.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt( controlPointIndex ).mData[0]);
			result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt( controlPointIndex ).mData[1]);
			result.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt( controlPointIndex ).mData[2]);
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt( controlPointIndex );
			result.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt( index ).mData[0]);
			result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt( index ).mData[1]);
			result.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt( index ).mData[2]);
		}
		break;
		default:
			DebugErrorMessage( "Error: Invalid vertex reference mode!" );
		}
		break;
	case FbxGeometryElement::eByPolygonVertex:
		switch ( vertexNormal->GetReferenceMode() )
		{
		case FbxGeometryElement::eDirect:
		{
			result.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt( vertexCounter ).mData[0]);
			result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt( vertexCounter ).mData[1]);
			result.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt( vertexCounter ).mData[2]);
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt( vertexCounter );
			result.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt( index ).mData[0]);
			result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt( index ).mData[1]);
			result.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt( index ).mData[2]);
		}
		break;
		default: DebugErrorMessage( "Error: Invalid vertex reference mode!" );
		}
		break;
	}
	return result;
}

static DirectX::XMFLOAT2 ReadUV( FbxMesh* mesh, int controlPointIndex, int uvIndex ) //vec3 ReadNormal //vec3 RadeBinormal
{
	if ( mesh->GetElementNormalCount() < 1 )
	{
		DebugErrorMessage( "Invalid ****** Number" );
	}
	FbxGeometryElementUV* vertexNormal = mesh->GetElementUV( 0 );

	DirectX::XMFLOAT2 result = {0,0};

	switch ( vertexNormal->GetMappingMode() )
	{
	case FbxGeometryElement::eByControlPoint:
		switch ( vertexNormal->GetReferenceMode() )
		{
		case FbxGeometryElement::eDirect:
		{
			result.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt( controlPointIndex ).mData[0]);
			result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt( controlPointIndex ).mData[1]);
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt( controlPointIndex );
			result.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt( index ).mData[0]);
			result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt( index ).mData[1]);
		}
		break;
		default:
			DebugErrorMessage( "Error: Invalid vertex reference mode!" );
		}
		break;
	case FbxGeometryElement::eByPolygonVertex:
		switch ( vertexNormal->GetReferenceMode() )
		{
		case FbxGeometryElement::eDirect:
		{
			result.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt( uvIndex ).mData[0]);
			result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt( uvIndex ).mData[1]);
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt( uvIndex );
			result.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt( index ).mData[0]);
			result.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt( index ).mData[1]);
		}
		break;
		default: DebugErrorMessage( "Error: Invalid vertex reference mode!" );
		}
		break;
	}
	float y = 1 - result.y;
	float x = result.x;
	result.x = x;
	result.y = y;
	return result;
}



static int AddMesh( FbxNodeAttribute* attr, std::vector<FbxMesh*>& meshs )
{
	if ( !(attr->GetAttributeType() == FbxNodeAttribute::eMesh) )
	{
		return -1;
	}
	auto* mesh = reinterpret_cast<FbxMesh*>(attr);
	DebugNormalMessage( "Mesh Find : " << mesh->GetName() << " : PolyCount : " << mesh->GetPolygonCount() );
	auto it = std::find( meshs.begin(), meshs.end(), mesh );
	if ( it == meshs.end() )
	{
		meshs.push_back( mesh );
		return meshs.size() - 1;
	}
	else
	{
		return std::distance( meshs.begin(), it );
	}
}


static KG::Utill::MeshData ConvertMesh( FbxMesh* mesh )
{
	KG::Utill::MeshData data;
	int vertexCount = 0;
	int controlpointCount = mesh->GetControlPointsCount();

	auto textureCount = mesh->GetUVLayerCount();
	data.uvs.resize( textureCount );
	for ( size_t i = 0; i < data.uvs.size(); i++ )
	{
		data.uvs[i].resize( controlpointCount );
	}
	data.positions.resize( controlpointCount );
	data.normals.resize( controlpointCount );
	data.biTangent.resize( controlpointCount );
	data.tangent.resize( controlpointCount );

	std::vector<UINT> vtx;
	vtx.reserve( 3 );
	for ( unsigned int i = 0; i < mesh->GetPolygonCount(); i++ )
	{
		vtx.clear();
		for ( unsigned int j = 0; j < 3; j++ )
		{
			UINT index = mesh->GetPolygonVertex( i, j );
			data.indices.push_back( index );
			data.positions[index] = DirectX::XMFLOAT3( mesh->GetControlPointAt( index )[0], mesh->GetControlPointAt( index )[1], mesh->GetControlPointAt( index )[2] );
			data.normals[index] = ReadNormal(mesh, index, vertexCount);
			data.uvs[0][index] = ReadUV( mesh, index, mesh->GetTextureUVIndex( i, j ) );
			if ( mesh->GetElementBinormalCount() < 1 || mesh->GetElementTangentCount() < 1 )
			{
				vtx.push_back( index );
			}
			else 
			{
				data.tangent[index] = ReadTangent( mesh, index, vertexCount );
				data.biTangent[index] = ReadBinormal( mesh, index, vertexCount );
			}
			vertexCount++;
		}
		if ( !vtx.empty() )
		{
			CalculateTangentBinormal( data, vtx[0], vtx[1], vtx[2] );
		}
	}
	FbxSkin* pfbxSkinDeformer = (FbxSkin*)mesh->GetDeformer( 0, FbxDeformer::eSkin );
	if ( pfbxSkinDeformer )
	{
		int nClusters = pfbxSkinDeformer->GetClusterCount();

		FbxAMatrix fbxmtxGeometryOffset = GetGeometricOffsetTransform( mesh->GetNode() );

		for ( int j = 0; j < nClusters; j++ )
		{
			FbxCluster* pfbxCluster = pfbxSkinDeformer->GetCluster( j );

			FbxNode* pfbxClusterLinkNode = pfbxCluster->GetLink();

			FbxAMatrix fbxmtxBindPoseMeshToRoot; //Cluster Transform
			pfbxCluster->GetTransformMatrix( fbxmtxBindPoseMeshToRoot );
			FbxAMatrix fbxmtxBindPoseBoneToRoot; //Cluster Link Transform
			pfbxCluster->GetTransformLinkMatrix( fbxmtxBindPoseBoneToRoot );

			//FbxAMatrix fbxmtxVertextToLinkNode = fbxmtxBindPoseBoneToRoot.Inverse() * fbxmtxBindPoseMeshToRoot * fbxmtxGeometryOffset;
			FbxAMatrix fbxmtxVertextToLinkNode = fbxmtxBindPoseBoneToRoot.Inverse() * fbxmtxBindPoseMeshToRoot * fbxmtxGeometryOffset;

			KG::Utill::BoneData bone;
			bone.nodeId = KG::Utill::HashString( pfbxClusterLinkNode->GetName() );
			fbxamatrix_to_xmfloat4x4( fbxmtxVertextToLinkNode, bone.offsetMatrix );
			data.bones.push_back( bone );
		}

		data.vertexBone.resize( controlpointCount );
		for ( int j = 0; j < nClusters; j++ )
		{
			FbxCluster* pfbxCluster = pfbxSkinDeformer->GetCluster( j );

			int* pnControlPointIndices = pfbxCluster->GetControlPointIndices();
			double* pfControlPointWeights = pfbxCluster->GetControlPointWeights();
			int nControlPointIndices = pfbxCluster->GetControlPointIndicesCount();
			for ( int k = 0; k < nControlPointIndices; k++ )
			{
				int nVertex = pnControlPointIndices[k];
				KG::Utill::VertexBoneData vbd;
				vbd.bondId = j;
				vbd.boneWeight = (float)pfControlPointWeights[k];
				data.vertexBone[nVertex].push_back( vbd );
			}
		}

		for ( auto& vertexBone : data.vertexBone)
		{
			//Weight 순으로 정렬
			std::sort( vertexBone.begin(), vertexBone.end(),
				[]( const KG::Utill::VertexBoneData& a, const KG::Utill::VertexBoneData& b )
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
				vertexBone.push_back( emptyData );
			}
			//4개로 Weight 제한
			vertexBone.resize( 4 );
			//4개 Weight 정규화
			float fullWeight = vertexBone[0].boneWeight + vertexBone[1].boneWeight + vertexBone[2].boneWeight + vertexBone[3].boneWeight;
			for ( size_t i = 0; i < 4; i++ )
			{
				vertexBone[i].boneWeight /= fullWeight;
			}
		}
	}
	return data;
}

static KG::Utill::ModelNode* ProcessNode( KG::Utill::ImportData* importData, FbxNode* pFbxNode, std::vector<FbxMesh*>& meshs )
{
	auto& importNode = importData->nodes.emplace_back();
	const char* nodeName = pFbxNode->GetName();
	importNode.name = nodeName;
	importNode.nodeId = KG::Utill::HashString( nodeName );

	auto trs_fbx = pFbxNode->EvaluateLocalTransform();
	DirectX::XMFLOAT4X4 trs;
	fbxamatrix_to_xmfloat4x4( trs_fbx, trs );

	DirectX::XMVECTOR t = {};
	DirectX::XMVECTOR r = {};
	DirectX::XMVECTOR s = {};

	DirectX::XMMatrixDecompose(&s,&r,&t, DirectX::XMLoadFloat4x4(&trs));
	DirectX::XMStoreFloat3( &importNode.position, t );
	DirectX::XMStoreFloat4( &importNode.rotation, r );
	DirectX::XMStoreFloat3( &importNode.scale, s );

	for ( int i = 0; i < pFbxNode->GetNodeAttributeCount(); ++i )
	{
		int id = AddMesh( pFbxNode->GetNodeAttributeByIndex( i ), meshs );
		if ( id != -1 )
		{
			importNode.meshs.push_back( id );
		}
	}


	int count = pFbxNode->GetChildCount();
	for ( int i = 0; i < count; ++i )
	{
		importNode.AddChild( ProcessNode( importData, pFbxNode->GetChild( i ), meshs ) );
	}
	return &importNode;
}

void KG::Utill::ImportData::LoadFromPathFBX( const std::string& path )
{
	DebugNormalMessage("Load FBX From " << path.c_str());
	FbxManager* pFbxManager = FbxManager::Create();
	FbxIOSettings* pFbxIOSettings = FbxIOSettings::Create( pFbxManager, IOSROOT );
	pFbxManager->SetIOSettings( pFbxIOSettings );

	FbxImporter* pFbxImporter = FbxImporter::Create( pFbxManager, "" );
	pFbxImporter->Initialize( path.c_str(), -1, pFbxManager->GetIOSettings() );

	pFbxScene = FbxScene::Create( pFbxManager, "SceneName" );
	pFbxImporter->Import( pFbxScene );
	pFbxImporter->Destroy();

	FbxAxisSystem directXAxis( FbxAxisSystem::eDirectX );
	directXAxis.DeepConvertScene( pFbxScene );
	FbxGeometryConverter conv( pFbxManager );
	conv.Triangulate( pFbxScene, true );
	//conv.RemoveBadPolygonsFromMeshes( pFbxScene, NULL );
	conv.SplitMeshesPerMaterial( pFbxScene, true );

	//FbxSystemUnit fbxSceneSystemUnit = pFbxScene->GetGlobalSettings().GetSystemUnit();
	//if ( fbxSceneSystemUnit.GetScaleFactor() != 1.0f ) FbxSystemUnit::cm.ConvertScene( pFbxScene );

	FbxNode* pFbxRootNode = pFbxScene->GetRootNode();

	int meshCount = pFbxRootNode->GetChildCount();

	std::vector<FbxMesh*> meshes;

	if ( pFbxRootNode )
	{
		this->root = ProcessNode( this, pFbxRootNode, meshes );
	}

	for ( size_t i = 0; i < meshes.size(); i++ )
	{
		DebugNormalMessage( meshes[i]->GetName() << " : Load Mesh" );
		this->meshs.push_back(ConvertMesh( meshes[i] ));
	}
	pFbxManager->Destroy();
}