/*
  Copyright (c) 2013 Alex Diener
  
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

#include "gamepad/Gamepad.h"
#include "gamepad/Gamepad_private.h"
#include <stdlib.h>

void (* Gamepad_deviceAttachCallback)(struct Gamepad_device * device) = NULL;
void (* Gamepad_deviceRemoveCallback)(struct Gamepad_device * device) = NULL;
void (* Gamepad_buttonDownCallback)(struct Gamepad_device * device, unsigned int buttonID, double timestamp) = NULL;
void (* Gamepad_buttonUpCallback)(struct Gamepad_device * device, unsigned int buttonID, double timestamp) = NULL;
void (* Gamepad_axisMoveCallback)(struct Gamepad_device * device, unsigned int buttonID, float value, double timestamp) = NULL;

void Gamepad_deviceAttachFunc(void (* callback)(struct Gamepad_device * device)) {
	Gamepad_deviceAttachCallback = callback;
}

void Gamepad_deviceRemoveFunc(void (* callback)(struct Gamepad_device * device)) {
	Gamepad_deviceRemoveCallback = callback;
}

void Gamepad_buttonDownFunc(void (* callback)(struct Gamepad_device * device, unsigned int buttonID, double timestamp)) {
	Gamepad_buttonDownCallback = callback;
}

void Gamepad_buttonUpFunc(void (* callback)(struct Gamepad_device * device, unsigned int buttonID, double timestamp)) {
	Gamepad_buttonUpCallback = callback;
}

void Gamepad_axisMoveFunc(void (* callback)(struct Gamepad_device * device, unsigned int axisID, float value, double timestamp)) {
	Gamepad_axisMoveCallback = callback;
}
