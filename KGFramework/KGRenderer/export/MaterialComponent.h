#pragma once
#include <vector>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
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
		void InitializeMaterial( const KG::Utill::HashString& materialID, UINT slotIndex = 0 );
		void InitializeShader( const KG::Utill::HashString& shaderID, UINT slotIndex = 0 );
		virtual void OnDestroy() override;
	public:
		bool isRawShader = false;
		KG::Utill::HashString materialID;
		UINT slotIndex = 0;

		MaterialComponent();
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		unsigned GetMaterialIndex( UINT slotIndex = 0 ) const;

		//Serialize Part
	private:
		KG::Core::SerializableProperty<bool> isRawShaderProp;
		KG::Core::SerializableProperty<KG::Utill::HashString> materialIDProp;
		KG::Core::SerializableProperty<UINT> slotIndexProp;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
	};
	REGISTER_COMPONENT_ID( MaterialComponent );
};
