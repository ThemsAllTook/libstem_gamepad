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

#include "gamepad/Gamepad.h"
#include <IOKit/hid/IOHIDLib.h>
#include <limits.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

struct HIDGamepadAxis {
	IOHIDElementCookie cookie;
	CFIndex logicalMin;
	CFIndex logicalMax;
};

struct HIDGamepadButton {
	IOHIDElementCookie cookie;
};

struct HIDGamepadDevice {
	IOHIDDeviceRef deviceRef;
	struct HIDGamepadAxis * axisElements;
	struct HIDGamepadButton * buttonElements;
};

struct Gamepad_queuedEvent {
	EventDispatcher * dispatcher;
	const char * eventType;
	void * eventData;
};

static IOHIDManagerRef hidManager = NULL;
static struct Gamepad_device ** devices = NULL;
static unsigned int numDevices = 0;
static unsigned int nextDeviceID = 0;

static struct Gamepad_queuedEvent * inputEventQueue = NULL;
static size_t inputEventQueueSize = 0;
static size_t inputEventCount = 0;

static struct Gamepad_queuedEvent * deviceEventQueue = NULL;
static size_t deviceEventQueueSize = 0;
static size_t deviceEventCount = 0;

static void onDeviceValueChanged(void * context, IOReturn result, void * sender, IOHIDValueRef value) {
	struct Gamepad_device * deviceRecord;
	struct HIDGamepadDevice * hidDeviceRecord;
	IOHIDElementRef element;
	IOHIDElementCookie cookie;
	unsigned int axisIndex, buttonIndex;
	static mach_timebase_info_data_t timebaseInfo;
	struct Gamepad_queuedEvent queuedEvent;
	
	if (timebaseInfo.denom == 0) {
		mach_timebase_info(&timebaseInfo);
	}
	
	deviceRecord = context;
	hidDeviceRecord = deviceRecord->privateData;
	element = IOHIDValueGetElement(value);
	cookie = IOHIDElementGetCookie(element);
	
	for (axisIndex = 0; axisIndex < deviceRecord->numAxes; axisIndex++) {
		if (hidDeviceRecord->axisElements[axisIndex].cookie == cookie) {
			struct Gamepad_axisEvent * axisEvent;
			CFIndex integerValue;
			
			integerValue = IOHIDValueGetIntegerValue(value);
			if (integerValue < hidDeviceRecord->axisElements[axisIndex].logicalMin) {
				hidDeviceRecord->axisElements[axisIndex].logicalMin = integerValue;
			}
			if (integerValue > hidDeviceRecord->axisElements[axisIndex].logicalMax) {
				hidDeviceRecord->axisElements[axisIndex].logicalMax = integerValue;
			}
			
			axisEvent = malloc(sizeof(struct Gamepad_axisEvent));
			axisEvent->device = deviceRecord;
			axisEvent->timestamp = IOHIDValueGetTimeStamp(value) * timebaseInfo.numer / timebaseInfo.denom * 0.000000001;
			axisEvent->axisID = axisIndex;
			axisEvent->value = (integerValue - hidDeviceRecord->axisElements[axisIndex].logicalMin) / (float) (hidDeviceRecord->axisElements[axisIndex].logicalMax - hidDeviceRecord->axisElements[axisIndex].logicalMin);
			
			deviceRecord->axisStates[axisIndex] = axisEvent->value;
			
			queuedEvent.dispatcher = deviceRecord->eventDispatcher;
			queuedEvent.eventType = GAMEPAD_EVENT_AXIS_MOVED;
			queuedEvent.eventData = axisEvent;
			
			if (inputEventCount >= inputEventQueueSize) {
				inputEventQueueSize = inputEventQueueSize == 0 ? 1 : inputEventQueueSize * 2;
				inputEventQueue = realloc(inputEventQueue, sizeof(struct Gamepad_queuedEvent) * inputEventQueueSize);
			}
			inputEventQueue[inputEventCount++] = queuedEvent;
			
			return;
		}
	}
	
	for (buttonIndex = 0; buttonIndex < deviceRecord->numButtons; buttonIndex++) {
		if (hidDeviceRecord->buttonElements[buttonIndex].cookie == cookie) {
			struct Gamepad_buttonEvent * buttonEvent;
			
			buttonEvent = malloc(sizeof(struct Gamepad_buttonEvent));
			buttonEvent->device = deviceRecord;
			buttonEvent->timestamp = IOHIDValueGetTimeStamp(value) * timebaseInfo.numer / timebaseInfo.denom * 0.000000001;
			buttonEvent->buttonID = buttonIndex;
			buttonEvent->down = IOHIDValueGetIntegerValue(value);
			
			deviceRecord->buttonStates[buttonIndex] = buttonEvent->down;
			
			queuedEvent.dispatcher = deviceRecord->eventDispatcher;
			queuedEvent.eventType = buttonEvent->down ? GAMEPAD_EVENT_BUTTON_DOWN : GAMEPAD_EVENT_BUTTON_UP;
			queuedEvent.eventData = buttonEvent;
			
			if (inputEventCount >= inputEventQueueSize) {
				inputEventQueueSize = inputEventQueueSize == 0 ? 1 : inputEventQueueSize * 2;
				inputEventQueue = realloc(inputEventQueue, sizeof(struct Gamepad_queuedEvent) * inputEventQueueSize);
			}
			inputEventQueue[inputEventCount++] = queuedEvent;
			
			return;
		}
	}
}

