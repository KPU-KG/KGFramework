#include "LobbyUIComponent.h"
#include "ImguiHelper.h"
#include "InputManager.h"
#include "IRender2DComponent.h"
#include "KGFramework.h"
#include "MathHelper.h"
#include "Scene.h"

#include <iostream>


using namespace KG::Math::Literal;

namespace KG::Component
{
    struct _Anim
    {
        KG::Component::IRender2DComponent* obj;
        _Anim(KG::Component::IRender2DComponent* obj)
            :obj(obj)
        {
        }

        auto& ON()
        {
            obj->SetVisible(true);
            obj->material2D.color.a = 1.0f;
            return *this;
        }

        auto& POSITION(float x, float y)
        {
            obj->transform2D.position = XMFLOAT2(x, y);
            return *this;
        }

        auto& MOVE(float x, float y, float current, float end, float start = 0)
        {
            if (current >= start && end >= current)
            {
                auto& p = obj->transform2D.position;
                p = Math::Lerp(p, XMFLOAT2(x, y), (current - start) / (end - start));
            }
            return *this;
        }

        auto& OFF()
        {
            obj->SetVisible(false);
            obj->material2D.color.a = 0.0f;
            return *this;
        }

        auto& FADEON(float current, float end, float start = 0)
        {
            obj->SetVisible(true);
            auto& a = obj->material2D.color.a;
            a = Math::Clamp(Math::Lerp(a, 1.0f, (current - start) / (end - start)), 0.0f, 1.0f);
            return *this;
        }

        auto& FADEOFF(float current, float end, float start = 0)
        {
            auto& a = obj->material2D.color.a;
            a = Math::Clamp(Math::Lerp(a, 0.0f, (current - start) / (end - start)), 0.0f, 1.0f);
            if (a < 0.05f)
            {
                a = 0.0f;
                obj->SetVisible(false);
            }
            return *this;
        }

        auto& COLOR(float r, float g, float b)
        {
            auto& p = obj->material2D.color;
            p.SetByFloat(r, g, b);
            return *this;
        }

        auto& ALPHA(float a)
        {
            auto& p = obj->material2D.color;
            p.a = a;
            return *this;
        }

        auto& FADECOLOR(float r, float g, float b, float current, float end, float start = 0)
        {
            if (current >= start && end >= current)
            {
                auto& p = obj->material2D.color;
                XMFLOAT4 color = Math::Lerp(XMFLOAT4(p.r, p.g, p.b, p.a), XMFLOAT4(r,g,b, p.a), (current - start) / (end - start));
                p.SetByFloat(color.x, color.y, color.z, color.w);
            }
            return *this;
        }
        auto& FADECOLOR(float r, float g, float b, float a, float current, float end, float start = 0)
        {
            if (current >= start && end >= current)
            {
                auto& p = obj->material2D.color;
                XMFLOAT4 color = Math::Lerp(XMFLOAT4(p.r, p.g, p.b, p.a), XMFLOAT4(r, g, b, a), (current - start) / (end - start));
                p.SetByFloat(color.x, color.y, color.z, color.w);
            }
            return *this;
        }
    };
    struct LobbyPimpl
    {
        KG::Core::Scene* scene;
        std::unordered_map<KG::Utill::hashType, KG::Component::IRender2DComponent*> objs;
        std::pair<KG::Utill::hashType, KG::Component::IRender2DComponent*> lastResult;
        std::unordered_map<KG::Utill::hashType, std::function<KG::Utill::hashType(float, float)>> fsmMap;
        KG::Utill::hashType currentStateId;
        float currentStateTime = 0.0f;

        static constexpr KG::Utill::hashType playerTitle[] = { "PLAYER1"_id, "PLAYER2"_id,"PLAYER3"_id,"PLAYER4"_id };
        static constexpr KG::Utill::hashType playerWait[] = { "WAIT1"_id, "WAIT2"_id,"WAIT3"_id,"WAIT4"_id };
        static constexpr KG::Utill::hashType playerReady[] = { "RD1"_id, "RD2"_id,"RD3"_id,"RD4"_id };

