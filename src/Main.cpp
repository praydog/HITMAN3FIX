#include <array>
#include <string>
#include <memory>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <windows.h>

#include "utility/Scan.hpp"
#include "utility/Patch.hpp"

HMODULE g_dinput = 0;
std::unique_ptr<Patch> g_patch{};

extern "C" {
    // DirectInput8Create wrapper for dinput8.dll
    __declspec(dllexport) HRESULT WINAPI direct_input8_create(HINSTANCE hinst, DWORD dw_version, const IID& riidltf, LPVOID* ppv_out, LPUNKNOWN punk_outer) {
#pragma comment(linker, "/EXPORT:DirectInput8Create=" __FUNCTION__)
        return ((decltype(DirectInput8Create)*)GetProcAddress(g_dinput, "DirectInput8Create"))(hinst, dw_version, riidltf, ppv_out, punk_outer);
    }
}

void failed() {
    MessageBox(0, "HITMAN3FIX: Unable to load the original dinput8.dll. Please report this to the developer.", "HITMAN3FIX", 0);
    ExitProcess(0);
}

void patch_shadercache() {
    const auto game = GetModuleHandle(0);
    

    // Look for "shadercache.bin"
    const auto patch_jmp_loc = utility::scan(game, "0F 57 C0 49 8B CC F3 0F ? ? ? E8"); // Exits CreateShaderCache early and reports a failure (return 0)

    // Make sure the bytes match because im too lazy to add pattern scanning
    if (!patch_jmp_loc) {
        MessageBox(0, "1. Current HITMAN3FIX incompatible with this version of the game.", "Error", 0);
        return;
    }


    const auto patch_replace_loc = utility::scan(*patch_jmp_loc, 0x200, "48 8B 0D ? ? ? ?"); // Stops the game from crashing because of some D3D func failure (GetCachedBlob)

    if (!patch_replace_loc) {
        MessageBox(0, "2. Current HITMAN3FIX incompatible with this version of the game.", "Error", 0);
        return;
    }

    std::vector<uint8_t> new_bytes{ 0xE9, 0x00, 0x00, 0x00, 0x00 };
    *(int32_t*)&new_bytes[1] = (int32_t)(*patch_jmp_loc - (*patch_replace_loc + 5));

    g_patch = Patch::create(*patch_replace_loc, new_bytes);

    MessageBeep(0);
}

void startup_thread() {
    wchar_t buffer[MAX_PATH]{ 0 };
    if (GetSystemDirectoryW(buffer, MAX_PATH) != 0) {
        // Load the original dinput8.dll
        if ((g_dinput = LoadLibraryW((std::wstring{ buffer } + L"\\dinput8.dll").c_str())) == NULL) {
            failed();
        }

        patch_shadercache();
    }
    else {
        failed();
    }
}

BOOL APIENTRY DllMain(HANDLE handle, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)startup_thread, nullptr, 0, nullptr);
    }

    return TRUE;
}