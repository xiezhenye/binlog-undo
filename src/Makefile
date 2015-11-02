MYSQL_SRC=/root/mysql-5.7.9

BOOST_INCLUDE=/usr/include/boost148
BOOST_LIB=/usr/lib64/boost148
CPP11=c++0x
LIB_MYSQL=$(MYSQL_SRC)/libmysql
LIB_BINLOG=$(MYSQL_SRC)/libbinlogevents
INCLUDES=-I$(LIB_BINLOG)/include -I$(LIB_BINLOG)/export -I$(BOOST_INCLUDE)

LIBBINLOGSTANDALONE=$(MYSQL_SRC)/libbinlogstandalone/lib/libbinlogstandalone.a
LIBMYSQLCLIENT=$(LIB_MYSQL)/libmysqlclient.a
ARCH=$(shell uname -ms|tr '[:upper:] ' '[:lower:].')
all:binlog_undo
.PHONY : all


zip:binlog_undo.$(ARCH).zip
.PHONY : all

binlog_undo.o:binlog_undo.cc binlog_undo.h
	g++ $(INCLUDES) binlog_undo.cc -c -std=$(CPP11) -g -O2 -Wall -o binlog_undo.o

main.o:main.cc binlog_undo.h
	g++ $(INCLUDES) main.cc -c -std=c++0x -O2 -g -Wall -o main.o

binlog_undo:main.o binlog_undo.o $(LIBBINLOGSTANDALONE) $(LIBMYSQLCLIENT)
	g++ binlog_undo.o main.o \
          $(LIBBINLOGSTANDALONE) \
          $(LIBMYSQLCLIENT) \
          $(BOOST_LIB)/libboost_program_options.a \
          -o binlog_undo 

binlog_undo.$(ARCH).zip:binlog_undo ../README.md ../LICENSE
	mkdir -p binlog_undo.$(ARCH)
	cp binlog_undo ../README.md ../LICENSE binlog_undo.$(ARCH)
	zip -r binlog_undo.$(ARCH).zip binlog_undo.$(ARCH)
	rm -rf binlog_undo.$(ARCH)

clean:
	rm -rf binlog_undo binlog_undo.o main.o binlog_undo.$(ARCH) binlog_undo.$(ARCH).zip 

