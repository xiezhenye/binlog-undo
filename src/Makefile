MYSQL_SRC=/root/mysql-5.7.8-rc

LIB_MYSQL=$(MYSQL_SRC)/libmysql
LIB_BINLOG=$(MYSQL_SRC)/libbinlogevents
INCLUDES=-I$(MYSQL_SRC)/include -I$(LIB_BINLOG)/include -I$(LIB_BINLOG)/export

all:binlog_undo
.PHONY : all

binlog_undo.o:
	g++ $(INCLUDES) binlog_undo.cc -c -std=c++11 -O2 -o binlog_undo.o

main.o:
	g++ $(INCLUDES) main.cc -c -O2 -o main.o

binlog_undo:main.o binlog_undo.o
	g++ binlog_undo.o main.o \
          "${MYSQL_SRC}/libbinlogstandalone/lib/libbinlogstandalone.a" \
          "${LIB_MYSQL}/libmysqlclient_r.a" \
          -lboost_program_options \
          -o binlog_undo 

clean:
	rm binlog_undo binlog_undo.o main.o
