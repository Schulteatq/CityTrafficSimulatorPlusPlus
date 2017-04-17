#ifndef CTS_CORE_LOG_H__
#define CTS_CORE_LOG_H__

#include <cts-core/coreapi.h>
#include <cts-core/base/scopeguard.h>
#include <cts-core/base/utils.h>

#include <memory>
#include <string>
#include <sstream>
#include <vector>


namespace cts
{
	namespace core
	{
		/// Base class for a logger.
		class CTS_CORE_API Logger
		{
		public:
			/// Severity of the log event
			enum class Level {
				Trace = 0,
				Debug = 1,
				Info = 2,
				Warning = 3,
				Error = 4,
				Fatal = 5
			};

			explicit Logger(bool showExtraInfo);

			/// Virtual Destructor
			virtual ~Logger();

			/// Logs the given message. Calls logImpl() if the message passes all registered filters.
			/// \param  level		Severity of the log event.
			/// \param  category	Category of the log event.
			/// \param  message		Log message.
			/// \param  extraInfo	Additional information on the log message (default log macros expand to FILE/FUNCTION/LINE).
			void log(Level level, const std::string& category, const std::string& message, const std::string& extraInfo);

			/// Adds a message filter for this logger. Only messages that match all filters will be logged.
			/// \param  minLevel	Minimum log level.
			/// \param  category	Category filter, may be empty.
			void addFilter(Level minLevel, const std::string& category);

		protected:
			struct Filter
			{
				Level level;			///< Minimum log level
				std::string category;	///< Category filter, may be empty
			};

			/// Performs the actual logging of the given message. Called by log() after filtering.
			/// To be implemented by specific loggers.
			/// \param  level		Severity of the log event.
			/// \param  category	Category of the log event.
			/// \param  message		Log message.
			/// \param  extraInfo	Additional information on the log message (default log macros expand to FILE/FUNCTION/LINE).
			virtual void logImpl(Level level, const std::string& category, const std::string& message, const std::string& extraInfo) = 0;
			

			std::vector<Filter> m_filters;	///< List of all registered filters.
			bool m_showExtraInfo;		///< Flag whether to show extended log info.
		};


		// ================================================================================================
		

		/// Central log singleton.
		class CTS_CORE_API LogManager : public utils::NotCopyable
		{
		public:
			/// Returns the singleton instance.
			static LogManager& get();

			/// Logs the given message on all registered loggers.
			/// \param  level		Severity of the log event.
			/// \param  category	Category of the log event.
			/// \param  message		Log message.
			/// \param  extraInfo	Additional information on the log message (default log macros expand to FILE/FUNCTION/LINE).
			void log(Logger::Level level, const std::string& category, const std::string& message, const std::string& extraInfo);

			/// Adds the given logger to the list of registered loggers.
			/// \param  logger 
			void addLogger(std::unique_ptr<Logger> logger);

		private:
			LogManager();

			std::vector< std::unique_ptr<Logger> > m_loggers;
		};


		// ================================================================================================


		class CTS_CORE_API ConsoleLogger : public Logger
		{
		public:
			ConsoleLogger(bool showExtraInfo, bool useColor);

		protected:
			virtual void logImpl(Level level, const std::string& category, const std::string& message, const std::string& extraInfo) override;
			
			bool m_useColor;
			void* m_handle;
		};

	}
}


#define LOG_TRACE_GUARD(cat) \
	std::ostringstream _tmpGuard; _tmpGuard << __FUNCTION__ << "()"; \
	auto _localTraceScopeGuard = cts::utils::makeScopeGuard( \
		[&_tmpGuard]() { LOG_TRACE(cat, "Entering " + _tmpGuard.str()); }, \
		[&_tmpGuard]() { LOG_TRACE(cat, "Leaving " + _tmpGuard.str()); } \
	);

#define LOG_TRACE(cat, msg) \
	do { \
		std::ostringstream _tmp; \
		_tmp << msg; \
		cts::core::LogManager::get().log(cts::core::Logger::Level::Trace, cat, _tmp.str(), ""); \
	} while (0)

#define LOG_DEBUG(cat, msg) \
	do { \
		std::ostringstream _tmp, _tmp2; \
		_tmp2 << __FUNCTION__  << "(), " << __FILE__ << "@" << __LINE__;\
		_tmp << msg; \
		cts::core::LogManager::get().log(cts::core::Logger::Level::Debug, cat, _tmp.str(), _tmp2.str()); \
	} while (0)

#define LOG_INFO(cat, msg) \
	do { \
		std::ostringstream _tmp, _tmp2; \
		_tmp2 << __FUNCTION__  << "(), " << __FILE__ << "@" << __LINE__;\
		_tmp << msg; \
		cts::core::LogManager::get().log(cts::core::Logger::Level::Info, cat, _tmp.str(), _tmp2.str()); \
	} while (0)

#define LOG_WARN(cat, msg) \
	do { \
		std::ostringstream _tmp, _tmp2; \
		_tmp2 << __FUNCTION__  << "(), " << __FILE__ << "@" << __LINE__;\
		_tmp << msg; \
		cts::core::LogManager::get().log(cts::core::Logger::Level::Warning, cat, _tmp.str(), _tmp2.str()); \
	} while (0)

#define LOG_ERROR(cat, msg) \
	do { \
		std::ostringstream _tmp, _tmp2; \
		_tmp2 << __FUNCTION__  << "(), " << __FILE__ << "@" << __LINE__;\
		_tmp << msg; \
		cts::core::LogManager::get().log(cts::core::Logger::Level::Error, cat, _tmp.str(), _tmp2.str()); \
	} while (0)

#define LOG_FATAL(cat, msg) \
	do { \
		std::ostringstream _tmp, _tmp2; \
		_tmp2 << __FUNCTION__  << "(), " << __FILE__ << "@" << __LINE__;\
		_tmp << msg; \
		cts::core::LogManager::get().log(cts::core::Logger::Level::Fatal, cat, _tmp.str(), _tmp2.str()); \
	} while (0)

#endif