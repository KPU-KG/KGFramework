#pragma once
#include <string>
#include <assert.h>
namespace KG::Utill
{
	using hashType = unsigned;
	struct HashString;

#ifdef _DEBUG_ID
	using ID = const char const*;
	using _ID = const char const*;
#else
	using ID = KG::Utill::HashString;
	using _ID = KG::Utill::hashType;

#endif // DEBUG

	/// @brief 컴파일 타임 문자열 해싱 함수입니다.
	/// 프레임워크 내에서 ID관련된 해싱은 모두 이 함수를 기반으로 작동합니다.
	/// @param s 해싱할 문자열입니다.
	/// @param count 문자열의 길이 입니다.
	/// @return 해싱된 UINT 값입니다.
	constexpr hashType Hash(const char* s, size_t count)
	{
		return ((count ? Hash(s, count - 1) : 11u) ^ s[count]) * 31u;
	}

#ifdef _DEBUG_ID
	/// @brief ID 디버그모드의 ID 계산 함수입니다.
	/// const char*를 그대로 리턴합니다.
	/// @param s ID화 시킬 문자열입니다.
	/// @param count 문자열의 길이입니다.
	/// @return ID로 사용될 값입니다. ( 디버깅 모드에서 문자열로 동작합니다. )
	constexpr const char* GetID(const char* s, size_t count)
	{
		return s;
	}
#else
	/// @brief ID 계산 함수입니다. 해당 문자열을 해싱합니다.
	/// @param s ID화 시킬 문자열입니다.
	/// @param count 문자열의 길이입니다.
	/// @return ID로 사용될 UINT 값입니다.
	constexpr _ID GetID(const char* s, size_t count)
	{
		return Hash(s, count);
	}
#endif


	/// @brief 해쉬된 값을 저장할 구조체입니다.
	struct HashString
	{
		/// @brief 해싱된 ID값입니다.
		hashType value;
#ifdef _DEBUG_ID
		std::string srcString = "NotDefined";
#endif
		/// @brief 이미 해싱된 값으로 생성합니다.
		/// @param hash 해시된 값입니다.
		HashString(hashType hash)
			:value(hash)
		{
		}

		/// @brief 문자열을 해싱하여 생성합니다.
		/// 디버그모드에서 원본 문자열을 저장합니다.
		/// @param str 해싱하여 저장할 문자열입니다.
		HashString(const std::string& str = "NotDefined")
		{
			this->value = Hash(str.c_str(), str.length());
#ifdef _DEBUG_ID
			this->srcString = str;
#endif
		}

		/// @brief 복사생성자입니다.
		HashString(const HashString& other)
			:value(other.value)
		{
#ifdef _DEBUG_ID
			this->srcString = other.srcString;
#endif
		}

		bool operator==(const HashString& other) const
		{
			return this->value == other.value;
		}
		bool operator<(const HashString& other) const
		{
			return this->value < other.value;
		}
		bool operator>(const HashString& other) const
		{
			return this->value > other.value;
		}
		operator hashType()
		{
			return this->value;
		}
	};



};


/// @brief 문자열 해싱을 위한 연산자 오버로딩 입니다.
/// @param s 해싱을 위한 문자열입니다.
/// @param count 문자열의 길이입니다.
/// @return 해싱된 UINT값입니다.
constexpr KG::Utill::hashType operator""_hash(const char* s, size_t count)
{
	return KG::Utill::Hash(s, count);
}



/// @brief 문자열 ID화를 위한 연산자 오버로딩 입니다.
/// @param s 해싱을 위한 문자열입니다.
/// @param count 문자열의 길이입니다.
/// @return 해싱된 UINT값입니다. (디버그모드에서 문자열로 동작합니다.)
constexpr auto operator""_id( const char* s, size_t count )
{
	return KG::Utill::GetID( s, count );
};
