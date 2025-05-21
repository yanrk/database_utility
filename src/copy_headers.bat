mkdir ..\include

xcopy mysql_connector\include ..\include /e /i /y

copy base\base.h ..\include
copy base\macros.h ..\include
copy mysql_helper\mysql_helper.hpp ..\include
