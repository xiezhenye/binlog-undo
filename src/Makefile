MYSQL_SRC=/root/mysql-5.7.8-rc

LIB_MYSQL=$(MYSQL_SRC)/libmysql
LIB_BINLOG=$(MYSQL_SRC)/libbinlogevents
INCLUDES=-I$(LIB_BINLOG)/include -I$(LIB_BINLOG)/export

LIBBINLOGSTANDALONE=$(MYSQL_SRC)/libbinlogstandalone/lib/libbinlogstandalone.a
LIBMYSQLCLIENT=$(LIB_MYSQL)/libmysqlclient.a

all:binlog_undo
.PHONY : all

binlog_undo.o:binlog_undo.cc binlog_undo.h
	g++ $(INCLUDES) binlog_undo.cc -c -std=c++11 -g -O2 -Wall -o binlog_undo.o

main.o:main.cc binlog_undo.h
	g++ $(INCLUDES) main.cc -c -std=c++11 -O2 -g -Wall -o main.o

binlog_undo:main.o binlog_undo.o $(LIBBINLOGSTANDALONE) $(LIBMYSQLCLIENT)
	g++ binlog_undo.o main.o \
          -static \
          $(LIBBINLOGSTANDALONE) \
          $(LIBMYSQLCLIENT) \
          -lboost_program_options \
          -o binlog_undo 

zip:binlog_undo
	mkdir -p binlog_undo.x86_64.linux
	cp binlog_undo ../README.md ../LICENSE binlog_undo.x86_64.linux
	zip -r binlog_undo.x86_64.linux.zip binlog_undo.x86_64.linux
	rm -rf binlog_undo.x86_64.linux

clean:
	rm -rf binlog_undo binlog_undo.o main.o binlog_undo.x86_64.linux 

