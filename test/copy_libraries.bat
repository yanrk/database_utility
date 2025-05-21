mkdir ..\lib
mkdir ..\lib\windows
xcopy ..\src\mysql_connector\lib\windows ..\lib\windows /e /i /y

mkdir ..\bin
mkdir ..\bin\windows
mkdir ..\bin\windows\dll_debug
mkdir ..\bin\windows\dll_debug_x64
mkdir ..\bin\windows\dll_release
mkdir ..\bin\windows\dll_release_x64
mkdir ..\bin\windows\lib_debug
mkdir ..\bin\windows\lib_debug_x64
mkdir ..\bin\windows\lib_release
mkdir ..\bin\windows\lib_release_x64
copy ..\lib\windows\dll_debug\*.dll ..\bin\windows\dll_debug
copy ..\lib\windows\dll_debug_x64\*.dll ..\bin\windows\dll_debug_x64
copy ..\lib\windows\dll_release\*.dll ..\bin\windows\dll_release
copy ..\lib\windows\dll_release_x64\*.dll ..\bin\windows\dll_release_x64
