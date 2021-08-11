#include "LobbyUIComponent.h"
#include "ImguiHelper.h"
#include "InputManager.h"
#include "IRender2DComponent.h"
#include "KGFramework.h"
#include "MathHelper.h"
#include "Scene.h"
#include "IKGNetwork.h"
#include "ClientLobbyComponent.h"
#include "ClientGameManagerComponent.h"

#include <iostream>

#define SERVER_LOGIN 1
#define TEST_ID 1

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

        auto& POSITIONX(float x)
        {
            obj->transform2D.position.x = x;
            return *this;
        }

        auto& POSITIONY(float y)
        {
            obj->transform2D.position.y = y;
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

        auto& MOVEX(float x, float current, float end, float start = 0)
        {
            if (current >= start && end >= current)
            {
                auto& p = obj->transform2D.position.x;
                p = Math::Lerp(p, x, (current - start) / (end - start));
            }
            return *this;
        }

        auto& MOVEY(float y, float current, float end, float start = 0)
        {
            if (current >= start && end >= current)
            {
                auto& p = obj->transform2D.position.y;
                p = Math::Lerp(p, y, (current - start) / (end - start));
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
        LobbyUIComponent* comp;
        CLobbyComponent* lobby;
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

        void Init(KG::Core::Scene* s, LobbyUIComponent* c)
        {
            this->scene = s;
            this->comp = c;
            currentStateId = "start"_id;
            // add list
            objs["bg"_id] = this->scene->FindObjectWithTag("BG"_id)->GetComponent<KG::Component::IRender2DComponent>();
            objs["btn_start"_id] = this->scene->FindObjectWithTag("btn_start"_id)->GetComponent<KG::Component::IRender2DComponent>();
            objs["title"_id] = this->scene->FindObjectWithTag("Title"_id)->GetComponent<KG::Component::IRender2DComponent>();
            objs["lobbyTitle"_id] = this->scene->FindObjectWithTag("lobbyTitle"_id)->GetComponent<KG::Component::IRender2DComponent>();
            objs["youText"_id] = this->scene->FindObjectWithTag("YOU"_id)->GetComponent<KG::Component::IRender2DComponent>();
            objs["youline1"_id] = this->scene->FindObjectWithTag("line_1"_id)->GetComponent<KG::Component::IRender2DComponent>();
            objs["youline2"_id] = this->scene->FindObjectWithTag("line_2"_id)->GetComponent<KG::Component::IRender2DComponent>();
            objs["WAITING"_id] = this->scene->FindObjectWithTag("WAITING"_id)->GetComponent<KG::Component::IRender2DComponent>();
            objs["Select"_id] = this->scene->FindObjectWithTag("Select"_id)->GetComponent<KG::Component::IRender2DComponent>();
            objs["Select_day"_id] = this->scene->FindObjectWithTag("Select_day"_id)->GetComponent<KG::Component::IRender2DComponent>();
            objs["Select_sunset"_id] = this->scene->FindObjectWithTag("Select_sunset"_id)->GetComponent<KG::Component::IRender2DComponent>();
            objs["StartLOGO"_id] = this->scene->FindObjectWithTag("StartLOGO"_id)->GetComponent<KG::Component::IRender2DComponent>();

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
                    Anim("youText"_id).OFF().POSITIONX(-1.5);
                    Anim("youline1"_id).OFF().POSITIONX(-1.5);
                    Anim("youline2"_id).OFF().POSITIONX(-1.5);
                    Anim("WAITING"_id).OFF();
                    Anim("Select"_id).OFF();
                    Anim("Select_day"_id).OFF();
                    Anim("Select_sunset"_id).OFF();
                    Anim("StartLOGO"_id).OFF().POSITION(-2.0f, 0.0f);
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
#if SERVER_LOGIN == 1
                    GameFramework::instance->StartClient();
                    this->lobby = this->scene->GetRootNode()->GetComponent<CLobbyComponent>();
                    this->lobby->SendLoginPacket();
                    this->lobby->PostStartFunction([this]() {
                        this->currentStateId = "ingameChange"_id; 
                        this->currentStateTime = 0.0f;
                });
#endif
                    return "change_lobby"_id;
                }
                return 0;
            };
            fsmMap["change_lobby"_id] = [this](float delta, float current) -> KG::Utill::hashType
            {
                int myId = TEST_ID;
#if SERVER_LOGIN == 1
                myId = lobby->GetMyId();
#endif
                float end1 = 2.0f;
                if (current <= end1)
                {
                    static float x[] = { -0.675f, -0.225, 0.225, 0.675 };
                    Anim("btn_start"_id).FADEOFF(current, end1).MOVEY(-0.75f, current, end1);
                    Anim("title"_id).MOVE(-0.350f, 0.75f, current, end1);
                    Anim("lobbyTitle"_id).FADEON(current, end1);
                    Anim("WAITING"_id).FADEON(current, end1);
                    Anim("youText"_id).FADEON(current, end1).MOVEX(x[myId],current, end1);
                    Anim("youline1"_id).FADEON(current, end1).MOVEX(x[myId], current, end1);
                    Anim("youline2"_id).FADEON(current, end1).MOVEX(x[myId], current, end1);
                    Anim("Select"_id).FADEON(current, end1);
                    Anim("Select_day"_id).FADEON(current, end1).FADECOLOR(1.0f, 1.0f, 1.0f, current, end1);
                    Anim("Select_sunset"_id).FADEON(current, end1).FADECOLOR(0.25f, 0.25f, 0.25f, current, end1);
                    for (size_t i = 0; i < 4; i++)
                    {
                        float color = 0.25f;
                        if (myId == i) color = 1.0f;
                        Anim(playerWait[i]).FADEON(current, end1).FADECOLOR(color, color, color, current, end1);
                        Anim(playerTitle[i]).FADEON(current, end1).FADECOLOR(color, color, color, current, end1);
                        Anim(playerReady[i]).FADEON(current, end1).FADECOLOR(0.25, 0.25, 0.25, current, end1);
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
                float b = 0.25f + 0.1f * sinf(current * 3);
                Anim("Select_day"_id).COLOR(b, b, b);
                Anim("Select_sunset"_id).COLOR(b, b, b);
                for (size_t i = 0; i < 4; i++)
                {
                    int myId = TEST_ID;
                    float o = 1.0f;
                    float w = 0, t = 0, r = 0;
                    char flag = 0;
#if SERVER_LOGIN == 1
                    flag = this->lobby->GetLobbyInfo((int)i);
                    myId = this->lobby->GetMyId();
#endif
                    switch (flag)
                    {
                    case LobbyState::Empty:
                        t = b; w = b; r = b;
                        break;
                    case LobbyState::Ready:
                        t = o; w = b; r = o;
                        break;
                    case LobbyState::Wait:
                        t = o; w = o; r = b;
                        break;
                    }
                    Anim(playerTitle[i]).COLOR(t, t, t);
                    Anim(playerWait[i]).COLOR(w, w, w);
                    Anim(playerReady[i]).COLOR(r, r, r);

                    if (i == myId && IsClicked(playerWait[i]))
                    {
#if SERVER_LOGIN == 1
                        this->lobby->SendWaitPacket();
#endif
                    }
                    if (i == myId && IsClicked(playerReady[i]))
                    {
#if SERVER_LOGIN == 1
                        this->lobby->SendReadyPacket();
#else
                        this->currentStateId = "ingameChange"_id;
                        this->currentStateTime = 0.0f;
#endif
                    }
                }
                return 0;
            };

            fsmMap["ingameChange"_id] = [this](float delta, float current) -> KG::Utill::hashType
            {
                float end = 2.0f;
                float out = 1.5f;
                Anim("title"_id).MOVEX(out, current, end);
                Anim("lobbyTitle"_id).MOVEX(out, current, end);
                Anim("youText"_id).MOVEX(out, current, end);
                Anim("youline1"_id).MOVEX(out, current, end);
                Anim("youline2"_id).MOVEX(out, current, end);
                Anim("WAITING"_id).MOVEX(out, current, end);
                for (size_t i = 0; i < 4; i++)
                {
                    Anim(playerTitle[i]).MOVEX(out, current, end);
                    Anim(playerWait[i]).MOVEX(out, current, end);
                    Anim(playerReady[i]).MOVEX(out, current, end);
                }
                Anim("Select"_id).MOVEX(out, current, end);
                Anim("Select_day"_id).MOVEX(out, current, end);
                Anim("Select_sunset"_id).MOVEX(out, current, end);
                Anim("StartLOGO"_id).ON().MOVEX(0, current, end);
                if (current > end)
                {
                    this->comp->GetGameObject()->GetScene()->LoadScene("Resource/Scenes/SceneData_Test_Post.xml");
#if SERVER_LOGIN == 1
                    auto* root = this->scene->GetRootNode();
                    auto* manager = root->GetComponent<CGameManagerComponent>();
                    manager->SendLoginPacket();
#endif
                    return "ingameFadeOut"_id;
                }
                return 0;
            };

            fsmMap["ingameFadeOut"_id] = [this](float delta, float current) -> KG::Utill::hashType
            {
                for (auto[id, ptr] : this->objs)
                {
                    Anim(id).FADEOFF(current, 2.0f);
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
                x = (x / float(setting.GetGameResolutionWidth())) * 2.0f - 1.0f;
                y = ((y / float(setting.GetGameResolutionHeigth())) * 2.0f - 1.0f) * -1;
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
    this->pimpl->Init(obj->GetScene(), this);
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
