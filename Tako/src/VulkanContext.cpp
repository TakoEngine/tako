#include <GraphicsContext.hpp>
#include "Window.hpp"
#include <Windows.h>
#include "Utility.hpp"
#include "FileSystem.hpp"
#include <algorithm>
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <set>
#include <limits>
#include "Math.hpp"

static std::array<const char*, 3> vkWinExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
static std::array<const char*, 1> vkDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
static std::array<const char*, 1> vkWinValidationLayers = { "VK_LAYER_LUNARG_standard_validation" };

namespace tako
{
    struct Vertex {
        Vector2 pos;
        Vector3 color;

        static constexpr VkVertexInputBindingDescription GetBindingDescription() {
            VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            return attributeDescriptions;
        }
    };

    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
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

    class GraphicsContext::ContextImpl
    {
    public:
        ContextImpl(HWND hwnd)
        {
            uint32_t extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
            LOG("extensions available: {}", extensionCount);
            std::vector<VkExtensionProperties> extensions(extensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
            for (const auto& extension : extensions)
            {
                LOG("{}", extension.extensionName);
            }

            {
                VkApplicationInfo appInfo = {};
                appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                appInfo.pApplicationName = "Tako Engine";
                appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
                appInfo.pEngineName = "tako";
                appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
                appInfo.apiVersion = VK_API_VERSION_1_0;

                VkInstanceCreateInfo createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                createInfo.pApplicationInfo = &appInfo;
                createInfo.enabledExtensionCount = vkWinExtensions.size();
                createInfo.ppEnabledExtensionNames = vkWinExtensions.data();
                createInfo.enabledLayerCount = vkWinValidationLayers.size();
                createInfo.ppEnabledLayerNames = vkWinValidationLayers.data();

                auto result = vkCreateInstance(&createInfo, nullptr, &vkInstance);
                ASSERT(result == VK_SUCCESS);
            }
            {
                VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                createInfo.pfnUserCallback = debugCallback;
                createInfo.pUserData = nullptr;

                auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT");
                auto result = func(vkInstance, &createInfo, nullptr, &callback);
            }

            {
                VkWin32SurfaceCreateInfoKHR createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
                createInfo.hwnd = hwnd;
                createInfo.hinstance = GetModuleHandle(nullptr);

                auto vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(vkInstance, "vkCreateWin32SurfaceKHR");
                auto result = vkCreateWin32SurfaceKHR(vkInstance, &createInfo, nullptr, &m_surface);
                ASSERT(result == VK_SUCCESS);
            }

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

                VkDeviceCreateInfo createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                createInfo.pQueueCreateInfos = queueCreateInfos.data();
                createInfo.queueCreateInfoCount = queueCreateInfos.size();
                createInfo.pEnabledFeatures = &deviceFeatures;
                createInfo.enabledExtensionCount = vkDeviceExtensions.size();
                createInfo.ppEnabledExtensionNames = vkDeviceExtensions.data();
                createInfo.enabledLayerCount = vkWinValidationLayers.size();
                createInfo.ppEnabledLayerNames = vkWinValidationLayers.data();

                auto result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_vkDevice);
                ASSERT(result == VK_SUCCESS);

                vkGetDeviceQueue(m_vkDevice, graphicsFamily, 0, &m_graphicsQueue);
                vkGetDeviceQueue(m_vkDevice, presentFamily, 0, &m_presentQueue);
            }

            VkExtent2D swapChainExtent;
            //Swapchain TODO: check if options are available
            const VkFormat imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
            {
                VkSurfaceCapabilitiesKHR capabilities;
                auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_surface, &capabilities);
                ASSERT(result == VK_SUCCESS);

                RECT clientRect;
                GetClientRect(hwnd, &clientRect);
                uint32_t clientWidth = clientRect.right - clientRect.left;
                uint32_t clientHeight = clientRect.bottom - clientRect.top;

                VkSwapchainCreateInfoKHR createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                createInfo.surface = m_surface;
                createInfo.minImageCount = capabilities.minImageCount + 1; // triple buffering
                createInfo.imageFormat = imageFormat;
                createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                createInfo.imageExtent = swapChainExtent = { clientWidth, clientHeight }; //TODO: clamp to allowed values
                createInfo.imageArrayLayers = 1;
                createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

                uint32_t queueFamiliyIndices[] = { graphicsFamily, presentFamily };
                if (graphicsFamily != presentFamily)
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
                createInfo.oldSwapchain = VK_NULL_HANDLE;

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

                VkSubpassDescription subpass = {};
                subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                subpass.colorAttachmentCount = 1;
                subpass.pColorAttachments = &colorAttachmentRef;

