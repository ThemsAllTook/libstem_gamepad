/*
  Copyright (c) 2014 Alex Diener
  
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
  
  Alex Diener alex@ludobloom.com
*/

#include "gamepad/Gamepad.h"
#include "gamepad/Gamepad_private.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <regstr.h>
#include <dinput.h>

#define __in
#define __out
#define __reserved
#include <XInput.h>

// Copy from MinGW-w64 to MinGW, along with wbemcli.h, wbemprov.h, wbemdisp.h, and wbemtran.h
#ifndef __MINGW_EXTENSION
#define __MINGW_EXTENSION
#endif
#define COBJMACROS 1
#include <wbemidl.h>
#include <oleauto.h>
// Super helpful info: http://www.wreckedgames.com/forum/index.php?topic=2584.0

#define INPUT_QUEUE_SIZE 32

struct diAxisInfo {
	DWORD offset;
	bool isPOV;
	bool isPOVSecondAxis;
};

struct Gamepad_devicePrivate {
	GUID guidInstance;
	IDirectInputDevice8 * deviceInterface;
	bool buffered;
	unsigned int sliderCount;
	unsigned int povCount;
	struct diAxisInfo * axisInfo;
	DWORD * buttonOffsets;
};

static struct Gamepad_device ** devices = NULL;
static unsigned int numDevices = 0;
static unsigned int nextDeviceID = 0;

static LPDIRECTINPUT directInputInterface;
static bool inited = false;

void Gamepad_init() {
	if (!inited) {
		HRESULT result;
		HMODULE module;
		HRESULT (* DirectInput8Create_proc)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
		
		//result = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, &IID_IDirectInput8, (void **) &directInputInterface, NULL);
		// Calling DirectInput8Create directly crashes in 64-bit builds for some reason. Loading it with GetProcAddress works though!
		
		module = LoadLibrary("DINPUT8.dll");
		DirectInput8Create_proc = (HRESULT (*)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN)) GetProcAddress(module, "DirectInput8Create");
		result = DirectInput8Create_proc(GetModuleHandle(NULL), DIRECTINPUT_VERSION, &IID_IDirectInput8, (void **) &directInputInterface, NULL);
		
		if (result != DI_OK) {
			fprintf(stderr, "Warning: DirectInput8Create returned 0x%X\n", (unsigned int) result);
		}
		
		inited = true;
		Gamepad_detectDevices();
	}
}

static void disposeDevice(struct Gamepad_device * deviceRecord) {
	struct Gamepad_devicePrivate * deviceRecordPrivate = deviceRecord->privateData;
	
	IDirectInputDevice8_Release(deviceRecordPrivate->deviceInterface);
	free(deviceRecordPrivate->axisInfo);
	free(deviceRecordPrivate->buttonOffsets);
	free(deviceRecordPrivate);
	
	free((void *) deviceRecord->description);
	free(deviceRecord->axisStates);
	free(deviceRecord->buttonStates);
	
	free(deviceRecord);
}

