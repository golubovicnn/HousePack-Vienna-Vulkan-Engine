/**
* The Vienna Vulkan Engine
*
* (c) bei Helmut Hlavacs, University of Vienna
*
*/


#include "VEInclude.h"



namespace ve {


	//---------------------------------------------------------------------
	//Scene node


	/**
	* \brief The constructor ot the VESceneNode class
	*
	* \param[in] name The name of this node.
	* \param[in] transf Position and orientation.
	* \param[in] parent A parent.
	*
	*/

	VESceneNode::VESceneNode(std::string name, glm::mat4 transf, VESceneNode *parent) : VENamedClass(name) {
		m_parent = parent;
		if (parent != nullptr) {
			parent->addChild(this);		//if there is a parent, add this scene node to the parent as a child
		}
		setTransform(transf);			//sets this MO also onto the dirty list to be updated
	}



	/**
	* \returns the scene node's local to parent transform.
	*/
	glm::mat4 VESceneNode::getTransform() {
		return m_transform;
	}

	/**
	* \brief Sets the scene node's local to parent transform.
	*/
	void VESceneNode::setTransform(glm::mat4 trans) {
		m_transform = trans;
	}

	/**
	* \brief Sets the scene node's position.
	*/
	void VESceneNode::setPosition(glm::vec3 pos) {
		m_transform[3] = glm::vec4(pos.x, pos.y, pos.z, 1.0f);
	};

	/**
	* \returns the scene nodes's position.
	*
	* The position of a scene node is the 4th column vector of its transform.
	*
	*/
	glm::vec3 VESceneNode::getPosition() {
		return glm::vec3(m_transform[3].x, m_transform[3].y, m_transform[3].z);
	};

	/**
	* \returns the entity's local x-axis in parent space
	*/
	glm::vec3 VESceneNode::getXAxis() {
		glm::vec4 x = m_transform[0];
		return glm::vec3(x.x, x.y, x.z);
	}

	/**
	* \returns the entity's local y-axis in parent space
	*/
	glm::vec3 VESceneNode::getYAxis() {
		glm::vec4 y = m_transform[1];
		return glm::vec3(y.x, y.y, y.z);
	}

	/**
	* \returns the entity's local z-axis in parent space
	*/
	glm::vec3 VESceneNode::getZAxis() {
		glm::vec4 z = m_transform[2];
		return glm::vec3(z.x, z.y, z.z);
	}

	/**
	*
	* \brief Multiplies the entity's transform with another 4x4 transform.
	*
	* The transform can be a translation, scaling, rotation etc.
	*
	* \param[in] trans The 4x4 transform that is multiplied from the left onto the entity's old transform.
	*
	*/
	void VESceneNode::multiplyTransform(glm::mat4 trans) {
		setTransform(trans*m_transform);
	};

	/**
	*
	* \brief An entity's world matrix is the local to parent transform multiplied by the parent's world matrix.
	*
	* \returns the entity's world (aka model) matrix.
	*
	*/
	glm::mat4 VESceneNode::getWorldTransform() {
		if (m_parent != nullptr) return m_parent->getWorldTransform() * m_transform;
		return m_transform;
	};


	/**
	*
	* \brief lookAt function for a left handed frame of reference
	*
	* \param[in] eye New position of the entity
	* \param[in] point Entity looks at this point (= new local z axis)
	* \param[in] up Up vector pointing up
	*
	*/
	void VESceneNode::lookAt(glm::vec3 eye, glm::vec3 point, glm::vec3 up) {
		m_transform[3] = glm::vec4(eye.x, eye.y, eye.z, 1.0f);
		glm::vec3 z = glm::normalize(point - eye);
		up = glm::normalize(up);
		float corr = glm::dot(z, up);	//if z, up are lined up (corr=1 or corr=-1), decorrelate them
		if (fabs(1.0f - fabs(corr)) < 0.00001f) {
			float sc = z.x + z.y + z.z;
			up = glm::normalize(glm::vec3(sc, sc, sc));
		}

		m_transform[2] = glm::vec4(z.x, z.y, z.z, 0.0f);
		glm::vec3 x = glm::normalize(glm::cross(up, z));
		m_transform[0] = glm::vec4(x.x, x.y, x.z, 0.0f);
		glm::vec3 y = glm::normalize(glm::cross(z, x));
		m_transform[1] = glm::vec4(y.x, y.y, y.z, 0.0f);

	}

	/**
	*
	* \brief Adds a child object to the list of children.
	*
	* \param[in] pObject Pointer to the new child.
	*
	*/
	void VESceneNode::addChild(VESceneNode * pObject) {
		if (pObject->m_parent != nullptr) {
			pObject->m_parent->removeChild(pObject);
		}

		pObject->m_parent = this;
		m_children.push_back(pObject);
	}

