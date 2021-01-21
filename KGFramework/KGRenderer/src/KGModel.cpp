#include "KGModel.h"
#include "fbxImpoter.h"

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
