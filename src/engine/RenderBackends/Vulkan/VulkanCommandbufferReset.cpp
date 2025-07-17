


#include "Syngine/engine/RenderBackends/Vulkan/VulkanCommandbufferReset.hpp"
#include "Syngine/engine/RenderBackends/Vulkan/VulkanError.hpp"

namespace VRenderer
{
	void VulkanCommandbufferReset::CleanUp(VkDevice l_device, VkCommandPool l_thisThreadPrimaryCmdPool) noexcept
	{
		if (VK_NULL_HANDLE != l_device && VK_NULL_HANDLE != l_thisThreadPrimaryCmdPool) {
			vkFreeCommandBuffers(l_device, l_thisThreadPrimaryCmdPool, 1U, &m_buffer);
		}
	}

	void VulkanCommandbufferReset::ResetBuffer()
	{
		VULKAN_CHECK(vkResetCommandBuffer(m_buffer, 0));
	}

	void VulkanCommandbufferReset::BeginRecording()
	{
		VkCommandBufferBeginInfo lv_cmdBufferBeginInfo{};
		lv_cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		lv_cmdBufferBeginInfo.pNext = nullptr;
		lv_cmdBufferBeginInfo.pInheritanceInfo = nullptr;
		lv_cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VULKAN_CHECK(vkBeginCommandBuffer(m_buffer, &lv_cmdBufferBeginInfo));
	}

	void VulkanCommandbufferReset::EndRecording()
	{
		VULKAN_CHECK(vkEndCommandBuffer(m_buffer));
	}
}