	/**
	*
	* \brief remove a child from the children list - child is NOT destroyed
	*
	* \param[in] pEntity Pointer to the child to be removed.
	*
	*/
	void VESceneNode::removeChild(VESceneNode *pEntity) {
		for (uint32_t i = 0; i < m_children.size(); i++) {
			if (pEntity == m_children[i]) {								//if child is found
				VESceneNode *last = m_children[m_children.size() - 1];	//replace it with the last child
				m_children[i] = last;
				m_children.pop_back();									//child is not destroyed
				return;
			}
		}
	}

	/**
	*
	* \brief Update the entity's UBO buffer with the current world matrix
	*
	* If there is a parent, get the parent's world matrix. If not, set the parent matrix to identity.
	* Then call update(parentWorldMatrix) to do the job.
	*
	* \param[in] imageIndex The index of the swapchain image that is currently used
	*
	*/
	void VESceneNode::update(uint32_t imageIndex) {
		glm::mat4 parentWorldMatrix = glm::mat4(1.0);
		if (m_parent != nullptr) {
			parentWorldMatrix = m_parent->getWorldTransform();
		}
		update(parentWorldMatrix, imageIndex );
	}

	/**
	*
	* \brief Update the entity's UBO buffer with the current world matrix
	*
	* Calculate the new world matrix (and inv transpose matix to transform normal vectors).
	* Then copy the struct content into the UBO. Then call all children to do the same.
	*
	* \param[in] parentWorldMatrix The parent's world matrix or an identity matrix.
	* \param[in] imageIndex The index of the swapchain image that is currently used
	*
	*/
	void VESceneNode::update(glm::mat4 parentWorldMatrix, uint32_t imageIndex ) {
		glm::mat4 worldMatrix = parentWorldMatrix * getTransform();		//compute the world matrix
		updateUBO( worldMatrix, imageIndex);					//call derived class for specific data like object color
		updateChildren( worldMatrix, imageIndex);				//update all children
	}


	/**
	* \brief Update the UBOs of all children of this entity
	*/
	void VESceneNode::updateChildren(glm::mat4 worldMatrix, uint32_t imageIndex) {
		for (auto pObject : m_children) {
			pObject->update(worldMatrix, imageIndex);	//update the children by giving them the current worldMatrix
		}
	}

	/**
	* \brief Get a default bounding sphere for this scene node
	*
	* \param[out] center The sphere center is also the position of the scene node
	* \param[out] radius The default radius of the sphere
	*
	*/
	void VESceneNode::getBoundingSphere(glm::vec3 *center, float *radius) {
		*center = getPosition();
		*radius = 1.0f;
	}

	/**
	*
	* \brief Get an OBB that exactly holds the given points.
	*
	* The OBB is oriented along the local axes of the scene node.
	*
	* \param[in] points The points that should be engulfed by the OBB, in world space
	* \param[in] t1 Used for interpolating between frustum edge points
	* \param[in] t2 Used for interpolating between frustum edge points
	* \param[out] center Center of the OBB
	* \param[out] width Width of the OBB
	* \param[out] height Height of the OBB
	* \param[out] depth Depth of the OBB
	*
	*/

	void VESceneNode::getOBB(std::vector<glm::vec4> &points, float t1, float t2,
		glm::vec3 &center, float &width, float &height, float &depth) {

		glm::mat4 W = getWorldTransform();

		std::vector<glm::vec4> axes;		//3 local axes, into pos and minus direction
		axes.push_back(-1.0f*W[0]);
		axes.push_back(W[0]);
		axes.push_back(-1.0f*W[1]);
		axes.push_back(W[1]);
		axes.push_back(-1.0f*W[2]);
		axes.push_back(W[2]);

		std::vector<glm::vec4> box;			//maxima points into the 6 directions
		std::vector<float> maxvalues;		//max ordinates
		box.resize(6);
		maxvalues.resize(6);
		for (uint32_t i = 0; i < 6; i++) {	//fill maxima with first point
			box[i] = points[0];
			maxvalues[i] = glm::dot(axes[i], points[0]);
		}

		for (uint32_t i = 1; i < points.size(); i++) {		//go through rest of the points and 6 axis directions
			for (uint32_t j = 0; j < 6; j++) {
				float tmp = glm::dot(axes[j], points[i]);
				if (maxvalues[j] < tmp) {
					box[j] = points[i];
					maxvalues[j] = tmp;
				}
			}
		}
		width = maxvalues[1] + maxvalues[0];
		height = maxvalues[3] + maxvalues[2];
		depth = maxvalues[5] + maxvalues[4];
		glm::vec4 center4 = W * glm::vec4(-maxvalues[0] + width / 2.0f,
			-maxvalues[2] + height / 2.0f,
			-maxvalues[4] + depth / 2.0f, 0.0f);
		center = glm::vec3(center4.x, center4.y, center4.z);
	}