                VkSubpassDependency dependency = {};
                dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                dependency.dstSubpass = 0;
                dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependency.srcAccessMask = 0;
                dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                VkRenderPassCreateInfo renderPassInfo = {};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                renderPassInfo.attachmentCount = 1;
                renderPassInfo.pAttachments = &colorAttachment;
                renderPassInfo.subpassCount = 1;
                renderPassInfo.pSubpasses = &subpass;
                renderPassInfo.dependencyCount = 1;
                renderPassInfo.pDependencies = &dependency;

                result = vkCreateRenderPass(m_vkDevice, &renderPassInfo, nullptr, &m_renderPass);
                ASSERT(result == VK_SUCCESS);
            }

            {
                const char* vertPath = "shaders/shader.vert.spv";
                const char* fragPath = "shaders/shader.frag.spv";
                VkShaderModule vertShaderModule = CreateShaderModule(vertPath);
                VkShaderModule fragShaderModule = CreateShaderModule(fragPath);

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

                auto bindingDescription = Vertex::GetBindingDescription();
                auto attributeDescriptions = Vertex::getAttributeDescriptions();

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
                viewport.width = (float)swapChainExtent.width;
                viewport.height = (float)swapChainExtent.height;
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;

                VkRect2D scissor = {};
                scissor.offset = { 0, 0 };
                scissor.extent = swapChainExtent;

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
                rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
                rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

                VkPipelineColorBlendStateCreateInfo colorBlending = {};
                colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                colorBlending.logicOpEnable = VK_FALSE;
                colorBlending.attachmentCount = 1;
                colorBlending.pAttachments = &colorBlendAttachment;

                VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
                pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

                auto result = vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
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
                pipelineInfo.layout = m_pipelineLayout;
                pipelineInfo.pColorBlendState = &colorBlending;
                pipelineInfo.renderPass = m_renderPass;
                pipelineInfo.subpass = 0;

                result = vkCreateGraphicsPipelines(m_vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline);
                ASSERT(result == VK_SUCCESS);

                vkDestroyShaderModule(m_vkDevice, vertShaderModule, nullptr);
                vkDestroyShaderModule(m_vkDevice, fragShaderModule, nullptr);
            }

