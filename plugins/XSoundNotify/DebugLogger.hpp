#ifndef _GRS_DEBUG_LOGGER_H
#define _GRS_DEBUG_LOGGER_H

#include <fstream>
#include <sstream>
#include <ctime>
#include <memory>

/**
 * ѕростой логгер, который можно использовать в любом месте дл€ вывода отладочной информации
 * ѕри выводе обычного сообщени€ надо использовать макрос GRS_DEBUG_LOG
 * ƒл€ вывода сложного сообщени€ с использованием операторов ввода/вывода макрос GRS_DEBUG_FORMAT_LOG
 * Ћог сохран€етс€ в рабочем каталоге под именем : grs_debug.log
 */

namespace grs
{

class DebugLogger
{
public:
	class Except : public std::exception
	{
	public:
		virtual const char * what() const throw() { return "pizda rulu"; }
	};

	DebugLogger()
	{
		try
		{
			_strm.open("D:\\grs_debug.log", std::ios::app);
		}
		catch (...)
		{
			throw Except();
		}
		if (!_strm.is_open())
			throw Except();

		log("Logger started");
	}

	void log(const std::string & str, const char * fileStr = 0, int line = 0)
	{
		if (!_strm.is_open())
			return ;
		
		time_t t(time(0));
		struct tm * timeinfo;
  		timeinfo = localtime (&t);
		char timeStr[9];
		strftime (timeStr, 9, "%H:%M:%S", timeinfo);
		_strm << "[" << timeStr << "] ";
		if (fileStr)
			_strm << fileStr << ":" << line<<" ";
		_strm <<"# "<< str << std::endl;
	}

	static DebugLogger * instance()
	{
		//static DebugLogger * logger = 0;
		static std::auto_ptr<DebugLogger> loggerPtr;
		if (loggerPtr.get() == 0)
			loggerPtr.reset(new DebugLogger());
		return loggerPtr.get();
	}

private:
	std::ofstream _strm;
};

}

#define GRS_DEBUG_FORMAT_LOG(data) {\
		grs::DebugLogger * l = grs::DebugLogger::instance();\
		if (l != 0)	\
		{\
			std::stringstream strm;\
			strm << data;\
			l->log(strm.str(), __FILE__, __LINE__);\
		}\
	}

#define GRS_DEBUG_LOG(data) {\
		grs::DebugLogger * l = grs::DebugLogger::instance();\
		if (l != 0)\
			l->log(data, __FILE__, __LINE__);\
	}

#endif
