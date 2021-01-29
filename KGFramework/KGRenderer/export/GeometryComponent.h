#pragma once
#include <vector>
#include "IRenderComponent.h"

namespace KG::Renderer
{
	class Geometry;
};

namespace KG::Component
{
	class Render3DComponent;
	class BoneTransformComponent;

	class DLL GeometryComponent : public IRenderComponent
	{
		friend Render3DComponent;
		friend BoneTransformComponent;
		std::vector<KG::Renderer::Geometry*> geometrys;
	public:
		void InitializeGeometry( const KG::Utill::HashString& geometryID, UINT subMeshIndex = 0, UINT slotIndex = 0 );
		bool HasBone() const;
	};
	REGISTER_COMPONENT_ID( GeometryComponent );
};
