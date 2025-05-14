mkdir ..\lib

mkdir ..\lib\mysql_connector
mkdir ..\lib\mysql_connector\lib
xcopy ..\src\mysql_connector\lib ..\lib\mysql_connector\lib /e /i /y

mkdir ..\bin
copy ..\lib\windows\dll_debug\*.dll ..\bin\windows\dll_debug
copy ..\lib\windows\dll_debug_x64\*.dll ..\bin\windows\dll_debug_x64
copy ..\lib\windows\dll_release\*.dll ..\bin\windows\dll_release
copy ..\lib\windows\dll_release_x64\*.dll ..\bin\windows\dll_release_x64
