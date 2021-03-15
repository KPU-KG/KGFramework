#include "ImguiHelper.h"
#include <vector>
#include <string>
bool ImGui::VectorStringGetter(void* data, int n, const char** out_str)
{
	std::vector<std::string>& ref = *static_cast<std::vector<std::string>*>(data);
	*out_str = ref[n].c_str();
	return true;
}