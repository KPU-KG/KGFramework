#pragma once
#include <string_view>
#include "tinyxml2.h"
#include "hash.h"
#include "XMLConverter.h"
namespace KG::Core
{
	class ISerializable
	{
	public:
		//상위 객체가 자신을 미리 정의했으면 내부에서, 구체적으로 정의되지 않았으면 (갯수 모름, 존재 모름 등 ) 외부에서 엘리먼트 검색
		virtual void OnDataLoad(tinyxml2::XMLElement* objectElement) = 0;

		//무조건 내부에서 엘리먼트 생성
		virtual void OnDataSave(tinyxml2::XMLElement* objectElement) = 0;

		virtual void OnDrawGUI() = 0;
	};
};
