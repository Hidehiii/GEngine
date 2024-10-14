#pragma once
#include <memory>

#define BIT(x) (1 << x)
#define GE_BIND_EVENT_FN(x) [this](auto&&... args) -> decltype(auto) { return this->x(std::forward<decltype(args)>(args)...); }
#define VAR_NAME(x) #x
#define EXPAND(...) __VA_ARGS__

#ifdef _WIN32
	#ifdef _WIN64
		#define GE_PLATFORM_WINDOWS
	#else
		#error "x86 Builds are not supported!"
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>
	#if TARGET_IPHONE_SIMULATOR
		#error "IOS simulator is not supported!"
	#elif TARGET_OS_IPHONE
		#define GE_PLATFORM_IOS
		#error "IOS is not supported!"
	#elif TARGET_OS_MAC
		#define GE_PLATFORM_MACOS
		#error "MacOS is not supported!"
	#else
		#error "Unknown Apple platform!"
	#endif
#elif defined(__ANDROID__)
	#define GE_PLATFORM_ANDROID
	#error "Android is not supported!"
#elif defined(__linux__)
	#define GE_PLATFORM_LINUX
	#error "Linux is not supported!"
#else
	#error "Unknown platform!"
#endif


#ifdef GE_PLATFORM_WINDOWS
	#ifdef GE_BUILD_DLL
		#define GENGINE_API __declspec(dllexport)
	#else
		#define GENGINE_API __declspec(dllimport)
	#endif
#else
	#error GEngine only supports Windows!
#endif



#ifdef GE_DEBUG
	#ifdef GE_PLATFORM_WINDOWS
		#define GE_DEBUGBREAK() __debugbreak()
	#elif defined(GE_PLATFORM_LINUX)
		#include <signal.h>
		#define GE_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
	#define GE_ENABLE_ASSERTS
#endif

#ifdef GE_ENABLE_ASSERTS
	#define GE_ASSERT(x, ...) {												\
			if(!(x)) {														\
				GE_CRITICAL("Assertion Failed In File: {0}", __FILE__);		\
				GE_CRITICAL("At Line : {0}",__LINE__);						\
				GE_CRITICAL("Message : {}" , ## EXPAND(__VA_ARGS__)); GE_DEBUGBREAK(); } }
	#define GE_CORE_ASSERT(x, ...) {										\
			if(!(x)) {														\
				GE_CORE_CRITICAL("Assertion Failed In File: {0}", __FILE__);\
				GE_CORE_CRITICAL("At Line : {0}",__LINE__);					\
				GE_CORE_CRITICAL("Message : {}" , ## EXPAND(__VA_ARGS__)); GE_DEBUGBREAK(); } }
#else
	#define GE_ASSERT(x, ...)
	#define GE_CORE_ASSERT(x, ...)
#endif



namespace GEngine {

	// Scope and Ref

	// Scope is a unique pointer
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}


	// Ref is a shared pointer
	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
	template<typename T>
	constexpr Ref<T> ToRef(T* t)
	{
		return std::shared_ptr<T>(t);
	}
}