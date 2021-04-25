#pragma once
#include "MathHelper.h"
#include "KGGeometry.h"
#include "hash.h"
#include "fbxImpoter.h"
#include "ResourceMetaData.h"

namespace KG::Resource
{
	using namespace DirectX;

	struct FrameModel
	{
		KG::Utill::ImportData data;
		void CreateFromFBX( const std::string& fileDir );
		void CreateFromMetadata( const KG::Resource::Metadata::GeometrySetData& metadata );
		FrameModel() = default;
		FrameModel(const FrameModel&) = delete;
		FrameModel& operator=(const FrameModel&) = delete;

		FrameModel(FrameModel&& other) noexcept;
		FrameModel& operator=(FrameModel&& other) noexcept;

		FrameModel( const KG::Resource::Metadata::GeometrySetData& metadata );

	};
}