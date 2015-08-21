MYSQL_SRC=/root/mysql-5.7.8-rc

LIB_MYSQL=$(MYSQL_SRC)/libmysql
LIB_BINLOG=$(MYSQL_SRC)/libbinlogevents
INCLUDES=-I$(MYSQL_SRC)/include -I$(LIB_BINLOG)/include -I$(LIB_BINLOG)/export

LIBBINLOGSTANDALONE=$(MYSQL_SRC)/libbinlogstandalone/lib/libbinlogstandalone.a
LIBMYSQLCLIENT=$(LIB_MYSQL)/libmysqlclient.a

all:binlog_undo
.PHONY : all

binlog_undo.o:binlog_undo.cc binlog_undo.h
	g++ $(INCLUDES) binlog_undo.cc -c -std=c++11 -g -O2 -o binlog_undo.o

main.o:main.cc binlog_undo.h
	g++ $(INCLUDES) main.cc -c -O2 -g -o main.o

binlog_undo:main.o binlog_undo.o $(LIBBINLOGSTANDALONE) $(LIBMYSQLCLIENT)
	g++ binlog_undo.o main.o \
          $(LIBBINLOGSTANDALONE) \
          $(LIBMYSQLCLIENT) \
          -lboost_program_options \
          -o binlog_undo 

clean:
	rm binlog_undo binlog_undo.o main.o
