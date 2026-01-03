project "GEngine"
	kind "SharedLib"
	staticruntime "off"
	--cppdialect "C++20"	
	language "C++"
	

	targetdir ("%{wks.location}\\bin\\" .. outputdir .. "\\%{prj.name}")
	objdir ("%{wks.location}\\bin-int\\" .. outputdir .. "\\%{prj.name}")

	pchheader "GEpch.h"
	pchsource "src/GEpch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",
		"vendor/glm/**.hpp",
		"vendor/glm/**.inl",
		"vendor/entt/include/**.hpp",
		"vendor/ImGuizmo/ImGuizmo.h",
		"vendor/ImGuizmo/ImGuizmo.cpp",
		
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"FBX_SHARED"
	}


	includedirs
	{
		"src",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.vender}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.SPIRVCross}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.vulkan}",
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.FBX}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.DirectXHeaders}",
		"%{IncludeDir.HLSLcc}",
		"%{IncludeDir.dxc}"
	}

	links
	{
		"GLFW",
		"GLAD",
		"ImGui",
		"yaml-cpp",
		"Box2D",
		"opengl32.lib",
		"d3dcompiler.lib",
		"D3D12.lib",
		"dxgi.lib",
		"dxguid.lib",
		"%{Library.mono}",
		"%{Library.Vulkan}",
	}

	filter "files:vendor/ImGuizmo/**.cpp"
		flags
		{
			"NoPCH"
		}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines
		{
			"GE_BUILD_DLL",
			"GE_PROFILE"
		}

		links
		{
			"%{Library.WinSock}",
			"%{Library.WinMM}",
			"%{Library.WinVersion}",
			"%{Library.BCrypt}",


			"%{Library.PhysX_64}",
			"%{Library.PhysXCommon_64}",
			"%{Library.PhysXFoundation_64}",
			"%{Library.PhysXCooking_64}",

			"%{Library.dxc_dxcompiler}",
			"%{Library.dxc_dxil}"
		}

		postbuildcommands
		{
			("for /d %%d in (\"%{wks.location}bin\\" .. outputdir .. "\\*\") do IF \"%%d\\\" neq \"%{cfg.buildtarget.directory}\" (xcopy /Q /Y /I \"%{cfg.buildtarget.directory}%{cfg.buildtarget.name}\" \"%%d\\\" > nul) ELSE (echo Skipping copy \"%{cfg.buildtarget.directory}%{cfg.buildtarget.name}\" to \"%%d\")"),
			("for /d %%d in (\"%{wks.location}bin\\" .. outputdir .. "\\*\") do {COPY} \"%{LibraryDir.PhysX}*.dll\" \"%%d\\\""),
			("for /d %%d in (\"%{wks.location}bin\\" .. outputdir .. "\\*\") do {COPY} \"%{LibraryDir.VulkanSDK_Bin}*.dll\" \"%%d\\\""),
			("for /d %%d in (\"%{wks.location}bin\\" .. outputdir .. "\\*\") do {COPY} \"%{LibraryDir.dxc_dll}*.dll\" \"%%d\\\""),
			
		}

	filter "configurations:Debug"
		defines "GE_DEBUG"
		runtime "Debug"
		symbols "On"

		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}",

			"%{Library.FBX_SDK_Debug}",
			"%{Library.FBX_XML2_Debug}",
			"%{Library.FBX_Z_Debug}",

			
		}
	
	filter "configurations:Release"
		defines "GE_RELEASE"
		runtime "Release"
		optimize "On"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}",

			"%{Library.FBX_SDK_Release}",
			"%{Library.FBX_XML2_Release}",
			"%{Library.FBX_Z_Release}",
		}

	filter "configurations:Dist"
		defines "GE_DIST"
		runtime "Release"
		optimize "On"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}",

			"%{Library.FBX_SDK_Release}",
			"%{Library.FBX_XML2_Release}",
			"%{Library.FBX_Z_Release}",
		}