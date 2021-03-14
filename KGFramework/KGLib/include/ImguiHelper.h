#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "hash.h"
#include <string>
namespace ImGui
{
    static int MyHashStringCallback(ImGuiInputTextCallbackData* data)
    {
        KG::Utill::HashString* my_Hash = (KG::Utill::HashString*)data->UserData;
        if ( data->EventFlag == ImGuiInputTextFlags_CallbackResize )
        {
            IM_ASSERT(my_Hash->srcString.data() == data->Buf);
            my_Hash->srcString.resize(data->BufSize); // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
            data->Buf = my_Hash->srcString.data();
        }
        return 0;
    }

    inline void HelpMarker(const char* desc)
    {
        ImGui::TextDisabled("(?)");
        if ( ImGui::IsItemHovered() )
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    static void HashView(KG::Utill::HashString* hash)
    {
        ImGui::TextDisabled("(!)");
        if ( ImGui::IsItemHovered() )
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::Text("%u", hash->value);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
    inline bool InputHashString(const char* label, KG::Utill::HashString* my_Hash, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0)
    {
        IM_ASSERT((flags & (ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackEdit)) == 0);
        bool ret = ImGui::InputText(label, my_Hash->srcString.data(), (size_t)my_Hash->srcString.length(), 
            flags | ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackEdit, MyHashStringCallback, (void*)my_Hash);
        if ( ret )
        {
            my_Hash->ReCalc();
        }
        SameLine();
        HashView(my_Hash);
        return ret;
    }
};