	//-----------------------------------------------------------------------------------------------------
	//Scene object

	/**
	*
	* \brief Constructor of the scene object class.
	*
	* If the object needs UBOs, then first the UBOs are created (one for each swap chain image), 
	* then the descriptor sets, then the UBOs and sets are connected.
	*
	* \param[in] name Name of the new scene object.
	* \param[in] transf Transform of the object, containing orientation and position.
	* \param[in] parent Parent of the object, or nullptr.
	* \param[in] sizeUBO Size of the object's UBO, if >0 then the object needs UBOs
	*
	*/
	VESceneObject::VESceneObject(std::string name, glm::mat4 transf, VESceneNode *parent, uint32_t sizeUBO ) : 
									VESceneNode(name, transf, parent) {

		if (sizeUBO > 0) {
			vh::vhBufCreateUniformBuffers(	getRendererPointer()->getVmaAllocator(),
											(uint32_t)getRendererPointer()->getSwapChainNumber(),
											sizeUBO, m_uniformBuffers, m_uniformBuffersAllocation);

			vh::vhRenderCreateDescriptorSets(getRendererForwardPointer()->getDevice(),
				(uint32_t)getRendererForwardPointer()->getSwapChainNumber(),
				getRendererForwardPointer()->getDescriptorSetLayoutPerObject(),
				getRendererForwardPointer()->getDescriptorPool(),
				m_descriptorSetsUBO);

			for (uint32_t i = 0; i < m_descriptorSetsUBO.size(); i++) {
				vh::vhRenderUpdateDescriptorSet(getRendererForwardPointer()->getDevice(),
					m_descriptorSetsUBO[i],
					{ m_uniformBuffers[i] },		//UBOs
					{ sizeUBO },					//UBO sizes
					{ { VK_NULL_HANDLE } },			//textureImageViews
					{ { VK_NULL_HANDLE } }			//samplers
				);
			}
		}
	}


	/**
	*
	* \brief Destructor of the scene object class.
	*
	* Destructs all UBOs that this objects owns.
	*
	*/
	VESceneObject::~VESceneObject() {
		for (uint32_t i = 0; i < m_uniformBuffers.size(); i++) {
			vmaDestroyBuffer(getRendererPointer()->getVmaAllocator(), m_uniformBuffers[i], m_uniformBuffersAllocation[i]);
		}
	}

	/**
	*
	* \brief Copy the local data of this scene object into the GPU UBO.
	*
	* \param[in] pUBO Pointer to the UBO that should be copied to the GPU.
	* \param[in] sizeUBO Size od the UBO.
	* \param[in] imageIndex Index of the swap chain image that is currently used.
	*
	*/
	void VESceneObject::updateUBO(void *pUBO, uint32_t sizeUBO, uint32_t imageIndex ) {
		void* data = nullptr;
		vmaMapMemory(getRendererPointer()->getVmaAllocator(), m_uniformBuffersAllocation[imageIndex], &data);
		memcpy(data, pUBO, sizeUBO);
		vmaUnmapMemory(getRendererPointer()->getVmaAllocator(), m_uniformBuffersAllocation[imageIndex]);
	}



	//---------------------------------------------------------------------
	//Entity


	/**
	*
	* \brief VEEntity constructor.
	*
	* Create a VEEntity from a mesh, material, using a transform and a parent.
	*
	* \param[in] name The name of the mesh.
	* \param[in] type Mesh type
	* \param[in] pMesh Pointer to the mesh.
	* \param[in] pMat Pointer to the material.
	* \param[in] transf The local to parent transform.
	* \param[in] parent Pointer to the entity's parent.
	*
	*/
	VEEntity::VEEntity(	std::string name, veEntityType type, 
						VEMesh *pMesh, VEMaterial *pMat, 
						glm::mat4 transf, VESceneNode *parent) :
							VESceneObject(name, transf, parent, (uint32_t) sizeof(veUBOPerObject_t)), m_entityType( type ) {

		setTransform(transf);

		if (pMesh != nullptr && pMat != nullptr) {
			m_pMesh = pMesh;
			m_pMaterial = pMat;
			m_drawEntity = true;
			m_castsShadow = true;
		}
	}


	/**
	*
	* \brief VEEntity destructor.
	*
	* Destroy the entity's UBOs.
	*
	*/
	VEEntity::~VEEntity() {
	}

