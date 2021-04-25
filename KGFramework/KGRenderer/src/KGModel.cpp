#include "KGModel.h"
#include "fbxImpoter.h"

KG::Resource::FrameModel::FrameModel(FrameModel&& other) noexcept
	:data(std::move(other.data))
{
}

KG::Resource::FrameModel& KG::Resource::FrameModel::operator=(FrameModel&& other) noexcept
{
	this->data = std::move(other.data);
	return *this;
}

KG::Resource::FrameModel::FrameModel( const KG::Resource::Metadata::GeometrySetData& metadata )
{
	CreateFromMetadata( metadata );
}

void KG::Resource::FrameModel::CreateFromFBX( const std::string& fileDir )
{
	this->data.LoadFromPathFBX( fileDir );
}

void KG::Resource::FrameModel::CreateFromMetadata( const KG::Resource::Metadata::GeometrySetData& metadata )
{
	this->data.LoadFromPathFBX( metadata.fileDir );
}
