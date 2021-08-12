#include "Render2DComponent.h"
#include "KGRenderQueue.h"
#include "KGDXRenderer.h"
#include "ResourceContainer.h"
#include "Transform.h"
#include "MathHelper.h"

using namespace KG::Math::Literal;

KG::Component::Render2DComponent::Render2DComponent()
    :
    positionProp("Position", transform2D.position),
    sizeProp("Size", transform2D.size),
    rotationAngleProp("RotationAngle", transform2D.rotationAngle),
    depthProp("Depth", transform2D.depth),
    materialIdProp("MaterialID", material2D.materialId),
    leftTopUVProp("LeftTopUV", material2D.leftTopUV),
    rightBottomUVProp("RightBottomUV", material2D.rightBottomUV),
    colorProp("Color", material2D.color),
    parentPivotProp("ParentPivot", this->transform2D.parentPivot,
        {
            { RectPivot::CENTER, "CENTER" },
            { RectPivot::CENTER_TOP, "CENTER_TOP" },
            { RectPivot::CENTER_BOTTOM, "CENTER_BOTTOM" },
            { RectPivot::LEFT_TOP, "LEFT_TOP" },
            { RectPivot::LEFT_CENTOR, "LEFT_CENTOR" },
            { RectPivot::LEFT_BOTTOM, "LEFT_BOTTOM" },
            { RectPivot::RIGHT_TOP, "RIGHT_TOP" },
            { RectPivot::RIGHT_CENTOR, "RIGHT_CENTOR" },
            { RectPivot::RIGHT_BOTTOM, "RIGHT_BOTTOM" }
        }),
    localPivotProp("LocalPivot", this->transform2D.localPivot,
        {
            { RectPivot::CENTER, "CENTER" },
            { RectPivot::CENTER_TOP, "CENTER_TOP" },
            { RectPivot::CENTER_BOTTOM, "CENTER_BOTTOM" },
            { RectPivot::LEFT_TOP, "LEFT_TOP" },
            { RectPivot::LEFT_CENTOR, "LEFT_CENTOR" },
            { RectPivot::LEFT_BOTTOM, "LEFT_BOTTOM" },
            { RectPivot::RIGHT_TOP, "RIGHT_TOP" },
            { RectPivot::RIGHT_CENTOR, "RIGHT_CENTOR" },
            { RectPivot::RIGHT_BOTTOM, "RIGHT_BOTTOM" }
        }),
    progressShapeProp(
        "ProgressShape", this->progress.shape,
        {
            {ProgressShape::RIGHT_LEFT, "RIGHT_LEFT"},
            {ProgressShape::TOP_BOTTOM, "TOP_BOTTOM"},
            {ProgressShape::BOTTOM_TOP, "BOTTOM_TOP"},
            {ProgressShape::CIRCLE_CLOCK, "CIRCLE_CLOCK"},
            {ProgressShape::CIRCLE_RCLOCK, "CIRCLE_RCLOCK"}
        }),
    progressValueProp("ProgressValue", this->progress.value),
    visibleProp("visible", this->visible),
    sourceProp("source", this->transform2D.source)
{
}

void KG::Component::Render2DComponent::OnCreate(KG::Core::GameObject* gameObject)
{
    this->uiGeometry = KG::Resource::ResourceContainer::GetInstance()->CreateFakeGeometry(D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_POINTLIST, 1);
    this->uiShader = KG::Resource::ResourceContainer::GetInstance()->LoadShader("UIDefault"_id);
    this->renderJob = KG::Renderer::KGDXRenderer::GetInstance()->GetRenderEngine()->GetRenderJob(this->uiShader, this->uiGeometry);
    this->renderJob->OnObjectAdd(true);
    this->ReloadRender();
}

