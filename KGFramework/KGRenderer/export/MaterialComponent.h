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
	struct MateiralDescs;
	class MaterialComponent;

	struct DLL MateiralDesc
	{
		friend KG::Component::MaterialComponent;
		friend KG::Component::MateiralDescs;
		bool isRawShader = false;
		KG::Utill::HashString materialID = 0;
		UINT slotIndex = 0;
	public:
		MateiralDesc();
		MateiralDesc(bool isRawShader, const KG::Utill::HashString& materialID, UINT slotIndex);
		MateiralDesc(const MateiralDesc& other);
		MateiralDesc(MateiralDesc&& other);
		MateiralDesc& operator=(const MateiralDesc& other);
		MateiralDesc& operator=(MateiralDesc&& other);
	private:
		KG::Core::SerializableProperty<bool> isRawShaderProp;
		KG::Core::SerializableProperty<KG::Utill::HashString> materialIDProp;
		KG::Core::SerializableProperty<UINT> slotIndexProp;

	};

	struct DLL MateiralDescs : public KG::Core::ISerializable
	{
		std::vector<MateiralDesc> descs;
		// ISerializable을(를) 통해 상속됨
		virtual void OnDataLoad(tinyxml2::XMLElement* objectElement) override;

		virtual void OnDataSave(tinyxml2::XMLElement* objectElement) override;

		virtual bool OnDrawGUI() override;

		auto size() const
		{
			return this->descs.size();
		}

		auto& operator[](size_t index)
		{
			return this->descs[index];
		}
	};


	class DLL MaterialComponent : public IRenderComponent
	{
		friend Render3DComponent;
	protected:
		std::vector<KG::Renderer::Shader*> shaders;
		std::vector<KG::Renderer::MaterialConstant*> shaderDatas;
		std::vector<UINT> materialIndexs;
		MateiralDescs materialDescs;
		void InitializeMaterial( const KG::Utill::HashString& materialID, UINT slotIndex = 0 );
		void InitializeShader( const KG::Utill::HashString& shaderID, UINT slotIndex = 0 );
		virtual void OnDestroy() override;
	public:
		MaterialComponent();
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		unsigned GetMaterialIndex( UINT slotIndex = 0 ) const;
		void PostMaterial(const KG::Utill::HashString& materialID, UINT slotIndex = 0);
		void PostShader(const KG::Utill::HashString& shaderID, UINT slotIndex = 0);
		void ReloadMaterial();
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
	};
	REGISTER_COMPONENT_ID( MaterialComponent );
};
