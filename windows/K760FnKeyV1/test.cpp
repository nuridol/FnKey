// test.cpp : main project file.

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <setupapi.h>

#pragma comment(lib, "setupapi")

#define RECEIVE_WAIT_MODE_NONE  0
#define RECEIVE_WAIT_MODE_WAIT  1

#define DEVICE_BUFSIZE       20
#define REMOCON_DATA_LENGTH   7

typedef void (__stdcall *DEF_HidD_GetHidGuid)(LPGUID HidGuid);

using namespace System;


// send fn key on/off report
int Transfer(HANDLE hDevice)
{
    int sts = -1;
    DWORD len;
	//const uint8_t k810_seq_fkeys_on[] = {0x10, 0xFF, 0x06, 0x15, 0x00, 0x00, 0x00};
	//const uint8_t k810_seq_fkeys_off[] = {0x10, 0xFF, 0x06, 0x15, 0x01, 0x00, 0x00};
	// use f1 with fn
	//const char unsigned data[] = {0x10, 0xff, 0x05, 0x14, 0x00, 0x00, 0x00};
	// use function key without fn
	//const char unsigned data[] = {0x10, 0xff, 0x05, 0x14, 0x01, 0x00, 0x00};

	BYTE buf[] = {0x10, 0xff, 0x05, 0x14, 0x01, 0x00, 0x00};

	if (!WriteFile(hDevice, buf, sizeof(buf), &len, NULL)) {
        fprintf(stderr, " - Fail to WriteFile(): err=%u\n", GetLastError());
        goto DONE;
    }

    sts = 0;
DONE:
    return sts;
}

