/**
* The Vienna Vulkan Engine
*
* (c) bei Helmut Hlavacs, University of Vienna
*
*/


#include "VEInclude.h"
int counter = 1;
int nameCounter = 1;
int randomNumber = 0;

namespace ve {

	///simple event listener for rotating objects
	class RotatorListener : public VEEventListener {
		VESceneNode *m_pObject = nullptr;
		float m_speed;
		glm::vec3 m_axis;
	public:
		///Constructor
		RotatorListener(std::string name, VESceneNode *pObject, float speed, glm::vec3 axis) :
			VEEventListener(name), m_pObject(pObject), m_speed(speed), m_axis(axis) {};

		void onFrameStarted(veEvent event) {
			glm::mat4 rot = glm::rotate( glm::mat4(1.0f), m_speed*(float)event.dt, m_axis );
			m_pObject->multiplyTransform(rot);
		}
	};



	///simple event listener for managing light movement
	class LightListener : public VEEventListener {
	public:
		///Constructor
		LightListener( std::string name) : VEEventListener(name) {};

		bool onKeyboard(veEvent event) {
			if ( event.idata3 == GLFW_RELEASE) return false;

			VELight *pLight = getSceneManagerPointer()->getLights()[0];		//first light

			float speed = 5.0f * (float)event.dt;

			switch (event.idata1) {
			case GLFW_KEY_Y:		//Z key on German keyboard!
				pLight->multiplyTransform(glm::translate(glm::mat4(1.0f), speed * glm::vec3(0.0f, -1.0f, 0.0f)));
				break;
			case GLFW_KEY_I:
				pLight->multiplyTransform(glm::translate(glm::mat4(1.0f), speed * glm::vec3(0.0f, 1.0f, 0.0f)));
				break;
			case GLFW_KEY_U:
				pLight->multiplyTransform( glm::translate(glm::mat4(1.0f), speed * glm::vec3(0.0f, 0.0f, 1.0f)));
				break;
			case GLFW_KEY_J:
				pLight->multiplyTransform(glm::translate(glm::mat4(1.0f), speed * glm::vec3(0.0f, 0.0f, -1.0f)));
				break;
			case GLFW_KEY_H:
				pLight->multiplyTransform(glm::translate(glm::mat4(1.0f), speed * glm::vec3(-1.0f, 0.0f, 0.0f)));
				break;
			case GLFW_KEY_K:
				pLight->multiplyTransform(glm::translate(glm::mat4(1.0f), speed * glm::vec3(1.0f, 0.0f, 0.0f)));
				break;
			}
			return false;
		}
	};




	///user defined manager class, derived from VEEngine
	class MyVulkanEngine : public VEEngine {
	protected:

	public:
		/**
		* \brief Constructor of my engine
		* \param[in] debug Switch debuggin on or off
		*/
		MyVulkanEngine( bool debug=false) : VEEngine(debug) {};
		~MyVulkanEngine() {};

		///Register an event listener to interact with the user
		virtual void registerEventListeners() {
			VEEngine::registerEventListeners();
			registerEventListener( new LightListener("LightListener"));
			//registerEventListener(new VEEventListenerNuklear("NuklearListener"));
			registerEventListener(new VEEventListenerNuklearDebug("NuklearDebugListener"));
		};

		///create many cubes
		void createHouse(uint32_t n) {

/*
				float stride = 50.0f;
				static std::default_random_engine e{ 12345 };
				static std::uniform_real_distribution<> d{ 1.0f, stride };
*/
			for (uint32_t i = 0; i < n; i++) {

				randomNumber = rand() % 7;

				VESceneNode *e4 = m_pSceneManager->loadModel("The Building" + std::to_string(nameCounter++), "models/buildings", "building" + std::to_string(randomNumber) + ".obj");
				e4->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-2000.0f, 1.0f, counter * 1000.0f)));
				e4->multiplyTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f)));

				randomNumber = rand() % 7;

				VESceneNode *e5 = m_pSceneManager->loadModel("The Building" + std::to_string(nameCounter++), "models/buildings", "building" + std::to_string(randomNumber) + ".obj");
				e5->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, counter++ * 1000.0f)));
				e5->multiplyTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f)));

				
			}
		}
				
		
				





