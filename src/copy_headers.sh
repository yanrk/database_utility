mkdir -p ../include

cp -rf mysql_connector/include/* ../include

cp base/base.h ../include
cp base/macros.h ../include
cp mysql_helper/mysql_helper.hpp ../include
cp sqlite_helper/sqlite_helper.h ../include