void Gamepad_shutdown() {
	unsigned int deviceIndex;
	
	if (inited) {
		for (deviceIndex = 0; deviceIndex < numDevices; deviceIndex++) {
			disposeDevice(devices[deviceIndex]);
		}
		free(devices);
		devices = NULL;
		numDevices = 0;
		inited = false;
	}
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

// Copied from http://msdn.microsoft.com/en-us/library/windows/desktop/ee417014(v=vs.85).aspx (and reformatted)
static bool isXInputDevice(const GUID * pGuidProductFromDirectInput) {
	IWbemLocator * pIWbemLocator = NULL;
	IEnumWbemClassObject * pEnumDevices = NULL;
	IWbemClassObject * pDevices[20] = {0};
	IWbemServices * pIWbemServices = NULL;
	BSTR bstrNamespace = NULL;
	BSTR bstrDeviceID = NULL;
	BSTR bstrClassName = NULL;
	DWORD uReturned = 0;
	bool bIsXinputDevice = false;
	UINT iDevice = 0;
	VARIANT var;
	HRESULT hr;
	
	hr = CoInitialize(NULL);
	bool bCleanupCOM = SUCCEEDED(hr);
	
	hr = CoCreateInstance(&CLSID_WbemLocator,
	                      NULL,
	                      CLSCTX_INPROC_SERVER,
	                      &IID_IWbemLocator,
	                      (LPVOID *) &pIWbemLocator);
	if (FAILED(hr) || pIWbemLocator == NULL) {
		goto LCleanup;
	}
	
	bstrNamespace = SysAllocString(L"\\\\.\\root\\cimv2"); if (bstrNamespace == NULL) {goto LCleanup;}
	bstrClassName = SysAllocString(L"Win32_PNPEntity");    if (bstrClassName == NULL) {goto LCleanup;}
	bstrDeviceID  = SysAllocString(L"DeviceID");           if (bstrDeviceID == NULL)  {goto LCleanup;}
	
	hr = IWbemLocator_ConnectServer(pIWbemLocator, bstrNamespace, NULL, NULL, 0L,
	                                0L, NULL, NULL, &pIWbemServices);
	if (FAILED(hr) || pIWbemServices == NULL) {
		goto LCleanup;
	}
	
	CoSetProxyBlanket((IUnknown *) pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
	                  RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	
	hr = IWbemServices_CreateInstanceEnum(pIWbemServices, bstrClassName, 0, NULL, &pEnumDevices);
	if (FAILED(hr) || pEnumDevices == NULL) {
		goto LCleanup;
	}
	
	for (;;) {
		hr = IEnumWbemClassObject_Next(pEnumDevices, 10000, 20, pDevices, &uReturned);
		if (FAILED(hr)) {
			goto LCleanup;
		}
		if (uReturned == 0) {
			break;
		}
		for (iDevice = 0; iDevice < uReturned; iDevice++) {
			hr = IWbemClassObject_Get(pDevices[iDevice], bstrDeviceID, 0L, &var, NULL, NULL);
			if (SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal != NULL) {
				if (wcsstr(var.bstrVal, L"IG_")) {
					DWORD dwPid = 0, dwVid = 0;
					WCHAR * strVid = wcsstr(var.bstrVal, L"VID_");
					if (strVid && swscanf(strVid, L"VID_%4X", &dwVid) != 1) {
						dwVid = 0;
					}
					WCHAR * strPid = wcsstr(var.bstrVal, L"PID_");
					if (strPid != NULL && swscanf(strPid, L"PID_%4X", &dwPid) != 1) {
						dwPid = 0;
					}
					DWORD dwVidPid = MAKELONG(dwVid, dwPid);
					if (dwVidPid == pGuidProductFromDirectInput->Data1) {
						bIsXinputDevice = true;
						goto LCleanup;
					}
				}
			}
			if (pDevices[iDevice] != NULL) {
				IWbemClassObject_Release(pDevices[iDevice]);
				pDevices[iDevice] = NULL;
			}
		}
	}
	
LCleanup:
	if (bstrNamespace != NULL) {
		SysFreeString(bstrNamespace);
	}
	if (bstrDeviceID != NULL) {
		SysFreeString(bstrDeviceID);
	}
	if (bstrClassName != NULL) {
		SysFreeString(bstrClassName);
	}
	for (iDevice = 0; iDevice < uReturned; iDevice++) {
		if (pDevices[iDevice] != NULL) {
			IWbemClassObject_Release(pDevices[iDevice]);
		}
	}
	if (pEnumDevices != NULL) {
		IEnumWbemClassObject_Release(pEnumDevices);
	}
	if (pIWbemLocator != NULL) {
		IWbemLocator_Release(pIWbemLocator);
	}
	if (pIWbemServices != NULL) {
		IWbemServices_Release(pIWbemServices);
	}
	
	if (bCleanupCOM) {
		CoUninitialize();
	}
	
	return bIsXinputDevice;
}

static BOOL CALLBACK countAxesCallback(LPCDIDEVICEOBJECTINSTANCE instance, LPVOID context) {
	struct Gamepad_device * deviceRecord = context;
	
	deviceRecord->numAxes++;
	if (instance->dwType & DIDFT_POV) {
		deviceRecord->numAxes++;
	}
	return DIENUM_CONTINUE;
}

static BOOL CALLBACK countButtonsCallback(LPCDIDEVICEOBJECTINSTANCE instance, LPVOID context) {
	struct Gamepad_device * deviceRecord = context;
	
	deviceRecord->numButtons++;
	return DIENUM_CONTINUE;
}

#define AXIS_MIN -32768
#define AXIS_MAX 32767

static BOOL CALLBACK enumAxesCallback(LPCDIDEVICEOBJECTINSTANCE instance, LPVOID context) {
	struct Gamepad_device * deviceRecord = context;
	struct Gamepad_devicePrivate * deviceRecordPrivate = deviceRecord->privateData;
	DWORD offset;
	
	deviceRecord->numAxes++;
	if (instance->dwType & DIDFT_POV) {
		offset = DIJOFS_POV(deviceRecordPrivate->povCount);
		deviceRecordPrivate->axisInfo[deviceRecord->numAxes - 1].offset = offset;
		deviceRecordPrivate->axisInfo[deviceRecord->numAxes - 1].isPOV = true;
		deviceRecord->numAxes++;
		deviceRecordPrivate->axisInfo[deviceRecord->numAxes - 1].offset = offset;
		deviceRecordPrivate->axisInfo[deviceRecord->numAxes - 1].isPOV = true;
		deviceRecordPrivate->povCount++;
		
	} else {
		DIPROPRANGE range;
		DIPROPDWORD deadZone;
		HRESULT result;
		
		if (!memcmp(&instance->guidType, &GUID_XAxis, sizeof(instance->guidType))) {
			offset = DIJOFS_X;
		} else if (!memcmp(&instance->guidType, &GUID_YAxis, sizeof(instance->guidType))) {
			offset = DIJOFS_Y;
		} else if (!memcmp(&instance->guidType, &GUID_ZAxis, sizeof(instance->guidType))) {
			offset = DIJOFS_Z;
		} else if (!memcmp(&instance->guidType, &GUID_RxAxis, sizeof(instance->guidType))) {
			offset = DIJOFS_RX;
		} else if (!memcmp(&instance->guidType, &GUID_RyAxis, sizeof(instance->guidType))) {
			offset = DIJOFS_RY;
		} else if (!memcmp(&instance->guidType, &GUID_RzAxis, sizeof(instance->guidType))) {
			offset = DIJOFS_RZ;
		} else if (!memcmp(&instance->guidType, &GUID_Slider, sizeof(instance->guidType))) {
			offset = DIJOFS_SLIDER(deviceRecordPrivate->sliderCount++);
		} else {
			offset = -1;
		}
		deviceRecordPrivate->axisInfo[deviceRecord->numAxes - 1].offset = offset;
		deviceRecordPrivate->axisInfo[deviceRecord->numAxes - 1].isPOV = false;
		
		range.diph.dwSize = sizeof(range);
		range.diph.dwHeaderSize = sizeof(range.diph);
		range.diph.dwObj = instance->dwType;
		range.diph.dwHow = DIPH_BYID;
		range.lMin = AXIS_MIN;
		range.lMax = AXIS_MAX;
		
		result = IDirectInputDevice8_SetProperty(deviceRecordPrivate->deviceInterface, DIPROP_RANGE, &range.diph);
		if (result != DI_OK) {
			fprintf(stderr, "Warning: IDIrectInputDevice8_SetProperty returned 0x%X\n", (unsigned int) result);
		}
		
		deadZone.diph.dwSize = sizeof(deadZone);
		deadZone.diph.dwHeaderSize = sizeof(deadZone.diph);
		deadZone.diph.dwObj = instance->dwType;
		deadZone.diph.dwHow = DIPH_BYID;
		deadZone.dwData = 0;
		result = IDirectInputDevice8_SetProperty(deviceRecordPrivate->deviceInterface, DIPROP_DEADZONE, &deadZone.diph);
		if (result != DI_OK) {
			fprintf(stderr, "Warning: IDIrectInputDevice8_SetProperty returned 0x%X\n", (unsigned int) result);
		}
	}
	return DIENUM_CONTINUE;
}

static BOOL CALLBACK enumButtonsCallback(LPCDIDEVICEOBJECTINSTANCE instance, LPVOID context) {
	struct Gamepad_device * deviceRecord = context;
	struct Gamepad_devicePrivate * deviceRecordPrivate = deviceRecord->privateData;
	
	deviceRecordPrivate->buttonOffsets[deviceRecord->numButtons] = DIJOFS_BUTTON(deviceRecord->numButtons);
	deviceRecord->numButtons++;
	return DIENUM_CONTINUE;
}

static BOOL CALLBACK enumDevicesCallback(const DIDEVICEINSTANCE * instance, LPVOID context) {
	struct Gamepad_device * deviceRecord;
	struct Gamepad_devicePrivate * deviceRecordPrivate;
	unsigned int deviceIndex;
	IDirectInputDevice * diDevice;
	IDirectInputDevice8 * di8Device;
	HRESULT result;
	DIPROPDWORD bufferSizeProp;
	bool buffered = true;
	
	for (deviceIndex = 0; deviceIndex < numDevices; deviceIndex++) {
		if (!memcmp(&((struct Gamepad_devicePrivate *) devices[deviceIndex]->privateData)->guidInstance, &instance->guidInstance, sizeof(GUID))) {
			return DIENUM_CONTINUE;
		}
	}
	
	if (isXInputDevice(&instance->guidProduct)) {
		printf("Enumeration detected an XInput device: \"%s\", \"%s\"\n", instance->tszInstanceName, instance->tszProductName);
		return DIENUM_CONTINUE;
	}
	
	result = IDirectInput8_CreateDevice(directInputInterface, &instance->guidInstance, &diDevice, NULL);
	if (result != DI_OK) {
		fprintf(stderr, "Warning: IDirectInput8_CreateDevice returned 0x%X\n", (unsigned int) result);
	}
	result = IDirectInputDevice8_QueryInterface(diDevice, &IID_IDirectInputDevice8, (LPVOID *) &di8Device);
	if (result != DI_OK) {
		fprintf(stderr, "Warning: IDirectInputDevice8_QueryInterface returned 0x%X\n", (unsigned int) result);
	}
	IDirectInputDevice8_Release(diDevice);
	
	result = IDirectInputDevice8_SetCooperativeLevel(di8Device, GetActiveWindow(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	if (result != DI_OK) {
		fprintf(stderr, "Warning: IDirectInputDevice8_SetCooperativeLevel returned 0x%X\n", (unsigned int) result);
	}
	
	result = IDirectInputDevice8_SetDataFormat(di8Device, &c_dfDIJoystick2);
	if (result != DI_OK) {
		fprintf(stderr, "Warning: IDirectInputDevice8_SetDataFormat returned 0x%X\n", (unsigned int) result);
	}
	
	bufferSizeProp.diph.dwSize = sizeof(DIPROPDWORD);
	bufferSizeProp.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	bufferSizeProp.diph.dwObj = 0;
	bufferSizeProp.diph.dwHow = DIPH_DEVICE;
	bufferSizeProp.dwData = INPUT_QUEUE_SIZE;
	result = IDirectInputDevice8_SetProperty(di8Device, DIPROP_BUFFERSIZE, &bufferSizeProp.diph);
	if (result == DI_POLLEDDEVICE) {
		buffered = false;
	} else if (result != DI_OK) {
		fprintf(stderr, "Warning: IDirectInputDevice8_SetProperty returned 0x%X\n", (unsigned int) result);
	}
	
	deviceRecord = malloc(sizeof(struct Gamepad_device));
	deviceRecordPrivate = malloc(sizeof(struct Gamepad_devicePrivate));
	deviceRecordPrivate->guidInstance = instance->guidInstance;
	deviceRecordPrivate->deviceInterface = di8Device;
	deviceRecordPrivate->buffered = buffered;
	deviceRecordPrivate->sliderCount = 0;
	deviceRecordPrivate->povCount = 0;
	deviceRecord->privateData = deviceRecordPrivate;
	deviceRecord->deviceID = nextDeviceID++;
	deviceRecord->description = strdup(instance->tszProductName);
	deviceRecord->vendorID = instance->guidProduct.Data1 & 0xFFFF;
	deviceRecord->productID = instance->guidProduct.Data1 >> 16 & 0xFFFF; // May be incorrect
	deviceRecord->numAxes = 0;
	IDirectInputDevice_EnumObjects(di8Device, countAxesCallback, deviceRecord, DIDFT_AXIS | DIDFT_POV);
	deviceRecord->numButtons = 0;
	IDirectInputDevice_EnumObjects(di8Device, countButtonsCallback, deviceRecord, DIDFT_BUTTON);
	deviceRecord->axisStates = calloc(sizeof(float), deviceRecord->numAxes);
	deviceRecord->buttonStates = calloc(sizeof(bool), deviceRecord->numButtons);
	deviceRecordPrivate->axisInfo = calloc(sizeof(struct diAxisInfo), deviceRecord->numAxes);
	deviceRecordPrivate->buttonOffsets = calloc(sizeof(DWORD), deviceRecord->numButtons);
	deviceRecord->numAxes = 0;
	IDirectInputDevice_EnumObjects(di8Device, enumAxesCallback, deviceRecord, DIDFT_AXIS | DIDFT_POV);
	deviceRecord->numButtons = 0;
	IDirectInputDevice_EnumObjects(di8Device, enumButtonsCallback, deviceRecord, DIDFT_BUTTON);
	devices = realloc(devices, sizeof(struct Gamepad_device *) * (numDevices + 1));
	devices[numDevices++] = deviceRecord;
	
	return DIENUM_CONTINUE;
}

void Gamepad_detectDevices() {
	HRESULT result;
	
	if (!inited) {
		return;
	}
	
	result = IDirectInput_EnumDevices(directInputInterface, DI8DEVCLASS_GAMECTRL, enumDevicesCallback, NULL, DIEDFL_ALLDEVICES);
	if (result != DI_OK) {
		fprintf(stderr, "Warning: EnumDevices returned 0x%X\n", (unsigned int) result);
	}
}

static double currentTime() {
	static LARGE_INTEGER frequency;
	LARGE_INTEGER currentTime;
	
	if (frequency.QuadPart == 0) {
		QueryPerformanceFrequency(&frequency);
	}
	QueryPerformanceCounter(&currentTime);
	
	return (double) currentTime.QuadPart / frequency.QuadPart;
}

static void updateButtonValue(struct Gamepad_device * device, unsigned int buttonIndex, bool down, double timestamp) {
	if (down != device->buttonStates[buttonIndex]) {
		device->buttonStates[buttonIndex] = down;
		if (down && Gamepad_buttonDownCallback != NULL) {
			Gamepad_buttonDownCallback(device, buttonIndex, timestamp, Gamepad_buttonDownContext);
		} else if (!down && Gamepad_buttonUpCallback != NULL) {
			Gamepad_buttonUpCallback(device, buttonIndex, timestamp, Gamepad_buttonUpContext);
		}
	}
}

static void updateAxisValue(struct Gamepad_device * device, unsigned int axisIndex, LONG ivalue, double timestamp) {
	float value, lastValue;
	
	value = (ivalue - AXIS_MIN) / (float) (AXIS_MAX - AXIS_MIN) * 2.0f - 1.0f;
	
	lastValue = device->axisStates[axisIndex];
	device->axisStates[axisIndex] = value;
	if (value != lastValue && Gamepad_axisMoveCallback != NULL) {
		Gamepad_axisMoveCallback(device, axisIndex, value, lastValue, timestamp, Gamepad_axisMoveContext);
	}
}

#define POV_UP 0
#define POV_RIGHT 9000
#define POV_DOWN 18000
#define POV_LEFT 27000

static void povToXY(DWORD pov, float * outX, float * outY) {
	if (LOWORD(pov) == 0xFFFF) {
		*outX = *outY = 0.0f;
		
	} else {
		if (pov > POV_UP && pov < POV_DOWN) {
			*outX = 1.0f;
			
		} else if (pov > POV_DOWN) {
			*outX = -1.0f;
			
		} else {
			*outX = 0.0f;
		}
		
		if (pov > POV_LEFT || pov < POV_RIGHT) {
			*outY = -1.0f;
			
		} else if (pov > POV_RIGHT && pov < POV_LEFT) {
			*outY = 1.0f;
			
		} else {
			*outY = 0.0f;
		}
	}
}

static void updatePOVAxisValues(struct Gamepad_device * device, unsigned int axisIndex, DWORD ivalue, double timestamp) {
	float x = 0.0f, y = 0.0f, value, lastValue;
	
	povToXY(ivalue, &x, &y);
	
	value = x;
	lastValue = device->axisStates[axisIndex];
	device->axisStates[axisIndex] = value;
	if (value != lastValue && Gamepad_axisMoveCallback != NULL) {
		Gamepad_axisMoveCallback(device, axisIndex, value, lastValue, timestamp, Gamepad_axisMoveContext);
	}
	
	axisIndex++;
	value = y;
	lastValue = device->axisStates[axisIndex];
	device->axisStates[axisIndex] = value;
	if (value != lastValue && Gamepad_axisMoveCallback != NULL) {
		Gamepad_axisMoveCallback(device, axisIndex, value, lastValue, timestamp, Gamepad_axisMoveContext);
	}
}

void Gamepad_processEvents() {
	static bool inProcessEvents;
	unsigned int deviceIndex, buttonIndex, axisIndex;
	struct Gamepad_device * device;
	struct Gamepad_devicePrivate * devicePrivate;
	HRESULT result;
	
	if (!inited || inProcessEvents) {
		return;
	}
	
	inProcessEvents = true;
	for (deviceIndex = 0; deviceIndex < numDevices; deviceIndex++) {
		device = devices[deviceIndex];
		devicePrivate = device->privateData;
		
		result = IDirectInputDevice8_Poll(devicePrivate->deviceInterface);
		if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED) {
			IDirectInputDevice8_Acquire(devicePrivate->deviceInterface);
			IDirectInputDevice8_Poll(devicePrivate->deviceInterface);
		}
		
		if (devicePrivate->buffered) {
			DWORD eventCount = INPUT_QUEUE_SIZE;
			DIDEVICEOBJECTDATA events[INPUT_QUEUE_SIZE];
			unsigned int eventIndex;
			
			result = IDirectInputDevice8_GetDeviceData(devicePrivate->deviceInterface, sizeof(DIDEVICEOBJECTDATA), events, &eventCount, 0);
			if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED) {
				IDirectInputDevice8_Acquire(devicePrivate->deviceInterface);
				result = IDirectInputDevice8_GetDeviceData(devicePrivate->deviceInterface, sizeof(DIDEVICEOBJECTDATA), events, &eventCount, 0);
			}
			if (result != DI_OK) {
				if (Gamepad_deviceRemoveCallback != NULL) {
					Gamepad_deviceRemoveCallback(device, Gamepad_deviceRemoveContext);
				}
				
				disposeDevice(device);
				numDevices--;
				for (; deviceIndex < numDevices; deviceIndex++) {
					devices[deviceIndex] = devices[deviceIndex + 1];
				}
				deviceIndex--;
				continue;
			}
			
			for (eventIndex = 0; eventIndex < eventCount; eventIndex++) {
				//printf("Got event: offset = %u, data = %u\n", (unsigned int) events[eventIndex].dwOfs, (unsigned int) events[eventIndex].dwData);
				for (buttonIndex = 0; buttonIndex < device->numButtons; buttonIndex++) {
					if (events[eventIndex].dwOfs == devicePrivate->buttonOffsets[buttonIndex]) {
						updateButtonValue(device, buttonIndex, !!events[eventIndex].dwData, events[eventIndex].dwTimeStamp / 1000.0);
					}
				}
				for (axisIndex = 0; axisIndex < device->numAxes; axisIndex++) {
					if (events[eventIndex].dwOfs == devicePrivate->axisInfo[axisIndex].offset) {
						if (devicePrivate->axisInfo[axisIndex].isPOV) {
							updatePOVAxisValues(device, axisIndex, events[eventIndex].dwData, events[eventIndex].dwTimeStamp / 1000.0);
							axisIndex++;
						} else {
							updateAxisValue(device, axisIndex, events[eventIndex].dwData, events[eventIndex].dwTimeStamp / 1000.0);
						}
					}
				}
			}
			
		} else {
			DIJOYSTATE2 state;
			
			result = IDirectInputDevice8_GetDeviceState(devicePrivate->deviceInterface, sizeof(DIJOYSTATE2), &state);
			if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED) {
				IDirectInputDevice8_Acquire(devicePrivate->deviceInterface);
				result = IDirectInputDevice8_GetDeviceState(devicePrivate->deviceInterface, sizeof(DIJOYSTATE2), &state);
			}
			
			if (result != DI_OK) {
				if (Gamepad_deviceRemoveCallback != NULL) {
					Gamepad_deviceRemoveCallback(device, Gamepad_deviceRemoveContext);
				}
				
				disposeDevice(device);
				numDevices--;
				for (; deviceIndex < numDevices; deviceIndex++) {
					devices[deviceIndex] = devices[deviceIndex + 1];
				}
				deviceIndex--;
				continue;
			}
			
			for (buttonIndex = 0; buttonIndex < device->numButtons; buttonIndex++) {
				updateButtonValue(device, buttonIndex, !!state.rgbButtons[buttonIndex], currentTime());
			}
			
			for (axisIndex = 0; axisIndex < device->numAxes; axisIndex++) {
				switch (devicePrivate->axisInfo[axisIndex].offset) {
					case DIJOFS_X:
						updateAxisValue(device, axisIndex, state.lX, currentTime());
						break;
					case DIJOFS_Y:
						updateAxisValue(device, axisIndex, state.lY, currentTime());
						break;
					case DIJOFS_Z:
						updateAxisValue(device, axisIndex, state.lZ, currentTime());
						break;
					case DIJOFS_RX:
						updateAxisValue(device, axisIndex, state.lRx, currentTime());
						break;
					case DIJOFS_RY:
						updateAxisValue(device, axisIndex, state.lRy, currentTime());
						break;
					case DIJOFS_RZ:
						updateAxisValue(device, axisIndex, state.lRz, currentTime());
						break;
					case DIJOFS_SLIDER(0):
						updateAxisValue(device, axisIndex, state.rglSlider[0], currentTime());
						break;
					case DIJOFS_SLIDER(1):
						updateAxisValue(device, axisIndex, state.rglSlider[1], currentTime());
						break;
					case DIJOFS_POV(0):
						updatePOVAxisValues(device, axisIndex, state.rgdwPOV[0], currentTime());
						axisIndex++;
						break;
					case DIJOFS_POV(1):
						updatePOVAxisValues(device, axisIndex, state.rgdwPOV[1], currentTime());
						axisIndex++;
						break;
					case DIJOFS_POV(2):
						updatePOVAxisValues(device, axisIndex, state.rgdwPOV[2], currentTime());
						axisIndex++;
						break;
					case DIJOFS_POV(3):
						updatePOVAxisValues(device, axisIndex, state.rgdwPOV[3], currentTime());
						axisIndex++;
						break;
				}
			}
		}
	}
	inProcessEvents = false;
}

