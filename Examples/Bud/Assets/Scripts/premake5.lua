local RootDir = '../../../..'
include (RootDir .. "/vendor/premake/premake_customisation/solution_items.lua")

workspace "Bud"
	architecture "x86_64"
	startproject "Bud"

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

	include (RootDir .. "/Djinn")

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Bud"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetdir ("Binaries")
	objdir ("Intermediates")

	namespace ("Bud")

	files 
	{
		"Source/**.cs",
		"Properties/**.cs"
	}

	links
	{
		"Djinn"
	}
	
	filter "configurations:Debug"
		optimize "Off"
		symbols "Default"

	filter "configurations:Release"
		optimize "On"
		symbols "Default"

	filter "configurations:Distribution"
		optimize "Full"
		symbols "Off"
