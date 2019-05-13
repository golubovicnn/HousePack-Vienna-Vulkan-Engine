/**
* The Vienna Vulkan Engine
*
* (c) bei Helmut Hlavacs, University of Vienna
*
*/

#pragma once


namespace ve {
	class VEEngine;

	/**
	* \brief Enumerates possible subsystems. 
	*
	* A subsystem determines the scope of events and how event data are interpreted. For example, GLFW is 
	* a subsystem using its own event types and keyboard IDs. Event listeners can use the subsystem to 
	* determine whether they can even understand the event. 
	*
	*/
	enum veEventSubsystem {
		VE_EVENT_SUBSYSTEM_GENERIC,		///<No subsystem given
		VE_EVENT_SUBSYSTEM_GLFW			///<GLFW
	};

	/**
	* \brief Enumerates event types.
	*
	* An event type identifies a class of events, and enables event listeners to further narrow down the events they
	* are interested in. 
	*
	*/
	enum veEventType {
		VE_EVENT_NONE=0,				///<Empty event
		VE_EVENT_FRAME_STARTED=1,		///<The frame has been started
		VE_EVENT_FRAME_ENDED=2,			///<The frame has been rendered and is ready to be presented
		VE_EVENT_KEYBOARD=4,			///<A keyboard event
		VE_EVENT_MOUSEMOVE=8,			///<The mouse has been moved
		VE_EVENT_MOUSEBUTTON=16,		///<A mouse button event
		VE_EVENT_MOUSESCROLL=32			///<Mouse scroll event
	};

	/**
	* \brief Lifetime of an event.
	*
	* A once-event will be destroyed right after event processing. A continuous event will remain in the
	* event list and will be sent to the listeners in the next loop.
	*
	*/
	enum veEventLifeTime {
		VE_EVENT_LIFETIME_ONCE,			///<One time event
		VE_EVENT_LIFETIME_CONTINUOUS	///<Continuous event
	};


	/**
	* \brief Structure for storing event related data.
	*/
	struct veEvent {
		veEventSubsystem	subsystem = VE_EVENT_SUBSYSTEM_GENERIC;		///<Event subsystem
		veEventType			type = VE_EVENT_NONE;						///<Event type
		veEventLifeTime		lifeTime = VE_EVENT_LIFETIME_ONCE;			///<Event lifetime
		uint64_t			notBeforeTime = 0;							///<Loop count minimum so that event can be processed
		double				dt;											///<Delta time since last loop
		int					idata1 = 0;									///<Integer information attached to the event
		int					idata2 = 0;									///<Integer information attached to the event
		int					idata3 = 0;									///<Integer information attached to the event
		int					idata4 = 0;									///<Integer information attached to the event
		float				fdata1 = 0.0f;								///<Arbitrary float information
		float				fdata2 = 0.0f;								///<Arbitrary float information
		float				fdata3 = 0.0f;								///<Arbitrary float information
		float				fdata4 = 0.0f;								///<Arbitrary float information
		void *				ptr=nullptr;								///<User pointer

		///Constructor using default subsystem
		veEvent(veEventType evt) { subsystem = VE_EVENT_SUBSYSTEM_GENERIC;  type = evt; };
		///Constructor
		veEvent(veEventSubsystem sub, veEventType evt) { subsystem = sub; type = evt; };
	};


	/**
	*
	* \brief Base class of all event listeners
	*
	* An event listener is a class that is sent events from the engine. The listener can examine the event and
	* can decide to act on it. Certain events can be consumed by returning true, like a keyboard stroke. Others like onFramStarted
	* cannot be consumed and are always sent to all event listeners.
	*
	*/
	class VEEventListener : public VENamedClass {
		friend VEEngine;

	protected:
		virtual bool onEvent(veEvent event);

		///Before frame rendering and all event processing. Event cannot be consumed.
		virtual void onFrameStarted(veEvent event) {};
		///After frame rendering and all event processing. Event cannot be consumed.
		virtual void onFrameEnded(veEvent event) {};
		///Keyboard event.  Event can be consumed.
		virtual bool onKeyboard(veEvent event) { return false; };
		///Mouse move event.  Event cann be consumed.
		virtual bool onMouseMove(veEvent event) { return false; };
		///Mouse button event.  Event can be consumed.
		virtual bool onMouseButton(veEvent event) { return false; };
		///Mouse scroll event.  Event can be consumed.
		virtual bool onMouseScroll(veEvent event) { return false; };

	public:
		VEEventListener( std::string name );
		virtual ~VEEventListener();
	};
}