	/**
	* \brief Sets the object parameter vector.
	*
	* This is usually used for texture animation.
	*
	* \param[in] param The new parameter vector
	*/
	void VEEntity::setParam(glm::vec4 param) {
		m_param = param;
	}


	/**
	*
	* \brief Update the entity's UBO.
	*
	* \param[in] worldMatrix The new world matrix of the entity
	* \param[in] imageIndex The Index of the swapchain image that is currently used.
	*
	*/
	void VEEntity::updateUBO( glm::mat4 worldMatrix, uint32_t imageIndex) {
		m_ubo = {};

		m_ubo.model = worldMatrix;
		m_ubo.modelInvTrans = glm::transpose(glm::inverse(worldMatrix));
		m_ubo.param = m_param;
		if (m_pMaterial != nullptr) {
			m_ubo.color = m_pMaterial->color;
		};

		VESceneObject::updateUBO( (void*)&m_ubo, (uint32_t)sizeof(veUBOPerObject_t), imageIndex);
	}


	/**
	* \brief Get a bounding sphere for this entity
	*
	* Return the bounding sphere of the mesh that this entity represents (if there is one).
	* 
	* \param[out] center Pointer to the sphere center to return
	* \param[out] radius Pointer to the radius to return
	*
	*/
	void VEEntity::getBoundingSphere(glm::vec3 *center, float *radius) {
		*center = getPosition();
		*radius = 1.0f;
		if (m_pMesh != nullptr) {
			*center = m_pMesh->m_boundingSphereCenter;
			*radius = m_pMesh->m_boundingSphereRadius;
		}
	}



	//-------------------------------------------------------------------------------------------------
	//camera

	/**
	*
	* \brief VECamera constructor. Set nearPlane, farPlane to a default value.
	*
	* \param[in] name Name of the camera.
	* \param[in] transf Transform of this camera, including orientation and position
	* \param[in] parent The parent of this camera, or nullptr
	*
	*/
	VECamera::VECamera(std::string name, glm::mat4 transf, VESceneNode *parent ) : 
							VESceneObject(name, transf, parent, (uint32_t)sizeof(veUBOPerCamera_t)) {
	};

	/**
	*
	* \brief VECamera constructor. 
	*
	* \param[in] name Name of the camera.
	* \param[in] nearPlane Distance of near plane to the camera origin
	* \param[in] farPlane Distance of far plane to the camera origin
	* \param[in] nearPlaneFraction If this is shadow cam of a directional light: fraction of frustum that is covered by this cam
	* \param[in] farPlaneFraction If this is shadow cam of a directional light: fraction of frustum that is covered by this cam
	* \param[in] transf Transform of this camera, including orientation and position
	* \param[in] parent The parent of this camera, or nullptr
	*
	*/
	VECamera::VECamera(	std::string name, 
						float nearPlane, float farPlane,
						float nearPlaneFraction, float farPlaneFraction,
						glm::mat4 transf, VESceneNode *parent ) :
							VESceneObject(name, transf, parent, (uint32_t)sizeof(veUBOPerCamera_t)), 
							m_nearPlane(nearPlane), m_farPlane(farPlane),
							m_nearPlaneFraction(nearPlaneFraction), m_farPlaneFraction(farPlaneFraction) {
	}

	/**
	*
	* \brief Update the UBO of this camera.
	*
	* \param[in] worldMatrix The new world matrix of this camera
	* \param[in] imageIndex Index of the swapchain image that is currently used.
	*
	*/
	void VECamera::updateUBO(glm::mat4 worldMatrix, uint32_t imageIndex) {
		m_ubo = {};

		m_ubo.model = worldMatrix;
		m_ubo.view = glm::inverse(worldMatrix);
		m_ubo.proj = getProjectionMatrix();
		m_ubo.param[0] = m_nearPlane;
		m_ubo.param[1] = m_farPlane;
		m_ubo.param[2] = m_nearPlaneFraction;		//needed only if this is a shadow cam
		m_ubo.param[3] = m_farPlaneFraction;		//needed only if this is a shadow cam

		VESceneObject::updateUBO((void*)&m_ubo, (uint32_t)sizeof(veUBOPerCamera_t), imageIndex );
	}


	/**
	* \brief Get a bounding sphere for this camera
	*
	* Return the bounding sphere of camera frustum. It consists of a center point and a radius.
	*
	* \param[out] center Pointer to the sphere center to return
	* \param[out] radius Pointer to the radius to return
	*
	*/
	void VECamera::getBoundingSphere(glm::vec3 *center, float *radius) {
		std::vector<glm::vec4> points;

		getFrustumPoints(points);					//get frustum points in world space

		glm::vec4 mean(0.0f, 0.0f, 0.0f, 1.0f);
		for (auto point : points) {
			mean += point;
		}
		mean /= (float)points.size();

		float maxsq = 0.0f;
		for (auto point : points) {
			float sq = glm::dot(mean - point, mean - point);
			maxsq = sq > maxsq ? sq : maxsq;
		}

		*center = glm::vec3( mean.x, mean.y, mean.z );
		*radius = sqrt(maxsq);
	}



