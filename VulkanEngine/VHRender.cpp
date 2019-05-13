/**
* The Vienna Vulkan Engine
*
* (c) bei Helmut Hlavacs, University of Vienna
*
*/

#include "VHHelper.h"

namespace vh {

	//-------------------------------------------------------------------------------------------------------
#	/**
	*
	* \brief Create a render pass for a light pass
	*
	* \param[in] device The logical Vulkan device
	* \param[in] swapChainImageFormat The swap chain image format
	* \param[in] depthFormat The depth map image format
	* \param[in] loadOp Operation what to do with the framebuffer when starting the render pass: clear, keep, or dont care
	* \param[out] renderPass The new render pass
	* \returns VK_SUCCESS or a Vulkan error code
	*
	*/

	VkResult vhRenderCreateRenderPass(  VkDevice device,
									VkFormat swapChainImageFormat, 
									VkFormat depthFormat, 
									VkAttachmentLoadOp loadOp,
									VkRenderPass *renderPass) {

		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = loadOp;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if (loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = depthFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = loadOp; 
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; 
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if (loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		return vkCreateRenderPass(device, &renderPassInfo, nullptr, renderPass);
	}



	/**
	*
	* \brief Create a render pass for a shadow pass
	*
	* \param[in] device The logical Vulkan device
	* \param[in] depthFormat The depth map image format
	* \param[out] renderPass The new render pass
	* \returns VK_SUCCESS or a Vulkan error code
	*
	*/
	VkResult vhRenderCreateRenderPassShadow(VkDevice device, VkFormat depthFormat, VkRenderPass *renderPass) {

		VkAttachmentDescription attachmentDescription{};
		attachmentDescription.format = depthFormat;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// Clear depth at beginning of the render pass
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// We will read from depth, so it's important to store the depth attachment results
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// We don't care about initial layout of the attachment
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;		// Attachment will be transitioned to shader read at render pass end

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 0;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;			// Attachment will be used as depth/stencil during render pass

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;													// No color attachments
		subpass.pDepthStencilAttachment = &depthReference;									// Reference to our depth attachment

																							// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = 0;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassCreateInfo = {};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &attachmentDescription;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassCreateInfo.pDependencies = dependencies.data();

		return vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, renderPass);
	}


	/**
	*
	* \brief Create a descriptor layout
	*
	* \param[in] device The logical Vulkan device
	* \param[in] counts The number of images in an array
	* \param[in] types Contains the resource types for the increasing bindings
	* \param[in] stageFlags Denotes in which stages they should be used
	* \param[out] descriptorSetLayout The new descriptor set layout
	* \returns VK_SUCCESS or a Vulkan error code
	*
	*/
	VkResult vhRenderCreateDescriptorSetLayout(	VkDevice device,
											std::vector<uint32_t> counts,
											std::vector<VkDescriptorType> types, 
											std::vector<VkShaderStageFlags> stageFlags,
											VkDescriptorSetLayout * descriptorSetLayout) {
		
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		bindings.resize( types.size() );

		for (uint32_t i = 0; i < types.size(); i++) {
			bindings[i].binding = i;
			bindings[i].descriptorCount = counts[i];
			bindings[i].descriptorType = types[i];
			bindings[i].pImmutableSamplers = nullptr;
			bindings[i].stageFlags = stageFlags[i];
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		return vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, descriptorSetLayout);
	}

	/**
	*
	* \brief Create a descriptor pool
	*
	* \param[in] device The logical Vulkan device
	* \param[in] types Contains the resource types in the pool
	* \param[in] numberDesc Denotes how many of them are in the pool
	* \param[out] descriptorPool The new descriptor pool
	* \returns VK_SUCCESS or a Vulkan error code
	*
	*/
	VkResult vhRenderCreateDescriptorPool(	VkDevice device,
										std::vector<VkDescriptorType> types,
										std::vector<uint32_t> numberDesc, 
										VkDescriptorPool * descriptorPool) {
		std::vector<VkDescriptorPoolSize> poolSizes = {};
		poolSizes.resize( types.size() );

		for (uint32_t i = 0; i < types.size(); i++) {
			poolSizes[i].type = types[i];
			poolSizes[i].descriptorCount = numberDesc[i];
		}

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = numberDesc[0];

		return vkCreateDescriptorPool(device, &poolInfo, nullptr, descriptorPool);
	}

	/**
	*
	* \brief Create a number of descriptor sets, one for each frame in the swapchain
	*
	* \param[in] device The logical Vulkan device
	* \param[in] numberDesc Number of sets to be created
	* \param[in] descriptorSetLayout The layout for the set
	* \param[in] descriptorPool The pool from which the set is drawn
	* \param[out] descriptorSets The new descriptor sets
	* \returns VK_SUCCESS or a Vulkan error code
	*
	*/
	VkResult vhRenderCreateDescriptorSets(	VkDevice device, uint32_t numberDesc,
										VkDescriptorSetLayout descriptorSetLayout, 	
										VkDescriptorPool descriptorPool, 
										std::vector<VkDescriptorSet> & descriptorSets) {

		std::vector<VkDescriptorSetLayout> layouts(numberDesc, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(numberDesc);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(numberDesc);
		return vkAllocateDescriptorSets(device, &allocInfo, &descriptorSets[0]);
	}
	

	/**
	*
	* \brief Update a descriptor set
	*
	* Since it can contain variable number of resources, the lists contain either uniformBuffers or imageViews/sampler.
	* The size of the lists MUST be the same!
	*
	* \param[in] device Logical Vulkan device
	* \param[in] descriptorSet The descriptor set to be updated
	* \param[in] uniformBuffers List of UBOs (can contain null handles)
	* \param[in] bufferRanges List of buffer sizes
	* \param[in] textureImageViews List of texture image views to be updated
	* \param[in] textureSamplers List of texture image samplers
	* \returns VK_SUCCESS or a Vulkan error code
	*
	*/
	VkResult vhRenderUpdateDescriptorSet(VkDevice device,
										VkDescriptorSet descriptorSet,
										std::vector<VkBuffer> uniformBuffers,
										std::vector<uint32_t> bufferRanges,
										std::vector<std::vector<VkImageView>> textureImageViews, 
										std::vector<std::vector<VkSampler>> textureSamplers) {

		std::vector<VkWriteDescriptorSet> descriptorWrites = {};
		descriptorWrites.resize(uniformBuffers.size());
		
		std::vector<VkDescriptorBufferInfo> bufferInfos = {};
		bufferInfos.resize(uniformBuffers.size());

		std::vector<std::vector<VkDescriptorImageInfo>> imageInfos = {};	//could be also arrays, so vector of vector
		imageInfos.resize(uniformBuffers.size());

		for (uint32_t i = 0; i < uniformBuffers.size(); i++) {

			descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[i].dstSet = descriptorSet;
			descriptorWrites[i].dstBinding = i;
			descriptorWrites[i].dstArrayElement = 0;
			descriptorWrites[i].descriptorCount = 1;

			if (uniformBuffers[i] != VK_NULL_HANDLE) {
				bufferInfos[i].buffer = uniformBuffers[i];
				bufferInfos[i].offset = 0;
				bufferInfos[i].range = bufferRanges[i];
				descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[i].pBufferInfo = &bufferInfos[i];
			}
			else if ( textureImageViews[i].size()>0 ) {
				for (uint32_t j = 0; j < textureImageViews[i].size(); j++) {
					VkDescriptorImageInfo imageInfo;

					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = textureImageViews[i][j];
					imageInfo.sampler = textureSamplers[i][j];

					imageInfos[i].push_back(imageInfo);
				}
				
				descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[i].pImageInfo = imageInfos[i].data();
				descriptorWrites[i].descriptorCount = (uint32_t)textureImageViews[i].size();
			}
			else {
				return VK_INCOMPLETE;
			}
		}

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		return VK_SUCCESS;
	}

	/**
	*
	* \brief Start rendering in a command buffer
	*
	* \param[in] commandBuffer The command buffer to record into
	* \param[in] renderPass The render pass that should be begun
	* \param[in] frameBuffer The framebuffer for the render pass
	* \param[in] extent Extent of the framebuffer images
	* \returns VK_SUCCESS or a Vulkan error code
	*
	*/
	VkResult vhRenderBeginRenderPass(VkCommandBuffer commandBuffer,
								 VkRenderPass renderPass, 
								 VkFramebuffer frameBuffer, 
								 VkExtent2D extent) {

		std::vector<VkClearValue> clearValues = {};
		VkClearValue cv1, cv2;
		cv1.color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues.push_back(cv1);
		cv2.depthStencil = { 1.0f, 0 };
		clearValues.push_back(cv2);

		return vhRenderBeginRenderPass(commandBuffer, renderPass, frameBuffer, clearValues, extent);
	}



	/**
	*
	* \brief Start rendering in a command buffer
	*
	* \param[in] commandBuffer The command buffer to record into
	* \param[in] renderPass The render pass that should be begun
	* \param[in] frameBuffer The framebuffer for the render pass
	* \param[in] clearValues A list of clear values to clear render targets 
	* \param[in] extent Extent of the framebuffer images
	* \returns VK_SUCCESS or a Vulkan error code
	*
	*/
	VkResult vhRenderBeginRenderPass(	VkCommandBuffer commandBuffer,
									VkRenderPass renderPass,
									VkFramebuffer frameBuffer,
									std::vector<VkClearValue> &clearValues,
									VkExtent2D extent) {

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = frameBuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		if(clearValues.size()>0)
			renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		return VK_SUCCESS;
	}




	/**
	*
	* \brief Present the result of a render operation to a window
	*
	* \param[in] presentQueue The present queue that the image is sent to
	* \param[in] swapChain The Vulkan swap chain
	* \param[in] imageIndex Index of the swap chain image currently used
	* \param[in] signalSemaphore Semaphore that should be signaled once the operation is done
	* \returns VK_SUCCESS or a Vulkan error code
	*
	*/
	VkResult vhRenderPresentResult(	VkQueue presentQueue, VkSwapchainKHR swapChain,
									uint32_t imageIndex, VkSemaphore signalSemaphore ) {
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		VkSemaphore signalSemaphores[] = { signalSemaphore };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		return vkQueuePresentKHR(presentQueue, &presentInfo);
	}


	//-------------------------------------------------------------------------------------------------------
	//PIPELINE

	/**
	*
	* \brief Create a shader module
	*
	* \param[in] device Logical Vulkan device
	* \param[in] code Blob of bytes that hold the SPIR-V shader code
	* \returns the shader module
	*
	*/
	VkShaderModule vhPipeCreateShaderModule(VkDevice device, const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	/**
	*
	* \brief Create a pipeline layout for drawing a light pass
	*
	* \param[in] device Logical Vulkan device
	* \param[in] descriptorSetLayouts Descriptor set layouts
	* \param[in] pushConstantRanges A list with push constant ranges
	* \param[in] pipelineLayout Resulting pipline layout
	* \returns VK_SUCCESS or a Vulkan error code
	*
	*/
	VkResult vhPipeCreateGraphicsPipelineLayout(VkDevice device, 
												std::vector<VkDescriptorSetLayout> descriptorSetLayouts, 
												std::vector<VkPushConstantRange>   pushConstantRanges, 
												VkPipelineLayout *pipelineLayout) {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		if (pushConstantRanges.size() > 0) {
			pipelineLayoutInfo.pushConstantRangeCount = (uint32_t)pushConstantRanges.size();
			pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
		}

		return vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, pipelineLayout);
	}



	/**
	*
	* \brief Create a pipeline state object (PSO) for a light pass
	*
	* \param[in] device Logical Vulkan device
	* \param[in] shaderFileNames List of filenames for the shaders: vertex, fragment, geometry, tess control, tess eval
	* \param[in] swapChainExtent Swapchain extent
	* \param[in] pipelineLayout Pipeline layout
	* \param[in] renderPass Renderpass to be used
	* \param[in] dynamicStates List of dynamic states that can be changed during usage of the pipeline
	* \param[out] graphicsPipeline The new PSO
	* \returns VK_SUCCESS or a Vulkan error code
	*
	*/
	VkResult vhPipeCreateGraphicsPipeline(	VkDevice device,
											std::vector<std::string> shaderFileNames, 
											VkExtent2D swapChainExtent,
											VkPipelineLayout pipelineLayout,
											VkRenderPass renderPass,
											std::vector<VkDynamicState> dynamicStates,
											VkPipeline *graphicsPipeline) {

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages; 

		auto vertShaderCode = vhFileRead(shaderFileNames[0]);

		VkShaderModule vertShaderModule = vhPipeCreateShaderModule(device, vertShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		shaderStages.push_back(vertShaderStageInfo);

		VkShaderModule fragShaderModule = VK_NULL_HANDLE;
		if (shaderFileNames.size()>1 && shaderFileNames[1].size()>0) {
			auto fragShaderCode = vhFileRead(shaderFileNames[1]);

			fragShaderModule = vhPipeCreateShaderModule(device, fragShaderCode);

			VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = fragShaderModule;
			fragShaderStageInfo.pName = "main";

			shaderStages.push_back(fragShaderStageInfo);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto bindingDescription = vhVertex::getBindingDescription();
		auto attributeDescriptions = vhVertex::getAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
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
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();
		if (dynamicStates.size() > 0) {
			dynamicState.pDynamicStates = dynamicStates.data();
		}

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = (uint32_t)shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		if (dynamicStates.size() > 0) {
			pipelineInfo.pDynamicState = &dynamicState;
		}
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		VHCHECKRESULT( vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, graphicsPipeline) );

		if(fragShaderModule != VK_NULL_HANDLE )
			vkDestroyShaderModule(device, fragShaderModule, nullptr);

		vkDestroyShaderModule(device, vertShaderModule, nullptr);
		return VK_SUCCESS;
	}




	/**
	*
	* \brief Create a pipeline state object (PSO) for a shadow pass
	*
	* \param[in] device Logical Vulkan device
	* \param[in] verShaderFilename Name of the vetex shader file
	* \param[in] shadowMapExtent Swapchain extent
	* \param[in] pipelineLayout Pipeline layout
	* \param[in] renderPass Renderpass to be used
	* \param[out] graphicsPipeline The new PSO
	* \returns VK_SUCCESS or a Vulkan error code
	*
	*/
	VkResult vhPipeCreateGraphicsShadowPipeline(VkDevice device,
												std::string verShaderFilename,
												VkExtent2D shadowMapExtent,
												VkPipelineLayout pipelineLayout,
												VkRenderPass renderPass,
												VkPipeline *graphicsPipeline) {

		auto vertShaderCode = vhFileRead(verShaderFilename);

		VkShaderModule vertShaderModule = vhPipeCreateShaderModule(device, vertShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto bindingDescription = vhVertex::getBindingDescription();
		auto attributeDescriptions = vhVertex::getAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)shadowMapExtent.width;
		viewport.height = (float)shadowMapExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = shadowMapExtent;

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
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 0; //1;
		colorBlending.pAttachments = nullptr; //  &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 1;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		VHCHECKRESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, graphicsPipeline ) );

		vkDestroyShaderModule(device, vertShaderModule, nullptr);
		return VK_SUCCESS;
	}


}


