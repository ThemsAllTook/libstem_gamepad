/*
  Copyright (c) 2010 Alex Diener
  
  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.
  
  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:
  
  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
  
  Alex Diener adiener@sacredsoftware.net
*/

#ifndef __GAMEPAD_H__
#define __GAMEPAD_H__

#include <stdbool.h>
#include "utilities/EventDispatcher.h"

// eventData -> struct Gamepad_device
#define GAMEPAD_EVENT_DEVICE_ATTACHED "GAMEPAD_EVENT_DEVICE_ATTACHED" // Only dispatched when Gamepad_init or Gamepad_detectDevices is called
#define GAMEPAD_EVENT_DEVICE_REMOVED  "GAMEPAD_EVENT_DEVICE_REMOVED" // Can be dispatched at any time

// eventData -> struct Gamepad_buttonEvent
#define GAMEPAD_EVENT_BUTTON_DOWN     "GAMEPAD_EVENT_BUTTON_DOWN" // Only dispatched when Gamepad_processEvents is called
#define GAMEPAD_EVENT_BUTTON_UP       "GAMEPAD_EVENT_BUTTON_UP" // Only dispatched when Gamepad_processEvents is called

// eventData -> struct Gamepad_axisEvent
#define GAMEPAD_EVENT_AXIS_MOVED      "GAMEPAD_EVENT_AXIS_MOVED" // Only dispatched when Gamepad_processEvents is called

struct Gamepad_buttonEvent {
	struct Gamepad_device * device;
	double timestamp;
	unsigned int buttonID;
	bool down;
};

struct Gamepad_axisEvent {
	struct Gamepad_device * device;
	double timestamp;
	unsigned int axisID;
	float value;
};

struct Gamepad_device {
	unsigned int deviceID;
	const char * description;
	unsigned int numAxes;
	unsigned int numButtons;
	float * axisStates;
	bool * buttonStates;
	
	// Broadcasts GAMEPAD_EVENT_BUTTON_DOWN, GAMEPAD_EVENT_BUTTON_UP, GAMEPAD_EVENT_AXIS_MOVED
	EventDispatcher * eventDispatcher;
	
	void * privateData;
};

void Gamepad_init();
void Gamepad_shutdown();

// Broadcasts GAMEPAD_EVENT_DEVICE_ATTACHED, GAMEPAD_EVENT_DEVICE_REMOVED
EventDispatcher * Gamepad_eventDispatcher();

unsigned int Gamepad_numDevices();
struct Gamepad_device * Gamepad_deviceAtIndex(unsigned int deviceIndex);

void Gamepad_detectDevices();
void Gamepad_processEvents();

#endif
