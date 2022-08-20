#pragma once
#include <IGraphicsContext.hpp>
#include <vulkan/vulkan.h>
#include <unordered_map>

namespace tako
{
	struct BufferMapEntry
	{
		VkBuffer buffer;
		VkDeviceMemory bufferMemory;
	};

	struct TextureMapEntry
	{
		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
	};

	struct MaterialMapEntry
	{
		VkDescriptorSet descriptorSet;
	};

	struct PipelineMapEntry
	{
		VkPipeline pipeline;
		VkPipelineLayout layout;
		//Pipeline uniform
		bool uniformUsed;
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorSet descriptor;
	};

	struct CameraUniformDescriptor
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorSet descriptor;
	};

	class VulkanContext final : public IGraphicsContext
	{
	public:
		VulkanContext(Window* window);
		virtual ~VulkanContext() override;
		virtual void Begin() override;
		virtual void End() override;
		virtual void Present() override;
		virtual void Resize(int width, int height) override;
		virtual void HandleEvent(Event& evt) override;

		virtual U32 GetWidth() override;
		virtual U32 GetHeight() override;

		virtual void BindPipeline(const Pipeline* pipeline) override;
		virtual void BindVertexBuffer(const Buffer* buffer) override;
		virtual void BindIndexBuffer(const Buffer* buffer) override;
		virtual void BindMaterial(const Material* material) override;
		virtual void UpdateCamera(const CameraUniformData& cameraData) override;
		virtual void UpdateUniform(const void* uniformData, size_t uniformSize) override;
		virtual void DrawIndexed(uint32_t indexCount, Matrix4 renderMatrix) override;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t matrixCount, const Matrix4* renderMatrix) override;

		virtual Pipeline CreatePipeline(const PipelineDescriptor& pipelineDescriptor) override;
		virtual Texture CreateTexture(const Bitmap& bitmap) override;
		virtual Buffer CreateBuffer(BufferType bufferType, const void* bufferData, size_t bufferSize) override;
		virtual Material CreateMaterial(const Texture* texture) override;

		VkShaderModule CreateShaderModule(U8* codeData, size_t codeSize);
		uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void CreateVulkanBuffer(VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void CreateDescriptorSets();

		VkCommandBuffer GetActiveCommandBuffer() const;
	private:
		struct FrameProgress
		{
			VkSemaphore imageAvailableSemaphore, renderFinishedSemaphore;
			VkFence renderFence;
			VkDescriptorPool descriptorPool;
			VkDescriptorSet modelDescriptor;
			BufferMapEntry modelBuffer;
			int modelIndex = 0;
		};

		FrameProgress& GetCurrentFrame();
		CameraUniformDescriptor MakeCameraDescriptor(const CameraUniformData& cameraData, VkDescriptorPool descriptorPool);
		void CreateSwapchain();
		void DestroySwapchain(bool skipSwapchain);

		VkExtent2D m_swapChainExtent;
		VkInstance vkInstance;
		VkDebugUtilsMessengerEXT callback;
		VkDevice m_vkDevice;

		uint32_t m_graphicsFamily;
		uint32_t m_presentFamily;

		VkQueue m_graphicsQueue;//* //TODO: maybe prefer merged graphics and present queue
		VkQueue m_presentQueue;//*
		VkSurfaceKHR m_surface;
		VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
		std::vector<VkImage> m_swapChainImages;
		std::vector<VkImageView> m_swapChainImageViews;
		std::vector<VkFramebuffer> m_swapChainFramebuffers;
		VkRenderPass m_renderPass;
		PipelineMapEntry* m_currentPipeline = nullptr;
		VkDescriptorSetLayout m_descriptorSetLayoutUniform;
		VkDescriptorSetLayout m_descriptorSetLayoutSampler;
		VkDescriptorSetLayout m_descriptorSetLayoutStorage;
		//VkPipeline m_graphicsPipeline;
		VkCommandPool m_commandPool;
		std::vector<VkCommandBuffer> m_commandBuffers; //*
		uint32_t m_acticeImageIndex;
		uint32_t m_currentFrame = 0;
		VkSampler m_pixelSampler;
		VkSampler m_linearSampler;
		std::vector<FrameProgress> m_frameProgresses;
		//remove when done with cameraUniform
		std::vector<VkBuffer> m_uniformBuffers;
		std::vector<VkDeviceMemory> m_uniformBuffersMemory;
		std::vector<VkDescriptorSet> m_descriptorSets; //*

		VkDescriptorPool m_descriptorPool;
		CameraUniformDescriptor m_currentCameraUniform;
		std::vector<CameraUniformDescriptor> m_cameraUniformFreeList;
		std::vector<CameraUniformDescriptor> m_cameraUniformUsedList;
		VkPhysicalDevice m_physicalDevice;
		std::unordered_map<U64, BufferMapEntry> m_bufferMap;
		std::unordered_map<U64, TextureMapEntry> m_textureMap;
		std::unordered_map<U64, MaterialMapEntry> m_materialMap;
		std::unordered_map<U64, PipelineMapEntry> m_pipelineMap;
		VkImage m_depthImage;
		VkDeviceMemory m_depthImageMemory;
		VkImageView m_depthImageView;
	};
}
