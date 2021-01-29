#pragma once
#include <vector>
#include "IRenderComponent.h"

namespace KG::Renderer
{
	class Shader;
	struct MaterialConstant;
};

namespace KG::Component
{
	class Render3DComponent;

	class DLL MaterialComponent : public IRenderComponent
	{
		friend Render3DComponent;
	protected:
		std::vector<KG::Renderer::Shader*> shaders;
		std::vector<KG::Renderer::MaterialConstant*> shaderDatas;
		std::vector<UINT> materialIndexs;
		virtual void OnDestroy() override;
	public:
		void InitializeMaterial( const KG::Utill::HashString& materialID, UINT slotIndex = 0 );
		void InitializeShader( const KG::Utill::HashString& shaderID, UINT slotIndex = 0 );
		unsigned GetMaterialIndex( UINT slotIndex = 0 ) const;
	};
	REGISTER_COMPONENT_ID( MaterialComponent );
};
