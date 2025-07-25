include "./vendor/premake/premake_customisation/solution_items.lua"
include "dependencies.lua"

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

group "Dependencies"
	include "vendor/premake"
	include "Zahra/vendor/GLFW"
	include "Zahra/vendor/Glad"
	include "Zahra/vendor/ImGui"
	include "Zahra/vendor/yaml-cpp"
	include "Zahra/vendor/Box2D"
	include "Zahra/vendor/msdf-atlas-gen"
	
group "Tools"
	include "Meadow"

group "ZahraCore"
	include "Zahra"
	include "Djinn"

group "Misc."
	include "Sandbox"