static void onDeviceMatched(void * context, IOReturn result, void * sender, IOHIDDeviceRef device) {
	CFArrayRef elements;
	CFIndex elementIndex;
	IOHIDElementRef element;
	CFStringRef cfProductName;
	struct Gamepad_device * deviceRecord;
	struct HIDGamepadDevice * hidDeviceRecord;
	IOHIDElementType type;
	char * description;
	struct Gamepad_queuedEvent queuedEvent;
	unsigned int axisIndex;
	
	deviceRecord = malloc(sizeof(struct Gamepad_device));
	deviceRecord->deviceID = nextDeviceID++;
	deviceRecord->numAxes = 0;
	deviceRecord->numButtons = 0;
	deviceRecord->eventDispatcher = EventDispatcher_create(deviceRecord);
	devices = realloc(devices, sizeof(struct Gamepad_device *) * (numDevices + 1));
	devices[numDevices++] = deviceRecord;
	
	hidDeviceRecord = malloc(sizeof(struct HIDGamepadDevice));
	hidDeviceRecord->deviceRef = device;
	hidDeviceRecord->axisElements = NULL;
	hidDeviceRecord->buttonElements = NULL;
	deviceRecord->privateData = hidDeviceRecord;
	
	cfProductName = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
	if (cfProductName == NULL || CFGetTypeID(cfProductName) != CFStringGetTypeID()) {
		description = malloc(strlen("[Unknown]" + 1));
		strcpy(description, "[Unknown]");
		
	} else {
		const char * cStringPtr;
		
		cStringPtr = CFStringGetCStringPtr(cfProductName, CFStringGetSmallestEncoding(cfProductName));
		description = malloc(strlen(cStringPtr + 1));
		strcpy(description, cStringPtr);
	}
	deviceRecord->description = description;
	
	elements = IOHIDDeviceCopyMatchingElements(device, NULL, kIOHIDOptionsTypeNone);
	for (elementIndex = 0; elementIndex < CFArrayGetCount(elements); elementIndex++) {
		element = (IOHIDElementRef) CFArrayGetValueAtIndex(elements, elementIndex);
		type = IOHIDElementGetType(element);
		
		// All of the axis elements I've ever detected have been kIOHIDElementTypeInput_Misc. kIOHIDElementTypeInput_Axis is only included for good faith...
		if (type == kIOHIDElementTypeInput_Misc ||
		    type == kIOHIDElementTypeInput_Axis) {
			hidDeviceRecord->axisElements = realloc(hidDeviceRecord->axisElements, sizeof(struct HIDGamepadAxis) * (deviceRecord->numAxes + 1));
			hidDeviceRecord->axisElements[deviceRecord->numAxes].logicalMin = IOHIDElementGetLogicalMin(element);
			hidDeviceRecord->axisElements[deviceRecord->numAxes].logicalMax = IOHIDElementGetLogicalMax(element);
			hidDeviceRecord->axisElements[deviceRecord->numAxes].cookie = IOHIDElementGetCookie(element);
			deviceRecord->numAxes++;
			
		} else if (type == kIOHIDElementTypeInput_Button) {
			hidDeviceRecord->buttonElements = realloc(hidDeviceRecord->buttonElements, sizeof(struct HIDGamepadButton) * (deviceRecord->numButtons + 1));
			hidDeviceRecord->buttonElements[deviceRecord->numButtons].cookie = IOHIDElementGetCookie(element);
			deviceRecord->numButtons++;
		}
	}
	CFRelease(elements);
	
	deviceRecord->axisStates = malloc(sizeof(float) * deviceRecord->numAxes);
	for (axisIndex = 0; axisIndex < deviceRecord->numAxes; axisIndex++) {
		deviceRecord->axisStates[axisIndex] = 0.5f;
	}
	deviceRecord->buttonStates = calloc(sizeof(bool), deviceRecord->numButtons);
	
	IOHIDDeviceRegisterInputValueCallback(device, onDeviceValueChanged, deviceRecord);
	
	queuedEvent.dispatcher = Gamepad_eventDispatcher();
	queuedEvent.eventType = GAMEPAD_EVENT_DEVICE_ATTACHED;
	queuedEvent.eventData = deviceRecord;
	
	if (deviceEventCount >= deviceEventQueueSize) {
		deviceEventQueueSize = deviceEventQueueSize == 0 ? 1 : deviceEventQueueSize * 2;
		deviceEventQueue = realloc(deviceEventQueue, sizeof(struct Gamepad_queuedEvent) * deviceEventQueueSize);
	}
	deviceEventQueue[deviceEventCount++] = queuedEvent;
}

