#pragma once
#include <Windows.h>
#include <sstream>
#include <fstream>

#define TOSTRING(X) #X

#if defined(_DEBUG) | defined(DEBUG)


#define DebugStream(P, X)\
	{std::wstringstream debugStream;\
	debugStream << PROJECT_NAME << " : " << P << " : " << X << std::endl;\
	OutputDebugString(debugStream.str().data());}

#define DebugNormalMessage(X) DebugStream("NORMAL_LOG", X)
#define DebugErrorMessage(X) DebugStream("ERROR_LOG", X)
#define DebugAssertion(P, X) if(!(P)){ DebugErrorMessage(X); } assert(P && "Check Log Message")
#define NotImplement(X) DebugNormalMessage(L""#X"는 아직 구현되지 않았습니다.")

#define FileLogStream(P, X)\
	{std::ofstream& debugStream = GetFileLog();\
	debugStream << PROJECT_NAME << " : " << P << " : " << X << std::endl;\
	}

#define FileLogStreamNone(X)\
	{std::ofstream& debugStream = GetFileLog();\
	debugStream << X << std::endl;\
	}


#define FileLogNormalMessage(X) FileLogStream("NORMAL_LOG", X)
#define FileLogErrorMessage(X) FileLogStream("ERROR_LOG", X)
#define FileLogAssertion(P, X) if(!(P)){ FileLogErrorMessage(X); } assert(P && "Check Log Message")

#include <DirectXMath.h>

extern std::ofstream& GetFileLog();

FORCEINLINE std::ostream& operator<<( std::ostream& os, const DirectX::XMFLOAT3& value )
{
	return os << value.x << ", " << value.y << ", " << value.z;
}
FORCEINLINE std::ostream& operator<<( std::ostream& os, const DirectX::XMFLOAT4& value )
{
	return os << value.x << ", " << value.y << ", " << value.z << ", " << value.z;
}

FORCEINLINE std::wostream& operator<<( std::wostream& os, const DirectX::XMFLOAT3& value )
{
	return os << value.x << ", " << value.y << ", " << value.z;
}
FORCEINLINE std::wostream& operator<<( std::wostream& os, const DirectX::XMFLOAT4& value )
{
	return os << value.x << ", " << value.y << ", " << value.z << ", " << value.z;
}

#else
#define DebugNormalMessage(X)
#define DebugErrorMessage(X)
#define DebugAssertion(P, X)
#define NotImplement(X)
#endif
