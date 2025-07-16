#pragma once



#include <volk.h>
#include "Syngine/engine/RenderBackends/Vulkan/VulkanQueueFamilyIndex.hpp"

namespace VRenderer
{
	struct VulkanQueueGraphics final
	{
	public:


	public:
		VulkanQueueFamilyIndex m_familyIndex{};
		VkQueue m_queue{};
	};
}