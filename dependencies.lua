
VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Zahra/vendor/GLFW/include"
IncludeDir["Box2D"] = "%{wks.location}/Zahra/vendor/Box2D/include"
IncludeDir["Glad"] = "%{wks.location}/Zahra/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Zahra/vendor/ImGui"
IncludeDir["glm"] = "%{wks.location}/Zahra/vendor/glm"
IncludeDir["stb_image"] = "%{wks.location}/Zahra/vendor/stb_image"
IncludeDir["EnTT"] = "%{wks.location}/Zahra/vendor/EnTT"
IncludeDir["yaml_cpp"] = "%{wks.location}/Zahra/vendor/yaml-cpp/include"
IncludeDir["ImGuizmo"] = "%{wks.location}/Zahra/vendor/ImGuizmo"
IncludeDir["shaderc"] = "%{wks.location}/Zahra/vendor/shaderc/include"
IncludeDir["SPIRV_Cross"] = "%{wks.location}/Zahra/vendor/SPIRV-Cross"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] = "%{wks.location}/Zahra/vendor/VulkanSDK/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"