	//-------------------------------------------------------------------------------------------------
	//camera projective

	/**
	*
	* \brief VECameraProjective constructor. Set nearPlane, farPlane, aspect ratio and fov to a default value.
	*
	* \param[in] name Name of the camera.
	* \param[in] transf Transform of this camera, including orientation and position
	* \param[in] parent The parent of this camera, or nullptr
	*
	*/
	VECameraProjective::VECameraProjective(std::string name, glm::mat4 transf, VESceneNode *parent) : VECamera(name, transf, parent ) {
	};

	/**
	*
	* \brief VECameraProjective constructor.
	*
	* \param[in] name Name of the camera.
	* \param[in] nearPlane Distance of near plane to the camera origin
	* \param[in] farPlane Distance of far plane to the camera origin
	* \param[in] aspectRatio Ratio between width and height. Can change due to window size change.
	* \param[in] fov Vertical field of view angle.
	* \param[in] nearPlaneFraction If this is shadow cam of a directional light: fraction of frustum that is covered by this cam
	* \param[in] farPlaneFraction If this is shadow cam of a directional light: fraction of frustum that is covered by this cam
	* \param[in] transf Transform of this camera, including orientation and position
	* \param[in] parent The parent of this camera, or nullptr
	*
	*/
	VECameraProjective::VECameraProjective(	std::string name, 
											float nearPlane, float farPlane, 
											float aspectRatio, float fov,
											float nearPlaneFraction, float farPlaneFraction,
											glm::mat4 transf, VESceneNode *parent) :
												VECamera(	name, 
															nearPlane, farPlane, 
															nearPlaneFraction, farPlaneFraction,
															transf, parent ), 
												m_aspectRatio(aspectRatio), m_fov(fov)   {
	};

	/**
	* \brief Get a projection matrix for this camera (left handed system)
	*
	* \param[in] width Width of the current game window.
	* \param[in] height Height of the current game window.
	* \returns the camera projection matrix.
	*
	*/
	glm::mat4 VECameraProjective::getProjectionMatrix(float width, float height) {
		m_aspectRatio = width / height;
		glm::mat4 pm = glm::perspectiveFov( glm::radians(m_fov), (float) width, (float)height, m_nearPlane, m_farPlane);
		pm[1][1] *= -1.0f;
		pm[2][2] *= -1.0f;		//camera looks down its positive z-axis, OpenGL function does it reverse
		pm[2][3] *= -1.0f;
		return pm;
	}

	/**
	* \brief Get a projection matrix for this camera.
	*
	* In this function the width and height parameters are set using aspect ratio and 1.0.
	* Then the getProjectionMatrix(float width, float height) function is called.
	*
	* \returns the camera projection matrix.
	*/
	glm::mat4 VECameraProjective::getProjectionMatrix() {
		return getProjectionMatrix( m_aspectRatio, 1.0f );
	}

	/**
	* \brief Get a list of 8 points making up the camera frustum
	*
	* The points are returned in world space.
	*
	* \param[in] z0 Startparameter for interpolating the frustum
	* \param[in] z1 Endparameter for interlopating the frustum
	* \param[out] points List of 8 points that make up the interpolated frustum in world space
	*
	*/
	void VECameraProjective::getFrustumPoints(std::vector<glm::vec4> &points, float z0, float z1) {
		float halfh = (float)tan( (m_fov/2.0f) * M_PI / 180.0f );
		float halfw = halfh * m_aspectRatio;

		glm::mat4 W = getWorldTransform();

		points.push_back( W*glm::vec4(-m_nearPlane * halfw, -m_nearPlane * halfh, m_nearPlane, 1.0f ) );
		points.push_back( W*glm::vec4( m_nearPlane * halfw, -m_nearPlane * halfh, m_nearPlane, 1.0f));
		points.push_back( W*glm::vec4(-m_nearPlane * halfw,  m_nearPlane * halfh, m_nearPlane, 1.0f));
		points.push_back( W*glm::vec4( m_nearPlane * halfw,  m_nearPlane * halfh, m_nearPlane, 1.0f));

		points.push_back( W*glm::vec4(-m_farPlane * halfw, -m_farPlane * halfh, m_farPlane, 1.0f));
		points.push_back( W*glm::vec4( m_farPlane * halfw, -m_farPlane * halfh, m_farPlane, 1.0f));
		points.push_back( W*glm::vec4(-m_farPlane * halfw,  m_farPlane * halfh, m_farPlane, 1.0f));
		points.push_back( W*glm::vec4( m_farPlane * halfw,  m_farPlane * halfh, m_farPlane, 1.0f));

		for (uint32_t i = 0; i < 4; i++) {						//interpolate with z0, z1 in [0,1]
			glm::vec4 diff = points[i + 4] - points[i + 0];
			points[i + 4] = points[i + 0] + z1*diff;
			points[i + 0] = points[i + 0] + z0*diff;
		}
	}


