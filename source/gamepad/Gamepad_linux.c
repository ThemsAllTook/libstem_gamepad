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
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/joystick.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

struct Gamepad_devicePrivate {
	int fd;
	char * path;
};

static struct Gamepad_device ** devices = NULL;
static unsigned int numDevices = 0;
static unsigned int nextDeviceID = 0;

static char ** findGamepadPaths(unsigned int * outNumGamepads) {
	DIR * dev_input, * dev;
	struct dirent * entity;
	unsigned int numGamepads = 0;
	char ** gamepadDevs = NULL;
	unsigned int charsConsumed;
	int num;
	
	dev_input = opendir("/dev/input");
	if (dev_input != NULL) {
		for (entity = readdir(dev_input); entity != NULL; entity = readdir(dev_input)) {
			charsConsumed = 0;
			if (sscanf(entity->d_name, "js%d%n", &num, &charsConsumed) && charsConsumed == strlen(entity->d_name)) {
				numGamepads++;
				gamepadDevs = realloc(gamepadDevs, sizeof(char *) * numGamepads);
				gamepadDevs[numGamepads - 1] = malloc(strlen(entity->d_name) + 1 + strlen("/dev/input/"));
				sprintf(gamepadDevs[numGamepads - 1], "/dev/input/%s", entity->d_name);
			}
		}
		closedir(dev_input);
	}
	dev = opendir("/dev");
	if (dev != NULL) {
		for (entity = readdir(dev); entity != NULL; entity = readdir(dev)) {
			if (sscanf(entity->d_name, "js%d%n", &num, &charsConsumed) && charsConsumed == strlen(entity->d_name)) {
				numGamepads++;
				gamepadDevs = realloc(gamepadDevs, sizeof(char *) * numGamepads);
				gamepadDevs[numGamepads - 1] = malloc(strlen(entity->d_name) + 1 + strlen("/dev/"));
				sprintf(gamepadDevs[numGamepads - 1], "/dev/%s", entity->d_name);
			}
		}
		closedir(dev);
	}
	
	*outNumGamepads = numGamepads;
	return gamepadDevs;
}

void Gamepad_init() {
	Gamepad_detectDevices();
}

static void disposeDevice(struct Gamepad_device * device) {
	device->eventDispatcher->dispose(device->eventDispatcher);
	
	close(((struct Gamepad_devicePrivate *) device->privateData)->fd);
	free(((struct Gamepad_devicePrivate *) device->privateData)->path);
	free(device->privateData);
	
	free((void *) device->description);
	free(device->axisStates);
	free(device->buttonStates);
	free(device->eventDispatcher);
	
	free(device);
}

