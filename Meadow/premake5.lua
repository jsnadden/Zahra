project "Meadow"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/Zahra/vendor/spdlog/include",
		"%{wks.location}/Zahra/src",
		"%{wks.location}/Zahra/vendor",
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