#pragma once

namespace Zahra
{
	// default values should trivialise checks
	struct GPURequirements
	{
		bool IsDiscreteGPU = false;
		bool AnisotropicFiltering = false;

		// TODO: add requirements for vram and so forth, and the corresponding
		// tests in VulkanContext::MeetsMinimimumRequirements
	};

	//struct GeneralRequirements
	//{
	//	uint32_t MinCPUCores;
	//	double MinClockSpeed; // in Hz
	//	uint64_t MinRAM; // in bytes
	//	uint64_t MinStorage; // in bytes
	//};

}
