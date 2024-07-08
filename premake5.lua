workspace "Zahra"
	architecture "x86_64"

	configurations
	{
		"Debug",
		"Release",
		"Distribution"
	}

	startproject "Meadow"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Zahra/vendor/GLFW/include"
IncludeDir["Glad"] = "Zahra/vendor/Glad/include"
IncludeDir["ImGui"] = "Zahra/vendor/ImGui"
IncludeDir["glm"] = "Zahra/vendor/glm"
IncludeDir["stb_image"] = "Zahra/vendor/stb_image"
IncludeDir["EnTT"] = "Zahra/vendor/EnTT"

group "Dependencies"
	include "Zahra/vendor/GLFW"
	include "Zahra/vendor/Glad"
	include "Zahra/vendor/ImGui"
group ""



project "Zahra"
	location "Zahra"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.EnTT}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"
	
	filter "configurations:Debug"
		defines 
		{
			"Z_DEBUG"
		}
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
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}"
	}

	links
	{
		"Zahra"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines 
		{
			"Z_DEBUG"
		}
		symbols "on"

	filter "configurations:Release"
		defines "Z_RELEASE"
		optimize "on"

	filter "configurations:Distribution"
		defines "Z_DIST"
		optimize "on"



project "Meadow"
	location "Meadow"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.EnTT}"
	}

	links
	{
		"Zahra"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines 
		{
			"Z_DEBUG"
		}
		symbols "on"

	filter "configurations:Release"
		defines "Z_RELEASE"
		optimize "on"

	filter "configurations:Distribution"
		defines "Z_DIST"
		optimize "on"