void Gamepad_shutdown() {
	unsigned int deviceIndex;
	
	for (deviceIndex = 0; deviceIndex < numDevices; deviceIndex++) {
		disposeDevice(devices[deviceIndex]);
	}
	free(devices);
	numDevices = 0;
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
	unsigned int numPaths;
	char ** gamepadPaths;
	bool duplicate;
	unsigned int pathIndex, gamepadIndex;
	struct stat statBuf;
	struct Gamepad_device * deviceRecord;
	struct Gamepad_devicePrivate * deviceRecordPrivate;
	int fd;
	char name[128];
	char * description;
	unsigned char axes, buttons;
	
	gamepadPaths = findGamepadPaths(&numPaths);
	
	for (pathIndex = 0; pathIndex < numPaths; pathIndex++) {
		duplicate = false;
		for (gamepadIndex = 0; gamepadIndex < numDevices; gamepadIndex++) {
			if (!strcmp(((struct Gamepad_devicePrivate *) devices[gamepadIndex]->privateData)->path, gamepadPaths[pathIndex])) {
				duplicate = true;
				break;
			}
		}
		if (duplicate) {
			free(gamepadPaths[pathIndex]);
			continue;
		}
		
		if (!stat(gamepadPaths[pathIndex], &statBuf)) {
			deviceRecord = malloc(sizeof(struct Gamepad_device));
			deviceRecord->deviceID = nextDeviceID++;
			deviceRecord->eventDispatcher = EventDispatcher_create(deviceRecord);
			devices = realloc(devices, sizeof(struct Gamepad_device *) * (numDevices + 1));
			devices[numDevices++] = deviceRecord;
			
			fd = open(gamepadPaths[pathIndex], O_RDONLY, 0);
			
			deviceRecordPrivate = malloc(sizeof(struct Gamepad_devicePrivate));
			deviceRecordPrivate->fd = fd;
			deviceRecordPrivate->path = gamepadPaths[pathIndex];
			deviceRecord->privateData = deviceRecordPrivate;
			
			if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) > 0) {
				description = malloc(strlen(name + 1));
				strcpy(description, name);
			} else {
				description = malloc(strlen(gamepadPaths[pathIndex] + 1));
				strcpy(description, gamepadPaths[pathIndex]);
			}
			deviceRecord->description = description;
			
			axes = 0;
			ioctl(fd, JSIOCGAXES, &axes);
			buttons = 0;
			ioctl(fd, JSIOCGBUTTONS, &buttons);
			
			deviceRecord->numAxes = axes;
			deviceRecord->numButtons = buttons;
			
			deviceRecord->axisStates = calloc(sizeof(float), deviceRecord->numAxes);
			deviceRecord->buttonStates = calloc(sizeof(bool), deviceRecord->numButtons);
			
			fcntl(fd, F_SETFL, O_NONBLOCK);
			
			Gamepad_eventDispatcher()->dispatchEvent(Gamepad_eventDispatcher(), GAMEPAD_EVENT_DEVICE_ATTACHED, deviceRecord);
		}
	}
}

void Gamepad_processEvents() {
	unsigned int gamepadIndex;
	struct Gamepad_device * device;
	struct Gamepad_devicePrivate * devicePrivate;
	struct js_event event;
	
	for (gamepadIndex = 0; gamepadIndex < numDevices; gamepadIndex++) {
		device = devices[gamepadIndex];
		devicePrivate = device->privateData;
		
		while (read(devicePrivate->fd, &event, sizeof(struct js_event)) != -1) {
			if (event.type == JS_EVENT_AXIS) {
				struct Gamepad_axisEvent axisEvent;
				
				if (event.number >= device->numAxes) {
					continue;
				}
				
				axisEvent.device = device;
				axisEvent.timestamp = event.time; // TODO: Normalize
				axisEvent.axisID = event.number;
				axisEvent.value = (event.value - SHRT_MIN) / (float) USHRT_MAX * 2.0f - 1.0f;
				
				device->axisStates[event.number] = axisEvent.value;
				
				device->eventDispatcher->dispatchEvent(device->eventDispatcher, GAMEPAD_EVENT_AXIS_MOVED, &axisEvent);
				
			} else if (event.type == JS_EVENT_BUTTON) {
				struct Gamepad_buttonEvent buttonEvent;
				
				if (event.number >= device->numButtons) {
					continue;
				}
				
				buttonEvent.device = device;
				buttonEvent.timestamp = event.time; // TODO: Normalize
				buttonEvent.buttonID = event.number;
				buttonEvent.down = event.value;
				
				device->buttonStates[event.number] = buttonEvent.down;
				
				device->eventDispatcher->dispatchEvent(device->eventDispatcher, buttonEvent.down ? GAMEPAD_EVENT_BUTTON_DOWN : GAMEPAD_EVENT_BUTTON_UP, &buttonEvent);
			}
		}
		if (errno != EAGAIN) {
			unsigned int gamepadIndex2;
			
			Gamepad_eventDispatcher()->dispatchEvent(Gamepad_eventDispatcher(), GAMEPAD_EVENT_DEVICE_REMOVED, device);
			
			disposeDevice(device);
			numDevices--;
			for (gamepadIndex2 = gamepadIndex; gamepadIndex2 < numDevices; gamepadIndex2++) {
				devices[gamepadIndex2] = devices[gamepadIndex2 + 1];
			}
			gamepadIndex--;
		}
	}
}

