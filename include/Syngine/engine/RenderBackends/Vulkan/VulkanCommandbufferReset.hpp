#pragma once



#include <volk.h>




namespace VRenderer
{

	struct VulkanCommandbufferReset final
	{
	public:

		void CleanUp(VkDevice l_device) noexcept;

		void ResetBuffer();

		void BeginRecording();
		void EndRecording();

	public:
		VkCommandPool m_pool{};
		VkCommandBuffer m_buffer{};
	};

}