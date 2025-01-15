project "Zahra"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "zpch.h"
	pchsource "src/zpch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
		"vendor/ImGuizmo/ImGuizmo.h",
		"vendor/ImGuizmo/ImGuizmo.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/VMA/**.h",
		"vendor/VMA/**.cpp",
	}

	defines 
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"IMGUI_DEFINE_MATH_OPERATORS"
	}

	includedirs
	{
		"src",
		"vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.FileWatch}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.EnTT}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.VMA}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.tinyobjloader}"
	}

	links
	{
		"Djinn",
		"GLFW",
		"Box2D",
		"Glad",
		"ImGui",
		"yaml-cpp",
		"opengl32.lib",
		"%{Library.mono}",
		"%{Library.Vulkan}"
	}

	filter "files:vendor/ImGuizmo/**.cpp"
		flags{"NoPCH"}

	filter "system:windows"
		systemversion "latest"
		defines
		{
			"Z_RENDERERAPI_VULKAN"
		}

		links
		{
			"%{Library.WinSock}",
			"%{Library.WinMultimedia}",
			"%{Library.WinVersion}",
			"%{Library.WinBCrypt}"
		}
	
	filter "configurations:Debug"
		defines "Z_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}

	filter "configurations:Release"
		defines "Z_RELEASE"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

	filter { "configurations:Debug or configurations:Release" }
		defines "Z_TRACK_MEMORY"

	filter "configurations:Distribution"
		defines "Z_DIST"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}