#pragma once

#include <string>

#include <vulkan/vulkan.h>

namespace Zahra
{
	namespace VulkanUtils
	{
		static void ValidateVkResult(VkResult result, const std::string& errorMessage = "")
		{
			if (result != VK_SUCCESS) throw std::runtime_error(errorMessage);
		}

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
			void* userData)
		{
			// TODO: make this a more robust report, using all available data

			switch (messageSeverity)
			{
			case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			{
				Z_VULKAN_INFO(callbackData->pMessage);
				break;
			}
			case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			{
				Z_VULKAN_WARN(callbackData->pMessage);
				break;
			}
			case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			{
				Z_VULKAN_ERROR(callbackData->pMessage);
				break;
			}
			default:
				break;
			}

			return VK_FALSE;
		}


	}
}