void KG::Component::Render2DComponent::OnPreRender()
{
    if ( this->renderJob != nullptr && this->visible )
    {
        int updateCount = renderJob->GetUpdateCount();
        auto setting = KG::Renderer::KGDXRenderer::GetInstance()->GetSetting();
        float wh = (float)setting.clientHeight / (float)setting.clientWidth;
        auto normalizeSize = this->transform2D.size;
        normalizeSize.x *= wh;
        if (transform2D.source.x > transform2D.source.y)
        {
            normalizeSize.x *= transform2D.source.x / transform2D.source.y;
        }
        else 
        {
            normalizeSize.y *= transform2D.source.y / transform2D.source.x;
        }

        renderJob->objectBuffer->mappedData[updateCount].object2d.position = this->transform2D.position;
        renderJob->objectBuffer->mappedData[updateCount].object2d.size = normalizeSize;
        renderJob->objectBuffer->mappedData[updateCount].object2d.leftTopUV = this->material2D.leftTopUV;
        renderJob->objectBuffer->mappedData[updateCount].object2d.rightBottomUV = this->material2D.rightBottomUV;
        renderJob->objectBuffer->mappedData[updateCount].object2d.color = this->material2D.color;
        renderJob->objectBuffer->mappedData[updateCount].object2d.rotationAngle = this->transform2D.rotationAngle;
        renderJob->objectBuffer->mappedData[updateCount].object2d.progressValue = this->progress.value;
        renderJob->objectBuffer->mappedData[updateCount].object2d.depth = this->transform2D.depth;
        renderJob->objectBuffer->mappedData[updateCount].object2d.parentPivot = static_cast<UINT>(this->transform2D.parentPivot);
        renderJob->objectBuffer->mappedData[updateCount].object2d.localPivot = static_cast<UINT>(this->transform2D.localPivot);
        renderJob->objectBuffer->mappedData[updateCount].object2d.progressShape = static_cast<UINT>(this->progress.shape);
        renderJob->objectBuffer->mappedData[updateCount].object2d.materialIndex = this->materialIndex;
    }
}

void KG::Component::Render2DComponent::SetVisible(bool visible)
{
    this->visible = visible;
}

bool KG::Component::Render2DComponent::GetVisible() const
{
    return this->visible;
}

void KG::Component::Render2DComponent::ReloadRender()
{
    this->materialIndex = KG::Renderer::KGDXRenderer::GetInstance()->QueryMaterialIndex(this->material2D.materialId);
}

void KG::Component::Render2DComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
    visibleProp.OnDataLoad(componentElement);

    positionProp.OnDataLoad(componentElement);
    sizeProp.OnDataLoad(componentElement);
    sourceProp.OnDataLoad(componentElement);
    rotationAngleProp.OnDataLoad(componentElement);
    depthProp.OnDataLoad(componentElement);
    parentPivotProp.OnDataLoad(componentElement);
    localPivotProp.OnDataLoad(componentElement);

    //Material2D
    materialIdProp.OnDataLoad(componentElement);
    leftTopUVProp.OnDataLoad(componentElement);
    rightBottomUVProp.OnDataLoad(componentElement);
    colorProp.OnDataLoad(componentElement);

    //Progress
    progressShapeProp.OnDataLoad(componentElement);
    progressValueProp.OnDataLoad(componentElement);
    this->ReloadRender();
}

void KG::Component::Render2DComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
    auto* componentElement = parentElement->InsertNewChildElement("Component");
    ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::Render2DComponent);
    visibleProp.OnDataSave(componentElement);

    positionProp.OnDataSave(componentElement);
    sizeProp.OnDataSave(componentElement);
    sourceProp.OnDataSave(componentElement);
    rotationAngleProp.OnDataSave(componentElement);
    depthProp.OnDataSave(componentElement);
    parentPivotProp.OnDataSave(componentElement);
    localPivotProp.OnDataSave(componentElement);

    //Material2D
    materialIdProp.OnDataSave(componentElement);
    leftTopUVProp.OnDataSave(componentElement);
    rightBottomUVProp.OnDataSave(componentElement);
    colorProp.OnDataSave(componentElement);

    //Progress
    progressShapeProp.OnDataSave(componentElement);
    progressValueProp.OnDataSave(componentElement);
}

bool KG::Component::Render2DComponent::OnDrawGUI()
{
    if ( ImGui::ComponentHeader<Render2DComponent>() )
    {
        visibleProp.OnDrawGUI();
        if ( ImGui::TreeNode("Transform2D") )
        {
            //this->positionProp.OnDrawGUI();
            ImGui::DragFloat2("position", (float*)&(this->transform2D.position), 0.05f);
            this->sizeProp.OnDrawGUI();
            this->sourceProp.OnDrawGUI();
            this->rotationAngleProp.OnDrawGUI();
            this->depthProp.OnDrawGUI();
            this->parentPivotProp.OnDrawGUI();
            this->localPivotProp.OnDrawGUI();
            ImGui::TreePop();
        }
        if ( ImGui::TreeNode("Material2D") )
        {
            this->materialIdProp.OnDrawGUI();
            this->leftTopUVProp.OnDrawGUI();
            this->rightBottomUVProp.OnDrawGUI();
            this->colorProp.OnDrawGUI();
            ImGui::TreePop();
        }
        if ( ImGui::TreeNode("Progress") )
        {
            this->progressShapeProp.OnDrawGUI();
            this->progressValueProp.OnDrawGUI();
            ImGui::TreePop();
        }
        if ( ImGui::Button("Reload") )
        {
            this->ReloadRender();
        }
    }
    return false;
}

