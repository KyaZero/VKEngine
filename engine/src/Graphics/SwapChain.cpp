#include "SwapChain.h"

#include <array>

namespace vke
{
	VkeSwapChain::VkeSwapChain(VkeDevice& device, VkExtent2D windowExtent) :
		m_Device(device),
		m_WindowExtent(windowExtent),
		m_CurrentFrame(0),
		m_RenderPass(),
		m_SwapChain(),
		m_SwapChainExtent(),
		m_SwapChainImageFormat()
	{
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateDepthResources();
		CreateFramebuffers();
		CreateSyncObjects();
	}

	VkeSwapChain::~VkeSwapChain()
	{
		for (auto imageView : m_SwapChainImageViews)
		{
			vkDestroyImageView(m_Device.Device(), imageView, nullptr);
		}
		m_SwapChainImageViews.clear();

		if (m_SwapChain != nullptr)
		{
			vkDestroySwapchainKHR(m_Device.Device(), m_SwapChain, nullptr);
			m_SwapChain = nullptr;
		}

		for (int i = 0; i < m_DepthImages.size(); ++i)
		{
			vkDestroyImageView(m_Device.Device(), m_DepthImageViews[i], nullptr);
			vkDestroyImage(m_Device.Device(), m_DepthImages[i], nullptr);
			vkFreeMemory(m_Device.Device(), m_DepthImageMemorys[i], nullptr);
		}

		for (auto framebuffer : m_SwapChainFramebuffers)
		{
			vkDestroyFramebuffer(m_Device.Device(), framebuffer, nullptr);
		}

		vkDestroyRenderPass(m_Device.Device(), m_RenderPass, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(m_Device.Device(), m_RenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_Device.Device(), m_ImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_Device.Device(), m_InFlightFences[i], nullptr);
		}
	}

	VkFormat VkeSwapChain::FindDepthFormat()
	{
		return m_Device.FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	VkResult VkeSwapChain::AcquireNextImage(uint32_t* imageIndex)
	{
		vkWaitForFences(m_Device.Device(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

		VkResult result = vkAcquireNextImageKHR(m_Device.Device(), m_SwapChain, std::numeric_limits<uint64_t>::max(), m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, imageIndex);

		return result;
	}

	VkResult VkeSwapChain::SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex)
	{
		if (m_ImagesInFlight[*imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(m_Device.Device(), 1, &m_ImagesInFlight[*imageIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
		}

		m_ImagesInFlight[*imageIndex] = m_InFlightFences[m_CurrentFrame];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = buffers;

		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_Device.Device(), 1, &m_InFlightFences[m_CurrentFrame]);
		if (vkQueueSubmit(m_Device.GraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = imageIndex;

		VkResult result = vkQueuePresentKHR(m_Device.PresentQueue(), &presentInfo);
		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		return result;
	}

	void VkeSwapChain::CreateSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = m_Device.GetSwapChainSupport();

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Device.Surface();

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = m_Device.FindPhysicalQueueFamilies();
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;      // Optional
			createInfo.pQueueFamilyIndices = nullptr;  // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_Device.Device(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to create swap chain!");
		}

		// We only specified a minimum number of images in the swap chain, so the implementation is
		// allowed to create a swap chain with more. That's why we'll first query the final number of
		// images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
		// retrieve the handles.
		vkGetSwapchainImagesKHR(m_Device.Device(), m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_Device.Device(), m_SwapChain, &imageCount, m_SwapChainImages.data());

		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;
	}

	void VkeSwapChain::CreateImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());
		for (size_t i = 0; i < m_SwapChainImages.size(); ++i)
		{
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = m_SwapChainImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = m_SwapChainImageFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_Device.Device(), &viewInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS)
			{
				FATAL_LOG("Failed to create texture image view!");
			}
		}
	}

	void VkeSwapChain::CreateDepthResources()
	{
		VkFormat depthFormat = FindDepthFormat();
		VkExtent2D swapChainExtent = GetSwapChainExtent();

		m_DepthImages.resize(ImageCount());
		m_DepthImageMemorys.resize(ImageCount());
		m_DepthImageViews.resize(ImageCount());

		for (int i = 0; i < m_DepthImages.size(); i++) 
		{
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = swapChainExtent.width;
			imageInfo.extent.height = swapChainExtent.height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = depthFormat;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.flags = 0;

			m_Device.CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImages[i], m_DepthImageMemorys[i]);

			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = m_DepthImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = depthFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_Device.Device(), &viewInfo, nullptr, &m_DepthImageViews[i]) != VK_SUCCESS) 
			{
				FATAL_LOG("Failed to create texture image view!");
			}
		}
	}

	void VkeSwapChain::CreateRenderPass()
	{
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = FindDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = GetSwapChainImageFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstSubpass = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(m_Device.Device(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) 
		{
			FATAL_LOG("Failed to create render pass!");
		}
	}

	void VkeSwapChain::CreateFramebuffers()
	{

	}
}