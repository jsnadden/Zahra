workspace "Zahra"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Distribution"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Zahra/vendor/GLFW/include"

include "Zahra/vendor/GLFW"

project "Zahra"
	location "Zahra"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "zpch.h"
	pchsource "%{prj.name}/src/zpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}"
	}

	links
	{
		"GLFW",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"Z_PLATFORM_WINDOWS",
			"Z_BUILD_DLL"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .."/Sandbox")
		}
	
	filter "configurations:Debug"
		defines "Z_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "Z_RELEASE"
		optimize "On"

	filter "configurations:Distribution"
		defines "Z_DIST"
		optimize "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Zahra/vendor/spdlog/include",
		"Zahra/src"
	}

	links
	{
		"Zahra"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"Z_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "Z_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "Z_RELEASE"
		optimize "On"

	filter "configurations:Distribution"
		defines "Z_DIST"
		optimize "On"