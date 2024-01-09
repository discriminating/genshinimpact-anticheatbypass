// credit to ibaseult for finding this exploit, i just rewrote it

#include <windows.h>
#include <psapi.h>
#include <TlHelp32.h>
#include <iostream>
#include <tchar.h>

#define STATUS_OK 0
#define STATUS_FAILED 1

typedef LONG(__stdcall* NTSUSPENDPROCESS)(IN HANDLE hProc);
typedef LONG(__stdcall* NTRESUMEPROCESS)(IN HANDLE hProc);

bool NtSuspendProc(DWORD procId) {
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);

    if (hProc == nullptr || hProc == NULL)
        return STATUS_FAILED;

    NTSUSPENDPROCESS NtSuspendProcess = (NTSUSPENDPROCESS)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtSuspendProcess");

    if (NtSuspendProcess == nullptr || NtSuspendProcess == NULL)
        return STATUS_FAILED;

    NtSuspendProcess(hProc);
    CloseHandle(hProc);

    return STATUS_OK;
}

bool NtResumeProc(DWORD procId) {
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);

    if (hProc == nullptr || hProc == NULL)
        return STATUS_FAILED;

    NTRESUMEPROCESS NtResumeProcess = (NTRESUMEPROCESS)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtResumeProcess");

    if (NtResumeProcess == nullptr || NtResumeProcess == NULL)
        return STATUS_FAILED;

    NtResumeProcess(hProc);
    CloseHandle(hProc);

    return STATUS_OK;
}

bool IsDriverLoaded(const wchar_t* driverName) {
    LPVOID buffer[1024];
    DWORD X;

    if (!(K32EnumDeviceDrivers(buffer, sizeof(buffer), &X) && X < sizeof(buffer))) {
        std::cout << "Buffer too small, increase" << std::endl;
        return STATUS_FAILED;
    }

    std::cout << "Searching for driver..." << std::endl;

    const int drivers = X / sizeof(buffer[0]);

    for (int i = 0; i < drivers; i++) {
        TCHAR driverSz[1024];

        if (!GetDeviceDriverBaseName(buffer[i], driverSz, sizeof(driverSz) / sizeof(driverSz[0]))) // If we cannot resolve the name continue
            continue;

        if (_tcsicmp(driverSz, driverName) != 0) // It is not the driver we want
            continue;

        std::cout << "Found driver (index " << i + 1 << " " << driverSz << ")" << std::endl;

        return true;
    }

    return false;
}

int main() {
    std::cout << "Hello gamers welcome to totally legit kernel level bypass v1.1.2.4.32.1.3.4" << std::endl;

    bool driverFound = false;

    while (!driverFound) {
        if (!IsDriverLoaded(L"HoYoKProtect.sys"))
            continue;
    
        std::cout << "hi" << std::endl;

        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);

        if (!Process32First(hSnap, &pe))
            return STATUS_FAILED;

        do {
            // We are now going to freeze the Genshin Impact process

            const wchar_t* procName = L"GenshinImpact.exe";
            if (_tcsicmp(pe.szExeFile, procName) != 0) // is not the process
                continue;

            NtSuspendProc(pe.th32ProcessID);
            std::cout << "Genshin Impact has been suspended, waiting for driver to crash and continuing with execution." << std::endl;

            while (IsDriverLoaded(L"HoYoKProtect.sys"))
                Sleep(200);

            std::cout << "Driver crashed, continuing execution." << std::endl;
            NtResumeProc(pe.th32ProcessID);

            break;
        } while (Process32Next(hSnap, &pe));
        
        CloseHandle(hSnap);
        driverFound = true;
    }

    system("pause");
}