static void disposeDevice(struct Gamepad_device * deviceRecord) {
	unsigned int inputEventIndex, deviceEventIndex;
	
	IOHIDDeviceRegisterInputValueCallback(((struct HIDGamepadDevice *) deviceRecord->privateData)->deviceRef, NULL, NULL);
	
	for (inputEventIndex = 0; inputEventIndex < inputEventCount; inputEventIndex++) {
		if (inputEventQueue[inputEventIndex].dispatcher == deviceRecord->eventDispatcher) {
			unsigned int inputEventIndex2;
			
			free(inputEventQueue[inputEventIndex].eventData);
			inputEventCount--;
			for (inputEventIndex2 = inputEventIndex; inputEventIndex2 < inputEventCount; inputEventIndex2++) {
				inputEventQueue[inputEventIndex2] = inputEventQueue[inputEventIndex2 + 1];
			}
			inputEventIndex--;
		}
	}
	
	for (deviceEventIndex = 0; deviceEventIndex < deviceEventCount; deviceEventIndex++) {
		if (deviceEventQueue[deviceEventIndex].dispatcher == deviceRecord->eventDispatcher) {
			unsigned int deviceEventIndex2;
			
			deviceEventCount--;
			for (deviceEventIndex2 = deviceEventIndex; deviceEventIndex2 < deviceEventCount; deviceEventIndex2++) {
				deviceEventQueue[deviceEventIndex2] = deviceEventQueue[deviceEventIndex2 + 1];
			}
			deviceEventIndex--;
		}
	}
	
	deviceRecord->eventDispatcher->dispose(deviceRecord->eventDispatcher);
	
	free(((struct HIDGamepadDevice *) deviceRecord->privateData)->axisElements);
	free(((struct HIDGamepadDevice *) deviceRecord->privateData)->buttonElements);
	free(deviceRecord->privateData);
	
	free((void *) deviceRecord->description);
	free(deviceRecord->axisStates);
	free(deviceRecord->buttonStates);
	free(deviceRecord->eventDispatcher);
	
	free(deviceRecord);
}

static void onDeviceRemoved(void * context, IOReturn result, void * sender, IOHIDDeviceRef device) {
	unsigned int deviceIndex;
	
	for (deviceIndex = 0; deviceIndex < numDevices; deviceIndex++) {
		if (((struct HIDGamepadDevice *) devices[deviceIndex]->privateData)->deviceRef == device) {
			Gamepad_eventDispatcher()->dispatchEvent(Gamepad_eventDispatcher(), GAMEPAD_EVENT_DEVICE_REMOVED, devices[deviceIndex]);
			
			disposeDevice(devices[deviceIndex]);
			numDevices--;
			for (; deviceIndex < numDevices; deviceIndex++) {
				devices[deviceIndex] = devices[deviceIndex + 1];
			}
			return;
		}
	}
}

