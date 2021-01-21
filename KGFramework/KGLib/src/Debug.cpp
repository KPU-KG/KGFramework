#include "Debug.h"
#include <chrono>
#include <ctime>
#include <iomanip>


static std::string TimepointToString( const std::chrono::system_clock::time_point& p_tpTime,
	const std::string& p_sFormat )
{
	auto converted_timep = std::chrono::system_clock::to_time_t( p_tpTime );
	std::ostringstream oss;
	tm buffer;
	localtime_s( &buffer, &converted_timep );
	oss << std::put_time( &buffer, p_sFormat.c_str() );

	return oss.str();
}

std::ofstream& GetFileLog()
{
	static std::ofstream logger;
	if ( !logger.is_open() )
	{
		std::chrono::system_clock::time_point p = std::chrono::system_clock::now();
		std::string sDate;
		sDate = TimepointToString( p, "%Y_%m_%d_%H_%M_%S" );
		sDate += ".log";
		logger.open( sDate );
		if ( !logger )
		{
			char errorBuffer[256];
			strerror_s( errorBuffer, errno );
			DebugErrorMessage( "Logger is Dead : " << errorBuffer );
		}
	}
	return logger;
}
