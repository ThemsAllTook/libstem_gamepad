#include "gamepad/Gamepad.h"
#include "glutshell/GLUTTarget.h"
#include "shell/Shell.h"
#include "shell/ShellKeyCodes.h"
#include "shell/Target.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#endif

static bool verbose = false;

void onButtonDown(struct Gamepad_device * device, unsigned int buttonID, double timestamp) {
	if (verbose) {
		printf("Button %u down on device %u at %f\n", buttonID, device->deviceID, timestamp);
	}
}

void onButtonUp(struct Gamepad_device * device, unsigned int buttonID, double timestamp) {
	if (verbose) {
		printf("Button %u up on device %u at %f\n", buttonID, device->deviceID, timestamp);
	}
}

void onAxisMoved(struct Gamepad_device * device, unsigned int axisID, float value, double timestamp) {
	if (verbose) {
		printf("Axis %u moved to %f on device %u at %f\n", axisID, value, device->deviceID, timestamp);
	}
}

void onDeviceAttached(struct Gamepad_device * device) {
	if (verbose) {
		printf("Device ID %u attached (vendor = 0x%X; product = 0x%X)\n", device->deviceID, device->vendorID, device->productID);
	}
}

void onDeviceRemoved(struct Gamepad_device * device) {
	if (verbose) {
		printf("Device ID %u removed\n", device->deviceID);
	}
}

static unsigned int windowWidth = 800, windowHeight = 600;

static void initGamepad() {
	Gamepad_deviceAttachFunc(onDeviceAttached);
	Gamepad_deviceRemoveFunc(onDeviceRemoved);
	Gamepad_buttonDownFunc(onButtonDown);
	Gamepad_buttonUpFunc(onButtonUp);
	Gamepad_axisMoveFunc(onAxisMoved);
	Gamepad_init();
}

void Target_init() {
	initGamepad();
	
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, windowWidth, windowHeight, 0.0f, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	
	Shell_mainLoop();
}

static void drawGlutString(int rasterPosX, int rasterPosY, const char * string) {
	size_t length, charIndex;
	
	glRasterPos2i(rasterPosX, rasterPosY);
	length = strlen(string);
	for (charIndex = 0; charIndex < length; charIndex++) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, string[charIndex]);
	}
}

#define POLL_ITERATION_INTERVAL 30

