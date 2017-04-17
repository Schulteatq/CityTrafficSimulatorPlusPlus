#include <cts-core/base/log.h>

#include <iomanip>
#include <iostream>

#ifdef _WIN32
	#include <windows.h>
	#include <wincon.h>
#endif // _WIN32

namespace cts { namespace core
{

	inline std::ostream& operator<< (std::ostream& s, const Logger::Level& level) {
		switch (level)
		{
		case Logger::Level::Trace:
			return (s << "Trace");
		case Logger::Level::Debug:
			return (s << "Debug");
		case Logger::Level::Info:
			return (s << "Info");
		case Logger::Level::Warning:
			return (s << "Warning");
		case Logger::Level::Error:
			return (s << "Error");
		case Logger::Level::Fatal:
			return (s << "Fatal");
		default:
			return s;
		}
	}


	Logger::Logger(bool showExtraInfo)
		: m_showExtraInfo(showExtraInfo)
	{

	}


	Logger::~Logger() {}


	void Logger::log(Logger::Level level, const std::string& category, const std::string& message, const std::string& extraInfo)
	{
		for (auto& filter : m_filters)
		{
			if (filter.level > level)
				return;

			if (category.find(filter.category, 0) != 0)
				return;
		}

		logImpl(level, category, message, extraInfo);
	}


	void Logger::addFilter(Level minLevel, const std::string& category)
	{
		m_filters.push_back({ minLevel, category });
	}


	// ================================================================================================


	ConsoleLogger::ConsoleLogger(bool showExtraInfo, bool useColor)
		: Logger(showExtraInfo)
		, m_useColor(useColor)
		, m_handle(nullptr)
	{
#ifdef _WIN32
		m_handle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
	}


	void ConsoleLogger::logImpl(Level level, const std::string& category, const std::string& message, const std::string& extraInfo)
	{
		const std::time_t t = std::time(nullptr);
		const std::tm tm = *std::localtime(&t);

#ifdef _WIN32
		auto setConsoleAttribs = [this](WORD attribs)
		{
			CONSOLE_SCREEN_BUFFER_INFO orig_buffer_info;
			GetConsoleScreenBufferInfo(m_handle, &orig_buffer_info);
			SetConsoleTextAttribute(m_handle, attribs);
			return orig_buffer_info.wAttributes; //return orig attribs
		};

		WORD originalAttribs = 0;
		if (m_useColor)
		{
			switch (level)
			{
			case Level::Trace:
				originalAttribs = setConsoleAttribs(FOREGROUND_GREEN | FOREGROUND_BLUE); // cyan
				break;
			case Level::Debug:
				originalAttribs = setConsoleAttribs(FOREGROUND_GREEN); // green
				break;
			case Level::Info:
				originalAttribs = setConsoleAttribs(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // white
				break;
			case Level::Warning:
				originalAttribs = setConsoleAttribs(FOREGROUND_RED | FOREGROUND_GREEN); // yellow
				break;
			case Level::Error:
				originalAttribs = setConsoleAttribs(FOREGROUND_RED); // red
				break;
			case Level::Fatal:
				originalAttribs = setConsoleAttribs(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_RED); // bold white on red background
				break;
			default:
				originalAttribs = setConsoleAttribs(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
				break;
			}
		}

		std::cout
			<< "[" << std::put_time(std::localtime(&t), "%c") << "]"
			<< " (" << category << ") "
			<< level << ": "
			<< message
			<< "\n";

		if (m_useColor)
			setConsoleAttribs(originalAttribs);

#else // _WIN32

		if (m_useColor)
		{
			switch (level)
			{
			case Level::Trace:
				std::cout << "\033[36m"; // cyan
				break;
			case Level::Debug:
				std::cout << "\033[32m"; // green
				break;
			case Level::Info:
				break; // default
			case Level::Warning:
				std::cout << "\033[2m\033[33m"; // orange
				break;
			case Level::Error:
				std::cout << "\033[31m"; // red
				break;
			case Level::Fatal:
				std::cout << "\033[41m\033[1m"; // bold black on red background
				break;
			default:
				break;
			}
		}

		std::cout
			<< "[" << std::put_time(std::localtime(&t), "%c") << "]"
			<< " (" << category << ") "
			<< level << ": "
			<< message
			<< (m_useColor ? "\033[00m" : "")
			<< "\n";

#endif // _WIN32

		if (m_showExtraInfo && !extraInfo.empty())
			std::cout << "    (" << extraInfo << ")\n";
	}


	// ================================================================================================


	LogManager& LogManager::get()
	{
		static LogManager lm;
		return lm;
	}


	void LogManager::log(Logger::Level level, const std::string& category, const std::string& message, const std::string& extraInfo)
	{
		for (auto& logger : m_loggers)
		{
			logger->log(level, category, message, extraInfo);
		}
	}


	void LogManager::addLogger(std::unique_ptr<Logger> logger)
	{
		m_loggers.push_back(std::move(logger));
	}


	LogManager::LogManager()
	{

	}


}
}