	//-------------------------------------------------------------------------------------------------
	//camera ortho

	/**
	*
	* \brief VECameraOrtho constructor. Set nearPlane, farPlane, aspect ratio and fov to a default value.
	*
	* \param[in] name Name of the camera.
	* \param[in] transf Transform of this camera, including orientation and position
	* \param[in] parent The parent of this camera, or nullptr
	*
	*/
	VECameraOrtho::VECameraOrtho(std::string name, glm::mat4 transf, VESceneNode *parent) : 
						VECamera(name, transf, parent ) {
	};

	/**
	*
	* \brief VECameraProjective constructor.
	*
	* \param[in] name Name of the camera.
	* \param[in] nearPlane Distance of near plane to the camera origin
	* \param[in] farPlane Distance of far plane to the camera origin
	* \param[in] width Width of the frustum
	* \param[in] height Height of the frustum
	* \param[in] nearPlaneFraction If this is shadow cam of a directional light: fraction of frustum that is covered by this cam
	* \param[in] farPlaneFraction If this is shadow cam of a directional light: fraction of frustum that is covered by this cam
	* \param[in] transf Transform of this camera, including orientation and position
	* \param[in] parent The parent of this camera, or nullptr
	*
	*/
	VECameraOrtho::VECameraOrtho(	std::string name, 
									float nearPlane, float farPlane, 
									float width, float height,
									float nearPlaneFraction, float farPlaneFraction,
									glm::mat4 transf, VESceneNode *parent) :
										VECamera(	name, 
													nearPlane, farPlane, 
													nearPlaneFraction, farPlaneFraction,
													transf, parent), 
										m_width(width), m_height(height) {
	};


	/**
	* \brief Get a projection matrix for this camera.
	* \param[in] width Width of the current game window.
	* \param[in] height Height of the current game window.
	* \returns the camera projection matrix.
	*/
	glm::mat4 VECameraOrtho::getProjectionMatrix(float width, float height) {
		glm::mat4 pm = glm::ortho(-width * m_width / 2.0f, width * m_width / 2.0f, -height * m_height / 2.0f, height * m_height / 2.0f, m_nearPlane, m_farPlane);
		pm[1][1] *= -1.0f;
		pm[2][2] *= -1.0;	//camera looks down its positive z-axis, OpenGL function does it reverse
		return pm;
	}

	/**
	* \brief Get a projection matrix for this camera.
	* \returns the camera projection matrix.
	*/
	glm::mat4 VECameraOrtho::getProjectionMatrix() {
		glm::mat4 pm = glm::ortho( -m_width/2.0f, m_width/2.0f, -m_height/2.0f, m_height/2.0f, m_nearPlane, m_farPlane);
		pm[2][2] *= -1;		//camera looks down its positive z-axis, OpenGL function does it reverse
		return pm;
	}


	/**
	* \brief Get a list of 8 points making up the camera frustum
	*
	* The points are returned in world space.
	*
	* \param[in] t1 Startparameter for interpolating the frustum
	* \param[in] t2 Endparameter for interlopating the frustum
	* \param[out] points List of 8 points that make up the interpolated frustum in world space
	*
	*/
	void VECameraOrtho::getFrustumPoints(std::vector<glm::vec4> &points, float t1, float t2) {
		float halfh = m_height / 2.0f;
		float halfw = m_width / 2.0f;

		glm::mat4 W = getWorldTransform();

		points.push_back( W*glm::vec4(-halfw, -halfh, m_nearPlane, 1.0f));
		points.push_back( W*glm::vec4( halfw, -halfh, m_nearPlane, 1.0f));
		points.push_back( W*glm::vec4(-halfw,  halfh, m_nearPlane, 1.0f));
		points.push_back( W*glm::vec4( halfw,  halfh, m_nearPlane, 1.0f));

		points.push_back( W*glm::vec4(-halfw, -halfh, m_farPlane, 1.0f));
		points.push_back( W*glm::vec4( halfw, -halfh, m_farPlane, 1.0f));
		points.push_back( W*glm::vec4(-halfw,  halfh, m_farPlane, 1.0f));
		points.push_back( W*glm::vec4( halfw,  halfh, m_farPlane, 1.0f));

		for (uint32_t i = 0; i < 4; i++) {						//interpolate
			glm::vec4 diff = points[i + 4] - points[i + 0];
			points[i + 4] = points[i + 0] + t2*diff;
			points[i + 0] = points[i + 0] + t1*diff;
		}
	}




