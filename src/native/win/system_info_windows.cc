/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
 * Copyright (C) 2021-2022 Rynnya
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#if defined(_WIN32)

#include <windows.h>
#include <intrin.h>
#include <cstring>

#include "../system_info.hh"

std::string shiro::native::system_info::get_architecture() {
    bool is_64bit = []() {
        int cpu_info[4];
        __cpuid(cpu_info, 0);
        return (cpu_info[3] & 0x20000000) || false;
    }();

    return is_64bit ? "x86_64" : "x86";
}

std::string shiro::native::system_info::get_host_name() {
    char buffer[16300];
    unsigned long char_count = 16300;

    if (!GetComputerName(buffer, &char_count)) {
        return "";
    }

    return buffer;
}

std::string shiro::native::system_info::get_hw_model() {
    return "";
}

// Original code was taken from https://stackoverflow.com/a/36545162
// Modified, so we need only 1 call of 'get_rtl_os_version' to has all data required
// Will run perfectly on any system that modern than Windows 2000 (as W2000 is NT 5.0, in this version RtlGetVersion was created)
// The reason why using this instead of 'GetVersionEx' is because Microsoft broke this function in Windows 8.1
// Read more here: https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getversionexa
namespace detail {

    // https://docs.microsoft.com/en-us/windows/win32/devnotes/rtlgetversion
    typedef LONG NTSTATUS, * PNTSTATUS;
    typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

    static RTL_OSVERSIONINFOW instance = { 0 };

    RTL_OSVERSIONINFOW get_rtl_os_version() {
        if (instance.dwOSVersionInfoSize == 0) {
            instance.dwOSVersionInfoSize = sizeof(instance);
            HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");

            if (hMod) {
                RtlGetVersionPtr fxPtr = reinterpret_cast<RtlGetVersionPtr>(::GetProcAddress(hMod, "RtlGetVersion"));
                if (fxPtr != nullptr && fxPtr(&instance) == 0x00000000) {
                    return instance;
                }
            }
        }
        
        return instance;
    }
}

std::string shiro::native::system_info::get_os_version() {
    auto info = detail::get_rtl_os_version();
    return std::to_string(info.dwMajorVersion) + "." + std::to_string(info.dwMinorVersion) + "." + std::to_string(info.dwBuildNumber);
}

std::string shiro::native::system_info::get_os_build() {
    auto info = detail::get_rtl_os_version();
    return std::to_string(info.dwBuildNumber);
}

#endif
