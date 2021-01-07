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

	/// @brief ������ Ÿ�� ���ڿ� �ؽ� �Լ��Դϴ�.
	/// �����ӿ�ũ ������ ID���õ� �ؽ��� ��� �� �Լ��� ������� �۵��մϴ�.
	/// @param s �ؽ��� ���ڿ��Դϴ�.
	/// @param count ���ڿ��� ���� �Դϴ�.
	/// @return �ؽ̵� UINT ���Դϴ�.
	constexpr hashType Hash(const char* s, size_t count)
	{
		return ((count ? Hash(s, count - 1) : 11u) ^ s[count]) * 31u;
	}

#ifdef _DEBUG_ID
	/// @brief ID ����׸���� ID ��� �Լ��Դϴ�.
	/// const char*�� �״�� �����մϴ�.
	/// @param s IDȭ ��ų ���ڿ��Դϴ�.
	/// @param count ���ڿ��� �����Դϴ�.
	/// @return ID�� ���� ���Դϴ�. ( ����� ��忡�� ���ڿ��� �����մϴ�. )
	constexpr const char* GetID(const char* s, size_t count)
	{
		return s;
	}
#else
	/// @brief ID ��� �Լ��Դϴ�. �ش� ���ڿ��� �ؽ��մϴ�.
	/// @param s IDȭ ��ų ���ڿ��Դϴ�.
	/// @param count ���ڿ��� �����Դϴ�.
	/// @return ID�� ���� UINT ���Դϴ�.
	constexpr _ID GetID(const char* s, size_t count)
	{
		return Hash(s, count);
	}
#endif


	/// @brief �ؽ��� ���� ������ ����ü�Դϴ�.
	struct HashString
	{
		/// @brief �ؽ̵� ID���Դϴ�.
		hashType value;
#ifdef _DEBUG_ID
		std::string srcString = "NotDefined";
#endif
		/// @brief �̹� �ؽ̵� ������ �����մϴ�.
		/// @param hash �ؽõ� ���Դϴ�.
		HashString(hashType hash)
			:value(hash)
		{
		}

		/// @brief ���ڿ��� �ؽ��Ͽ� �����մϴ�.
		/// ����׸�忡�� ���� ���ڿ��� �����մϴ�.
		/// @param str �ؽ��Ͽ� ������ ���ڿ��Դϴ�.
		HashString(const std::string& str = "NotDefined")
		{
			this->value = Hash(str.c_str(), str.length());
#ifdef _DEBUG_ID
			this->srcString = str;
#endif
		}

		/// @brief ����������Դϴ�.
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


/// @brief ���ڿ� �ؽ��� ���� ������ �����ε� �Դϴ�.
/// @param s �ؽ��� ���� ���ڿ��Դϴ�.
/// @param count ���ڿ��� �����Դϴ�.
/// @return �ؽ̵� UINT���Դϴ�.
constexpr KG::Utill::hashType operator""_hash(const char* s, size_t count)
{
	return KG::Utill::Hash(s, count);
}



/// @brief ���ڿ� IDȭ�� ���� ������ �����ε� �Դϴ�.
/// @param s �ؽ��� ���� ���ڿ��Դϴ�.
/// @param count ���ڿ��� �����Դϴ�.
/// @return �ؽ̵� UINT���Դϴ�. (����׸�忡�� ���ڿ��� �����մϴ�.)
constexpr auto operator""_id( const char* s, size_t count )
{
	return KG::Utill::GetID( s, count );
};
