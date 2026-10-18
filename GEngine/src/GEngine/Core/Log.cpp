#include "GEpch.h"
#include "GEngine/Core/Log.h"
#if GE_ENABLE_LOCAL_LOG_SAVE
#include "spdlog/sinks/rotating_file_sink.h"
#endif

#include <charconv>
#include <cstdlib>
#include <filesystem>
#include <string_view>

namespace GEngine
{
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	bool Log::s_FileLoggingInitialized = false;

	namespace
	{
#if GE_ENABLE_LOCAL_LOG_SAVE
		constexpr size_t DefaultMaxBytes = 5 * 1024 * 1024;
		constexpr size_t DefaultMaxFiles = 3;

		size_t ReadSize(const char* name, size_t fallback, size_t minimum, size_t maximum)
		{
			const char* value = std::getenv(name);
			if (!value || !*value)
				return fallback;
			size_t parsed = 0;
			const auto end = value + std::char_traits<char>::length(value);
			const auto result = std::from_chars(value, end, parsed);
			if (result.ec != std::errc{} || result.ptr != end || parsed < minimum || parsed > maximum)
			{
				GE_CORE_WARN("Invalid {}='{}'; using {}.", name, value, fallback);
				return fallback;
			}
			return parsed;
		}
#endif
	}
	
	void Log::Init()
	{
		if (s_CoreLogger && s_ClientLogger)
			return;
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("GEngine");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);
	}

	void Log::InitFileLogging()
	{
		if (s_FileLoggingInitialized || !s_CoreLogger || !s_ClientLogger)
			return;

		const char* levelValue = std::getenv("GENGINE_LOG_LEVEL");
		spdlog::level::level_enum level = spdlog::level::trace;
		if (levelValue && *levelValue)
		{
			level = spdlog::level::from_str(levelValue);
			if (level == spdlog::level::off && std::string_view(levelValue) != "off")
			{
				GE_CORE_WARN("Invalid GENGINE_LOG_LEVEL='{}'; using trace.", levelValue);
				level = spdlog::level::trace;
			}
		}

#if GE_ENABLE_LOCAL_LOG_SAVE
		const size_t maxBytes = ReadSize("GENGINE_LOG_MAX_BYTES", DefaultMaxBytes, 1024, 1024ull * 1024 * 1024);
		const size_t maxFiles = ReadSize("GENGINE_LOG_MAX_FILES", DefaultMaxFiles, 1, 10);
		const char* fileValue = std::getenv("GENGINE_LOG_FILE");
		const std::filesystem::path path = fileValue && *fileValue ? fileValue : "Logs/GEngine.log";
		try
		{
			if (path.has_parent_path())
				std::filesystem::create_directories(path.parent_path());
			auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path.string(), maxBytes, maxFiles);
			sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %n: %v");
			s_CoreLogger->sinks().reserve(s_CoreLogger->sinks().size() + 1);
			s_ClientLogger->sinks().reserve(s_ClientLogger->sinks().size() + 1);
			s_CoreLogger->sinks().push_back(sink);
			s_ClientLogger->sinks().push_back(sink);
			s_FileLoggingInitialized = true;
		}
		catch (const std::exception& error)
		{
			GE_CORE_WARN("File logging unavailable at '{}': {}. Continuing with stdout.", path.string(), error.what());
		}
#endif
		s_CoreLogger->set_level(level);
		s_ClientLogger->set_level(level);
		s_CoreLogger->flush_on(spdlog::level::info);
		s_ClientLogger->flush_on(spdlog::level::info);
#if GE_ENABLE_LOCAL_LOG_SAVE
		if (s_FileLoggingInitialized)
			GE_CORE_INFO("Runtime log file: {} ({} bytes/file, {} rotated files).", path.string(), maxBytes, maxFiles);
#endif
	}

	void Log::Shutdown()
	{
		if (s_CoreLogger)
			s_CoreLogger->flush();
		if (s_ClientLogger)
			s_ClientLogger->flush();
		s_CoreLogger.reset();
		s_ClientLogger.reset();
		spdlog::drop("GEngine");
		spdlog::drop("APP");
		s_FileLoggingInitialized = false;
	}
}
