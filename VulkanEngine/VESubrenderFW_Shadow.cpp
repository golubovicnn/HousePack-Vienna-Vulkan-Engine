/**
* The Vienna Vulkan Engine
*
* (c) bei Helmut Hlavacs, University of Vienna
*
*/


#include "VEInclude.h"


namespace ve {

	/**
	* \brief Initialize the subrenderer
	*
	* Create descriptor set layout, pipeline layout and the PSO
	*
	*/
	void VESubrenderFW_Shadow::initSubrenderer() {
		VESubrender::initSubrenderer();

		VkDescriptorSetLayout perObjectLayout = getRendererForwardPointer()->getDescriptorSetLayoutPerObject();
		vh::vhPipeCreateGraphicsPipelineLayout(getRendererForwardPointer()->getDevice(),
			{ perObjectLayout, perObjectLayout, getRendererForwardPointer()->getDescriptorSetLayoutShadow(), perObjectLayout },
			{ },
			&m_pipelineLayout);

		m_pipelines.resize(1);
		vh::vhPipeCreateGraphicsShadowPipeline(getRendererForwardPointer()->getDevice(),
			"shader/Forward/Shadow/vert.spv", 
			getRendererForwardPointer()->getShadowMapExtent(),
			m_pipelineLayout, getRendererForwardPointer()->getRenderPassShadow(),
			&m_pipelines[0]);
	}

	/**
	*
	* \brief Add an entity to the list of associated entities.
	*
	* \param[in] pEntity Pointer to the entity to include into the list.
	*
	*/
	void VESubrenderFW_Shadow::addEntity(VEEntity *pEntity) {
		m_entities.push_back(pEntity);
	}


	/**
	*
	* \brief Bind default descriptor sets
	*
	* The function binds the default descriptor sets. Can be overloaded.
	*
	* \param[in] commandBuffer The command buffer to record into all draw calls
	* \param[in] imageIndex Index of the current swap chain image
	* \param[in] entity Pointer to the entity to draw
	*
	*/
	void VESubrenderFW_Shadow::bindDescriptorSetsPerEntity(VkCommandBuffer commandBuffer, uint32_t imageIndex, VEEntity *entity) {

		//set 0...cam UBO
		//set 1...light resources
		//set 2...shadow maps
		//set 3...per object UBO
		//set 4...additional per object resources

		std::vector<VkDescriptorSet> sets = { entity->m_descriptorSetsUBO[imageIndex] };

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 3, (uint32_t)sets.size(), sets.data(), 0, nullptr);
	}


	/**
	* \brief Draw all associated entities for the shadow pass
	*
	* The subrenderer maintains a list of all associated entities. In this function it goes through all of them
	* and draws them. A vector is used in order to be able to parallelize this in case thousands or objects are in the list
	*
	* \param[in] commandBuffer The command buffer to record into all draw calls
	* \param[in] imageIndex Index of the current swap chain image
	* \param[in] numPass The number of the light that has been rendered
	* \param[in] pCamera Pointer to the current light camera
	* \param[in] pLight Pointer to the current light
	* \param[in] descriptorSetsShadow The shadow maps to be used.
	*
	*/
	void VESubrenderFW_Shadow::draw(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t numPass,
									VECamera *pCamera, VELight *pLight,
									std::vector<VkDescriptorSet> descriptorSetsShadow) {

		bindPipeline(commandBuffer);

		bindDescriptorSetsPerFrame(commandBuffer, imageIndex, pCamera, pLight, descriptorSetsShadow);

		//go through all entities and draw them
		for (auto object : getSceneManagerPointer()->m_sceneNodes) {
			VESceneNode *pObject = object.second;
			if (pObject->getNodeType() == VESceneNode::VE_OBJECT_TYPE_ENTITY) {
				VEEntity *pEntity = (VEEntity*)pObject;

				if (pEntity->m_drawEntity && pEntity->m_castsShadow) {
					bindDescriptorSetsPerEntity(commandBuffer, imageIndex, pEntity);	//bind the entity's descriptor sets
					drawEntity(commandBuffer, imageIndex, pEntity);
				}
			}
		}
	}
}

