#pragma once


#include "GEngine/Core/Core.h"


#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"
#include <type_traits>

namespace fmt
{
	template <typename T, typename Char>
	struct formatter<T, Char, std::enable_if_t<std::is_enum_v<T>>>
		: formatter<std::underlying_type_t<T>, Char>
	{
		template <typename FormatContext>
		auto format(T value, FormatContext& context) const
		{
			return formatter<std::underlying_type_t<T>, Char>::format(
				static_cast<std::underlying_type_t<T>>(value), context);
		}
	};
}

#ifndef GE_ENABLE_LOCAL_LOG_SAVE
#define GE_ENABLE_LOCAL_LOG_SAVE 1
#endif

namespace GEngine
{
	class GENGINE_API Log
	{
	public:
		static void Init();
		static void InitFileLogging();
		static void Shutdown();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
		static bool s_FileLoggingInitialized;
	};
}



// Core log macros
#define GE_CORE_TRACE(...)			::GEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define GE_CORE_INFO(...)			::GEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define GE_CORE_WARN(...)			::GEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define GE_CORE_ERROR(...)			::GEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define GE_CORE_CRITICAL(...)		::GEngine::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define GE_TRACE(...)				::GEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define GE_INFO(...)				::GEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define GE_WARN(...)				::GEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define GE_ERROR(...)				::GEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define GE_CRITICAL(...)			::GEngine::Log::GetClientLogger()->critical(__VA_ARGS__)
