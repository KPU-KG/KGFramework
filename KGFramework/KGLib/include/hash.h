#pragma once
#include <string>
#include <assert.h>
namespace KG::Utill
{
	using hashType = unsigned;
	struct HashString;

#ifdef _DEBUG
	using ID = const char const*;
	using _ID = const char const*;
#else
	using ID = KG::Utill::HashString;
	using _ID = KG::Utill::hashType;

#endif // DEBUG

	constexpr hashType Hash(const char* s, size_t count)
	{
		return ((count ? Hash(s, count - 1) : 11u) ^ s[count]) * 31u;
	}

#ifdef _DEBUG
	constexpr const char* GetID(const char* s, size_t count)
	{
		return s;
	}
#else
	constexpr _ID GetID(const char* s, size_t count)
	{
		return Hash(s, count);
	}
#endif


	struct HashString
	{
		hashType value;
#ifdef _DEBUG
		std::string srcString = "NotDefined";
#endif
		HashString(hashType hash)
			:value(hash)
		{
		}

		HashString(const std::string& str)
		{
			this->value = Hash(str.c_str(), str.length());
#ifdef _DEBUG
			this->srcString = str;
#endif
		}

		HashString(const HashString& other)
			:value(other.value)
		{
#ifdef _DEBUG
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

/// <summary>
/// 문자열 해싱을 위한 연산자 오버로딩 입니다.
/// </summary>
/// <param name="s">오버로딩할 문자열입니다.</param>
/// <param name="count">문자열의 길이입니다. ( 자동으로 채워집니다. )</param>
/// <returns>unsigned 입니다.</returns>
constexpr KG::Utill::hashType operator""_hash(const char* s, size_t count)
{
	return KG::Utill::Hash(s, count);
}



/// <summary>
/// ID를 위한 연산자 오버로딩 입니다. 디버그 모드에서는 std::string, 릴리즈 모드에서는 unsigned로 간주됩니다.
/// </summary>
/// <param name="s">오버로딩할 문자열입니다.</param>
/// <param name="count">문자열의 길이입니다. ( 자동으로 채워집니다. )</param>
/// <returns> 디버그 모드에서는 std::string, 릴리즈 모드에서는 unsigned로 간주됩니다.</returns>
constexpr auto operator""_id( const char* s, size_t count )
{
	return KG::Utill::GetID( s, count );
};
