#include "KGModel.h"
#include "AssimpImpoter.h"

KG::Resource::FrameModel::FrameModel( const KG::Resource::Metadata::GeometrySetData& metadata )
{
	CreateFromMetadata( metadata );
}

void KG::Resource::FrameModel::CreateFromAssimp( const std::string& fileDir )
{
	this->data.LoadFromPathAssimp( fileDir );
}

void KG::Resource::FrameModel::CreateFromMetadata( const KG::Resource::Metadata::GeometrySetData& metadata )
{
	data.LoadFromPathAssimp( metadata.fileDir );
}
