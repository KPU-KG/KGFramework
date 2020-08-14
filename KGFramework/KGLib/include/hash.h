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
/// ���ڿ� �ؽ��� ���� ������ �����ε� �Դϴ�.
/// </summary>
/// <param name="s">�����ε��� ���ڿ��Դϴ�.</param>
/// <param name="count">���ڿ��� �����Դϴ�. ( �ڵ����� ä�����ϴ�. )</param>
/// <returns>unsigned �Դϴ�.</returns>
constexpr KG::Utill::hashType operator""_hash(const char* s, size_t count)
{
	return KG::Utill::Hash(s, count);
}



/// <summary>
/// ID�� ���� ������ �����ε� �Դϴ�. ����� ��忡���� std::string, ������ ��忡���� unsigned�� ���ֵ˴ϴ�.
/// </summary>
/// <param name="s">�����ε��� ���ڿ��Դϴ�.</param>
/// <param name="count">���ڿ��� �����Դϴ�. ( �ڵ����� ä�����ϴ�. )</param>
/// <returns> ����� ��忡���� std::string, ������ ��忡���� unsigned�� ���ֵ˴ϴ�.</returns>
constexpr auto operator""_id( const char* s, size_t count )
{
	return KG::Utill::GetID( s, count );
};
