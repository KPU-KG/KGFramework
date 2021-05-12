#pragma once
#include <vector>
#include <DirectXMath.h>
#include "IRenderComponent.h"
#include "KGColor.h"

namespace KG::Component
{
    class ICubeCameraComponent;

    enum class RectPivot
    {
        CENTER_TOP, 
        CENTER,
        CENTER_BOTTOM,
        LEFT_TOP,
        LEFT_CENTOR,
        LEFT_BOTTOM,
        RIGHT_TOP,
        RIGHT_CENTOR,
        RIGHT_BOTTOM,
    };

    
    enum class ProgressShape
    { 
        RIGHT_LEFT, //START_END
        TOP_BOTTOM, //START_END
        BOTTOM_TOP, //START_END
        CIRCLE_CLOCK, //SHAPE_DIR
        CIRCLE_RCLOCK //SHAPE_DIR
    };

    struct Transform2D
    {
        DirectX::XMFLOAT2 position = DirectX::XMFLOAT2(0, 0); //-1 ~ 1 정규화된 좌표
        DirectX::XMFLOAT2 size = DirectX::XMFLOAT2(1, 1);
        float rotationAngle = 0;
        float depth = 0;
        RectPivot parentPivot = RectPivot::CENTER;
        RectPivot localPivot = RectPivot::CENTER;
    };
    struct Material2D
    {
        KG::Utill::HashString materialId = KG::Utill::HashString("UIWhite");;
        DirectX::XMFLOAT2 leftTopUV = DirectX::XMFLOAT2(0, 0);
        DirectX::XMFLOAT2 rightBottomUV = DirectX::XMFLOAT2(1,1);
        KG::Utill::Color color = KG::Utill::Color(1,1,1,1);
    };
    struct Progress
    {
        ProgressShape shape = ProgressShape::RIGHT_LEFT;
        float value = 1.0f;
    };

    class DLL IRender2DComponent : public IRenderComponent
    {
        virtual void OnCreate(KG::Core::GameObject* gameObject) override = 0;
    public:
        Transform2D transform2D;
        Material2D material2D;
        Progress progress;
        virtual void OnPreRender() override = 0;
        virtual void SetVisible(bool visible) = 0;
        virtual bool GetVisible() const = 0;
        virtual void ReloadRender() = 0;
    public:
        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) = 0;
        virtual void OnDataSave(tinyxml2::XMLElement* parentElement) = 0;
        virtual bool OnDrawGUI() = 0;
    };
    REGISTER_COMPONENT_ID(IRender2DComponent);
};
