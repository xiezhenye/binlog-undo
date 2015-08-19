LIB_MYSQL=/root/mysql-5.7.8-rc/libmysql
LIB_BINLOG=/root/mysql-binlog-events-1.0.0-src/libbinlogevents

c++ -I"${LIB_BINLOG}/include" -I"${LIB_BINLOG}/export" binlog_undo.cc -c -std=c++11 -o binlog_undo.o
c++ -I"${LIB_BINLOG}/include" -I"${LIB_BINLOG}/export" main.cc -c -o main.o
c++ binlog_undo.o main.o -L"${LIB_BINLOG}/lib" "${LIB_BINLOG}/lib/libbinlogevents.a" "${LIB_MYSQL}/libmysqlclient_r.a" -o binlog_undo 

