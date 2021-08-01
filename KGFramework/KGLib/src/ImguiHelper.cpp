#include "ImguiHelper.h"
#include <vector>
#include <string>
bool ImGui::TextureView(ImTextureID id, ImVec2 size, const char* popup)
{
    bool ret = false;
    ImGui::Image(id, size);
    ImGui::OpenPopupOnItemClick(popup, ImGuiPopupFlags_MouseButtonLeft);
    ImGui::SetNextWindowSize(ImVec2(800, 800));
    ret = ImGui::BeginPopup(popup);
    if (ret)
    {
        float width = ImGui::GetColumnWidth();
        ImGui::Image(id, ImVec2(width, width));
        ImGui::EndPopup();
    }
    return ret;
}
bool ImGui::VectorStringGetter(void* data, int n, const char** out_str)
{
    std::vector<std::string>& ref = *static_cast<std::vector<std::string>*>(data);
    *out_str = ref[n].c_str();
    return true;
}