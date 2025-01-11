project "Meadow"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"Source/**.h",
		"Source/**.cpp"
	}

	includedirs
	{
		"Source",
		"%{wks.location}/Zahra/vendor/spdlog/include",
		"%{wks.location}/Zahra/src",
		"%{wks.location}/Zahra/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.EnTT}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.yaml_cpp}"
	}

	links
	{
		"Zahra"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "Z_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "Z_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Distribution"
		defines "Z_DIST"
		runtime "Release"
		optimize "on"