KG::Component::RenderSpriteComponent::RenderSpriteComponent()
    :
    positionProp("Position", transform2D.position),
    sizeProp("Size", transform2D.size),
    rotationAngleProp("RotationAngle", transform2D.rotationAngle),
    depthProp("Depth", transform2D.depth),
    materialIdProp("MaterialID", material2D.materialId),
    leftTopUVProp("LeftTopUV", material2D.leftTopUV),
    rightBottomUVProp("RightBottomUV", material2D.rightBottomUV),
    colorProp("Color", material2D.color),
    parentPivotProp("ParentPivot", this->transform2D.parentPivot,
        {
            { RectPivot::CENTER, "CENTER" },
            { RectPivot::CENTER_TOP, "CENTER_TOP" },
            { RectPivot::CENTER_BOTTOM, "CENTER_BOTTOM" },
            { RectPivot::LEFT_TOP, "LEFT_TOP" },
            { RectPivot::LEFT_CENTOR, "LEFT_CENTOR" },
            { RectPivot::LEFT_BOTTOM, "LEFT_BOTTOM" },
            { RectPivot::RIGHT_TOP, "RIGHT_TOP" },
            { RectPivot::RIGHT_CENTOR, "RIGHT_CENTOR" },
            { RectPivot::RIGHT_BOTTOM, "RIGHT_BOTTOM" }
        }),
    localPivotProp("LocalPivot", this->transform2D.localPivot,
        {
            { RectPivot::CENTER, "CENTER" },
            { RectPivot::CENTER_TOP, "CENTER_TOP" },
            { RectPivot::CENTER_BOTTOM, "CENTER_BOTTOM" },
            { RectPivot::LEFT_TOP, "LEFT_TOP" },
            { RectPivot::LEFT_CENTOR, "LEFT_CENTOR" },
            { RectPivot::LEFT_BOTTOM, "LEFT_BOTTOM" },
            { RectPivot::RIGHT_TOP, "RIGHT_TOP" },
            { RectPivot::RIGHT_CENTOR, "RIGHT_CENTOR" },
            { RectPivot::RIGHT_BOTTOM, "RIGHT_BOTTOM" }
        }),
    progressShapeProp(
        "ProgressShape", this->progress.shape,
        {
            {ProgressShape::RIGHT_LEFT, "RIGHT_LEFT"},
            {ProgressShape::TOP_BOTTOM, "TOP_BOTTOM"},
            {ProgressShape::BOTTOM_TOP, "BOTTOM_TOP"},
            {ProgressShape::CIRCLE_CLOCK, "CIRCLE_CLOCK"},
            {ProgressShape::CIRCLE_RCLOCK, "CIRCLE_RCLOCK"}
        }),
    progressValueProp("ProgressValue", this->progress.value)
{
}

void KG::Component::RenderSpriteComponent::OnCreate(KG::Core::GameObject* gameObject)
{
    this->transform = gameObject->GetComponent<KG::Component::TransformComponent>();
    this->uiGeometry = KG::Resource::ResourceContainer::GetInstance()->CreateFakeGeometry(D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_POINTLIST, 1);
    this->uiShader = KG::Resource::ResourceContainer::GetInstance()->LoadShader("SpriteDefault"_id);
    //this->material2D.materialId = KG::Utill::HashString("SpriteWhite");
    this->renderJob = KG::Renderer::KGDXRenderer::GetInstance()->GetRenderEngine()->GetRenderJob(this->uiShader, this->uiGeometry);
    this->renderJob->OnObjectAdd(true);
    this->ReloadRender();
}