        auto Anim(KG::Utill::hashType id)
        {
            return _Anim(objs[id]);
        }

        void Init(KG::Core::Scene* s)
        {
            this->scene = s;
            currentStateId = "start"_id;
            // add list
            objs["btn_start"_id] = this->scene->FindObjectWithTag("btn_start"_id)->GetComponent<KG::Component::IRender2DComponent>();
            objs["title"_id] = this->scene->FindObjectWithTag("Title"_id)->GetComponent<KG::Component::IRender2DComponent>();
            objs["lobbyTitle"_id] = this->scene->FindObjectWithTag("lobbyTitle"_id)->GetComponent<KG::Component::IRender2DComponent>();

            for (size_t i = 0; i < 4; i++)
            {
                auto id = playerTitle[i]; objs[id] = this->scene->FindObjectWithTag(id)->GetComponent<KG::Component::IRender2DComponent>();
                id = playerWait[i]; objs[id] = this->scene->FindObjectWithTag(id)->GetComponent<KG::Component::IRender2DComponent>();
                id = playerReady[i]; objs[id] = this->scene->FindObjectWithTag(id)->GetComponent<KG::Component::IRender2DComponent>();
            }

            // add fsm
            fsmMap["start"_id] = [this](float delta, float current) -> KG::Utill::hashType
            {
                if (current < 1.5f)
                {
                    Anim("title"_id).POSITION(0, 0.4f).FADEON(current, 1.5f);
                    Anim("btn_start"_id).POSITION(0, -0.4f).FADEON(current, 1.5f);
                    Anim("lobbyTitle"_id).OFF();
                    for (size_t i = 0; i < 4; i++)
                    {
                        Anim(playerWait[i]).OFF().COLOR(0,0,0);
                        Anim(playerTitle[i]).OFF().COLOR(0, 0, 0);
                        Anim(playerReady[i]).OFF().COLOR(0, 0, 0);
                    }
                    return 0;
                }
                else
                {
                    return "title"_id;
                }
            };
            fsmMap["title"_id] = [this](float delta, float current) -> KG::Utill::hashType
            {
                if (this->IsClicked("btn_start"_id))
                {
                    std::cout << "btnStart" << std::endl;
                    return "change_lobby"_id;
                }
                return 0;
            };
            fsmMap["change_lobby"_id] = [this](float delta, float current) -> KG::Utill::hashType
            {
                float end1 = 2.0f;
                if (current <= end1)
                {
                    Anim("btn_start"_id).MOVE(0.650f, -0.75f,current, end1);
                    Anim("title"_id).MOVE(-0.350f, 0.75f, current, end1);
                    Anim("lobbyTitle"_id).FADEON(current, end1);
                    for (size_t i = 0; i < 4; i++)
                    {
                        Anim(playerWait[i]).FADEON(current, end1).FADECOLOR(0.25f, 0.25f, 0.25f, current, end1);
                        Anim(playerTitle[i]).FADEON(current, end1).FADECOLOR(0.25f, 0.25f, 0.25f, current, end1);
                        Anim(playerReady[i]).FADEON(current, end1).FADECOLOR(0.25f, 0.25f, 0.25f, current, end1);
                    }
                }
                else 
                {
                    return "lobby"_id;
                }
                return 0;
            };
            fsmMap["lobby"_id] = [this](float delta, float current) -> KG::Utill::hashType
            {
                for (size_t i = 0; i < 4; i++)
                {
                    float b = 0.25f + 0.1f * sinf(current * 3);
                    Anim(playerWait[i]).COLOR(b, b, b);
                    Anim(playerTitle[i]).COLOR(b, b, b);
                    Anim(playerReady[i]).COLOR(b, b, b);
                }
                if (this->IsClicked("btn_start"_id))
                {
                    return "start"_id;
                }
                return 0;
            };

            fsmMap["ingame"_id] = [this](float delta, float current) -> KG::Utill::hashType
            {
                Anim("title"_id).OFF();
                Anim("btn_start"_id).OFF();
                for (size_t i = 0; i < 4; i++)
                {
                    Anim(playerWait[i]).OFF();
                    Anim(playerTitle[i]).OFF();
                    Anim(playerReady[i]).OFF();
                }
                return 0;
            };
        }

