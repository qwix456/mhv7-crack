#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <filesystem>
#include <minhook.h>
#include <fstream>
#include <Psapi.h>

unsigned char auctor[256] = {
	0x12, 0x19, 0xCE, 0xD9, 0x6C, 0x1D, 0x2F, 0x7B, 0x64, 0x84, 0x29, 0x1B,
	0x83, 0x58, 0x99, 0x71, 0xFB, 0xF4, 0xDB, 0x51, 0x71, 0x45, 0x7D, 0xA0,
	0x8A, 0xBB, 0x53, 0x09, 0x5A, 0x05, 0x8C, 0xAC, 0xB2, 0x14, 0xBF, 0x76,
	0x28, 0x9C, 0x84, 0xFE, 0x2B, 0x69, 0xC4, 0xAB, 0x1D, 0x3C, 0x9A, 0x89,
	0x34, 0x73, 0x81, 0x64, 0xA0, 0xA2, 0xF9, 0x63, 0x69, 0xFC, 0x51, 0x3A,
	0xEC, 0x47, 0xD2, 0xA3, 0xDC, 0xCB, 0x6A, 0x1F, 0x6F, 0x0F, 0xA6, 0x44,
	0x9C, 0x4A, 0xF2, 0xFD, 0xDC, 0xA9, 0xA0, 0xBB, 0x4D, 0xE6, 0x8C, 0xE9,
	0x03, 0x3B, 0x0D, 0x92, 0x73, 0x3B, 0x18, 0xF3, 0x6E, 0x09, 0xC1, 0x8F,
	0x91, 0x63, 0x4F, 0x4D, 0x4A, 0x98, 0xDC, 0xE4, 0x5A, 0xC1, 0x46, 0xA9,
	0x46, 0x8A, 0xA5, 0xD1, 0x27, 0x2F, 0x17, 0x9F, 0x51, 0x7E, 0x05, 0x63,
	0xD0, 0x6D, 0x1A, 0xFE, 0xAF, 0xEF, 0xB9, 0x89, 0x7F, 0x7F, 0x5C, 0xDD,
	0xFF, 0x56, 0xB0, 0x9D, 0xEC, 0x38, 0xD3, 0x5E, 0x41, 0x3A, 0x91, 0xA3,
	0x52, 0xDE, 0x54, 0x2E, 0xEC, 0x04, 0x30, 0x7B, 0xFE, 0xA6, 0xA7, 0x84,
	0x38, 0x2C, 0x1C, 0xF0, 0x5B, 0xB0, 0x27, 0xCD, 0xC2, 0xB3, 0x17, 0xB5,
	0x2E, 0xD6, 0xC9, 0x92, 0x33, 0x9A, 0xFC, 0x25, 0x50, 0x3A, 0xC8, 0x29,
	0xC2, 0x6B, 0x31, 0x0A, 0x2C, 0xDB, 0xE6, 0x2A, 0x4F, 0x50, 0x34, 0xDF,
	0xBD, 0x8F, 0x69, 0x6C, 0x59, 0xD4, 0x2D, 0x8D, 0xFF, 0x52, 0x24, 0xAA,
	0xEC, 0x51, 0x4B, 0x7C, 0x19, 0xDB, 0xEE, 0x65, 0xF5, 0xF3, 0x6A, 0x55,
	0xFE, 0x03, 0x4F, 0x60, 0xC3, 0x0C, 0x33, 0xDC, 0xC2, 0xE1, 0x1D, 0xBF,
	0x9A, 0xA3, 0xBF, 0x3C, 0xFD, 0xD2, 0x4B, 0x3D, 0x53, 0x26, 0x13, 0x9D,
	0x2E, 0xEF, 0xB8, 0xE1, 0x0E, 0xDA, 0x0B, 0x04, 0xF3, 0x57, 0x5E, 0xAD,
	0x89, 0x63, 0x80, 0x19
};

std::string dest_path = std::string( getenv( "LOCALAPPDATA" ) ) + "\\Septima";

using TRegQueryValueExA = LSTATUS(WINAPI*)(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
TRegQueryValueExA originalRegQueryValueExA = nullptr;

bool TerminateString(const char* moduleName, const char* targetString) {
    auto moduleHandle = GetModuleHandleA(moduleName);
    if (!moduleHandle)
        return false;

    LPSTR moduleAddress = reinterpret_cast<LPSTR>(moduleHandle);

    MODULEINFO moduleInfo;
    GetModuleInformation(GetCurrentProcess(), moduleHandle, &moduleInfo, sizeof(moduleInfo));

    size_t moduleLength = moduleInfo.SizeOfImage;
    size_t offset = std::search(moduleAddress, moduleAddress + moduleLength, targetString, targetString + strlen(targetString)) - moduleAddress;
    size_t targetLength = strlen(targetString);
	
    if (offset != moduleLength) {
        DWORD oldProtect;
        if (VirtualProtect(moduleAddress + offset, targetLength, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            memset(moduleAddress + offset, '\0', targetLength);
            VirtualProtect(moduleAddress + offset, targetLength, oldProtect, &oldProtect);
        }
        return true;
    }
    return false;
}

LSTATUS WINAPI hk_RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
	if (lpValueName && std::string(lpValueName) == "MachineGuid") {
		std::string guid_str = "a8f3ef4a-6cbe-4069-a2b1-9e787666a398";
		if (lpcbData && lpData && *lpcbData >= guid_str.size() + 1) {
			if (lpType)
				*lpType = REG_SZ;
			std::copy(guid_str.begin(), guid_str.end(), lpData);
			lpData[guid_str.size()] = '\0';
			*lpcbData = guid_str.size();
			return ERROR_SUCCESS;
		}
	}
	return originalRegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

void auctor_create() {
    if (std::filesystem::create_directory(dest_path)) {
        std::ofstream file(dest_path + "\\.auctor", std::ios::out | std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<const char*>(auctor), sizeof(auctor));
            file.close();
        }
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hinstDLL);
		auctor_create();

        if (LoadLibraryA("hackpro.dll")) {
            TerminateString("hackpro.dll", "nettik.co.uk");
            MH_Initialize();
            MH_CreateHook(&RegQueryValueExA, &hk_RegQueryValueExA, reinterpret_cast<LPVOID*>(&originalRegQueryValueExA));
            MH_EnableHook(MH_ALL_HOOKS);
        }
	}
	return TRUE;
}