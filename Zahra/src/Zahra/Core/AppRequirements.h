#pragma once

namespace Zahra
{
	// choose default values to trivialise checks, and make sure to add the
	// corresponding tests in VulkanSwapchain::MeetsMinimimumRequirements
	struct GPURequirements
	{
		bool IsDiscreteGPU = false;
		bool AnisotropicFiltering = false;

		uint32_t MinBoundTextureSlots = 1;
	};

	//struct GeneralRequirements
	//{
	//	uint32_t MinCPUCores;
	//	double MinClockSpeed; // in Hz
	//	uint64_t MinRAM; // in bytes
	//	uint64_t MinStorage; // in bytes
	//};

}