        void FSMUpdate(float deltaTime)
        {
            CheckClicked();
            currentStateTime += deltaTime;
            auto result = fsmMap[currentStateId](deltaTime, currentStateTime);
            if (result == 0 || result == currentStateId) return;
            else {
                currentStateId = result;
                currentStateTime = 0.0f;
            }
        }
        void CheckClicked()
        {
            auto* input = KG::Input::InputManager::GetInputManager();
            if (input->GetKeyState(VK_LBUTTON) == KG::Input::KeyState::Down)
            {
                auto setting = KG::GameFramework::setting;
                float x = input->GetCurrentMousePosition().x;
                float y = input->GetCurrentMousePosition().y;
                x = (x / float(setting.clientWidth)) * 2.0f - 1.0f;
                y = ((y / float(setting.clientHeight)) * 2.0f - 1.0f) * -1;
                std::cout << "Clicked : " << x << ", " << y << std::endl;
                lastResult = GetPositionObject(x, y);
            }
            else
            {
                lastResult = { 0, 0 };
            }
        }
        bool IsClicked(KG::Utill::hashType id)
        {
            return lastResult.first == id;
        }
        std::pair<KG::Utill::hashType, KG::Component::IRender2DComponent*> GetPositionObject(float x, float y)
        {
            std::pair<KG::Utill::hashType, KG::Component::IRender2DComponent*> result = { 0, 0 };
            for (auto pair : objs)
            {
                auto& id = pair.first;
                auto& ptr = pair.second;
                auto setting = KG::GameFramework::setting;
                float wh = (float)setting.clientHeight / (float)setting.clientWidth;
                auto parentPivot = KG::Component::rectPivots[int(ptr->transform2D.parentPivot)];
                auto localPivot = KG::Component::rectPivots[int(ptr->transform2D.localPivot)];
                auto position = ptr->transform2D.position;
                auto size = ptr->transform2D.size;
                size.x *= wh;
                if (ptr->transform2D.source.x > ptr->transform2D.source.y)
                {
                    size.x *= ptr->transform2D.source.x / ptr->transform2D.source.y;
                }
                else
                {
                    size.y *= ptr->transform2D.source.y / ptr->transform2D.source.x;
                }
                float ty, tx, by, bx;
                DirectX::XMFLOAT2 animatedPosition = parentPivot - (localPivot * (size * 0.5f)) + position;
                tx = animatedPosition.x + (size.x * 0.5f);
                bx = animatedPosition.x - (size.x * 0.5f);
                ty = animatedPosition.y + (size.y * 0.5f);
                by = animatedPosition.y - (size.y * 0.5f);
                if (tx > x && x > bx && ty > y && y > by && ptr->GetVisible())
                {
                    if (!result.second || result.second->transform2D.depth > ptr->transform2D.depth)
                        result = pair;
                }
            }
            return result;
        }
    };
}

void KG::Component::LobbyUIComponent::OnCreate(KG::Core::GameObject* obj)
{
    if (this->pimpl == nullptr) this->pimpl = new LobbyPimpl();
    this->pimpl->Init(obj->GetScene());
}

void KG::Component::LobbyUIComponent::Update(float elapsedTime)
{
    this->pimpl->FSMUpdate(elapsedTime);
}

void KG::Component::LobbyUIComponent::OnDestroy()
{
}

void KG::Component::LobbyUIComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::LobbyUIComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
    auto* componentElement = parentElement->InsertNewChildElement("Component");
    ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::LobbyUIComponent);
}

bool KG::Component::LobbyUIComponent::OnDrawGUI()
{
    if (ImGui::ComponentHeader<LobbyUIComponent>())
    {
    }
    return false;
}
