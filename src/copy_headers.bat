mkdir ..\include

mkdir ..\include\mysql_connector
mkdir ..\include\mysql_connector\include
xcopy mysql_connector\include ..\include\mysql_connector\include /e /i /y

copy base\base.h ..\include
copy base\macros.h ..\include
copy mysql_helper\mysql_helper.hpp ..\include
