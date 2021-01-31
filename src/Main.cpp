#include <array>
#include <string>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <windows.h>

HMODULE g_dinput = 0;

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
    const auto game = (uintptr_t)GetModuleHandle(0);

    const auto patch_jmp_loc = game + 0xE6E1E2; // Exits CreateShaderCache early and reports a failure (return 0)
    const auto patch_replace_loc = game + 0xE6E24E; // Stops the game from crashing because of some D3D func failure (GetCachedBlob)

    std::array<uint8_t, 3> required_bytes_jmp{ 0x0F, 0x57, 0xC0 }; // xorps xmm0, xmm0
    std::array<uint8_t, 3> required_bytes_replace{ 0x48, 0x8B, 0x0D }; // mov rcx, cs:DEADBEEF

    // Make sure the bytes match because im too lazy to add pattern scanning
    if (memcmp((void*)patch_jmp_loc, required_bytes_jmp.data(), required_bytes_jmp.size()) != 0) {
        MessageBox(0, "1. Current HITMAN3FIX incompatible with this version of the game.", "Error", 0);
        return;
    }

    if (memcmp((void*)patch_replace_loc, required_bytes_replace.data(), required_bytes_replace.size()) != 0) {
        MessageBox(0, "2. Current HITMAN3FIX incompatible with this version of the game.", "Error", 0);
        return;
    }

    std::array<uint8_t, 5> new_bytes{ 0xE9, 0x00, 0x00, 0x00, 0x00 };
    *(int32_t*)&new_bytes[1] = (int32_t)(patch_jmp_loc - (patch_replace_loc + 5));

    DWORD old_protect{ 0 };
    VirtualProtect((void*)patch_replace_loc, new_bytes.size(), PAGE_EXECUTE_READWRITE, &old_protect);

    memcpy((void*)patch_replace_loc, new_bytes.data(), new_bytes.size());

    VirtualProtect((void*)patch_replace_loc, new_bytes.size(), old_protect, &old_protect);

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