/**
* The Vienna Vulkan Engine
*
* (c) bei Helmut Hlavacs, University of Vienna
*
*/

#pragma once

#include <fstream>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <array>
#include <set>
#include <map>
#include <unordered_map>
#include <thread>
#include <random>
#include <cmath>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/transform.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"
#include <stb_image.h>
#include <stb_image_write.h>
#include <gli/gli.hpp>
#include <ThreadPool.h>
#include "CLInclude.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>



namespace vh {

	///need only for start up
	struct QueueFamilyIndices {
		int graphicsFamily = -1;	///<Index of graphics family
		int presentFamily = -1;		///<Index of present family

		///\returns true if the structure is filled completely
		bool isComplete() {
			return graphicsFamily >= 0 && presentFamily >= 0;
		}
	};

	///need only for start up
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;			///<Surface capabilities
		std::vector<VkSurfaceFormatKHR> formats;		///<Surface formats available
		std::vector<VkPresentModeKHR> presentModes;		///<Possible present modes
	};

	//the following structs are used to fill in uniform buffers, and are used as they are in GLSL shaders

	///per vertex data that is stored in the vertex buffers
	struct vhVertex {
		glm::vec3 pos;			///<Vertex position
		glm::vec3 normal;		///<Vertex normal vector
		glm::vec3 tangent;		///<Tangent vector
		glm::vec2 texCoord;		///<Texture coordinates

		///\returns the binding description of this vertex data structure
		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(vhVertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		} 

		///\returns the vertex attribute description of the vertex data
		static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(vhVertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(vhVertex, normal);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(vhVertex, tangent);

			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(vhVertex, texCoord);

			return attributeDescriptions;
		}

		///Operator for comparing two vertices
		bool operator==(const vhVertex& other) const {
			return pos == other.pos && normal == other.normal && tangent == other.tangent && texCoord == other.texCoord;
		}
	};


	//--------------------------------------------------------------------------------------------------------------------------------
	//declaration of all helper functions

	//use this macro to check the function result, if its not VK_SUCCESS then return the error
	#define VHCHECKRESULT(x) { \
		VkResult retval = (x); \
		if (retval != VK_SUCCESS) { \
			return retval; \
		} \
	}

	

	//--------------------------------------------------------------------------------------------------------------------------------
	//instance (device)

	std::chrono::high_resolution_clock::time_point vhTimeNow();
	float vhTimeDuration(std::chrono::high_resolution_clock::time_point t_prev);
	float vhAverage(float new_val, float avgerage, float weight = 0.8f );

	VkResult vhDevCreateInstance(std::vector<const char*> &extensions, std::vector<const char*> &validationLayers, VkInstance *instance);

	//physical device
	VkResult vhDevPickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, std::vector<const char*> requiredExtensions, VkPhysicalDevice *physicalDevice);
	QueueFamilyIndices vhDevFindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
	VkFormat vhDevFindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat vhDevFindDepthFormat(VkPhysicalDevice physicalDevice);


	//--------------------------------------------------------------------------------------------------------------------------------
	//logical device
	VkResult vhDevCreateLogicalDevice(	VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, 
										std::vector<const char*> requiredDeviceExtensions,
										std::vector<const char*> requiredValidationLayers, 
										VkDevice *device, VkQueue *graphicsQueue, VkQueue *presentQueue);

	//--------------------------------------------------------------------------------------------------------------------------------
	//swapchain
	SwapChainSupportDetails vhDevQuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	VkResult vhSwapCreateSwapChain(	VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice device, VkExtent2D frameBufferExtent,
									VkSwapchainKHR *swapChain, 
									std::vector<VkImage> &swapChainImages, std::vector<VkImageView> &swapChainImageViews,
									VkFormat *swapChainImageFormat, VkExtent2D *swapChainExtent);


	//--------------------------------------------------------------------------------------------------------------------------------
	//buffer
	VkResult vhBufCreateBuffer( VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
								VmaMemoryUsage vmaUsage, VkBuffer *buffer, VmaAllocation *allocation);
	VkResult vhBufCopyBuffer(	VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkResult vhBufCreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageViewType viewtype, uint32_t layerCount, VkImageAspectFlags aspectFlags, VkImageView *imageView);
	VkResult vhBufCreateDepthResources(	VkDevice device, VmaAllocator allocator, VkQueue graphicsQueue,
										VkCommandPool commandPool, VkExtent2D swapChainExtent, VkFormat depthFormat,
										VkImage *depthImage, VmaAllocation *depthImageAllocation, VkImageView * depthImageView);
	VkResult vhBufCreateImage(	VmaAllocator allocator, uint32_t width, uint32_t height,
								uint32_t miplevels, uint32_t arrayLayers,
								VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags,
								VkImage* image, VmaAllocation* allocation);

	VkResult vhBufCopyBufferToImage(VkDevice device, VkQueue queue, VkCommandPool commandPool,
									VkBuffer buffer, VkImage image, uint32_t layerCount, uint32_t width, uint32_t height);

	VkResult vhBufCopyBufferToImage(VkDevice device, VkQueue queue, VkCommandPool commandPool,
									VkBuffer buffer, VkImage image, std::vector<VkBufferImageCopy> &regions,
									uint32_t width, uint32_t height);
	VkResult vhBufCopyImageToBuffer(VkDevice device, VkQueue queue, VkCommandPool commandPool,
									VkImage image, VkImageAspectFlagBits aspect, VkBuffer buffer, uint32_t layerCount, uint32_t width, uint32_t height);
	VkResult vhBufCopyImageToBuffer(VkDevice device, VkQueue queue, VkCommandPool commandPool,
									VkImage image, VkBuffer buffer, std::vector<VkBufferImageCopy> &regions,
									uint32_t width, uint32_t height);
	VkResult vhBufTransitionImageLayout(VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool,
									VkImage image, VkFormat format, VkImageAspectFlagBits aspect, uint32_t miplevels, uint32_t layerCount,
									VkImageLayout oldLayout, VkImageLayout newLayout);
	VkResult vhBufTransitionImageLayout(VkDevice device, VkQueue graphicsQueue, VkCommandBuffer commandBuffer,
									VkImage image, VkFormat format, VkImageAspectFlagBits aspect, uint32_t miplevels, uint32_t layerCount,
									VkImageLayout oldLayout, VkImageLayout newLayout);
	VkResult vhBufCreateTextureImage(VkDevice device, VmaAllocator allocator, VkQueue graphicsQueue, VkCommandPool commandPool, std::string basedir, std::vector<std::string> names, VkImageCreateFlags flags, VkImage *textureImage, VmaAllocation *textureImageAllocation, VkExtent2D *extent);
	VkResult vhBufCreateTexturecubeImage(VkDevice device, VmaAllocator allocator, VkQueue graphicsQueue, VkCommandPool commandPool, gli::texture_cube &cube, VkImage *textureImage, VmaAllocation *textureImageAllocation, VkFormat *pformat);
	VkResult vhBufCreateTextureSampler(VkDevice device, VkSampler *textureSampler);
	VkResult vhBufCreateFramebuffers(VkDevice device, std::vector<VkImageView> imageViews,
									std::vector<VkImageView> depthImageViews, VkRenderPass renderPass, VkExtent2D extent,
									std::vector<VkFramebuffer> &frameBuffers);
	VkResult vhBufCopySwapChainImageToHost(	VkDevice device, VmaAllocator allocator, VkQueue graphicsQueue,
											VkCommandPool commandPool, VkImage image, VkImageAspectFlagBits aspect, gli::byte *bufferData,
											uint32_t width, uint32_t height, uint32_t imageSize);
	VkResult vhBufCopyImageToHost(	VkDevice device, VmaAllocator allocator, VkQueue graphicsQueue,
									VkCommandPool commandPool,
									VkImage image, VkFormat format, VkImageAspectFlagBits aspect, VkImageLayout layout,
									gli::byte *bufferData, uint32_t width, uint32_t height, uint32_t imageSize);
	VkResult vhBufCreateVertexBuffer(VkDevice device, VmaAllocator allocator,
									VkQueue graphicsQueue, VkCommandPool commandPool,
									std::vector<vh::vhVertex> &vertices,
									VkBuffer *vertexBuffer, VmaAllocation *vertexBufferAllocation);
	VkResult vhBufCreateIndexBuffer(VkDevice device, VmaAllocator allocator, VkQueue graphicsQueue, VkCommandPool commandPool,
									std::vector<uint32_t> &indices,
									VkBuffer *indexBuffer, VmaAllocation *indexBufferAllocation);
	VkResult vhBufCreateUniformBuffers(	VmaAllocator allocator,
									uint32_t numberBuffers, VkDeviceSize bufferSize, 
									std::vector<VkBuffer> &uniformBuffers, 
									std::vector<VmaAllocation> &uniformBuffersAllocation);

	//--------------------------------------------------------------------------------------------------------------------------------
	//rendering
	VkResult vhRenderCreateRenderPass( VkDevice device, VkFormat swapChainImageFormat, VkFormat depthFormat, VkAttachmentLoadOp loadOp, VkRenderPass *renderPass);
	VkResult vhRenderCreateRenderPassShadow( VkDevice device, VkFormat depthFormat, VkRenderPass *renderPass);

	VkResult vhRenderCreateDescriptorSetLayout(	VkDevice device, std::vector<uint32_t> counts, std::vector<VkDescriptorType> types,
											std::vector<VkShaderStageFlags> stageFlags, VkDescriptorSetLayout * descriptorSetLayout);
	VkResult vhRenderCreateDescriptorPool(	VkDevice device, std::vector<VkDescriptorType> types,
											std::vector<uint32_t> numberDesc, VkDescriptorPool * descriptorPool);
	VkResult vhRenderCreateDescriptorSets(	VkDevice device, uint32_t numberDesc,
											VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool,
											std::vector<VkDescriptorSet> & descriptorSets);
	VkResult vhRenderUpdateDescriptorSet(VkDevice device, VkDescriptorSet descriptorSet,
										std::vector<VkBuffer> uniformBuffers,
										std::vector<uint32_t> bufferRanges,
										std::vector<std::vector<VkImageView>> textureImageViews,
										std::vector<std::vector<VkSampler>> textureSamplers);
	VkResult vhRenderBeginRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer frameBuffer, VkExtent2D extent);
	VkResult vhRenderBeginRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer frameBuffer,
									std::vector<VkClearValue> &clearValues, VkExtent2D extent);
	VkResult vhRenderPresentResult(	VkQueue presentQueue, VkSwapchainKHR swapChain,
									uint32_t imageIndex, VkSemaphore signalSemaphore);

	VkResult vhPipeCreateGraphicsPipelineLayout(VkDevice device, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, std::vector<VkPushConstantRange> pushConstantRanges, VkPipelineLayout *pipelineLayout);
	VkResult vhPipeCreateGraphicsPipeline(	VkDevice device, std::vector<std::string> shaderFileNames,
											VkExtent2D swapChainExtent, VkPipelineLayout pipelineLayout, VkRenderPass renderPass,
											std::vector<VkDynamicState> dynamicStates, VkPipeline *graphicsPipeline);
	VkResult vhPipeCreateGraphicsShadowPipeline(VkDevice device, std::string verShaderFilename,
												VkExtent2D shadowMapExtent, VkPipelineLayout pipelineLayout,
												VkRenderPass renderPass, VkPipeline *graphicsPipeline);

	//--------------------------------------------------------------------------------------------------------------------------------
	//file
	std::vector<char> vhFileRead(const std::string& filename);

	//--------------------------------------------------------------------------------------------------------------------------------
	//command
	VkResult vhCmdCreateCommandPool(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkCommandPool *commandPool);

	VkResult vhCmdCreateCommandBuffers(	VkDevice device, VkCommandPool commandPool,
										VkCommandBufferLevel level, uint32_t count, VkCommandBuffer *pBuffers);
	VkResult vhCmdBeginCommandBuffer(	VkDevice device, VkCommandBuffer commandBuffer,
										VkCommandBufferUsageFlagBits usageFlags);
	VkResult vhCmdSubmitCommandBuffer(	VkDevice device, VkQueue graphicsQueue, VkCommandBuffer commandBuffer,
										VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence waitFence);
	VkCommandBuffer vhCmdBeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
	VkResult vhCmdEndSingleTimeCommands(VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkCommandBuffer commandBuffer);
	VkResult vhCmdEndSingleTimeCommands(VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkCommandBuffer commandBuffer,
										VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence waitFence);

	//--------------------------------------------------------------------------------------------------------------------------------
	//memory
	uint32_t vhMemFindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkResult vhMemCreateVMAAllocator(VkPhysicalDevice physicalDevice, VkDevice device, VmaAllocator &allocator);

	//--------------------------------------------------------------------------------------------------------------------------------
	//debug
	VKAPI_ATTR VkBool32 VKAPI_CALL vhDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
	VkResult vhDebugCreateReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
	void vhDebugDestroyReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
	void vhSetupDebugCallback(VkInstance instance, VkDebugReportCallbackEXT *callback);

}



#include "nuklear-glfw-vulkan.h"

