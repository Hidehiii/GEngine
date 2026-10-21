project "GLFW"
	kind "StaticLib"
	language "C"
	staticruntime "off"
	warnings "off"

	targetdir ("%{wks.location}/dist/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/build/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/include/GLFW/glfw3.h",
		"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/include/GLFW/glfw3native.h",
		"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/context.c",
		"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/init.c",
		"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/input.c",
		"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/monitor.c",
		"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/null_init.c",
		"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/null_joystick.c",
		"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/null_monitor.c",
		"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/null_window.c",
		"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/platform.c",
		"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/vulkan.c",
		"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/window.c",
	}

	includedirs
	{
		"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/include",
		"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src",
	}

	filter "system:windows"
		systemversion "latest"
		files
		{
			"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/win32_init.c",
			"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/win32_joystick.c",
			"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/win32_module.c",
			"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/win32_monitor.c",
			"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/win32_time.c",
			"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/win32_thread.c",
			"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/win32_window.c",
			"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/wgl_context.c",
			"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/egl_context.c",
			"%{wks.location}/GEngine/GEngine-ThirdParty/glfw/src/osmesa_context.c",
		}
		defines
		{
			"_GLFW_WIN32",
			"_CRT_SECURE_NO_WARNINGS",
			"GLFW_EXPOSE_NATIVE_WIN32",
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "speed"

	filter "configurations:Dist"
		runtime "Release"
		optimize "speed"
		symbols "off"
