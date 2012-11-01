/**
 * get-device-parent 
 * =====================
 *
 *
 * LICENSE (MIT):
 * ==============
 *
 * Copyright (C) 2012 Marcin Kielar
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 *
 * DESCRIPTION:
 * ============
 *
 * This program finds Device Instance ID of a parent of a Device given by another Device Instance ID.
 * The main reason to create this program was to be able to obtain "bus relation" of Win32_PnPEntity objects 
 * connected to Win32_UsbController, because plain WMI cannot do that. See below, and read docs for the `main` function 
 * for details.
 *
 * Example:
 * --------
 *
 * Say one is observing - using WMI - (dis)connection of USB Drives.
 * One can listen to Win32_LogicalDisk events using WMI, and from that, one can go as far as Win32_PnpEntity of type "USBSTOR".
 * Such device has Device Instance ID = "USBSTOR\DISK&VEN_GENERIC&PROD_STORAGE_DEVICE&REV_0207\000000000207&0" (or similar).
 * 
 * But, there is another Win32_PnPEntity under the Win32_UsbController object, and it's Device Instance ID is:
 *
 *     USB\VID_05E3&PID_0727\000000000207
 *
 * which is what one needs if one is to get numerical values of Vendor ID, Product ID and Serial number.
 * 
 * To make things harder, sometimes the tree is deeper than two elements - if a USB Drive is available 
 * via USB Composite Device (like a SD Card inside a Smartphone).
 *
 * Unfortunetly, WMI does not have any binging or relation between the two, and one needs to fallback to C code, to get the relation.
 * This program does exactly what WMI cannot.
 *
 *
 * BUILD INSTRUCTION:
 * ==================
 *
 * This program compiled without problems with "Visual Studio 2012 Express Edition for Desktop" and "Windows Driver Kit 7600.16385.1" installed.
 *
 * Dependencies:
 * -------------
 *
 *   setupapi.lib  - Installed with Driver Developer Kit
 *   cfgmgr32.lib  - Installed with Driver Developer Kit
 *
 */

#pragma comment (lib, "setupapi.lib")
#pragma comment (lib, "cfgmgr32.lib")

#include <windows.h>  

#include <setupapi.h> 
#include <cfgmgr32.h>

#include <regex>

#include <stdio.h>    

#define ERR_BAD_ARGUMENTS 1;
#define ERR_NO_DEVICES_FOUND 2;
#define ERR_NO_DEVICE_INFO 3;

/**
 * Finds a parent Device Instance ID for given hCurrentDeviceInstanceId and returns it's value (as string) and handle.
 * 
 * @param [out] szParentDeviceInstanceId 
 *			pointer to memory location where parent Device Instance ID value will be stored
 * @param [out] hParentDeviceInstanceId
 *          pointer to memory location where parent Device Instance ID handle will be stored
 * @param [in] hCurrentDeviceInstanceId
 *          reference to Device Instance ID of the device, whose parent is to be found
 *
 * @return TRUE if the parent was found, otherwise FALSE
 */