	//-------------------------------------------------------------------------------------------------
	//light

	/**
	* \brief Simple VELight constructor, default is directional light
	*
	* \param[in] name Name of the camera
	* \param[in] transf Transform of this light, including orientation and position
	* \param[in] parent The parent of this light, or nullptr
	*
	*/
	VELight::VELight(std::string name, glm::mat4 transf, VESceneNode *parent ) : 
						VESceneObject(name, transf, parent, (uint32_t)sizeof(veUBOPerLight_t)) {
	};


	/**
	*
	* \brief Update the UBO of this light.
	*
	* This function updates the UBO of this light, but also calls updates on the shadow
	* cameras of this light.
	*
	* \param[in] worldMatrix The new world matrix of this light
	* \param[in] imageIndex Index of the swapchain image that is currently used.
	*
	*/
	void VELight::updateUBO(glm::mat4 worldMatrix, uint32_t imageIndex) {
		m_ubo = {};

		m_ubo.type[0] = getLightType();
		m_ubo.model = worldMatrix;
		m_ubo.col_ambient = m_col_ambient;
		m_ubo.col_diffuse = m_col_diffuse;
		m_ubo.col_specular = m_col_specular;
		m_ubo.param = m_param;

		VECamera *pCam = getSceneManagerPointer()->getCamera();

		updateShadowCameras(pCam, imageIndex);						//copy shadow cam UBOs to GPU
		for (uint32_t i = 0; i < m_shadowCameras.size(); i++ ) {	//copy shadow cam UBOs to light UBO
			m_ubo.shadowCameras[i] = m_shadowCameras[i]->m_ubo;
		}

		VESceneObject::updateUBO((void*)&m_ubo, (uint32_t)sizeof(veUBOPerLight_t), imageIndex);
	}


	/**
	*
	* \brief Destructor of the light class.
	*
	*/
	VELight::~VELight() {
		for (auto pCam : m_shadowCameras) {
			delete pCam;
		}
		m_shadowCameras.clear();
	};


	//------------------------------------------------------------------------------------------------
	//derive light classes

	/**
	*
	* \brief Constructor of the directional light class.
	*
	* \param[in] name The name of this directional light
	* \param[in] transf The transform including orientation and position
	* \param[in] parent The parent of this light
	*
	*/
	VEDirectionalLight::VEDirectionalLight(std::string name, glm::mat4 transf, VESceneNode *parent) :
					VELight(name, transf, parent) {

		for (uint32_t i = 0; i < 4; i++) {
			m_shadowCameras.push_back(new VECameraOrtho("ShadowCamDirOrtho") );	//no parent - > transform is also world matrix
		}
	};


	/**
	*
	* \brief Update all shadow cameras of this light.
	*
	* \param[in] pCamera Pointer to the currently used light camera.
	* \param[in] imageIndex Index of the swapchain image that is currently used.
	*
	*/
	void VEDirectionalLight::updateShadowCameras(VECamera *pCamera, uint32_t imageIndex) {

		std::vector<float> limits = { 0.0f, 0.05f, 0.15f, 0.50f, 1.0f };	//the frustum is split into 4 segments

		for (uint32_t i = 0; i < m_shadowCameras.size(); i++) {
			VECameraOrtho *pShadowCamera = (VECameraOrtho *)m_shadowCameras[i];

			std::vector<glm::vec4> pointsW;
			pCamera->getFrustumPoints(pointsW, limits[i], limits[i+1]);		//get the ith frustum segment

			glm::vec3 center;
			getOBB(pointsW, 0.0f, 1.0f, center, pShadowCamera->m_width, pShadowCamera->m_height, pShadowCamera->m_farPlane);
			pShadowCamera->m_farPlane *= 5.0f;			//TODO - do NOT set too high or else shadow maps wont get drawn!

			glm::mat4 W = getWorldTransform();
			pShadowCamera->setTransform(W);
			pShadowCamera->setPosition(center - pShadowCamera->m_farPlane*0.9f * glm::vec3(W[2].x, W[2].y, W[2].z));
			pShadowCamera->m_nearPlaneFraction = limits[i];
			pShadowCamera->m_farPlaneFraction = limits[i+1];

			pShadowCamera->update( imageIndex );
		}
	}


