/*
 * Copyright (c) 2014, Peter Thorson. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the WebSocket++ Project nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PETER THORSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef WEBSOCKETPP_COMMON_CPP11_HPP
#define WEBSOCKETPP_COMMON_CPP11_HPP

/**
 * This header sets up some constants based on the state of C++11 support
 */

// Hide clang feature detection from other compilers
#ifndef __has_feature         // Optional of course.
  #define __has_feature(x) 0  // Compatibility with non-clang compilers.
#endif
#ifndef __has_extension
  #define __has_extension __has_feature // Compatibility with pre-3.0 compilers.
#endif

// Rynnya: shiro won't ever compile if you use something less than C++17
// Microsoft for some reasons decided that __cplusplus macro isn't required to be correct and just throw it away
// You may read about this here. If you want quick answer: simply add /Zc:__cplusplus and macro will be correct
// https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
    
// This is defined to allow other WebSocket++ common headers to enable
// C++11 features when they are detected by this file rather than
// duplicating the above logic in every common header.
#define _WEBSOCKETPP_CPP11_INTERNAL_
    
// _WEBSOCKETPP_CPP11_STRICT_
//
// This define reports to WebSocket++ that 100% of the language and library
// features of C++11 are available. Using this define on a non-C++11
// compiler will result in problems.
    
// _WEBSOCKETPP_CPP11_STL_ 
//
// This define enables *most* C++11 options that were implemented early on
// by compilers. It is typically used for compilers that have many, but not
// all C++11 features. It should be safe to use on GCC 4.7-4.8 and perhaps
// earlier. 
#ifndef _WEBSOCKETPP_NOEXCEPT_TOKEN_
    #define _WEBSOCKETPP_NOEXCEPT_TOKEN_ noexcept
#endif
#ifndef _WEBSOCKETPP_CONSTEXPR_TOKEN_
    #define _WEBSOCKETPP_CONSTEXPR_TOKEN_ constexpr
#endif
#ifndef _WEBSOCKETPP_INITIALIZER_LISTS_
    #define _WEBSOCKETPP_INITIALIZER_LISTS_
#endif
#ifndef _WEBSOCKETPP_NULLPTR_TOKEN_
    #define _WEBSOCKETPP_NULLPTR_TOKEN_ nullptr
#endif
#ifndef _WEBSOCKETPP_MOVE_SEMANTICS_
    #define _WEBSOCKETPP_MOVE_SEMANTICS_
#endif
#ifndef _WEBSOCKETPP_DEFAULT_DELETE_FUNCTIONS_
    #define _WEBSOCKETPP_DEFAULT_DELETE_FUNCTIONS_
#endif
    
#ifndef __GNUC__
    // GCC as of version 4.9 (latest) does not support std::put_time yet.
    // so ignore it
    #define _WEBSOCKETPP_PUTTIME_
#endif

#endif // WEBSOCKETPP_COMMON_CPP11_HPP
