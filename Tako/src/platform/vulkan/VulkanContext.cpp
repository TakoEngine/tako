#include "VulkanContext.hpp"
#include "Window.hpp"
#ifdef TAKO_WIN32
#include <Windows.h>
#endif
#include "Utility.hpp"
#include <algorithm>
#include "SmallVec.hpp"
#include <vector>
#include <array>
#include <set>
#include <limits>
#include "Math.hpp"
#include <chrono>

#ifdef  TAKO_WIN32
static std::array<const char*, 2> vkWinExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
#endif
static std::array<const char*, 2> vkDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME };
static std::array<const char*, 1> vkWinValidationLayers = { "VK_LAYER_KHRONOS_validation" };

namespace tako
{
	

	VkFormat PipelineAttributeToVkFormat(PipelineVectorAttribute att)
	{
		switch (att)
		{
		case PipelineVectorAttribute::Vec2: return VK_FORMAT_R32G32_SFLOAT;
		case PipelineVectorAttribute::Vec3: return VK_FORMAT_R32G32B32_SFLOAT;
		}
		ASSERT(false);
	}

	size_t PipelineAttributeToSize(PipelineVectorAttribute att)
	{
		switch (att)
		{
		case PipelineVectorAttribute::Vec2: return sizeof(Vector2);
		case PipelineVectorAttribute::Vec3: return sizeof(Vector3);
		}
		ASSERT(false);
	}

	static VkVertexInputBindingDescription GetVertexBindingDescription(const PipelineDescriptor& pipelineDescriptor) {
		uint32_t stride = 0;
		for (int i = 0; i < pipelineDescriptor.vertexAttributeSize; i++)
		{
			stride += PipelineAttributeToSize(pipelineDescriptor.vertexAttributes[i]);
		}
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = stride;
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> GetVertexAttributeDescriptions(const PipelineDescriptor& pipelineDescriptor) {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		attributeDescriptions.resize(pipelineDescriptor.vertexAttributeSize);

		size_t offset = 0;
		for (int i = 0; i < pipelineDescriptor.vertexAttributeSize; i++)
		{
			attributeDescriptions[i].binding = 0;
			attributeDescriptions[i].location = i;
			attributeDescriptions[i].format = PipelineAttributeToVkFormat(pipelineDescriptor.vertexAttributes[i]);
			attributeDescriptions[i].offset = offset;
			offset += PipelineAttributeToSize(pipelineDescriptor.vertexAttributes[i]);
		}

		return attributeDescriptions;
	}

	struct MeshPushConstants {
		Matrix4 renderMatrix;
	};

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			LOG("vk: {}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			LOG_WARN("vk: {}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			LOG_ERR("vk: {}", pCallbackData->pMessage);
			break;
		default:
			LOG_WARN("unkown messageSeverity: {}", messageSeverity);
			LOG("vk: {}", pCallbackData->pMessage);
			break;
		}

		return VK_FALSE;
	}

	VulkanContext::VulkanContext(Window* window)
	{
		WindowHandle windowHandle = window->GetHandle();
		LOG("handle: {}", (uint64_t)windowHandle);
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		LOG("extensions available: {}", extensionCount);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		for (const auto& extension : extensions)
		{
			LOG("{}", extension.extensionName);
		}

		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		for (const auto& layer : availableLayers)
		{
			LOG("{}: {}", layer.layerName, layer.description);
		}


		SmallVec<const char*, 50> vulkanExtensions;
#ifndef NDEBUG
		vulkanExtensions.Push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
#ifdef TAKO_WIN32
		vulkanExtensions.PushArray(vkWinExtensions.data(), vkWinExtensions.size());
#endif
#ifdef TAKO_GLFW
		uint32_t glfwExtCount;
		auto glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);
		vulkanExtensions.PushArray(glfwExts, glfwExtCount);
#endif
		{
			VkApplicationInfo appInfo = {};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "Tako Engine";
			appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
			appInfo.pEngineName = "tako";
			appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
			appInfo.apiVersion = VK_API_VERSION_1_1;

			VkInstanceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;
			createInfo.enabledExtensionCount = vulkanExtensions.GetLength();
			createInfo.ppEnabledExtensionNames = vulkanExtensions.GetData();
#ifndef NDEBUG
			createInfo.enabledLayerCount = vkWinValidationLayers.size();
			createInfo.ppEnabledLayerNames = vkWinValidationLayers.data();
#else
			createInfo.enabledLayerCount = 0;
#endif


			auto result = vkCreateInstance(&createInfo, nullptr, &vkInstance);
			ASSERT(result == VK_SUCCESS);
		}
#ifndef NDEBUG
		{
			VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createInfo.pfnUserCallback = debugCallback;
			createInfo.pUserData = nullptr;

			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT");
			auto result = func(vkInstance, &createInfo, nullptr, &callback);
			ASSERT(result == VK_SUCCESS);
		}
#endif
#if TAKO_GLFW
		{
			auto result = glfwCreateWindowSurface(vkInstance, windowHandle, NULL, &m_surface);
			ASSERT(result == VK_SUCCESS);
		}
#elif TAKO_WIN32
		{
			VkWin32SurfaceCreateInfoKHR createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			createInfo.hwnd = windowHandle;
			createInfo.hinstance = GetModuleHandle(nullptr);

			auto vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(vkInstance, "vkCreateWin32SurfaceKHR");
			auto result = vkCreateWin32SurfaceKHR(vkInstance, &createInfo, nullptr, &m_surface);
			ASSERT(result == VK_SUCCESS);
		}
#endif

