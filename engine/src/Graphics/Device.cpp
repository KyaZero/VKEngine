#include "Device.h"
#include <Core/Logger.h>
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

namespace vke
{
	// local callback functions
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, [[maybe_unused]] void* pUserData)
	{
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT || messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
			VERBOSE_LOG(pCallbackData->pMessage);

		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			WARNING_LOG(pCallbackData->pMessage);

		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			ERROR_LOG(pCallbackData->pMessage);

		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance m_Instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(m_Instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance m_Instance, VkDebugUtilsMessengerEXT m_DebugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(m_Instance, m_DebugMessenger, pAllocator);
		}
	}

	VkeDevice::VkeDevice(VkeWindow& window) : m_Window(window)
	{
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
		CreateCommandPool();
	}

	VkeDevice::~VkeDevice()
	{
		vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
		vkDestroyDevice(m_Device, nullptr);

		if (EnableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		vkDestroyInstance(m_Instance, nullptr);
	}

	void VkeDevice::CreateInstance()
	{
		if (EnableValidationLayers && !CheckValidationLayerSupport())
		{
			FATAL_LOG("Validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "VKEngine App";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "VKEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (EnableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS) {
			FATAL_LOG("Failed to create m_Instance!");
		}

		HasGflwRequiredInstanceExtensions();
	}

	void VkeDevice::PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			FATAL_LOG("Failed to find GPUs with Vulkan support!");
		}
		INFO_LOG("Device count: %d", deviceCount);
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device))
			{
				m_PhysicalDevice = device;
				break;
			}
		}

		if (m_PhysicalDevice == VK_NULL_HANDLE) {
			FATAL_LOG("Failed to find a suitable GPU!");
		}

		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);
		INFO_LOG("Physical Device: %s", m_Properties.deviceName);
	}

	void VkeDevice::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		// might not really be necessary anymore because device specific validation layers
		// have been deprecated
		if (EnableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to create logical device!");
		}

		vkGetDeviceQueue(m_Device, indices.graphicsFamily, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, indices.presentFamily, 0, &m_PresentQueue);
	}

	void VkeDevice::CreateCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = FindPhysicalQueueFamilies();

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		poolInfo.flags =
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to create command pool!");
		}
	}

	void VkeDevice::CreateSurface()
	{
		m_Window.CreateWindowSurface(m_Instance, &m_Surface);
	}

	bool VkeDevice::IsDeviceSuitable(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = FindQueueFamilies(device);

		bool extensionsSupported = CheckDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) 
		{
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.IsComplete() && extensionsSupported && swapChainAdequate &&
			supportedFeatures.samplerAnisotropy;
	}

	void VkeDevice::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
		createInfo.pUserData = nullptr;  // Optional
	}

	void VkeDevice::SetupDebugMessenger()
	{
		if (!EnableValidationLayers)
			return;
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo);
		if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to set up debug messenger!");
		}
	}

	bool VkeDevice::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
				return false;
		}

		return true;
	}

	std::vector<const char*> VkeDevice::GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (EnableValidationLayers)
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return extensions;
	}

	void VkeDevice::HasGflwRequiredInstanceExtensions()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		VERBOSE_LOG("Available Extensions:");
		std::unordered_set<std::string> available;
		for (const auto& extension : extensions)
		{
			VERBOSE_LOG("\t %s", extension.extensionName);
			available.insert(extension.extensionName);
		}

		VERBOSE_LOG("Required Extensions:");
		auto requiredExtensions = GetRequiredExtensions();
		for (const auto& required : requiredExtensions)
		{
			VERBOSE_LOG("\t %s", required);
			if (available.find(required) == available.end())
			{
				VERBOSE_LOG("Missing required glfw extension");
			}
		}
	}

	bool VkeDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices VkeDevice::FindQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
				indices.graphicsFamilyHasValue = true;
			}
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
			if (queueFamily.queueCount > 0 && presentSupport)
			{
				indices.presentFamily = i;
				indices.presentFamilyHasValue = true;
			}
			if (indices.IsComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	SwapChainSupportDetails VkeDevice::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				device,
				m_Surface,
				&presentModeCount,
				details.presentModes.data());
		}
		return details;
	}

	VkFormat VkeDevice::FindSupportedFormat(
		const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}
		
		FATAL_LOG("Failed to find supported format!");
	}

	uint32_t VkeDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		FATAL_LOG("Failed to find suitable memory type!");
	}

	void VkeDevice::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to create vertex buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
	}

	VkCommandBuffer VkeDevice::BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}

	void VkeDevice::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_GraphicsQueue);

		vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
	}

	void VkeDevice::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;  // Optional
		copyRegion.dstOffset = 0;  // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(commandBuffer);
	}

	void VkeDevice::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layerCount;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		EndSingleTimeCommands(commandBuffer);
	}

	void VkeDevice::CreateImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		if (vkCreateImage(m_Device, &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(m_Device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to allocate image memory!");
		}

		if (vkBindImageMemory(m_Device, image, imageMemory, 0) != VK_SUCCESS)
		{
			FATAL_LOG("Failed to bind image memory!");
		}
	}
}