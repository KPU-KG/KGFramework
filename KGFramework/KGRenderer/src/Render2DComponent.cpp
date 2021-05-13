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
    rotationAngleProp("Rotation Angle", transform2D.rotationAngle),
    depthProp("Depth", transform2D.depth),
    materialIdProp("Material ID", material2D.materialId),
    leftTopUVProp("LeftTop UV", material2D.leftTopUV),
    rightBottomUVProp("RightBottom UV", material2D.rightBottomUV),
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
        "Progress Shape", this->progress.shape,
        {
            {ProgressShape::RIGHT_LEFT, "RIGHT_LEFT"},
            {ProgressShape::TOP_BOTTOM, "TOP_BOTTOM"},
            {ProgressShape::BOTTOM_TOP, "BOTTOM_TOP"},
            {ProgressShape::CIRCLE_CLOCK, "CIRCLE_CLOCK"},
            {ProgressShape::CIRCLE_RCLOCK, "CIRCLE_RCLOCK"}
        }),
    progressValueProp("Progress Value", this->progress.value)
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
    if ( this->renderJob != nullptr )
    {
        int updateCount = renderJob->GetUpdateCount();
        auto setting = KG::Renderer::KGDXRenderer::GetInstance()->GetSetting();
        float wh = (float)setting.clientHeight / (float)setting.clientWidth;
        auto normalizeSize = this->transform2D.size;
        normalizeSize.x *= wh;

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
}

bool KG::Component::Render2DComponent::GetVisible() const
{
    return false;
}

void KG::Component::Render2DComponent::ReloadRender()
{
    this->materialIndex = KG::Renderer::KGDXRenderer::GetInstance()->QueryMaterialIndex(this->material2D.materialId);
}

void KG::Component::Render2DComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::Render2DComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
}

bool KG::Component::Render2DComponent::OnDrawGUI()
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

KG::Component::RenderSpriteComponent::RenderSpriteComponent()
    :
    positionProp("Position", transform2D.position),
    sizeProp("Size", transform2D.size),
    rotationAngleProp("Rotation Angle", transform2D.rotationAngle),
    depthProp("Depth", transform2D.depth),
    materialIdProp("Material ID", material2D.materialId),
    leftTopUVProp("LeftTop UV", material2D.leftTopUV),
    rightBottomUVProp("RightBottom UV", material2D.rightBottomUV),
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
        "Progress Shape", this->progress.shape,
        {
            {ProgressShape::RIGHT_LEFT, "RIGHT_LEFT"},
            {ProgressShape::TOP_BOTTOM, "TOP_BOTTOM"},
            {ProgressShape::BOTTOM_TOP, "BOTTOM_TOP"},
            {ProgressShape::CIRCLE_CLOCK, "CIRCLE_CLOCK"},
            {ProgressShape::CIRCLE_RCLOCK, "CIRCLE_RCLOCK"}
        }),
    progressValueProp("Progress Value", this->progress.value)
{
}

void KG::Component::RenderSpriteComponent::OnCreate(KG::Core::GameObject* gameObject)
{
    this->transform = gameObject->GetComponent<KG::Component::TransformComponent>();
    this->uiGeometry = KG::Resource::ResourceContainer::GetInstance()->CreateFakeGeometry(D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_POINTLIST, 1);
    this->uiShader = KG::Resource::ResourceContainer::GetInstance()->LoadShader("SpriteDefault"_id);
    this->material2D.materialId = KG::Utill::HashString("SpriteWhite");
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
}

void KG::Component::RenderSpriteComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
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
