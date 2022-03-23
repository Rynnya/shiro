cmake_minimum_required(VERSION 3.10)
project(shiro C CXX)

set(CMAKE_CXX_STANDARD 17)
if (WIN32)
    include(${CMAKE_TOOLCHAIN_FILE})
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

file(GLOB_RECURSE SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hh)

if (NOT CMAKE_BUILD_TYPE)
    message(WARNING "No build type has been selected, defaulting to Release.")
    set(CMAKE_BUILD_TYPE RELEASE)
endif()

# Verify if CPU supports any crypto optimizations
include("cmake/CPUID.cmake")

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter")

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -ggdb3")

    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG=1")

    if (AVX2_SUPPORT)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
    elseif (AVX_SUPPORT)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
    endif()

    if (SSE4_1_SUPPORT)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
    endif()

    if (AES_NI_SUPPORT)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -maes")
    endif()
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-undefined")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -rdynamic")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wl,--export-dynamic")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # These should already be setup from the "windows_setup_build" script
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DNOMINMAX /UIN")
    include_directories("external/sqlpp11/include")
    include_directories("external/sqlpp11-connector-mysql/include")

    find_path(MYSQL_INCLUDE_DIR NAMES mysql.h PATH_SUFFIXES mysql)
    include_directories("${MYSQL_INCLUDE_DIR}")

    find_path(MYSQL_LIB_DIR NAMES libmysql.lib PATH_SUFFIXES lib)
    link_directories("${MYSQL_LIB_DIR}")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DNDEBUG=1 /MP /w")

    if (AVX2_SUPPORT)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
    elseif (AVX_SUPPORT)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX")
    endif()

    add_compile_definitions(_WIN32_WINNT=0x0601)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
endif()

# Threading library (Win32 threads, pthreads)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Boost
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED COMPONENTS system iostreams date_time)

# cURL
find_package(CURL REQUIRED)

# OpenSSL
find_package(OpenSSL REQUIRED)

if (NOT OPENSSL_CRYPTO_LIBRARY)
    message(FATAL_ERROR "Unable to find libcrypto provided by OpenSSL.")
endif()

# zlib
find_package(ZLIB REQUIRED)

# liblzma
find_package(LibLZMA REQUIRED)

# Hinnant Date
find_package(HinnantDate REQUIRED)

# Tcmalloc
find_package(Tcmalloc)

if (Tcmalloc_FOUND)
    set(CMAKE_LINK_LIBS ${CMAKE_LINK_LIBS} ${Tcmalloc_LIBRARIES})

    if (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-malloc -fno-builtin-calloc")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-realloc -fno-builtin-free")
    endif()
endif()

# Enable logging with streams in Loguru
# add_definitions(-DLOGURU_REDEFINE_ASSERT=1)

# Resources
file(GLOB_RECURSE RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/*)

foreach (RESOURCE ${RESOURCES})
    get_filename_component(RESOURCE_BASENAME ${RESOURCE} NAME)

    if (NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${RESOURCE_BASENAME})
        configure_file("${RESOURCE}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${RESOURCE_BASENAME}" COPYONLY)
    endif()
endforeach()

set(CMAKE_LINK_LIBS ${CMAKE_LINK_LIBS} cpp_redis tacopie)

if (WIN32)
    set(CMAKE_LINK_LIBS libmysql pdh OpenSSL::Crypto ${CMAKE_LINK_LIBS})
    
    link_directories("external/sqlpp11-connector-mysql/build/src")
    set(CMAKE_LINK_LIBS optimized "Release/sqlpp-mysql" ${CMAKE_LINK_LIBS})
    set(CMAKE_LINK_LIBS debug "Debug/sqlpp-mysql" ${CMAKE_LINK_LIBS})
elseif (APPLE)
    set(CMAKE_LINK_LIBS dl sqlpp-mysql mysqlclient crypto stdc++fs maxminddb ${CMAKE_LINK_LIBS})
elseif (UNIX) # Linux
    set(CMAKE_LINK_LIBS dl sqlpp-mysql mysqlclient crypto stdc++fs maxminddb ${CMAKE_LINK_LIBS})
endif()

include_directories(${CURL_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${HinnantDate_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${LIBLZMA_INCLUDE_DIRS})
add_executable(shiro ${SRC})
target_link_libraries(shiro general Threads::Threads ${CURL_LIBRARIES} ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${LIBLZMA_LIBRARIES} ${CMAKE_LINK_LIBS})

if (MSVC)
	file(GLOB_RECURSE _source_list *.hpp *.c* *.h*)
	foreach(_source IN ITEMS ${_source_list})
		get_filename_component(_source_path "${_source}" PATH)
		string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
		string(REPLACE "/" "\\" _group_path "${_group_path}")
		source_group("${_group_path}" FILES "${_source}")
	endforeach()
endif()