void Gamepad_init() {
	if (hidManager == NULL) {
		CFStringRef keys[2];
		int value;
		CFNumberRef values[2];
		CFDictionaryRef dictionaries[3];
		CFArrayRef array;
		
		hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
		IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone);
		IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		
		keys[0] = CFSTR(kIOHIDDeviceUsagePageKey);
		keys[1] = CFSTR(kIOHIDDeviceUsageKey);
		
		value = kHIDPage_GenericDesktop;
		values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
		value = kHIDUsage_GD_Joystick;
		values[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
		dictionaries[0] = CFDictionaryCreate(kCFAllocatorDefault, (const void **) keys, (const void **) values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFRelease(values[0]);
		CFRelease(values[1]);
		
		value = kHIDPage_GenericDesktop;
		values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
		value = kHIDUsage_GD_GamePad;
		values[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
		dictionaries[1] = CFDictionaryCreate(kCFAllocatorDefault, (const void **) keys, (const void **) values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFRelease(values[0]);
		CFRelease(values[1]);
		
		value = kHIDPage_GenericDesktop;
		values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
		value = kHIDUsage_GD_MultiAxisController;
		values[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
		dictionaries[2] = CFDictionaryCreate(kCFAllocatorDefault, (const void **) keys, (const void **) values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFRelease(values[0]);
		CFRelease(values[1]);
		
		array = CFArrayCreate(kCFAllocatorDefault, (const void **) dictionaries, 3, &kCFTypeArrayCallBacks);
		CFRelease(dictionaries[0]);
		CFRelease(dictionaries[1]);
		CFRelease(dictionaries[2]);
		IOHIDManagerSetDeviceMatchingMultiple(hidManager, array);
		CFRelease(array);
		
		IOHIDManagerRegisterDeviceMatchingCallback(hidManager, onDeviceMatched, NULL);
		IOHIDManagerRegisterDeviceRemovalCallback(hidManager, onDeviceRemoved, NULL);
	}
}

void Gamepad_shutdown() {
	if (hidManager != NULL) {
		unsigned int deviceIndex;
		
		IOHIDManagerUnscheduleFromRunLoop(hidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		IOHIDManagerClose(hidManager, 0);
		CFRelease(hidManager);
		hidManager = NULL;
		
		for (deviceIndex = 0; deviceIndex < numDevices; deviceIndex++) {
			disposeDevice(devices[deviceIndex]);
		}
		free(devices);
		numDevices = 0;
	}
}

EventDispatcher * Gamepad_eventDispatcher() {
	static EventDispatcher * eventDispatcher = NULL;
	
	if (eventDispatcher == NULL) {
		eventDispatcher = EventDispatcher_create(NULL);
	}
	return eventDispatcher;
}

unsigned int Gamepad_numDevices() {
	return numDevices;
}

struct Gamepad_device * Gamepad_deviceAtIndex(unsigned int deviceIndex) {
	if (deviceIndex >= numDevices) {
		return NULL;
	}
	return devices[deviceIndex];
}

void Gamepad_detectDevices() {
	unsigned int eventIndex;
	
	for (eventIndex = 0; eventIndex < deviceEventCount; eventIndex++) {
		deviceEventQueue[eventIndex].dispatcher->dispatchEvent(deviceEventQueue[eventIndex].dispatcher, deviceEventQueue[eventIndex].eventType, deviceEventQueue[eventIndex].eventData);
	}
	deviceEventCount = 0;
}

void Gamepad_processEvents() {
	unsigned int eventIndex;
	
	for (eventIndex = 0; eventIndex < inputEventCount; eventIndex++) {
		inputEventQueue[eventIndex].dispatcher->dispatchEvent(inputEventQueue[eventIndex].dispatcher, inputEventQueue[eventIndex].eventType, inputEventQueue[eventIndex].eventData);
		free(inputEventQueue[eventIndex].eventData);
	}
	inputEventCount = 0;
}
