#include "pch.h"
#include "tinyxml2.h"
#include "KGResourceLoader.h"
#include "hash.h"
#include "debug.h"

#include <string>
#include <fstream>
#include <iterator>
#include <algorithm>

ID3DBlob* KG::Resource::ResourceLoader::CompileShaderFromFile(const WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile)
{
    ID3DBlob* ppd3dShaderBlob = nullptr;
    UINT nCompileFlags = 0;
#if defined(_DEBUG)
    nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* blob;
    HRESULT hr = ::D3DCompileFromFile(pszFileName, NULL,
        D3D_COMPILE_STANDARD_FILE_INCLUDE, pszShaderName,
        pszShaderProfile, nCompileFlags, 0,
        &ppd3dShaderBlob, &blob);

    if (blob != nullptr)
    {
        DebugErrorMessage("셰이더 컴파일 오류" << (char*)blob->GetBufferPointer());
    }
    return ppd3dShaderBlob;
}

ID3DBlob* KG::Resource::ResourceLoader::LoadShaderFromFile(const std::string& xmlFileDir, const KG::Utill::HashString& targetId)
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile(xmlFileDir.c_str());
    auto shaders = doc.FirstChildElement("Root")->FirstChildElement("ShaderCode");
    bool isDirty = false;
    ID3DBlob* result = nullptr;
    while (shaders)
    {
        auto id = shaders->Attribute("id");
        auto hash = shaders->Attribute("hash_id");
        unsigned hash_id;
        if (!hash)
        {
            hash_id = KG::Utill::HashString(hash).value;
            shaders->SetAttribute("hash_id", hash_id);
        }
        else
        {
            hash_id = std::atoi(hash);
        }
        if (hash_id == targetId.value)
        {
            std::string fileDir = shaders->Attribute("fileDir");
            auto entry = shaders->Attribute("entry");
            auto type = shaders->Attribute("type");
            std::wstring wfileDir(fileDir.begin(), fileDir.end());
            result = CompileShaderFromFile(wfileDir.c_str(), entry, type);
            break;
        }
        else 
        {
            shaders = shaders->NextSiblingElement();
        }
    }
    if(isDirty)
        doc.SaveFile(xmlFileDir.c_str());
    return result;
}