            {
                m_swapChainFramebuffers.resize(m_swapChainImageViews.size());
                for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
                {
                    VkImageView attachments[] =
                    {
                        m_swapChainImageViews[i]
                    };

                    VkFramebufferCreateInfo framebufferInfo = {};
                    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                    framebufferInfo.renderPass = m_renderPass;
                    framebufferInfo.attachmentCount = 1;
                    framebufferInfo.pAttachments = attachments;
                    framebufferInfo.width = swapChainExtent.width;
                    framebufferInfo.height = swapChainExtent.height;
                    framebufferInfo.layers = 1;

                    auto result = vkCreateFramebuffer(m_vkDevice, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]);
                    ASSERT(result == VK_SUCCESS);
                }
            }

            {
                VkCommandPoolCreateInfo poolInfo = {};
                poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                poolInfo.queueFamilyIndex = graphicsFamily;

                auto result = vkCreateCommandPool(m_vkDevice, &poolInfo, nullptr, &m_commandPool);
                ASSERT(result == VK_SUCCESS);
            }

            {
                VkBufferCreateInfo bufferInfo = {};
                bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufferInfo.size = sizeof(vertices[0]) * vertices.size();
                bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                result = vkCreateBuffer(m_vkDevice, &bufferInfo, nullptr, &m_vertexBuffer);
                ASSERT(result == VK_SUCCESS);

                VkMemoryRequirements memRequirements;
                vkGetBufferMemoryRequirements(m_vkDevice, m_vertexBuffer, &memRequirements);

                VkMemoryAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;
                allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                
                result = vkAllocateMemory(m_vkDevice, &allocInfo, nullptr, &m_vertexBufferMemory);
                ASSERT(result == VK_SUCCESS);

                result = vkBindBufferMemory(m_vkDevice, m_vertexBuffer, m_vertexBufferMemory, 0);
                ASSERT(result == VK_SUCCESS);

                void* data;
                result = vkMapMemory(m_vkDevice, m_vertexBufferMemory, 0, bufferInfo.size, 0, &data);
                ASSERT(result == VK_SUCCESS);
                memcpy(data, vertices.data(), (size_t) bufferInfo.size);
                vkUnmapMemory(m_vkDevice, m_vertexBufferMemory);
            }

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


            for (size_t i = 0; i < m_commandBuffers.size(); i++)
            {
                VkCommandBufferBeginInfo beginInfo = {};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

                auto result = vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo);
                ASSERT(result == VK_SUCCESS);

                VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
                VkRenderPassBeginInfo renderPassInfo = {};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = m_renderPass;
                renderPassInfo.framebuffer = m_swapChainFramebuffers[i];
                renderPassInfo.renderArea.offset = { 0, 0 };
                renderPassInfo.renderArea.extent = swapChainExtent;
                renderPassInfo.clearValueCount = 1;
                renderPassInfo.pClearValues = &clearColor;

                vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

                VkBuffer vertexBuffers[] = { m_vertexBuffer };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);

                vkCmdDraw(m_commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);

                vkCmdEndRenderPass(m_commandBuffers[i]);
                result = vkEndCommandBuffer(m_commandBuffers[i]);
                ASSERT(result == VK_SUCCESS);
            }

            {
                VkSemaphoreCreateInfo semaphoreInfo = {};
                semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

                auto result = vkCreateSemaphore(m_vkDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore);
                ASSERT(result == VK_SUCCESS);
                result = vkCreateSemaphore(m_vkDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore);
                ASSERT(result == VK_SUCCESS);
            }
        }

        VkShaderModule CreateShaderModule(const char* codePath)
        {
            size_t fileSize = FileSystem::GetFileSize(codePath);
            std::vector<U8> code(fileSize);
            size_t bytesRead = 0;
            bool readSuccess = FileSystem::ReadFile(codePath, code.data(), code.size(), bytesRead);
            ASSERT(readSuccess && fileSize == bytesRead);

            VkShaderModuleCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = code.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

            VkShaderModule shaderModule;
            auto result = vkCreateShaderModule(m_vkDevice, &createInfo, nullptr, &shaderModule);
            ASSERT(result == VK_SUCCESS);

            return shaderModule;
        }

        ~ContextImpl()
        {
            ASSERT(vkDeviceWaitIdle(m_vkDevice) == VK_SUCCESS); //TODO: wait somewhere else

            vkDestroySemaphore(m_vkDevice, m_renderFinishedSemaphore, nullptr);
            vkDestroySemaphore(m_vkDevice, m_imageAvailableSemaphore, nullptr);
            vkDestroyCommandPool(m_vkDevice, m_commandPool, nullptr);
            for (auto framebuffer : m_swapChainFramebuffers)
            {
                vkDestroyFramebuffer(m_vkDevice, framebuffer, nullptr);
            }

            vkDestroyPipeline(m_vkDevice, m_graphicsPipeline, nullptr);
            vkDestroyPipelineLayout(m_vkDevice, m_pipelineLayout, nullptr);
            vkDestroyRenderPass(m_vkDevice, m_renderPass, nullptr);
            for (auto imageView : m_swapChainImageViews)
            {
                vkDestroyImageView(m_vkDevice, imageView, nullptr);
            }
            vkDestroySwapchainKHR(m_vkDevice, m_swapChain, nullptr);
            vkDestroyBuffer(m_vkDevice, m_vertexBuffer, nullptr);
            vkFreeMemory(m_vkDevice, m_vertexBufferMemory, nullptr);
            vkDestroyDevice(m_vkDevice, nullptr);
            vkDestroySurfaceKHR(vkInstance, m_surface, nullptr);
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT");
            func(vkInstance, callback, nullptr);
            vkDestroyInstance(vkInstance, nullptr);
        }

        void Draw()
        {
            auto result = vkQueueWaitIdle(m_presentQueue);
            ASSERT(result == VK_SUCCESS);

            uint32_t imageIndex;
            result = vkAcquireNextImageKHR(m_vkDevice, m_swapChain, (std::numeric_limits<uint64_t>::max)(), m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
            ASSERT(result == VK_SUCCESS);

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphore };
            VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];
            VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphore };
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            ASSERT(result == VK_SUCCESS);

            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;
            VkSwapchainKHR swapChains[] = { m_swapChain };
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;
            presentInfo.pImageIndices = &imageIndex;

            result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
            ASSERT(result == VK_SUCCESS);
        }

        uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
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
    private:
        VkInstance vkInstance;
        VkDebugUtilsMessengerEXT callback;
        VkDevice m_vkDevice;
        VkQueue m_graphicsQueue; //TODO: maybe prefer merged graphics and present queue
        VkQueue m_presentQueue;
        VkSurfaceKHR m_surface;
        VkSwapchainKHR m_swapChain;
        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;
        std::vector<VkFramebuffer> m_swapChainFramebuffers;
        VkRenderPass m_renderPass;
        VkPipelineLayout m_pipelineLayout;
        VkPipeline m_graphicsPipeline;
        VkCommandPool m_commandPool;
        std::vector<VkCommandBuffer> m_commandBuffers;
        VkSemaphore m_imageAvailableSemaphore;
        VkSemaphore m_renderFinishedSemaphore;
        VkBuffer m_vertexBuffer;
        VkDeviceMemory m_vertexBufferMemory;
    };

    GraphicsContext::GraphicsContext(Window& window) : m_impl(new ContextImpl(window.GetHandle()))
    {
    }

    GraphicsContext::~GraphicsContext() = default;

    void GraphicsContext::Present()
    {
        m_impl->Draw();
    }
}
