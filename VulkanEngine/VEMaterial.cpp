/**
* The Vienna Vulkan Engine
*
* (c) bei Helmut Hlavacs, University of Vienna
*
*/


#include "VEInclude.h"


namespace ve {

	//---------------------------------------------------------------------
	//Mesh

	/**
	*
	* \brief VEMesh constructor from an Assimp aiMesh.
	*
	* Create a VEMesh from an Assmip aiMesh input.
	*
	* \param[in] name The name of the mesh.
	* \param[in] paiMesh Pointer to the Assimp aiMesh that is the source of this mesh.
	*
	*/

	VEMesh::VEMesh(	std::string name, const aiMesh *paiMesh) : VENamedClass(name) {
		std::vector<vh::vhVertex>	vertices;	//vertex array
		std::vector<uint32_t>		indices;	//index array

		//copy the mesh vertex data
		m_vertexCount = paiMesh->mNumVertices;
		m_boundingSphereRadius = 0.0f;
		m_boundingSphereCenter = glm::vec3( 0.0f, 0.0f, 0.0f );
		for (uint32_t i = 0; i < paiMesh->mNumVertices; i++) {
			vh::vhVertex vertex;
			vertex.pos.x = paiMesh->mVertices[i].x;								//copy 3D position in local space
			vertex.pos.y = paiMesh->mVertices[i].y;
			vertex.pos.z = paiMesh->mVertices[i].z;

			m_boundingSphereRadius = std::max(
										std::max(	std::max( vertex.pos.x*vertex.pos.x, vertex.pos.y*vertex.pos.y ),
													vertex.pos.z*vertex.pos.z),
										m_boundingSphereRadius);

			if (paiMesh->HasNormals()) {										//copy normals
				vertex.normal.x = paiMesh->mNormals[i].x;
				vertex.normal.y = paiMesh->mNormals[i].y;
				vertex.normal.z = paiMesh->mNormals[i].z;
			}

			if (paiMesh->HasTangentsAndBitangents() && paiMesh->mTangents) {	//copy tangents
				vertex.tangent.x = paiMesh->mTangents[i].x;
				vertex.tangent.y = paiMesh->mTangents[i].y;
				vertex.tangent.z = paiMesh->mTangents[i].z;
			}

			if (paiMesh->HasTextureCoords(0)) {									//copy texture coordinates
				vertex.texCoord.x = paiMesh->mTextureCoords[0][i].x;
				vertex.texCoord.y = paiMesh->mTextureCoords[0][i].y;
			}

			vertices.push_back(vertex);
		}
		m_boundingSphereRadius = sqrt(m_boundingSphereRadius);

		//got through the aiMesh faces, and copy the indices
		m_indexCount = 0;
		for (uint32_t i = 0; i < paiMesh->mNumFaces; i++) {
			for (uint32_t j = 0; j < paiMesh->mFaces[i].mNumIndices; j++) {
				indices.push_back(paiMesh->mFaces[i].mIndices[j]);
				m_indexCount++;
			}
		}

		//create the vertex buffer
		VECHECKRESULT(vh::vhBufCreateVertexBuffer(	getRendererPointer()->getDevice(), getRendererPointer()->getVmaAllocator(),
													getRendererPointer()->getGraphicsQueue(), getRendererPointer()->getCommandPool(),
													vertices, &m_vertexBuffer, &m_vertexBufferAllocation),
					"Could not create vertex buffer for " + name );

		//create the index buffer
		VECHECKRESULT( vh::vhBufCreateIndexBuffer(	getRendererPointer()->getDevice(), getRendererPointer()->getVmaAllocator(),
													getRendererPointer()->getGraphicsQueue(), getRendererPointer()->getCommandPool(),
													indices, &m_indexBuffer, &m_indexBufferAllocation),
					"Could not create index buffer for " + name);

	}


	/**
	*
	* \brief VEMesh constructor from a vertex and an index list
	*
	* \param[in] name The name of the mesh.
	* \param[in] vertices A list of vertices to be used
	* \param[in] indices A list of indices to be used
	*
	*/

	VEMesh::VEMesh(std::string name, std::vector<vh::vhVertex> vertices, std::vector<uint32_t> indices) : VENamedClass(name) {

		//copy the mesh vertex data
		m_vertexCount = (uint32_t)vertices.size();
		m_boundingSphereRadius = 0.0f;
		m_boundingSphereCenter = glm::vec3(0.0f, 0.0f, 0.0f);
		for (uint32_t i = 0; i < vertices.size(); i++) {

			m_boundingSphereRadius = std::max (
										std::max(	std::max( vertices[i].pos.x*vertices[i].pos.x, vertices[i].pos.y*vertices[i].pos.y ), 
													vertices[i].pos.z*vertices[i].pos.z ), 
										m_boundingSphereRadius);
		}
		m_boundingSphereRadius = sqrt(m_boundingSphereRadius);

		//create the vertex buffer
		VECHECKRESULT( vh::vhBufCreateVertexBuffer(	getRendererPointer()->getDevice(), getRendererPointer()->getVmaAllocator(),
													getRendererPointer()->getGraphicsQueue(), getRendererPointer()->getCommandPool(),
													vertices, &m_vertexBuffer, &m_vertexBufferAllocation),
						"Could not create vertex buffer for " + name);

		//create the index buffer
		VECHECKRESULT( vh::vhBufCreateIndexBuffer(	getRendererPointer()->getDevice(), getRendererPointer()->getVmaAllocator(),
													getRendererPointer()->getGraphicsQueue(), getRendererPointer()->getCommandPool(),
													indices, &m_indexBuffer, &m_indexBufferAllocation),
						"Could not create index buffer for " + name);
	}