	/**
	*
	* \brief Constructor of the point light class.
	*
	* \param[in] name The name of this point light
	* \param[in] transf The transform including orientation and position
	* \param[in] parent The parent of this light
	*
	*/
	VEPointLight::VEPointLight(std::string name, glm::mat4 transf, VESceneNode *parent) :
					VELight( name, transf, parent ) {

		for (uint32_t i = 0; i < 6; i++) {
			m_shadowCameras.push_back(new VECameraProjective("ShadowCamPointProj"));	//no parent - > transform is also world matrix
		}
	};


	/**
	*
	* \brief Update all shadow cameras of this light.
	*
	* \param[in] pCamera Pointer to the currently used light camera.
	* \param[in] imageIndex Index of the swapchain image that is currently used.
	*
	*/

	void VEPointLight::updateShadowCameras(VECamera *pCamera, uint32_t imageIndex) {

		float lnear = 0.1f;
		float llength = m_param[0];
		glm::vec4 pos4 = getWorldTransform()[3];
		glm::vec3 pos = glm::vec3(pos4.x, pos4.y, pos4.z);

		for (uint32_t i = 0; i < m_shadowCameras.size(); i++) {

			VECameraProjective * pShadowCamera = (VECameraProjective *)m_shadowCameras[i];

			pShadowCamera->m_aspectRatio = 1.0f;			//TODO: for comparing with light cam
			pShadowCamera->m_fov = 91.0f;					//a sector has always 90 deg fov
			pShadowCamera->m_nearPlane = lnear;				//no cascade for point light
			pShadowCamera->m_farPlane = lnear + llength;
			pShadowCamera->m_nearPlaneFraction = 0.0f;
			pShadowCamera->m_farPlaneFraction =  1.0f;

			std::vector<glm::vec3> zaxis =
			{
				glm::vec3(1.0f,  0.0f,  0.0f),
				glm::vec3(-1.0f,  0.0f,  0.0f),
				glm::vec3(0.0f,  1.0f,  0.0f),
				glm::vec3(0.0f, -1.0f,  0.0f),
				glm::vec3(0.0f,  0.0f,  1.0f),
				glm::vec3(0.0f,  0.0f, -1.0f)
			};
			std::vector<glm::vec3> up =
			{
				glm::vec3(0.0f,  1.0f,  0.0f),
				glm::vec3(0.0f,  1.0f,  0.0f),
				glm::vec3(0.0f,  0.0f, -1.0f),
				glm::vec3(0.0f,  0.0f,  1.0f),
				glm::vec3(0.0f,  1.0f,  0.0f),
				glm::vec3(0.0f,  1.0f,  0.0f)
			};
			pShadowCamera->lookAt( pos, pos + zaxis[i], up[i]);
			pShadowCamera->update(imageIndex);
		}
	}

	/**
	*
	* \brief Constructor of the spot light class.
	*
	* \param[in] name The name of this spot light
	* \param[in] transf The transform including orientation and position
	* \param[in] parent The parent of this light
	*
	*/
	VESpotLight::VESpotLight(std::string name, glm::mat4 transf, VESceneNode *parent) :
					VELight(name, transf, parent) {

		for (uint32_t i = 0; i < 1; i++) {
			m_shadowCameras.push_back(new VECameraProjective("ShadowCamSpotProj"));	//no parent - > transform is also world matrix
		};
	};


	/**
	*
	* \brief Update all shadow cameras of this light.
	*
	* \param[in] pCamera Pointer to the currently used light camera.
	* \param[in] imageIndex Index of the swapchain image that is currently used.
	*
	*/
	void VESpotLight::updateShadowCameras(VECamera *pCamera, uint32_t imageIndex) {

		//std::vector<float> limits = { 0.0f, 0.05f, 0.15f, 0.50f, 1.0f };	//the frustum is split into 4 segments
		std::vector<float> limits = { 0.0f, 1.0f };		//the frustum is split into 1 segment

		for (uint32_t i = 0; i < m_shadowCameras.size(); i++) {

			VECameraProjective * pShadowCamera = (VECameraProjective *)m_shadowCameras[i];

			float lnear = 0.1f;
			float llength = m_param[0];		//reach of light

			pShadowCamera->setTransform(getWorldTransform());

			pShadowCamera->m_aspectRatio = 1.0f;			//TODO: for comparing with light cam
			pShadowCamera->m_fov = 90.0f;						//TODO: depends on light parameters
			pShadowCamera->m_nearPlane = lnear + limits[i] * llength;
			pShadowCamera->m_farPlane  = lnear + limits[i+1] * llength;
			pShadowCamera->m_nearPlaneFraction = limits[i];
			pShadowCamera->m_farPlaneFraction  = limits[i+1];

			pShadowCamera->update(imageIndex);
		}
	}

}