void KG::Component::RenderSpriteComponent::OnPreRender()
{
    if ( this->renderJob != nullptr )
    {
        int updateCount = renderJob->GetUpdateCount();
        auto setting = KG::Renderer::KGDXRenderer::GetInstance()->GetSetting();
        float wh = (float)setting.clientHeight / (float)setting.clientWidth;
        auto newPosition = this->transform->GetWorldPosition() + DirectX::XMFLOAT3(this->transform2D.position.x, this->transform2D.position.y, 0.0);
        renderJob->objectBuffer->mappedData[updateCount].object2d.position = DirectX::XMFLOAT2(newPosition.x, newPosition.y);
        renderJob->objectBuffer->mappedData[updateCount].object2d.size = this->transform2D.size;
        renderJob->objectBuffer->mappedData[updateCount].object2d.leftTopUV = this->material2D.leftTopUV;
        renderJob->objectBuffer->mappedData[updateCount].object2d.rightBottomUV = this->material2D.rightBottomUV;
        renderJob->objectBuffer->mappedData[updateCount].object2d.color = this->material2D.color;
        renderJob->objectBuffer->mappedData[updateCount].object2d.rotationAngle = this->transform2D.rotationAngle;
        renderJob->objectBuffer->mappedData[updateCount].object2d.progressValue = this->progress.value;
        renderJob->objectBuffer->mappedData[updateCount].object2d.depth = newPosition.z;
        renderJob->objectBuffer->mappedData[updateCount].object2d.parentPivot = static_cast<UINT>(this->transform2D.parentPivot);
        renderJob->objectBuffer->mappedData[updateCount].object2d.localPivot = static_cast<UINT>(this->transform2D.localPivot);
        renderJob->objectBuffer->mappedData[updateCount].object2d.progressShape = static_cast<UINT>(this->progress.shape);
        renderJob->objectBuffer->mappedData[updateCount].object2d.materialIndex = this->materialIndex;
    }
}

void KG::Component::RenderSpriteComponent::SetVisible(bool visible)
{
}

bool KG::Component::RenderSpriteComponent::GetVisible() const
{
    return false;
}

void KG::Component::RenderSpriteComponent::ReloadRender()
{
    this->materialIndex = KG::Renderer::KGDXRenderer::GetInstance()->QueryMaterialIndex(this->material2D.materialId);
}

void KG::Component::RenderSpriteComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
    //visibleProp.OnDataLoad(componentElement);

    positionProp.OnDataLoad(componentElement);
    sizeProp.OnDataLoad(componentElement);
    //sourceProp.OnDataLoad(componentElement);
    rotationAngleProp.OnDataLoad(componentElement);
    depthProp.OnDataLoad(componentElement);
    parentPivotProp.OnDataLoad(componentElement);
    localPivotProp.OnDataLoad(componentElement);

    //Material2D
    materialIdProp.OnDataLoad(componentElement);
    leftTopUVProp.OnDataLoad(componentElement);
    rightBottomUVProp.OnDataLoad(componentElement);
    colorProp.OnDataLoad(componentElement);

    //Progress
    progressShapeProp.OnDataLoad(componentElement);
    progressValueProp.OnDataLoad(componentElement);
    this->ReloadRender();
}

void KG::Component::RenderSpriteComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
    auto* componentElement = parentElement->InsertNewChildElement("Component");
    ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::RenderSpriteComponent);
    //visibleProp.OnDataSave(componentElement);

    positionProp.OnDataSave(componentElement);
    sizeProp.OnDataSave(componentElement);
    //sourceProp.OnDataSave(componentElement);
    rotationAngleProp.OnDataSave(componentElement);
    depthProp.OnDataSave(componentElement);
    parentPivotProp.OnDataSave(componentElement);
    localPivotProp.OnDataSave(componentElement);

    //Material2D
    materialIdProp.OnDataSave(componentElement);
    leftTopUVProp.OnDataSave(componentElement);
    rightBottomUVProp.OnDataSave(componentElement);
    colorProp.OnDataSave(componentElement);

    //Progress
    progressShapeProp.OnDataSave(componentElement);
    progressValueProp.OnDataSave(componentElement);
}

bool KG::Component::RenderSpriteComponent::OnDrawGUI()
{
    if ( ImGui::ComponentHeader<Render2DComponent>() )
    {
        if ( ImGui::TreeNode("Transform2D") )
        {
            this->positionProp.OnDrawGUI();
            this->sizeProp.OnDrawGUI();
            this->rotationAngleProp.OnDrawGUI();
            this->depthProp.OnDrawGUI();
            this->parentPivotProp.OnDrawGUI();
            this->localPivotProp.OnDrawGUI();
            ImGui::TreePop();
        }
        if ( ImGui::TreeNode("Material2D") )
        {
            this->materialIdProp.OnDrawGUI();
            this->leftTopUVProp.OnDrawGUI();
            this->rightBottomUVProp.OnDrawGUI();
            this->colorProp.OnDrawGUI();
            ImGui::TreePop();
        }
        if ( ImGui::TreeNode("Progress") )
        {
            this->progressShapeProp.OnDrawGUI();
            this->progressValueProp.OnDrawGUI();
            ImGui::TreePop();
        }
        if ( ImGui::Button("Reload") )
        {
            this->ReloadRender();
        }
    }
    return false;
}
