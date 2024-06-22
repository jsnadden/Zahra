workspace "Zahra"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Distribution"
	}

	startproject "Sandbox"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Zahra/vendor/GLFW/include"
IncludeDir["Glad"] = "Zahra/vendor/Glad/include"
IncludeDir["imgui"] = "Zahra/vendor/imgui"
IncludeDir["glm"] = "Zahra/vendor/glm"
IncludeDir["stb_image"] = "Zahra/vendor/stb_image"

group "Dependencies"
	include "Zahra/vendor/GLFW"
	include "Zahra/vendor/Glad"
	include "Zahra/vendor/imgui"
group ""

project "Zahra"
	location "Zahra"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	externalwarnings "Off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "zpch.h"
	pchsource "%{prj.name}/src/zpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp"
	}

	defines 
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}"
	}

	links
	{
		"GLFW",
		"Glad",
		"imgui",
		"opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"Z_PLATFORM_WINDOWS",
			"Z_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}
	
	filter "configurations:Debug"
		defines "Z_DEBUG"
		symbols "on"

	filter "configurations:Release"
		defines "Z_RELEASE"
		optimize "on"

	filter "configurations:Distribution"
		defines "Z_DIST"
		optimize "on"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	externalwarnings "Off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl"
	}

	includedirs
	{
		"Zahra/vendor/spdlog/include",
		"Zahra/src",
		"Zahra/vendor",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}"
	}

	links
	{
		"Zahra"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"Z_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "Z_DEBUG"
		symbols "on"

	filter "configurations:Release"
		defines "Z_RELEASE"
		optimize "on"

	filter "configurations:Distribution"
		defines "Z_DIST"
		optimize "on"
