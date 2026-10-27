#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shellapi.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifndef GE_GAME_EXECUTABLE_NAME
#error GE_GAME_EXECUTABLE_NAME must define the runtime game executable name
#endif

namespace
{
    std::wstring QuoteArgument(const std::wstring& value)
    {
        std::wstring result = L"\"";
        for (const auto character : value)
        {
            if (character == L'"') result += L"\\\"";
            else result += character;
        }
        return result + L"\"";
    }

    std::filesystem::path RuntimeGamePath()
    {
        std::wstring module(32768, L'\0');
        const auto length = GetModuleFileNameW(nullptr, module.data(), static_cast<DWORD>(module.size()));
        if (!length || length >= module.size())
        {
            throw std::runtime_error("Cannot locate the game launcher.");
        }
        module.resize(length);
        return std::filesystem::path(module).parent_path() / L"Runtime" / TEXT(GE_GAME_EXECUTABLE_NAME);
    }
}

int main()
{
    try
    {
        const auto game = RuntimeGamePath();
        if (!std::filesystem::exists(game))
        {
            std::cerr << "Game runtime executable is missing: " << game.string() << '\n';
            return 1;
        }

        int argumentCount = 0;
        const auto arguments = CommandLineToArgvW(GetCommandLineW(), &argumentCount);
        if (!arguments)
        {
            std::cerr << "Cannot parse launcher arguments.\n";
            return 1;
        }
        std::wstring commandLine = QuoteArgument(game.wstring());
        for (int index = 1; index < argumentCount; ++index)
        {
            commandLine += L' ';
            commandLine += QuoteArgument(arguments[index]);
        }
        LocalFree(arguments);

        STARTUPINFOW startup{};
        startup.cb = sizeof(startup);
        PROCESS_INFORMATION process{};
        if (!CreateProcessW(game.c_str(), commandLine.data(), nullptr, nullptr, TRUE, 0, nullptr,
            std::filesystem::current_path().wstring().c_str(), &startup, &process))
        {
            std::cerr << "Cannot start the game runtime executable.\n";
            return 1;
        }
        CloseHandle(process.hThread);
        WaitForSingleObject(process.hProcess, INFINITE);
        DWORD exitCode = 1;
        if (!GetExitCodeProcess(process.hProcess, &exitCode)) exitCode = 1;
        CloseHandle(process.hProcess);
        return static_cast<int>(exitCode);
    }
    catch (const std::exception& error)
    {
        std::cerr << "Game launcher failed: " << error.what() << '\n';
        return 1;
    }
}
