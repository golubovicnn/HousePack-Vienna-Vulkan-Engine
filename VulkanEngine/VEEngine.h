/**
* The Vienna Vulkan Engine
*
* (c) bei Helmut Hlavacs, University of Vienna
*
*/

#pragma once

#ifndef getRendererPointer
#define getEnginePointer() g_pVEEngineSingleton
#endif

namespace ve {

	//use this macro to check the function result, if its not VK_SUCCESS then return the error
	#define VECHECKRESULT(x, msg) { \
		VkResult retval = (x); \
		if (retval != VK_SUCCESS) { \
			throw std::runtime_error(msg); \
		} \
	}


	class VEWindow;
	class VERenderer;
	class VEEventListener;
	class VESceneManager;

	class VEEngine;
	extern VEEngine* g_pVEEngineSingleton;	///<Pointer to the only class instance 

	/**
	*
	* \brief The engine core class.
	*
	* VEEngine is the core of the whole engine. It is responsible for creating instances of the other
	* management classes VEWindow, VERenderer, and VESceneManager, and running the render loop. 
	* In the render loop, it also handles events and sends them to the registered event listeners.
	*
	*/

	class VEEngine {
		friend VEWindow;
		friend VERenderer;
		friend VEEventListener;
		friend VESceneManager;

	protected:
		VkInstance m_instance = VK_NULL_HANDLE;			///<Vulkan app instance
		VEWindow * m_pWindow = nullptr;					///<Pointer to the only Window instance
		VERenderer * m_pRenderer = nullptr;				///<Pointer to the only renderer instance
		VESceneManager * m_pSceneManager = nullptr;		///<Pointer to the only scene manager instance
		VkDebugReportCallbackEXT callback;				///<Debug callback handle

		std::vector<veEvent> m_eventlist;				///<List of events that should be handled in the next loop
		std::vector<VEEventListener*> m_eventListener;	///<set of registered event listeners

		double m_dt = 0.0;								///<Delta time since the last loop
		double m_time = 0.0;							///<Absolute game time since start of the render loop
		uint32_t m_loopCount = 0;						///<Counts up the render loop

		float m_AvgUpdateTime = 0.0f;					///<Average time for OBO updates (s)
		float m_AvgFrameTime = 0.0f;					///<Average time per frame (s)
		float m_AvgDrawTime = 0.0f;						///<Average time for baking cmd buffers and calling commit (s)

		bool m_framebufferResized = false;				///<Flag indicating whether the window size has changed.
		bool m_end_running = false;						///<Flag indicating that the engine should leave the render loop
		bool m_debug = true;							///<Flag indicating whether debugging is enabled or not

		virtual std::vector<const char*> getRequiredInstanceExtensions(); //Return a list of required Vulkan instance extensions
		virtual std::vector<const char*> getValidationLayers();	//Returns a list of required Vulkan validation layers
		void callListeners(double dt, veEvent event);	//Call all event listeners and give them certain event
		void callListeners(double dt, veEvent event, uint32_t startIdx, uint32_t endIdx);
		void processEvents(double dt);			//Start handling all events
		void windowSizeChanged();				//Callback for window if window size has changed

		//startup routines, can be overloaded to create different managers
		virtual void createWindow();			//Create the only window
		virtual void createRenderer();			//Create the only renderer
		virtual void createSceneManager();		//Create the only scene manager
		virtual void registerEventListeners();	//Register all default event listeners, can be overloaded
		virtual void closeEngine();				//Close down the engine

	public:
		ThreadPool *m_threadPool;				///<A threadpool for parallel processing

		VEEngine( bool debug = false );								//Only create ONE instance of the engine!
		~VEEngine() {};

		virtual void initEngine();							//Create all engine components
		virtual void run();									//Enter the render loop
		virtual void fatalError(std::string message);		//Show an error message and close down the engine
		virtual void end();									//end the render loop
		void registerEventListener(VEEventListener *lis);	//Register a new event listener.
		void removeEventListener(std::string name);			//Remove an event listener - it is NOT deleted automatically!
		void deleteEventListener(std::string name);			//Delete an event listener
		void addEvent(veEvent event);						//Add an event to the event list - will be handled in the next loop
		void deleteEvent(veEvent event);					//Delete an event from the event list

		VkInstance		 getInstance();				//Return the Vulkan instance
		VEWindow       * getWindow();				//Return a pointer to the window instance
		VESceneManager * getSceneManager();			//Return a  pointer to the scene manager instance
		VERenderer     * getRenderer();				//Return a pointer to the renderer instance
		uint32_t		 getLoopCount();			//Return the number of the current render loop
		///\returns the average frame time (s)
		float			 getAvgFrameTime() { return m_AvgFrameTime;  };
		///\returns the average update time (s)
		float			 getAvgUpdateTime() { return m_AvgUpdateTime; };
	};



}