bool Target_draw() {
	unsigned int gamepadIndex;
	struct Gamepad_device * device;
	unsigned int axesPerRow, buttonsPerRow;
	unsigned int axisRowIndex, axisIndex;
	unsigned int buttonRowIndex, buttonIndex;
	float axisState;
	char indexString[16];
	static unsigned int iterationsToNextPoll = POLL_ITERATION_INTERVAL;
	char descriptionString[256];
	
	iterationsToNextPoll--;
	if (iterationsToNextPoll == 0) {
		Gamepad_detectDevices();
		iterationsToNextPoll = POLL_ITERATION_INTERVAL;
	}
	Gamepad_processEvents();
	
	axesPerRow = (windowWidth - 10) / 60;
	buttonsPerRow = (windowWidth - 10) / 30;
	
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(5.0f, 20.0f, 0.0f);
	for (gamepadIndex = 0; gamepadIndex < Gamepad_numDevices(); gamepadIndex++) {
		device = Gamepad_deviceAtIndex(gamepadIndex);
		
		glColor3f(0.0f, 0.0f, 0.0f);
		snprintf(descriptionString, 256, "%s (0x%X 0x%X %u)", device->description, device->vendorID, device->productID, device->deviceID);
		drawGlutString(0, 0, descriptionString);
		
		for (axisRowIndex = 0; axisRowIndex <= device->numAxes / axesPerRow; axisRowIndex++) {
			glPushMatrix();
			for (axisIndex = axisRowIndex * axesPerRow; axisIndex < (axisRowIndex + 1) * axesPerRow && axisIndex < device->numAxes; axisIndex++) {
				axisState = device->axisStates[axisIndex];
				
				sprintf(indexString, "a%d", axisIndex);
				glColor3f(0.0f, 0.0f, 0.0f);
				drawGlutString(2, 28, indexString);
				
				glBegin(GL_QUADS);
				glVertex2f(2.0f, 5.0f);
				glVertex2f(58.0f, 5.0f);
				glVertex2f(58.0f, 15.0f);
				glVertex2f(2.0f, 15.0f);
				glColor3f(0.5f, 1.0f, 0.5f);
				glVertex2f(29.0f + axisState * 26, 6.0f);
				glVertex2f(31.0f + axisState * 26, 6.0f);
				glVertex2f(31.0f + axisState * 26, 14.0f);
				glVertex2f(29.0f + axisState * 26, 14.0f);
				glEnd();
				glTranslatef(60.0f, 0.0f, 0.0f);
			}
			glPopMatrix();
			glTranslatef(0.0f, 32.0f, 0.0f);
		}
		
		for (buttonRowIndex = 0; buttonRowIndex <= device->numButtons / buttonsPerRow; buttonRowIndex++) {
			glPushMatrix();
			for (buttonIndex = buttonRowIndex * buttonsPerRow; buttonIndex < (buttonRowIndex + 1) * buttonsPerRow && buttonIndex < device->numButtons; buttonIndex++) {
				sprintf(indexString, "b%d", buttonIndex);
				glColor3f(0.0f, 0.0f, 0.0f);
				drawGlutString(2, 32, indexString);
				
				glBegin(GL_QUADS);
				glColor3f(0.0f, 0.0f, 0.0f);
				glVertex2f(2.0f, 2.0f);
				glVertex2f(28.0f, 2.0f);
				glVertex2f(28.0f, 18.0f);
				glVertex2f(2.0f, 18.0f);
				if (device->buttonStates[buttonIndex]) {
					glColor3f(0.5f, 1.0f, 0.5f);
					glVertex2f(3.0f, 3.0f);
					glVertex2f(27.0f, 3.0f);
					glVertex2f(27.0f, 17.0f);
					glVertex2f(3.0f, 17.0f);
				}
				glEnd();
				glTranslatef(30.0f, 0.0f, 0.0f);
			}
			glPopMatrix();
			glTranslatef(0.0f, 38.0f, 0.0f);
		}
		glTranslatef(0.0f, 40.0f, 0.0f);
	}
	
	if (gamepadIndex == 0) {
		glLoadIdentity();
		glTranslatef(5.0f, 20.0f, 0.0f);
		glColor3f(0.0f, 0.0f, 0.0f);
		drawGlutString(0, 0, "No devices found; plug in a USB gamepad and it will be detected automatically");
	}
	
	Shell_redisplay();
	return true;
}

void Target_keyDown(unsigned int charCode, unsigned int keyCode, unsigned int keyModifiers) {
	if (keyCode == KEYBOARD_R) {
		Gamepad_shutdown();
		initGamepad();
	}
}

void Target_keyUp(unsigned int keyCode, unsigned int keyModifiers) {
}

void Target_keyModifiersChanged(unsigned int keyModifiers) {
}

void Target_mouseDown(unsigned int buttonNumber, float x, float y) {
}

void Target_mouseUp(unsigned int buttonNumber, float x, float y) {
}

void Target_mouseMoved(float x, float y) {
}

void Target_mouseDragged(unsigned int buttonMask, float x, float y) {
}

void Target_resized(unsigned int newWidth, unsigned int newHeight) {
	windowWidth = newWidth;
	windowHeight = newHeight;
	glViewport(0, 0, newWidth, newHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, windowWidth, windowHeight, 0.0f, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
}

void GLUTTarget_configure(int argc, const char ** argv, struct GLUTShellConfiguration * configuration) {
	int	argIndex;
	
	for (argIndex = 1; argIndex < argc; argIndex++) {
		if (!strcmp(argv[argIndex], "-v")) {
			verbose = true;
		}
	}
	configuration->windowTitle = "Gamepad Test Harness";
}
