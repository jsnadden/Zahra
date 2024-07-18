include "./vendor/premake/premake_customisation/solution_items.lua"

workspace "Zahra"
	architecture "x86_64"
	startproject "Meadow"

	configurations
	{
		"Debug",
		"Release",
		"Distribution"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Zahra/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Zahra/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Zahra/vendor/ImGui"
IncludeDir["glm"] = "%{wks.location}/Zahra/vendor/glm"
IncludeDir["stb_image"] = "%{wks.location}/Zahra/vendor/stb_image"
IncludeDir["EnTT"] = "%{wks.location}/Zahra/vendor/EnTT"
IncludeDir["yaml_cpp"] = "%{wks.location}/Zahra/vendor/yaml-cpp/include"
IncludeDir["ImGuizmo"] = "%{wks.location}/Zahra/vendor/ImGuizmo"

group "Dependencies"
	include "vendor/premake"
	include "Zahra/vendor/GLFW"
	include "Zahra/vendor/Glad"
	include "Zahra/vendor/ImGui"
	include "Zahra/vendor/yaml-cpp"
group ""

include "Zahra"
include "Sandbox"
include "Meadow"