BOOL GetParentDeviceInstanceId(_Out_ PWCHAR pszParentDeviceInstanceId, _Out_ PDEVINST phParentDeviceInstanceId, _In_ DEVINST hCurrentDeviceInstanceId) {
	
	// Handle to parent Device Instance ID
	BOOL result = CM_Get_Parent(phParentDeviceInstanceId, hCurrentDeviceInstanceId, 0);
	if (result == CR_SUCCESS) {
				
		// Device ID as String
		memset(pszParentDeviceInstanceId, 0, MAX_DEVICE_ID_LEN);
		result = CM_Get_Device_ID(*phParentDeviceInstanceId, pszParentDeviceInstanceId, MAX_DEVICE_ID_LEN, 0);
		if (result  == CR_SUCCESS) {
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Tests, whether given Device Instance ID matches given pattern.
 * 
 * @param pszDeviceInstanceId 
 *          pointer to Device Instance ID value
 * @param pszPattern
 *          pointer to pattern string
 *
 * @return TRUE if the instance id matches pattern, otherwise FALSE 
 */
BOOL DeviceIdMatchesPattern(_In_ PWCHAR pszDeviceInstanceId, _In_ PWCHAR pszPattern) {

	std::wstring sParentDeviceInstanceId(pszDeviceInstanceId);
	std::wregex rRegex(pszPattern);
	
	return std::regex_match(sParentDeviceInstanceId, rRegex);

}

/**
 * Prints program usage.
 */
void ShowHelp(PWCHAR pszProgramName) {

	wprintf(L"Usage:\n\n");

	wprintf(L"\t%s DII PATTERN\n\n", pszProgramName);

	wprintf(L"Arguments:\n\n");

	wprintf(L"\tDII     - Device Instance ID of the Device whose parent is to be found\n");
	wprintf(L"\tPATTERN - Regular expression to match Parent's Device Instance ID\n");
    wprintf(L"\n");

	wprintf(L"Examples:\n\n");

	wprintf(L"Example 1. Get immediate parent:\n\n");
	wprintf(L"\t%s \"%s\" \"%s\"\n\n", pszProgramName, L"USBSTOR\\DISK&VEN_GENERIC&PROD_STORAGE_DEVICE&REV_0207\\000000000207&0", L".*");
	wprintf(L"In this case the \".*\" will cause first found parent to be returned.\n");
	wprintf(L"\n");
	
	wprintf(L"Example 2. Get usb hub the device is connected to:\n\n");
	wprintf(L"\t%s \"%s\" \"%s\"\n\n", pszProgramName, L"USBSTOR\\DISK&VEN_GENERIC&PROD_STORAGE_DEVICE&REV_0207\\000000000207&0", L".*\\\\ROOT_HUB.*");
	wprintf(L"The program will search \"up\" the device tree until it finds a parent with a matching Device Instance ID.\n");
	wprintf(L"\n");

}

/**
 * Returns executable file name from full path.
 * 
 * @param pszExecutablePath
 *          pointer to full path of executable
 *
 * @return pointer to executable name
 */
PWCHAR GetExecutableName(PWCHAR pszExecutablePath) {
	
	PWCHAR pszExecutableName = (PWCHAR) pszExecutablePath + lstrlen(pszExecutablePath);

    for (; pszExecutableName > pszExecutablePath; pszExecutableName--) {
    
        if ((*pszExecutableName == '\\') || (*pszExecutableName == '/'))
        {
            pszExecutableName++;
            break;
        }
    }

	return pszExecutableName;
}

/**
 * get-device-parent entry point.
 *
 * Program takes two arguments:
 *
 * 1. pszSearchedDeviceInstanceId - This is Device Instance ID of a device whose parent is to be found.
 *                    This should be in a form returned by WMI, e.g. "USBSTOR\DISK&VEN_GENERIC&PROD_STORAGE_DEVICE&REV_0207\000000000207&0".
 *                    The value is used as second argument (i.e. the Enumerator) for the SetupDiGetClassDevs - see WDK for details.
 *
 * 2. pszParentDeviceInstanceIdPattern - Regular expression handled by <regex> library, to match the structure of the Parent Device Instance ID.
 *                    The program searches up the parent tree until it finds a parent, whose Device Instance ID matches given pattern.
 *                    If there is no Parent with matching Device Instance ID, nothing is returned.
 *
 * Error handling is quite minimalistic - no exceptions are caught and "bad" return values just cause the program to exit witho no output.
 *
 */
int main(void) {

	// Input arguments as WCHAR - not using main function arguments, as they are "hardly" convertable to PWCHAR
	int argc;
	PWCHAR *argv = CommandLineToArgvW(GetCommandLine(), &argc);

	// Check argument count
	if (argc != 3) {
		ShowHelp(GetExecutableName(argv[0]));
		return ERR_BAD_ARGUMENTS;
	}

	// Device ID - e.g.: "USBSTOR\DISK&VEN_GENERIC&PROD_STORAGE_DEVICE&REV_0207\000000000207&0"
	PWCHAR pszSearchedDeviceInstanceId = argv[1];
	
	// Pattern to match parent's Device Instance ID - w.g. "USB\\VID_(\w+)&PID_(\w+)\\(?!.*[&_].*)(\w+)"
	PWCHAR pszParentDeviceInstanceIdPattern = argv[2];

	// Get matching devices info
	HDEVINFO devInfo = SetupDiGetClassDevs(NULL, pszSearchedDeviceInstanceId, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	
	// Device Instance ID as string
	WCHAR szDeviceInstanceId[MAX_DEVICE_ID_LEN];

	if (devInfo != INVALID_HANDLE_VALUE) {

		DWORD devIndex = 0;
		SP_DEVINFO_DATA devInfoData;
		devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		// Loop over devices found in SetupDiGetClassDevs
		while (SetupDiEnumDeviceInfo(devInfo, devIndex, &devInfoData)) {
			
			// Read Device Instance ID of current device
			memset(szDeviceInstanceId, 0, MAX_DEVICE_ID_LEN);
			SetupDiGetDeviceInstanceId(devInfo, &devInfoData, szDeviceInstanceId, MAX_PATH, 0);

			// Case insensitive comparison (because Device Instance IDs can vary?)
			if (lstrcmpi(pszSearchedDeviceInstanceId, szDeviceInstanceId) == 0) {
				
				// Handle of current defice instance id
				DEVINST hCurrentDeviceInstanceId = devInfoData.DevInst;

				// Handle of parent Device Instance ID
				DEVINST hParentDeviceInstanceId;

				// Parent Device Instance ID as string
				WCHAR pszParentDeviceInstanceId[MAX_DEVICE_ID_LEN];
	
				// Search "up" parent tree until a parent with matching Device Instance ID is found.
				while (true) {

					// Initialize / clean variables
					memset(szDeviceInstanceId, 0, MAX_DEVICE_ID_LEN);
					hParentDeviceInstanceId = NULL;
			
					if (GetParentDeviceInstanceId(pszParentDeviceInstanceId, &hParentDeviceInstanceId, hCurrentDeviceInstanceId)) {

						if (DeviceIdMatchesPattern(pszParentDeviceInstanceId, pszParentDeviceInstanceIdPattern)) {
							
							// Parent Device Instance ID matches given regexp - print it out and exit
							wprintf(L"%s\n", pszParentDeviceInstanceId);
							exit(0);
							
						}

						// Parent Device Instance ID does not match the pattern - check parent's parent
						hCurrentDeviceInstanceId = hParentDeviceInstanceId;

					} else {

						// There is no parent. Stop the loop.
						break;

					}
				}
			}

			devIndex++;
		}

		if (devIndex == 0)
		{
			return ERR_NO_DEVICES_FOUND;
		}
 		
	} else {
		return ERR_NO_DEVICE_INFO;
	}

	return 0;
}
