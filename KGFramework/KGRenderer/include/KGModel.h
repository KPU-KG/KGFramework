#pragma once
#include "MathHelper.h"
#include "KGGeometry.h"
#include "hash.h"
#include "Assimpimpoter.h"
#include "ResourceMetaData.h"

namespace KG::Resource
{
	using namespace DirectX;

	struct FrameModel
	{
		KG::Utill::ImportData data;
		void CreateFromAssimp( const std::string& fileDir );
		void CreateFromMetadata( const KG::Resource::Metadata::GeometrySetData& metadata );
		FrameModel() = default;
		FrameModel( const KG::Resource::Metadata::GeometrySetData& metadata );
	};
}