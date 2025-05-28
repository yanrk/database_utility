
:: build mongo-c-driver win32 debug
H:
cd H:\test\mongo-c-driver-1.23.2
mkdir cmake-build-win32-debug
cd cmake-build-win32-debug
cmake -G "Visual Studio 15 2017" "-DENABLE_SSL=WINDOWS" "-DENABLE_SASL=SSPI" "-DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF" "-DCMAKE_BUILD_TYPE=Debug" "-DCMAKE_INSTALL_PREFIX=H:\test\mongo-c-driver-1.23.2\outs\win32_debug" ..
cmake -LH .
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin\MSBuild.exe" /p:Configuration=Debug ALL_BUILD.vcxproj
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin\MSBuild.exe" /p:Configuration=Debug INSTALL.vcxproj

echo "build mongo-c-driver win32 debug finished"
pause

:: build mongo-c-driver win32 release
H:
cd H:\test\mongo-c-driver-1.23.2
mkdir cmake-build-win32-release
cd cmake-build-win32-release
cmake -G "Visual Studio 15 2017" "-DENABLE_SSL=WINDOWS" "-DENABLE_SASL=SSPI" "-DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF" "-DCMAKE_BUILD_TYPE=Release" "-DCMAKE_INSTALL_PREFIX=H:\test\mongo-c-driver-1.23.2\outs\win32_release" ..
cmake -LH .
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin\MSBuild.exe" /p:Configuration=Release ALL_BUILD.vcxproj
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin\MSBuild.exe" /p:Configuration=Release INSTALL.vcxproj

echo "build mongo-c-driver win32 release finished"
pause

:: build mongo-c-driver win64 debug
H:
cd H:\test\mongo-c-driver-1.23.2
mkdir cmake-build-win64-debug
cd cmake-build-win64-debug
cmake -G "Visual Studio 15 2017 Win64" "-DENABLE_SSL=WINDOWS" "-DENABLE_SASL=SSPI" "-DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF" "-DCMAKE_BUILD_TYPE=Debug" "-DCMAKE_INSTALL_PREFIX=H:\test\mongo-c-driver-1.23.2\outs\win64_debug" ..
cmake -LH .
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin\MSBuild.exe" /p:Configuration=Debug ALL_BUILD.vcxproj
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin\MSBuild.exe" /p:Configuration=Debug INSTALL.vcxproj

echo "build mongo-c-driver win64 debug finished"
pause

:: build mongo-c-driver win64 release
H:
cd H:\test\mongo-c-driver-1.23.2
mkdir cmake-build-win64-release
cd cmake-build-win64-release
cmake -G "Visual Studio 15 2017 Win64" "-DENABLE_SSL=WINDOWS" "-DENABLE_SASL=SSPI" "-DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF" "-DCMAKE_BUILD_TYPE=Release" "-DCMAKE_INSTALL_PREFIX=H:\test\mongo-c-driver-1.23.2\outs\win64_release" ..
cmake -LH .
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin\MSBuild.exe" /p:Configuration=Release ALL_BUILD.vcxproj
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin\MSBuild.exe" /p:Configuration=Release INSTALL.vcxproj

echo "build mongo-c-driver win64 release finished"
pause
