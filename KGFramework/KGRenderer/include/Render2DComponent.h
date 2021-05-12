#pragma once
#include <vector>
#include "IRenderComponent.h"
#include "IRender2DComponent.h"
#include "IDXRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"

namespace KG::Renderer
{
    struct KGRenderJob;
    class Geometry;
    class Shader;
};

namespace KG::Component
{
    class TransformComponent;
    class RenderSpriteComponent : public IRenderSpriteComponent, IDXRenderComponent
    {
        KG::Renderer::KGRenderJob* renderJob = nullptr;
        UINT materialIndex = 0;
        KG::Renderer::Geometry* uiGeometry = nullptr;
        KG::Renderer::Shader* uiShader = nullptr;
        KG::Component::TransformComponent* transform = nullptr;
        //Transform2D
        KG::Core::SerializableProperty<DirectX::XMFLOAT2> positionProp;
        KG::Core::SerializableProperty<DirectX::XMFLOAT2> sizeProp;
        KG::Core::SerializableProperty<float> rotationAngleProp;
        KG::Core::SerializableProperty<float> depthProp;
        KG::Core::SerializableEnumProperty<RectPivot> parentPivotProp;
        KG::Core::SerializableEnumProperty<RectPivot> localPivotProp;

        //Material2D
        KG::Core::SerializableProperty<KG::Utill::HashString> materialIdProp;
        KG::Core::SerializableProperty<DirectX::XMFLOAT2> leftTopUVProp;
        KG::Core::SerializableProperty<DirectX::XMFLOAT2> rightBottomUVProp;
        KG::Core::SerializableProperty<KG::Utill::Color> colorProp;

        //Progress
        KG::Core::SerializableEnumProperty<ProgressShape> progressShapeProp;
        KG::Core::SerializableProperty<float> progressValueProp;

    public:
        RenderSpriteComponent();
        virtual void OnCreate(KG::Core::GameObject* gameObject) override;
        virtual void OnPreRender() override;
        virtual void SetVisible(bool visible) override;
        virtual bool GetVisible() const override;
        virtual void ReloadRender() override;
        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
        virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
        virtual bool OnDrawGUI() override;
    };
    REGISTER_COMPONENT_ID_REPLACE(RenderSpriteComponent, IRenderSpriteComponent);

    class Render2DComponent : public IRender2DComponent, IDXRenderComponent
    {
        KG::Renderer::KGRenderJob* renderJob = nullptr;
        UINT materialIndex = 0;
        KG::Renderer::Geometry* uiGeometry = nullptr;
        KG::Renderer::Shader* uiShader = nullptr;
        //Transform2D
        KG::Core::SerializableProperty<DirectX::XMFLOAT2> positionProp;
        KG::Core::SerializableProperty<DirectX::XMFLOAT2> sizeProp;
        KG::Core::SerializableProperty<float> rotationAngleProp;
        KG::Core::SerializableProperty<float> depthProp;
        KG::Core::SerializableEnumProperty<RectPivot> parentPivotProp;
        KG::Core::SerializableEnumProperty<RectPivot> localPivotProp;

        //Material2D
        KG::Core::SerializableProperty<KG::Utill::HashString> materialIdProp;
        KG::Core::SerializableProperty<DirectX::XMFLOAT2> leftTopUVProp;
        KG::Core::SerializableProperty<DirectX::XMFLOAT2> rightBottomUVProp;
        KG::Core::SerializableProperty<KG::Utill::Color> colorProp;

        //Progress
        KG::Core::SerializableEnumProperty<ProgressShape> progressShapeProp;
        KG::Core::SerializableProperty<float> progressValueProp;

    public:
        Render2DComponent();
        virtual void OnCreate(KG::Core::GameObject* gameObject) override;
        virtual void OnPreRender() override;
        virtual void SetVisible(bool visible) override;
        virtual bool GetVisible() const override;
        virtual void ReloadRender() override;
        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
        virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
        virtual bool OnDrawGUI() override;
    };
    REGISTER_COMPONENT_ID_REPLACE(Render2DComponent, IRender2DComponent);
};
