#pragma once
#include <IGraphicsContext.hpp>
#include <vulkan/vulkan.h>

namespace tako
{
    class VulkanContext final : public IGraphicsContext
    {
    public:
        VulkanContext(Window* window);
        virtual ~VulkanContext() override;
        virtual void Present() override;
        virtual void Resize(int width, int height) override;
        virtual void HandleEvent(Event& evt) override;
        virtual Texture CreateTexture(const Bitmap& bitmap) override;

        VkShaderModule CreateShaderModule(const char* codePath);
        void UpdateUniformBuffer(uint32_t currentImage);
        uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
        void CreateBuffer(VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void CreateDescriptorSets();
    private:
        VkExtent2D m_swapChainExtent;
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
        VkDescriptorSetLayout m_descriptorSetLayout;
        VkPipelineLayout m_pipelineLayout;
        VkPipeline m_graphicsPipeline;
        VkCommandPool m_commandPool;
        std::vector<VkCommandBuffer> m_commandBuffers;
        VkSemaphore m_imageAvailableSemaphore;
        VkSemaphore m_renderFinishedSemaphore;
        VkBuffer m_vertexBuffer;
        VkDeviceMemory m_vertexBufferMemory;
        VkBuffer m_indexBuffer;
        VkDeviceMemory m_indexBufferMemory;
        std::vector<VkBuffer> m_uniformBuffers;
        std::vector<VkDeviceMemory> m_uniformBuffersMemory;
        VkDescriptorPool m_descriptorPool;
        std::vector<VkDescriptorSet> m_descriptorSets;
    };
}