/*	PROBA: DA LI SU SVE ZGRADE OK

				VESceneNode *e3 = m_pSceneManager->loadModel("The Building1", "models/buildings", "building1.obj");
				e3->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 1.0f, 3000.0f)));
				e3->multiplyTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f)));

				VESceneNode *e4 = m_pSceneManager->loadModel("The Building2", "models/buildings", "building2.obj");
				e4->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 1.0f, 4000.0f)));
				e4->multiplyTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f)));

				VESceneNode *e5 = m_pSceneManager->loadModel("The Building3", "models/buildings", "building3.obj");
				e5->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 1.0f, 5000.0f)));
				e5->multiplyTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f)));

				VESceneNode *e6 = m_pSceneManager->loadModel("The Building4", "models/buildings", "building4.obj");
				e6->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 1.0f, 6000.0f)));
				e6->multiplyTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f)));

				VESceneNode *e7 = m_pSceneManager->loadModel("The Building5", "models/buildings", "building5.obj");
				e7->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 1.0f, 7000.0f)));
				e7->multiplyTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f)));

				VESceneNode *e8 = m_pSceneManager->loadModel("The Building5", "models/buildings", "building6.obj");
				e8->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 1.0f, 8000.0f)));
				e8->multiplyTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f)));
*/
			
		

		

		///Load the first level into the game engine
		//The engine uses Y-UP, Left-handed
		void loadLevel() {

			VESceneNode *sp1 = m_pSceneManager->createSkybox("The Sky", "models/test/sky/cloudy",
			{ "bluecloud_ft.jpg", "bluecloud_bk.jpg", "bluecloud_up.jpg", "bluecloud_dn.jpg", "bluecloud_rt.jpg", "bluecloud_lf.jpg" });		
			RotatorListener *pRot = new RotatorListener("CubemapRotator", sp1, 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
			getEnginePointer()->registerEventListener(pRot);

			VESceneNode *e4 = m_pSceneManager->loadModel("The Plane", "models/test", "plane_t_n_s.obj");
			e4->setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(1000.0f, 1.0f, 1000.0f)));
			VEEntity *pE4 = (VEEntity*)m_pSceneManager->getSceneNode("The Plane/plane_t_n_s.obj/plane/Entity_0");
			pE4->setParam( glm::vec4(1000.0f, 1000.0f, 0.0f, 0.0f) );

			VESceneNode *pointLight = getSceneManager()->getSceneNode("StandardPointLight");
			VESceneNode *eL = m_pSceneManager->loadModel("The Light", "models/test/sphere", "sphere.obj", 0 , pointLight);
			eL->multiplyTransform(glm::scale(glm::vec3(0.02f,0.02f,0.02f)));
			VEEntity *pE = (VEEntity*)getSceneManager()->getSceneNode("The Light/sphere.obj/default/Entity_0");
			pE->m_castsShadow = false;
/*
			VESceneNode *e1 = m_pSceneManager->loadModel("The Cube",  "models/test/crate0", "cube.obj");
			e1->setTransform(glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 1.0f, 1.0f)));
			e1->multiplyTransform( glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f)));
			*/
			createHouse(5);
			

			//VESceneNode *pSponza = m_pSceneManager->loadModel("Sponza", "models/sponza", "sponza.dae", aiProcess_FlipWindingOrder);
			//pSponza->setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f)));

		};
	};
}


using namespace ve;

int main() {

	MyVulkanEngine mve(true);	//enable or disable debugging (=callback, valication layers)

	try {
		mve.initEngine();
		mve.loadLevel();
		mve.run();
	}
	catch ( const std::runtime_error & err ) {
		if (mve.getLoopCount() == 0) {							//engine was not initialized
			std::cout << "Error: " << err.what() << std::endl;	//just output to console
			char in = getchar();
			return 1;
		}

		getEnginePointer()->fatalError(err.what());		//engine has been initialized
		mve.run();										//output error in window
		return 1;
	}

	return 0;
}