		uint32_t deviceCount = 0;
		auto result = vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
		ASSERT(result == VK_SUCCESS);
		std::vector<VkPhysicalDevice> devices(deviceCount);
		result = vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());
		ASSERT(result == VK_SUCCESS);

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		for (const auto& device : devices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			LOG("{}", deviceProperties.deviceName);
			physicalDevice = device;
		}
		m_physicalDevice = physicalDevice;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
		uint32_t graphicsFamily = -1;
		uint32_t presentFamily = -1;
		uint32_t i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueCount <= 0)
			{
				continue;
			}
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			auto result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_surface, &presentSupport);
			ASSERT(result == VK_SUCCESS);
			if (presentSupport)
			{
				presentFamily = i;
			}

			if (graphicsFamily >= 0 && presentFamily >= 0)
			{
				break;
			}
			i++;
		}
		ASSERT(graphicsFamily >= 0 && presentFamily >= 0);
		m_graphicsFamily = graphicsFamily;
		m_presentFamily = presentFamily;

		{
			std::set<uint32_t> uniqueQueueFamilies = { graphicsFamily, presentFamily };
			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(uniqueQueueFamilies.size());

			float queuePriority = 1.0f;
			int i = 0;
			for (uint32_t queueFamily : uniqueQueueFamilies)
			{
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;

				queueCreateInfos[i] = queueCreateInfo;
				i++;
			}

			VkPhysicalDeviceFeatures deviceFeatures = {};
			VkPhysicalDeviceShaderDrawParametersFeatures shaderDrawParameters = {};
			shaderDrawParameters.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
			shaderDrawParameters.pNext = nullptr;
			shaderDrawParameters.shaderDrawParameters = VK_TRUE;

			VkDeviceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			createInfo.pQueueCreateInfos = queueCreateInfos.data();
			createInfo.queueCreateInfoCount = queueCreateInfos.size();
			createInfo.pEnabledFeatures = &deviceFeatures;
			createInfo.enabledExtensionCount = vkDeviceExtensions.size();
			createInfo.ppEnabledExtensionNames = vkDeviceExtensions.data();
			createInfo.enabledLayerCount = vkWinValidationLayers.size();
			createInfo.ppEnabledLayerNames = vkWinValidationLayers.data();
			createInfo.pNext = &shaderDrawParameters;

			auto result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_vkDevice);
			ASSERT(result == VK_SUCCESS);

			vkGetDeviceQueue(m_vkDevice, graphicsFamily, 0, &m_graphicsQueue);
			vkGetDeviceQueue(m_vkDevice, presentFamily, 0, &m_presentQueue);
		}

		const VkFormat imageFormat = VK_FORMAT_B8G8R8A8_UNORM; //TODO: check available
		auto depthFormat = VK_FORMAT_D32_SFLOAT; //TODO: check available
		{
			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format = imageFormat;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription depthAttachment = {};
			depthAttachment.flags = 0;
			depthAttachment.format = depthFormat;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthAttachmentRef = {};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;
			subpass.pDepthStencilAttachment = &depthAttachmentRef;

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 2;
			renderPassInfo.pAttachments = &attachments[0];
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			result = vkCreateRenderPass(m_vkDevice, &renderPassInfo, nullptr, &m_renderPass);
			ASSERT(result == VK_SUCCESS);
		}

		//MARKER


		// Create Camera Buffer Layout
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding = {};
			uboLayoutBinding.binding = 0;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

			VkDescriptorSetLayoutCreateInfo layoutInfo = {};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &uboLayoutBinding;

			result = vkCreateDescriptorSetLayout(m_vkDevice, &layoutInfo, nullptr, &m_descriptorSetLayoutUniform);
			ASSERT(result == VK_SUCCESS);
		}

		{
			VkDescriptorSetLayoutBinding textureBinding = {};
			textureBinding.binding = 0;
			textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			textureBinding.descriptorCount = 1;
			textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			VkDescriptorSetLayoutCreateInfo layoutInfo = {};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &textureBinding;

			result = vkCreateDescriptorSetLayout(m_vkDevice, &layoutInfo, nullptr, &m_descriptorSetLayoutSampler);
			ASSERT(result == VK_SUCCESS);
		}

		{
			VkDescriptorSetLayoutBinding storageBinding = {};
			storageBinding.binding = 0;
			storageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			storageBinding.descriptorCount = 1;
			storageBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			VkDescriptorSetLayoutCreateInfo layoutInfo = {};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &storageBinding;

			result = vkCreateDescriptorSetLayout(m_vkDevice, &layoutInfo, nullptr, &m_descriptorSetLayoutStorage);
			ASSERT(result == VK_SUCCESS);
		}

		{
			VkSamplerCreateInfo samplerInfo = {};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.pNext = nullptr;

			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

			result = vkCreateSampler(m_vkDevice, &samplerInfo, nullptr, &m_pixelSampler);
			ASSERT(result == VK_SUCCESS);
		}

		{
			VkSamplerCreateInfo samplerInfo = {};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.pNext = nullptr;

			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

			result = vkCreateSampler(m_vkDevice, &samplerInfo, nullptr, &m_linearSampler);
			ASSERT(result == VK_SUCCESS);
		}

		{
			VkCommandPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = graphicsFamily;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			auto result = vkCreateCommandPool(m_vkDevice, &poolInfo, nullptr, &m_commandPool);
			ASSERT(result == VK_SUCCESS);
		}

		// createDescriptorPool
		{
			VkDescriptorPoolSize poolSize = {};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = 100;//static_cast<uint32_t>(m_swapChainImages.size());

			VkDescriptorPoolSize texSize = {};
			texSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			texSize.descriptorCount = 100;

			VkDescriptorPoolSize storageBufferSize = {};
			storageBufferSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			storageBufferSize.descriptorCount = 100;

			VkDescriptorPoolSize sizes[] = { poolSize, texSize, storageBufferSize };
			VkDescriptorPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = 3;
			poolInfo.pPoolSizes = sizes;
			poolInfo.maxSets = 100;
			poolInfo.flags = 0;

			result = vkCreateDescriptorPool(m_vkDevice, &poolInfo, nullptr, &m_descriptorPool);
			ASSERT(result == VK_SUCCESS);
		}

		m_currentCameraUniform = MakeCameraDescriptor({ Matrix4::identity, Matrix4::identity, Matrix4::identity }, m_descriptorPool);

		CreateSwapchain();
	}

	void VulkanContext::CreateSwapchain()
	{
		//Swapchain TODO: check if options are available
		const VkFormat imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
		{
			VkSurfaceCapabilitiesKHR capabilities;
			auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities);
			ASSERT(result == VK_SUCCESS);

			/*RECT clientRect;
			GetClientRect(hwnd, &clientRect);
			uint32_t clientWidth = clientRect.right - clientRect.left;
			uint32_t clientHeight = clientRect.bottom - clientRect.top;*/
			uint32_t clientWidth = capabilities.currentExtent.width;
			uint32_t clientHeight = capabilities.currentExtent.height;

			VkSwapchainCreateInfoKHR createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = m_surface;
			createInfo.minImageCount = std::min(capabilities.maxImageCount, std::max(capabilities.minImageCount, (uint32_t) 3)); // triple buffering
			createInfo.imageFormat = imageFormat;
			createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
			createInfo.imageExtent = m_swapChainExtent = { clientWidth, clientHeight }; //TODO: clamp to allowed values
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			uint32_t queueFamiliyIndices[] = { m_graphicsFamily, m_presentFamily };
			if (m_graphicsFamily != m_presentFamily)
			{
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamiliyIndices;
			}
			else
			{
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				createInfo.queueFamilyIndexCount = 0;
				createInfo.pQueueFamilyIndices = nullptr;
			}

			createInfo.preTransform = capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			createInfo.clipped = VK_TRUE;
			createInfo.oldSwapchain = m_swapChain;

			result = vkCreateSwapchainKHR(m_vkDevice, &createInfo, nullptr, &m_swapChain);
			ASSERT(result == VK_SUCCESS);
		}

		{
			uint32_t imageCount;
			auto result = vkGetSwapchainImagesKHR(m_vkDevice, m_swapChain, &imageCount, nullptr);
			ASSERT(result == VK_SUCCESS);
			m_swapChainImages.resize(imageCount);
			result = vkGetSwapchainImagesKHR(m_vkDevice, m_swapChain, &imageCount, m_swapChainImages.data());
			ASSERT(result == VK_SUCCESS);
		}

		m_swapChainImageViews.resize(m_swapChainImages.size());
		for (size_t i = 0; i < m_swapChainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = imageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			auto result = vkCreateImageView(m_vkDevice, &createInfo, nullptr, &m_swapChainImageViews[i]);
			ASSERT(result == VK_SUCCESS);
		}

		auto depthFormat = VK_FORMAT_D32_SFLOAT; //TODO: check available
		{
			//Depth buffer
			VkExtent3D depthImageExtent =
			{
				m_swapChainExtent.width,
				m_swapChainExtent.height,
				1
			};
			VkImageCreateInfo imageCreateInfo = {};
			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCreateInfo.pNext = nullptr;

			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;

			imageCreateInfo.format = depthFormat;
			imageCreateInfo.extent = depthImageExtent;

			imageCreateInfo.mipLevels = 1;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

			auto result = vkCreateImage(m_vkDevice, &imageCreateInfo, nullptr, &m_depthImage);
			ASSERT(result == VK_SUCCESS);

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(m_vkDevice, m_depthImage, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = FindMemoryType(m_physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			result = vkAllocateMemory(m_vkDevice, &allocInfo, nullptr, &m_depthImageMemory);
			ASSERT(result == VK_SUCCESS);

			vkBindImageMemory(m_vkDevice, m_depthImage, m_depthImageMemory, 0);

			VkImageViewCreateInfo imageViewCreateInfo = {};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.pNext = nullptr;

			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.image = m_depthImage;
			imageViewCreateInfo.format = depthFormat;
			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			imageViewCreateInfo.subresourceRange.levelCount = 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;
			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			result = vkCreateImageView(m_vkDevice, &imageViewCreateInfo, nullptr, &m_depthImageView);
			ASSERT(result == VK_SUCCESS);
		}

		{
			m_swapChainFramebuffers.resize(m_swapChainImageViews.size());
			for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
			{
				VkImageView attachments[] =
				{
					m_swapChainImageViews[i],
					m_depthImageView
				};

				VkFramebufferCreateInfo framebufferInfo = {};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = m_renderPass;
				framebufferInfo.attachmentCount = 2;
				framebufferInfo.pAttachments = attachments;
				framebufferInfo.width = m_swapChainExtent.width;
				framebufferInfo.height = m_swapChainExtent.height;
				framebufferInfo.layers = 1;

				auto result = vkCreateFramebuffer(m_vkDevice, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]);
				ASSERT(result == VK_SUCCESS);
			}
		}

		{
			VkDeviceSize bufferSize = sizeof(CameraUniformData);

			m_uniformBuffers.resize(m_swapChainImages.size());
			m_uniformBuffersMemory.resize(m_swapChainImages.size());

			for (size_t i = 0; i < m_swapChainImages.size(); i++)
			{
				CreateVulkanBuffer(m_physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);
			}
		}

		CreateDescriptorSets();

		{
			m_commandBuffers.resize(m_swapChainFramebuffers.size());

			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = m_commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

			auto result = vkAllocateCommandBuffers(m_vkDevice, &allocInfo, m_commandBuffers.data());
			ASSERT(result == VK_SUCCESS);
		}

		for (int i = 0; i < m_swapChainFramebuffers.size(); i++)
		{
			if (m_frameProgresses.size() > i) continue;
			FrameProgress prog;
			VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.pNext = nullptr;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			auto result = vkCreateFence(m_vkDevice, &fenceInfo, nullptr, &prog.renderFence);
			ASSERT(result == VK_SUCCESS);

			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			result = vkCreateSemaphore(m_vkDevice, &semaphoreInfo, nullptr, &prog.imageAvailableSemaphore);
			ASSERT(result == VK_SUCCESS);
			result = vkCreateSemaphore(m_vkDevice, &semaphoreInfo, nullptr, &prog.renderFinishedSemaphore);
			ASSERT(result == VK_SUCCESS);

			//Frame specifc descriptor pool
			VkDescriptorPoolSize poolSize = {};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = 42;

			VkDescriptorPoolSize sizes[] = { poolSize };
			VkDescriptorPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = 1;
			poolInfo.pPoolSizes = sizes;
			poolInfo.maxSets = 42;
			poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

			result = vkCreateDescriptorPool(m_vkDevice, &poolInfo, nullptr, &prog.descriptorPool);
			ASSERT(result == VK_SUCCESS);

			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &m_descriptorSetLayoutStorage;

			result = vkAllocateDescriptorSets(m_vkDevice, &allocInfo, &prog.modelDescriptor);
			ASSERT(result == VK_SUCCESS);

			CreateVulkanBuffer(m_physicalDevice, sizeof(Matrix4) * 300000, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, prog.modelBuffer.buffer, prog.modelBuffer.bufferMemory);

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = prog.modelBuffer.buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = VK_WHOLE_SIZE;

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = prog.modelDescriptor;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(m_vkDevice, 1, &descriptorWrite, 0, nullptr);

			m_frameProgresses.push_back(std::move(prog));
		}
	}

	Pipeline VulkanContext::CreatePipeline(const PipelineDescriptor& pipelineDescriptor)
	{
		PipelineMapEntry entry;
		VkShaderModule vertShaderModule = CreateShaderModule(pipelineDescriptor.vertCode, pipelineDescriptor.vertSize);
		VkShaderModule fragShaderModule = CreateShaderModule(pipelineDescriptor.fragCode, pipelineDescriptor.fragSize);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		auto bindingDescription = GetVertexBindingDescription(pipelineDescriptor);
		auto attributeDescriptions = GetVertexAttributeDescriptions(pipelineDescriptor);

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)m_swapChainExtent.width;
		viewport.height = (float)m_swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = m_swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		/*
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		*/

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.pNext = nullptr;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.minDepthBounds = 0.0f;
		depthStencilState.maxDepthBounds = 1.0f;
		depthStencilState.stencilTestEnable = VK_FALSE;

		std::vector<VkPushConstantRange> pushConstants;
		pushConstants.resize(pipelineDescriptor.pushConstantsSize);
		{
			auto offset = 0;
			for (int i = 0; i < pipelineDescriptor.pushConstantsSize; i++)
			{
				pushConstants[i].offset = offset;
				pushConstants[i].size = pipelineDescriptor.pushConstants[i];
				pushConstants[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				offset += pipelineDescriptor.pushConstants[i];
			}
		}

		entry.uniformUsed = pipelineDescriptor.pipelineUniformSize > 0;

		SmallVec<VkDescriptorSetLayout, 4> layouts;
		layouts.Push(m_descriptorSetLayoutUniform);
		if (entry.uniformUsed)
		{
			layouts.Push(m_descriptorSetLayoutUniform);
		}

		layouts.Push(m_descriptorSetLayoutSampler);
		layouts.Push(m_descriptorSetLayoutStorage);
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = layouts.GetLength();
		pipelineLayoutInfo.pSetLayouts = layouts.GetData();
		pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
		pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size();

		auto result = vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutInfo, nullptr, &entry.layout);
		ASSERT(result == VK_SUCCESS);

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.layout = entry.layout;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.renderPass = m_renderPass;
		pipelineInfo.subpass = 0;

		result = vkCreateGraphicsPipelines(m_vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &entry.pipeline);
		ASSERT(result == VK_SUCCESS);

		vkDestroyShaderModule(m_vkDevice, vertShaderModule, nullptr);
		vkDestroyShaderModule(m_vkDevice, fragShaderModule, nullptr);

		if (pipelineDescriptor.pipelineUniformSize > 0)
		{
			CreateVulkanBuffer(m_physicalDevice, sizeof(pipelineDescriptor.pipelineUniformSize), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, entry.buffer, entry.memory);

			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &m_descriptorSetLayoutUniform;

			result = vkAllocateDescriptorSets(m_vkDevice, &allocInfo, &entry.descriptor);
			ASSERT(result == VK_SUCCESS);

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = entry.buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = VK_WHOLE_SIZE;

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = entry.descriptor;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(m_vkDevice, 1, &descriptorWrite, 0, nullptr);
		}

		static U64 curIndex = 0;
		m_pipelineMap[curIndex] = entry;
		curIndex++;
		return { curIndex - 1 };
	}

	VkShaderModule VulkanContext::CreateShaderModule(U8* codeData, size_t codeSize)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = codeSize;
		createInfo.pCode = reinterpret_cast<const uint32_t*>(codeData);

		VkShaderModule shaderModule;
		auto result = vkCreateShaderModule(m_vkDevice, &createInfo, nullptr, &shaderModule);
		ASSERT(result == VK_SUCCESS);

		return shaderModule;
	}

	VulkanContext::~VulkanContext() noexcept
	{
		ASSERT(vkQueueWaitIdle(m_graphicsQueue) == VK_SUCCESS);
		ASSERT(vkQueueWaitIdle(m_presentQueue) == VK_SUCCESS);
		ASSERT(vkDeviceWaitIdle(m_vkDevice) == VK_SUCCESS); //TODO: wait somewhere else

		for (int i = 0; i < m_frameProgresses.size(); i++)
		{
			vkDestroySemaphore(m_vkDevice, m_frameProgresses[i].renderFinishedSemaphore, nullptr);
			vkDestroySemaphore(m_vkDevice, m_frameProgresses[i].imageAvailableSemaphore, nullptr);
			vkDestroyFence(m_vkDevice, m_frameProgresses[i].renderFence, nullptr);
		}
		vkDestroyImageView(m_vkDevice, m_depthImageView, nullptr);
		vkDestroyImage(m_vkDevice, m_depthImage, nullptr);
		vkFreeMemory(m_vkDevice, m_depthImageMemory, nullptr);
		//vkDestroyCommandPool(m_vkDevice, m_commandPool, nullptr);
		for (auto framebuffer : m_swapChainFramebuffers)
		{
			vkDestroyFramebuffer(m_vkDevice, framebuffer, nullptr);
		}

		//vkDestroyPipeline(m_vkDevice, m_graphicsPipeline, nullptr);
		//vkDestroyPipelineLayout(m_vkDevice, m_pipelineLayout, nullptr);
		vkDestroyRenderPass(m_vkDevice, m_renderPass, nullptr);
		for (auto imageView : m_swapChainImageViews)
		{
			vkDestroyImageView(m_vkDevice, imageView, nullptr);
		}
		for (auto image : m_swapChainImages)
		{
			vkDestroyImage(m_vkDevice, image, nullptr);
		}
		
		vkDestroyDescriptorSetLayout(m_vkDevice, m_descriptorSetLayoutUniform, nullptr);
		vkDestroyDescriptorSetLayout(m_vkDevice, m_descriptorSetLayoutSampler, nullptr);
		for (size_t i = 0; i < m_uniformBuffers.size(); i++)
		{
			vkDestroyBuffer(m_vkDevice, m_uniformBuffers[i], nullptr);
			vkFreeMemory(m_vkDevice, m_uniformBuffersMemory[i], nullptr);
		}
		for (auto [_, buffer] : m_bufferMap)
		{
			vkDestroyBuffer(m_vkDevice, buffer.buffer, nullptr);
			vkFreeMemory(m_vkDevice, buffer.bufferMemory, nullptr);
		}

		for (auto uniformDescriptor : m_cameraUniformFreeList)
		{
			//TODO: descriptor set?
			vkDestroyBuffer(m_vkDevice, uniformDescriptor.buffer, nullptr);
			vkFreeMemory(m_vkDevice, uniformDescriptor.memory, nullptr);
		}
		for (auto uniformDescriptor : m_cameraUniformUsedList)
		{
			//TODO: descriptor set?
			vkDestroyBuffer(m_vkDevice, uniformDescriptor.buffer, nullptr);
			vkFreeMemory(m_vkDevice, uniformDescriptor.memory, nullptr);
		}

		for (auto [_, texture] : m_textureMap)
		{
			vkDestroyImageView(m_vkDevice, texture.imageView, nullptr);
			vkDestroyImage(m_vkDevice, texture.image, nullptr);
			vkFreeMemory(m_vkDevice, texture.imageMemory, nullptr);
		}

		for (auto [_, material] : m_materialMap)
		{
			//TODO:
		}

		for (auto [_, pipeline] : m_pipelineMap)
		{
			vkDestroyPipelineLayout(m_vkDevice, pipeline.layout, nullptr);
			//TODO: descriptor set?
			vkDestroyBuffer(m_vkDevice, pipeline.buffer, nullptr);
			vkFreeMemory(m_vkDevice, pipeline.memory, nullptr);
			vkDestroyPipeline(m_vkDevice, pipeline.pipeline, nullptr);
		}

		DestroySwapchain(false);


		vkDestroySampler(m_vkDevice, m_pixelSampler, nullptr);
		vkDestroySampler(m_vkDevice, m_linearSampler, nullptr);

		vkDestroyCommandPool(m_vkDevice, m_commandPool, nullptr);

		vkDestroyDescriptorPool(m_vkDevice, m_descriptorPool, nullptr);
		vkDestroyDevice(m_vkDevice, nullptr);
		vkDestroySurfaceKHR(vkInstance, m_surface, nullptr);
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT");
		func(vkInstance, callback, nullptr);
		vkDestroyInstance(vkInstance, nullptr);
	}

	U32 VulkanContext::GetWidth()
	{
		return m_swapChainExtent.width;
	}

	U32 VulkanContext::GetHeight()
	{
		return m_swapChainExtent.height;
	}

	void VulkanContext::DestroySwapchain(bool skipSwapchain)
	{
		if (!skipSwapchain)
		{
			vkDestroySwapchainKHR(m_vkDevice, m_swapChain, nullptr);
		}
	}

	VulkanContext::FrameProgress& VulkanContext::GetCurrentFrame()
	{
		return m_frameProgresses[m_currentFrame % m_frameProgresses.size()];
	}

	void VulkanContext::Begin()
	{
		auto& frame = GetCurrentFrame();
		m_cameraUniformFreeList.insert(m_cameraUniformFreeList.end(), m_cameraUniformUsedList.begin(), m_cameraUniformUsedList.end());
		m_cameraUniformUsedList.clear();
		m_acticeImageIndex = 0;
		frame.modelIndex = 0;
		//auto result = vkQueueWaitIdle(m_graphicsQueue);
		//result = vkQueueWaitIdle(m_presentQueue);
		auto result = vkWaitForFences(m_vkDevice, 1, &GetCurrentFrame().renderFence, true, 10000000000);
		ASSERT(result == VK_SUCCESS);
		result = vkResetFences(m_vkDevice, 1, &GetCurrentFrame().renderFence);
		ASSERT(result == VK_SUCCESS);

		{
			uint32_t imageIndex;
			result = vkAcquireNextImageKHR(m_vkDevice, m_swapChain, (std::numeric_limits<uint64_t>::max)(), GetCurrentFrame().imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
			ASSERT(result == VK_SUCCESS);
			m_acticeImageIndex = imageIndex;
		}

		result = vkResetDescriptorPool(m_vkDevice, frame.descriptorPool, 0);
		ASSERT(result == VK_SUCCESS);

		auto commandBuffer = m_commandBuffers[m_acticeImageIndex];
		auto swapChainFramebuffer = m_swapChainFramebuffers[m_acticeImageIndex];
		result = vkResetCommandBuffer(commandBuffer, 0);
		ASSERT(result == VK_SUCCESS);
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		ASSERT(result == VK_SUCCESS);

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		VkClearValue clearDepth;
		clearDepth.depthStencil.depth = 1.0f;
		VkClearValue clearValues[] = { clearColor, clearDepth };
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_swapChainExtent;
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = &clearValues[0];

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanContext::End()
	{
		auto commandBuffer = m_commandBuffers[m_acticeImageIndex];

		vkCmdEndRenderPass(commandBuffer);
		auto result = vkEndCommandBuffer(commandBuffer);
		ASSERT(result == VK_SUCCESS);
	}

	void VulkanContext::Present()
	{
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { GetCurrentFrame().imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandBuffers[m_acticeImageIndex];
		VkSemaphore signalSemaphores[] = { GetCurrentFrame().renderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		auto result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, GetCurrentFrame().renderFence);
		ASSERT(result == VK_SUCCESS);

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		VkSwapchainKHR swapChains[] = { m_swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &m_acticeImageIndex;

		result = vkQueuePresentKHR(m_graphicsQueue, &presentInfo);
		ASSERT(result == VK_SUCCESS);
		m_currentFrame++;
	}

	void VulkanContext::BindPipeline(const Pipeline* pipeline)
	{
		auto commandBuffer = GetActiveCommandBuffer();
		auto& entry = m_pipelineMap[pipeline->value];
		m_currentPipeline = &entry;
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, entry.pipeline);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, entry.layout, 0, 1, &m_currentCameraUniform.descriptor, 0, nullptr);
		//TODO
		if (entry.uniformUsed)
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, entry.layout, 1, 1, &entry.descriptor, 0, nullptr);
		}
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, entry.layout, 3, 1, &GetCurrentFrame().modelDescriptor, 0, nullptr);
		//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[m_acticeImageIndex], 0, nullptr);
	}

	void VulkanContext::BindVertexBuffer(const Buffer* buffer)
	{
		auto commandBuffer = GetActiveCommandBuffer();
		auto entry = m_bufferMap[buffer->value];
		VkBuffer vertexBuffers[] = { entry.buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	}

	void VulkanContext::BindIndexBuffer(const Buffer* buffer)
	{
		auto commandBuffer = GetActiveCommandBuffer();
		auto entry = m_bufferMap[buffer->value];
		vkCmdBindIndexBuffer(commandBuffer, entry.buffer, 0, VK_INDEX_TYPE_UINT16);
	}

	void VulkanContext::BindMaterial(const Material* material)
	{
		auto commandBuffer = GetActiveCommandBuffer();
		const auto& entry = m_materialMap[material->value];
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_currentPipeline->layout, 1 + m_currentPipeline->uniformUsed, 1, &entry.descriptorSet, 0, nullptr);
	}

	void VulkanContext::DrawIndexed(uint32_t indexCount, Matrix4 renderMatrix)
	{
		auto commandBuffer = GetActiveCommandBuffer();

		auto& frame = GetCurrentFrame();
		void* data;
		auto result = vkMapMemory(m_vkDevice, frame.modelBuffer.bufferMemory, frame.modelIndex * sizeof(Matrix4), sizeof(Matrix4), 0, &data);
		ASSERT(result == VK_SUCCESS);
		*reinterpret_cast<Matrix4*>(data) = renderMatrix;
		vkUnmapMemory(m_vkDevice, frame.modelBuffer.bufferMemory);

		/*
		MeshPushConstants constants;
		constants.renderMatrix = renderMatrix;
		vkCmdPushConstants(commandBuffer, m_currentPipeline->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);
		*/
		

		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, frame.modelIndex++);
	}

	void VulkanContext::DrawIndexed(uint32_t indexCount, uint32_t matrixCount, const Matrix4* renderMatrix)
	{
		auto commandBuffer = GetActiveCommandBuffer();

		auto& frame = GetCurrentFrame();
		void* data;
		auto result = vkMapMemory(m_vkDevice, frame.modelBuffer.bufferMemory, frame.modelIndex * sizeof(Matrix4), sizeof(Matrix4) * matrixCount, 0, &data);
		ASSERT(result == VK_SUCCESS);

		memcpy(data, renderMatrix, sizeof(Matrix4) * matrixCount);
		vkUnmapMemory(m_vkDevice, frame.modelBuffer.bufferMemory);

		auto firstInstance = frame.modelIndex;
		frame.modelIndex += matrixCount;
		vkCmdDrawIndexed(commandBuffer, indexCount, matrixCount, 0, 0, firstInstance);
	}

	VkCommandBuffer VulkanContext::GetActiveCommandBuffer() const
	{
		return m_commandBuffers[m_acticeImageIndex];
	}

	void VulkanContext::UpdateCamera(const CameraUniformData& cameraData)
	{
		m_cameraUniformUsedList.push_back(m_currentCameraUniform);
		if (m_cameraUniformFreeList.size() > 0)
		{
			m_currentCameraUniform = m_cameraUniformFreeList.back();
			m_cameraUniformFreeList.pop_back();

			void* data;
			auto result = vkMapMemory(m_vkDevice, m_currentCameraUniform.memory, 0, sizeof(CameraUniformData), 0, &data);
			ASSERT(result == VK_SUCCESS);
			memcpy(data, &cameraData, sizeof(CameraUniformData));
			vkUnmapMemory(m_vkDevice, m_currentCameraUniform.memory);
		}
		else
		{
			m_currentCameraUniform = MakeCameraDescriptor(cameraData, m_descriptorPool);
		}

		auto commandBuffer = GetActiveCommandBuffer();
		if (m_currentPipeline)
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_currentPipeline->layout, 0, 1, &m_currentCameraUniform.descriptor, 0, nullptr);
		}
	}

	void VulkanContext::UpdateUniform(const void* uniformData, size_t uniformSize)
	{
		ASSERT(m_currentPipeline);
		void* data;
		auto result = vkMapMemory(m_vkDevice, m_currentPipeline->memory, 0, VK_WHOLE_SIZE, 0, &data); //TODO: figure out offset?
		ASSERT(result == VK_SUCCESS);
		memcpy(data, uniformData, uniformSize);
		vkUnmapMemory(m_vkDevice, m_currentPipeline->memory);
	}

	uint32_t VulkanContext::FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		ASSERT(false);
	}

	void VulkanContext::CreateVulkanBuffer(VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto result = vkCreateBuffer(m_vkDevice, &bufferInfo, nullptr, &buffer);
		ASSERT(result == VK_SUCCESS);

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_vkDevice, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

		result = vkAllocateMemory(m_vkDevice, &allocInfo, nullptr, &bufferMemory);
		ASSERT(result == VK_SUCCESS);

		result = vkBindBufferMemory(m_vkDevice, buffer, bufferMemory, 0);
		ASSERT(result == VK_SUCCESS);
	}

	void VulkanContext::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		auto result = vkAllocateCommandBuffers(m_vkDevice, &allocInfo, &commandBuffer);
		ASSERT(result == VK_SUCCESS);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		ASSERT(result == VK_SUCCESS);

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		result = vkEndCommandBuffer(commandBuffer);
		ASSERT(result == VK_SUCCESS);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_graphicsQueue);

		vkFreeCommandBuffers(m_vkDevice, m_commandPool, 1, &commandBuffer);
	}

	CameraUniformDescriptor VulkanContext::MakeCameraDescriptor(const CameraUniformData& cameraData, VkDescriptorPool descriptorPool)
	{
		CameraUniformDescriptor camUniform;

		CreateVulkanBuffer(m_physicalDevice, sizeof(CameraUniformData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, camUniform.buffer, camUniform.memory);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_descriptorSetLayoutUniform;

		auto result = vkAllocateDescriptorSets(m_vkDevice, &allocInfo, &camUniform.descriptor);
		ASSERT(result == VK_SUCCESS);

		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = camUniform.buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = camUniform.descriptor;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(m_vkDevice, 1, &descriptorWrite, 0, nullptr);

		void* data;
		result = vkMapMemory(m_vkDevice, camUniform.memory, 0, sizeof(CameraUniformData), 0, &data);
		ASSERT(result == VK_SUCCESS);
		memcpy(data, &cameraData, sizeof(CameraUniformData));
		vkUnmapMemory(m_vkDevice, camUniform.memory);

		return camUniform;
	}

	void VulkanContext::CreateDescriptorSets()
	{
		if (m_swapChainImages.size() == m_swapChainImages.size()) {
			return;
		}
		std::vector<VkDescriptorSetLayout> layouts(m_swapChainImages.size(), m_descriptorSetLayoutUniform);
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(m_swapChainImages.size());
		allocInfo.pSetLayouts = layouts.data();

		m_descriptorSets.resize(m_swapChainImages.size());
		auto result = vkAllocateDescriptorSets(m_vkDevice, &allocInfo, m_descriptorSets.data());
		ASSERT(result == VK_SUCCESS);

		for (size_t i = 0; i < m_swapChainImages.size(); i++)
		{
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = m_uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = VK_WHOLE_SIZE;

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(m_vkDevice, 1, &descriptorWrite, 0, nullptr);
		}
	}

	void VulkanContext::Resize(int width, int height) {
		LOG("RESIZXZEFRZLOSADF")
		m_swapChainExtent = { (uint32_t)width, (uint32_t)height };
		ASSERT(vkQueueWaitIdle(m_graphicsQueue) == VK_SUCCESS);
		ASSERT(vkQueueWaitIdle(m_presentQueue) == VK_SUCCESS);
		ASSERT(vkDeviceWaitIdle(m_vkDevice) == VK_SUCCESS); //TODO: wait somewhere else
		auto oldSwap = m_swapChain;
		DestroySwapchain(true);
		CreateSwapchain();
		vkDestroySwapchainKHR(m_vkDevice, oldSwap, nullptr);
	}

	void VulkanContext::HandleEvent(Event& evt) {}
	Texture VulkanContext::CreateTexture(const Bitmap& bitmap)
	{
		auto pixelData = reinterpret_cast<const void*>(bitmap.GetData());
		VkDeviceSize imageSize = sizeof(Color) * bitmap.Width() * bitmap.Height();
		VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateVulkanBuffer(m_physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		auto result = vkMapMemory(m_vkDevice, stagingBufferMemory, 0, imageSize, 0, &data);
		ASSERT(result == VK_SUCCESS);
		memcpy(data, pixelData, static_cast<size_t>(imageSize));
		vkUnmapMemory(m_vkDevice, stagingBufferMemory);

		VkExtent3D imageExtent;
		imageExtent.width = bitmap.Width();
		imageExtent.height = bitmap.Height();
		imageExtent.depth = 1;

		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;

		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;

		imageCreateInfo.format = imageFormat;
		imageCreateInfo.extent = imageExtent;

		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
		VkDescriptorSet texSet;

		result = vkCreateImage(m_vkDevice, &imageCreateInfo, nullptr, &image);
		ASSERT(result == VK_SUCCESS);

		{
			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(m_vkDevice, image, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = FindMemoryType(m_physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			result = vkAllocateMemory(m_vkDevice, &allocInfo, nullptr, &imageMemory);
			ASSERT(result == VK_SUCCESS);

			vkBindImageMemory(m_vkDevice, image, imageMemory, 0);
		}

		{
			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = m_commandPool;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			auto result = vkAllocateCommandBuffers(m_vkDevice, &allocInfo, &commandBuffer);
			ASSERT(result == VK_SUCCESS);

			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
			ASSERT(result == VK_SUCCESS);

			VkImageSubresourceRange range;
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = 1;

			VkImageMemoryBarrier imageBarrierTransfer = {};
			imageBarrierTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

			imageBarrierTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageBarrierTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrierTransfer.image = image;
			imageBarrierTransfer.subresourceRange = range;

			imageBarrierTransfer.srcAccessMask = 0;
			imageBarrierTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			//barrier the image into the transfer-receive layout
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrierTransfer);

			VkBufferImageCopy copyRegion = {};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;

			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageExtent = imageExtent;

			//copy the buffer into the image
			vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			VkImageMemoryBarrier imageBarrierReadable = imageBarrierTransfer;

			imageBarrierReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrierReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			imageBarrierReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageBarrierReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			//barrier the image into the shader readable layout
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrierReadable);

			result = vkEndCommandBuffer(commandBuffer);
			ASSERT(result == VK_SUCCESS);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(m_graphicsQueue);

			vkFreeCommandBuffers(m_vkDevice, m_commandPool, 1, &commandBuffer);
		}

		vkDestroyBuffer(m_vkDevice, stagingBuffer, nullptr);
		vkFreeMemory(m_vkDevice, stagingBufferMemory, nullptr);

		{
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = image;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			result = vkCreateImageView(m_vkDevice, &createInfo, nullptr, &imageView);
			ASSERT(result == VK_SUCCESS);
		}

		static U64 curIndex = 0;
		m_textureMap[curIndex] = { image, imageMemory, imageView };
		curIndex++;
		return { curIndex - 1 };
	}

	Material VulkanContext::CreateMaterial(const Texture* texture)
	{
		auto& tex = m_textureMap[texture->handle.value];
		VkDescriptorSet descriptorSet;

		VkDescriptorSetAllocateInfo descriptorAlloc = {};
		descriptorAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorAlloc.pNext = nullptr;
		descriptorAlloc.descriptorPool = m_descriptorPool;
		descriptorAlloc.descriptorSetCount = 1;
		descriptorAlloc.pSetLayouts = &m_descriptorSetLayoutSampler;

		auto result = vkAllocateDescriptorSets(m_vkDevice, &descriptorAlloc, &descriptorSet);
		ASSERT(result == VK_SUCCESS);

		VkDescriptorImageInfo imageBufferInfo;
		imageBufferInfo.sampler = m_linearSampler;
		imageBufferInfo.imageView = tex.imageView;
		imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstBinding = 0;
		write.dstSet = descriptorSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &imageBufferInfo;

		vkUpdateDescriptorSets(m_vkDevice, 1, &write, 0, nullptr);

		static U64 curIndex = 0;
		m_materialMap[curIndex] = { descriptorSet };
		curIndex++;
		return { curIndex - 1 };
	}

	constexpr VkBufferUsageFlagBits ConvertBufferType(BufferType bufferType)
	{
		switch (bufferType)
		{
		case BufferType::Vertex: return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		case BufferType::Index: return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		default: ASSERT(false);
		}
	}

	Buffer VulkanContext::CreateBuffer(BufferType bufferType, const void* bufferData, size_t bufferSize)
	{
		BufferMapEntry entry;
		//const VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateVulkanBuffer(m_physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		auto result = vkMapMemory(m_vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		ASSERT(result == VK_SUCCESS);
		memcpy(data, bufferData, static_cast<size_t>(bufferSize));
		vkUnmapMemory(m_vkDevice, stagingBufferMemory);

		//VK_BUFFER_USAGE_INDEX_BUFFER_BIT
		CreateVulkanBuffer(m_physicalDevice, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | ConvertBufferType(bufferType), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			entry.buffer, entry.bufferMemory);

		CopyBuffer(stagingBuffer, entry.buffer, bufferSize);

		vkDestroyBuffer(m_vkDevice, stagingBuffer, nullptr);
		vkFreeMemory(m_vkDevice, stagingBufferMemory, nullptr);

		static U64 curIndex = 0;
		m_bufferMap[curIndex] = entry;
		curIndex++;
		return { curIndex - 1 };
	}
}