	/**
	* \brief Destroy the vertex and index buffers
	*/
	VEMesh::~VEMesh() {
		vmaDestroyBuffer(getRendererPointer()->getVmaAllocator(), m_indexBuffer, m_indexBufferAllocation);
		vmaDestroyBuffer(getRendererPointer()->getVmaAllocator(), m_vertexBuffer, m_vertexBufferAllocation);
	}


	//---------------------------------------------------------------------
	//Material

	/**
	* \brief Destroy the material textures
	*/
	VEMaterial::~VEMaterial() {
		if (mapDiffuse != nullptr) delete mapDiffuse;
		if (mapBump != nullptr) delete mapBump;
		if (mapNormal != nullptr) delete mapNormal;
		if (mapHeight != nullptr) delete mapHeight;
	};


	//---------------------------------------------------------------------
	//Texture

	/**
	*
	* \brief VETexture constructor from a list of textures.
	*
	* Create a VETexture from a list of textures. The textures must lie in the same directory and are stored in a texture array.
	* This can be used also as a cube map.
	*
	* \param[in] name The name of the mesh.
	* \param[in] basedir Name of the directory the files are in.
	* \param[in] texNames List of filenames of the textures.
	* \param[in] flags Vulkan flags for creating the textures.
	* \param[in] viewType Vulkan view tape for the image view.
	*
	*/
	VETexture::VETexture(	std::string name, 
							std::string &basedir, std::vector<std::string> texNames,
							VkImageCreateFlags flags, VkImageViewType viewType) : VENamedClass(name) {
		if (texNames.size() == 0) return;

		VECHECKRESULT(vh::vhBufCreateTextureImage(getRendererPointer()->getDevice(), getRendererPointer()->getVmaAllocator(),
							getRendererPointer()->getGraphicsQueue(), getRendererPointer()->getCommandPool(),
							basedir, texNames, flags, &m_image, &m_deviceAllocation, &m_extent),
					"Could not create texture image for " + basedir + "/" + texNames[0] );

		m_format = VK_FORMAT_R8G8B8A8_UNORM;
		VECHECKRESULT(vh::vhBufCreateImageView(getRendererPointer()->getDevice(), m_image,
							m_format, viewType, (uint32_t)texNames.size(), VK_IMAGE_ASPECT_COLOR_BIT, &m_imageView),
					"Could not create image view for " + basedir + "/" + texNames[0]);

		VECHECKRESULT(vh::vhBufCreateTextureSampler(getRendererPointer()->getDevice(), &m_sampler),
					"Could not create texture sampler for " + basedir + "/" + texNames[0]);
	}

	/**
	*
	* \brief VETexture constructor from a GLI cube map file.
	*
	* Create a VETexture from a GLI cubemap file. This has been loaded using GLI from a ktx or dds file.
	*
	* \param[in] name The name of the mesh.
	* \param[in] texCube The GLI cubemap structure
	* \param[in] flags Create flags for the images (e.g. Cube compatible or array)
	* \param[in] viewType Type for the image views
	*
	*/
	VETexture::VETexture(std::string name, gli::texture_cube &texCube,
		VkImageCreateFlags flags, VkImageViewType viewType) : VENamedClass(name) {

		VECHECKRESULT(vh::vhBufCreateTexturecubeImage(getRendererPointer()->getDevice(), getRendererPointer()->getVmaAllocator(),
							getRendererPointer()->getGraphicsQueue(), getRendererPointer()->getCommandPool(),
							texCube, &m_image, &m_deviceAllocation, &m_format),
					"Could not create texture cubemap for " + name);

		m_extent.width = texCube.extent().x;
		m_extent.height = texCube.extent().y;

		VECHECKRESULT(vh::vhBufCreateImageView(getRendererPointer()->getDevice(), m_image,
							m_format, VK_IMAGE_VIEW_TYPE_CUBE, 6, VK_IMAGE_ASPECT_COLOR_BIT, &m_imageView),
					"Could not create image view for cubemap for " + name);

		VECHECKRESULT(vh::vhBufCreateTextureSampler(getRendererPointer()->getDevice(), &m_sampler),
					"Could not create texture sampler for cubemap " + name );
	}

	/**
	* \brief VETexture destructor - destroy the sampler, image view and image
	*/
	VETexture::~VETexture() {
		if (m_sampler != VK_NULL_HANDLE) vkDestroySampler(getRendererPointer()->getDevice(), m_sampler, nullptr);
		if (m_imageView != VK_NULL_HANDLE) vkDestroyImageView(getRendererPointer()->getDevice(), m_imageView, nullptr);
		if (m_image != VK_NULL_HANDLE) vmaDestroyImage(getRendererPointer()->getVmaAllocator(), m_image, m_deviceAllocation);
	}


}


