MYSQL_SRC="/root/mysql-5.7.8-rc"
BINLOG_SRC="/root/mysql-binlog-events-1.0.0-src"

LIB_MYSQL="${MYSQL_SRC}/libmysql"
LIB_BINLOG="${BINLOG_SRC}/libbinlogevents"
BINDINGS="${BINLOG_SRC}/bindings"

INCLUDES="-I${LIB_BINLOG}/include -I${LIB_BINLOG}/export -I${BINDINGS}/include"

c++ ${INCLUDES} binlog_undo.cc -c -std=c++11 -o binlog_undo.o
c++ ${INCLUDES} main.cc -c -o main.o

c++ binlog_undo.o main.o \
    "${BINLOG_SRC}/lib/libmysqlstream.a" \
    "${LIB_BINLOG}/lib/libbinlogevents.a" \
    "${LIB_MYSQL}/libmysqlclient_r.a" \
    -o binlog_undo 

