cmake_minimum_required(VERSION 3.1)

include(CheckCXXSourceRuns)

check_cxx_source_runs("
#ifndef _MSC_VER
#   include <cpuid.h>
#else
#   include <intrin.h>
#endif

#include <array>

int main() {
#ifdef _MSC_VER
    std::array<int, 4> cpui;
    __cpuid(cpui.data(), 0);
    int ids = cpui[0];
#else
    std::array<unsigned int, 4> cpui;
    int ids = __get_cpuid_max(0, 0);
#endif

if (ids >= 1) {
#ifdef _MSC_VER
    __cpuidex(cpui.data(), 1, 0);
#else
    __cpuid_count(1, 0, cpui[0], cpui[1], cpui[2], cpui[3]);
#endif

    return (!(cpui[3] >> 26 & 1));
}

return 1;
}
"
SSE2_SUPPORT)

check_cxx_source_runs("
#ifndef _MSC_VER
#   include <cpuid.h>
#else
#   include <intrin.h>
#endif

#include <array>

int main() {
#ifdef _MSC_VER
    std::array<int, 4> cpui;
    __cpuid(cpui.data(), 0);
    int ids = cpui[0];
#else
    std::array<unsigned int, 4> cpui;
    int ids = __get_cpuid_max(0, 0);
#endif

if (ids >= 1) {
#ifdef _MSC_VER
    __cpuidex(cpui.data(), 1, 0);
#else
    __cpuid_count(1, 0, cpui[0], cpui[1], cpui[2], cpui[3]);
#endif

    return (!(cpui[2] >> 19 & 1));
}

return 1;
}
"
SSE4_1_SUPPORT)

check_cxx_source_runs("
#ifndef _MSC_VER
#   include <cpuid.h>
#else
#   include <intrin.h>
#endif

#include <array>

int main() {
#ifdef _MSC_VER
    std::array<int, 4> cpui;
    __cpuid(cpui.data(), 0);
    int ids = cpui[0];
#else
    std::array<unsigned int, 4> cpui;
    int ids = __get_cpuid_max(0, 0);
#endif

if (ids >= 1) {
#ifdef _MSC_VER
    __cpuidex(cpui.data(), 1, 0);
#else
    __cpuid_count(1, 0, cpui[0], cpui[1], cpui[2], cpui[3]);
#endif

    return (!(cpui[2] >> 28 & 1));
}

return 1;
}
"
AVX_SUPPORT)

check_cxx_source_runs("
#ifndef _MSC_VER
#   include <cpuid.h>
#else
#   include <intrin.h>
#endif

#include <array>

int main() {
#ifdef _MSC_VER
    std::array<int, 4> cpui;
    __cpuid(cpui.data(), 0);
    int ids = cpui[0];
#else
    std::array<unsigned int, 4> cpui;
    int ids = __get_cpuid_max(0, 0);
#endif

if (ids >= 1) {
#ifdef _MSC_VER
    __cpuidex(cpui.data(), 1, 0);
#else
    __cpuid_count(1, 0, cpui[0], cpui[1], cpui[2], cpui[3]);
#endif

    return (!(cpui[2] >> 25 & 1));
}

return 1;
}
"
AES_NI_SUPPORT)

check_cxx_source_runs("
#ifndef _MSC_VER
#   include <cpuid.h>
#else
#   include <intrin.h>
#endif

#include <array>

int main() {
#ifdef _MSC_VER
    std::array<int, 4> cpui;
    __cpuid(cpui.data(), 0);
    int ids = cpui[0];
#else
    std::array<unsigned int, 4> cpui;
    int ids = __get_cpuid_max(0, 0);
#endif

if (ids >= 7) {
#ifdef _MSC_VER
    __cpuidex(cpui.data(), 7, 0);
#else
    __cpuid_count(7, 0, cpui[0], cpui[1], cpui[2], cpui[3]);
#endif

    return (!(cpui[1] >> 5 & 1));
}

return 1;
}
"
AVX2_SUPPORT)

if (SSE2_SUPPORT)
    add_compile_definitions(_SSE2_AVAILABLE)
endif()

if (SSE4_1_SUPPORT)
    add_compile_definitions(_SSE4_1_AVAILABLE)
endif()

if (AVX_SUPPORT)
    add_compile_definitions(_AVX_AVAILABLE)
endif()

if (AVX2_SUPPORT)
    add_compile_definitions(_AVX2_AVAILABLE)
endif()

if (AES_NI_SUPPORT)
    add_compile_definitions(_AES_NI_AVAILABLE)
endif()