// setup fn key setting
bool setupFnKey(IN char *szDevicePath) {
	HANDLE hDevice = INVALID_HANDLE_VALUE;
	bool flag = false;

	hDevice = CreateFile(szDevicePath, GENERIC_WRITE|GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (hDevice == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		fprintf(stderr, " - Fail to CreateFile(): err=%u\n", err);
		goto DONE;
	}
	if (Transfer(hDevice) == 0) {
		Console::WriteLine(L" - Standard function key setting is done.");
		flag = true;
	}
	else {
		Console::WriteLine(L" - Standard function key setting is failed.");
	}

DONE:
    if (hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(hDevice);
    }

	return flag;
}

// search devices and setup
DWORD searchDevice()
{
	// logitech K760
    char *szIdStr1 = "vid_046d&pid_b316";
    // for windows 8.1
    char *szIdStr2 = "046d_pid&b316";

	// test
	//char *szIdStr1 = "vid_04d9&pid_2011";

    int i;
    HDEVINFO DeviceInfoTable = NULL;
    SP_DEVICE_INTERFACE_DATA DeviceIfData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceIfDetailData;
    SP_DEVINFO_DATA DevInfoData;
    DWORD InterfaceIndex, dwSize, dwError = ERROR_SUCCESS;
    HGLOBAL hMem = NULL;
    GUID InterfaceClassGuid;
    HMODULE hHidDLL;
    DEF_HidD_GetHidGuid pHidD_GetHidGuid;

    //GUID InterfaceClassGuid = {0x4d1e55b2, 0xf16f, 0x11cf, 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30};
    // HIDClass の GUID を取得
    hHidDLL = LoadLibrary("hid.dll");
    pHidD_GetHidGuid = (DEF_HidD_GetHidGuid)GetProcAddress(hHidDLL, "HidD_GetHidGuid");
    pHidD_GetHidGuid(&InterfaceClassGuid);
    FreeLibrary(hHidDLL);

    // HIDClass に属するデバイス群の含まれるデバイス情報セットを取得
    DeviceInfoTable = SetupDiGetClassDevs(&InterfaceClassGuid,
                            NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	DevInfoData.cbSize = sizeof(DevInfoData);
    DeviceIfData.cbSize = sizeof(DeviceIfData);

	if(!DeviceInfoTable) {
        dwError = GetLastError();
        goto DONE;
    }

    // find devices
    for (InterfaceIndex = 0; SetupDiEnumDeviceInfo(DeviceInfoTable, InterfaceIndex, &DevInfoData); InterfaceIndex++) {
        if(SetupDiEnumDeviceInterfaces(DeviceInfoTable, NULL,
                            &InterfaceClassGuid, InterfaceIndex, &DeviceIfData)) {
            dwError = GetLastError();
            if (dwError == ERROR_NO_MORE_ITEMS) {
                goto DONE;
            }
        }
        else {
            dwError = GetLastError();
            goto DONE;
        }

		DWORD DataT;
        LPTSTR buffer = NULL;
		LPTSTR hidBuffer = NULL;
        DWORD buffersize = 0;
		DWORD hidBufferSize = 0;

		while (!SetupDiGetDeviceRegistryProperty(
            DeviceInfoTable,
            &DevInfoData,
            SPDRP_DEVICEDESC,
            &DataT,
            (PBYTE)buffer,
            buffersize,
            &buffersize))
        {
            if (GetLastError() == 
                ERROR_INSUFFICIENT_BUFFER)
            {
                // Change the buffer size.
                if (buffer) LocalFree(buffer);
                // Double the size to avoid problems on 
                // W2k MBCS systems per KB 888609. 
                buffer = (LPTSTR)LocalAlloc(LPTR,buffersize * 2);
            }
            else
            {
                // Insert error handling here.
                break;
            }
        }

        while (!SetupDiGetDeviceRegistryProperty(
            DeviceInfoTable,
            &DevInfoData,
            SPDRP_HARDWAREID,
            &DataT,
            (PBYTE)hidBuffer,
            hidBufferSize,
            &hidBufferSize))
        {
            if (GetLastError() == 
                ERROR_INSUFFICIENT_BUFFER)
            {
                // Change the buffer size.
                if (hidBuffer) LocalFree(hidBuffer);
                // Double the size to avoid problems on 
                // W2k MBCS systems per KB 888609. 
                hidBuffer = (LPTSTR)LocalAlloc(LPTR,hidBufferSize * 2);
            }
            else
            {
                // Insert error handling here.
                break;
            }
        }

		// to lowercase
        int len = strlen(hidBuffer);
		char *hidString = (char*)malloc(sizeof(char)*len);
        for (i = 0; i < (int)len; i++) {
            hidString[i] = tolower(hidBuffer[i]);
        }

		// bingo!
		if (strstr(hidString, szIdStr1) || strstr(hidString, szIdStr2))
        {
            fprintf(stderr, "New device found : %s\n", buffer);
			fprintf(stderr, " %s\n", hidBuffer);

			// set key
			SetupDiGetDeviceInterfaceDetail(DeviceInfoTable, &DeviceIfData,
													NULL, 0, &dwSize, NULL);
			hMem = GlobalAlloc(0, dwSize);
			if (!hMem) {
				dwError = GetLastError();
				fprintf(stderr, " - Fail to SetupDiGetDeviceInterfaceDetail(): err=%u\n", dwError);
				//goto DONE;
			}
			else {
				pDeviceIfDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)hMem;
				pDeviceIfDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
				if (SetupDiGetDeviceInterfaceDetail(DeviceInfoTable, &DeviceIfData,
											pDeviceIfDetailData, dwSize, NULL, NULL)) {
					fprintf(stderr, " Path: %s\n", pDeviceIfDetailData->DevicePath);
					setupFnKey(pDeviceIfDetailData->DevicePath);
					dwError = ERROR_SUCCESS;
				}
				else {
					dwError = GetLastError();
					fprintf(stderr, " - Fail to SetupDiGetDeviceInterfaceDetail(): err=%u\n", dwError);
				}

				if (hMem) {
					GlobalFree(hMem);
					hMem = NULL;
				}
			}
			Console::WriteLine(L"");
		}

        if (buffer) LocalFree(buffer);
		if (hidBuffer) LocalFree(hidBuffer);
		if (hidString) free(hidString);
	}

	DONE:
    if (hMem) {
        GlobalFree(hMem);
    }
    if (DeviceInfoTable) {
        SetupDiDestroyDeviceInfoList(DeviceInfoTable);
    }

	return dwError;
}

int main(array<System::String ^> ^args)
{
	Console::WriteLine(L"****************");
    Console::WriteLine(L" K760FnKey V1.0");
	Console::WriteLine(L"****************");
	Console::WriteLine(L"");

	 // do it
    searchDevice();
	Console::WriteLine(L"Done.");

	Sleep(3000);
    return 0;
}
