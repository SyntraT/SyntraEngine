#pragma once

#include "Syngine/engine/RenderBackends/Vulkan/VulkanFoundationalElements.hpp"
#include "Syngine/engine/RenderBackends/Vulkan/VulkanSwapchain.hpp"
#include "Syngine/engine/RenderBackends/Vulkan/VulkanQueueGraphics.hpp"
#include "Syngine/engine/RenderBackends/Vulkan/VulkanCommandbufferReset.hpp"
#include "Syngine/engine/RenderBackends/Vulkan/VulkanSwapchainAndPresentSync.hpp"
#include "Syngine/engine/RenderBackends/Vulkan/VulkanResourceManager.hpp"
#include "Syngine/engine/RenderBackends/Vulkan/VulkanDescriptorSetAllocator.hpp"
#include <vma/vk_mem_alloc.h>

struct SDL_Window;

namespace VRenderer
{
	class Renderer final
	{
	public:

		VulkanCommandbufferReset& GetCurrentFrameGraphicsCmdBuffer();
		VulkanSwapchainAndPresentSync& GetCurrentFrameSwapchainPresentSyncPrimitives();
		uint32_t GetCurrentFrameInflightIndex() const;

		void Init(SDL_Window* l_window);
		void InitCleanUp();

		void Draw();

		~Renderer();
	private:
		void CleanUp() noexcept;

		void RecordCommands(VkCommandBuffer l_cmd, const uint32_t l_swapchainIndex, const uint32_t l_frameInflightIndex);

		void InitializeVulkanFoundationalElementsAndGraphicsQueue(SDL_Window* l_window);
		void InitializeVulkanSwapchain(SDL_Window* l_window);
		void InitializeVulkanGraphicsCommandPoolAndbuffers();
		void InitializeVulkanSwapchainAndPresentSyncPrimitives();
		void TransitionImageLayoutSwapchainImagesToPresentUponCreation();
		void InitializeVmaAllocator();
		void InitializeDescriptorSetPool();

	public:

		VulkanFoundationalElements m_vulkanFoundational{};
		VulkanSwapchain m_vulkanSwapchain{};
		VulkanQueueGraphics m_vulkanQueue{};
		VkDevice m_device{};
		VmaAllocator m_vmaAlloc{};
		VulkanResourceManager m_vulkanResManager{};
		VulkanDescriptorSetAllocator m_mainDescriptorSetAlloc{};
		VkCommandPool m_thisThreadMainPrimaryCmdPool{};

	private:
		static constexpr uint32_t m_maxCommandBuffers{ 2U };
		uint64_t m_currentGraphicsCmdBufferAndSwapchainPresentSyncIndex{};
		std::array<VulkanCommandbufferReset, m_maxCommandBuffers> m_vulkanGraphicsCmdBuffers{};
		std::array<VulkanSwapchainAndPresentSync, m_maxCommandBuffers> m_swapchainPresentSyncPrimitives{};

		//Test code
		std::array<VkDescriptorSet, m_maxCommandBuffers> m_testComputeSets{};
	};
}