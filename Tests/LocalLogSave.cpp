#include <GEngine/Core/Log.h>

#include <filesystem>
#include <stdexcept>

int main()
{
	GEngine::Log::Init();
	GEngine::Log::InitFileLogging();
	GE_CORE_INFO("Local log save macro core check.");
	GE_INFO("Local log save macro app check.");
	GEngine::Log::Shutdown();

	const bool fileExists = std::filesystem::exists("Logs/GEngine.log");
	if (fileExists != static_cast<bool>(GE_ENABLE_LOCAL_LOG_SAVE))
		throw std::runtime_error("Local log save macro behavior mismatch.");
	return 0;
}
