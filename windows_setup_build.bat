@echo off

for /f "tokens=*" %%a in ('where vcpkg.exe') do set VCPKG_ROOT=%%~dpa

if "%VCPKG_ROOT%"=="" (
    echo === UNABLE TO FIND vcpkg.exe ===
    echo Please make sure that you have installed vcpkg.exe, run `vcpkg integrate install` and placed the vcpkg root into your PATH
    pause
    exit
)

echo === VCPKG_ROOT is %VCPKG_ROOT% ===

REM install 64bit packages that we need
vcpkg install --triplet x64-windows "boost" "curl" "cryptopp" "zlib" "liblzma" "date" "libmysql" "libmaxminddb" "cpp-redis" "tacopie"

pushd "%~dp0"
    pushd external
        git clone "https://github.com/HowardHinnant/date"
        git clone "https://github.com/rbock/sqlpp11.git"

        pushd sqlpp11
            git apply ../sqlpp11.patch

            mkdir build
            cd build

            cmake -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_EXPORT_COMPILE_COMMANDS=true -DVCPKG_TARGET_TRIPLET=x64-windows -DVCPKG_INCLUDE_DIR=%VCPKG_ROOT%\installed\x64-windows\include\ ..
            cmake --build . --config Release --target sqlpp-mysql
            cmake --build . --config Debug --target sqlpp-mysql
        popd
    popd

    mkdir build
    pushd build
        cmake -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_EXPORT_COMPILE_COMMANDS=true ..
    popd
popd

pause