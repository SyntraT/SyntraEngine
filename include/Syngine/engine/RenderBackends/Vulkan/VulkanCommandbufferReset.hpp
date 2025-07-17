#pragma once



#include <volk.h>




namespace VRenderer
{

	struct VulkanCommandbufferReset final
	{
	public:

		void CleanUp(VkDevice l_device, VkCommandPool l_thisThreadPrimaryCmdPool) noexcept;

		void ResetBuffer();

		void BeginRecording();
		void EndRecording();

	public:
		VkCommandBuffer m_buffer{};
	};

}