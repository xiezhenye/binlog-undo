# binlog-undo
Undo lastest MySQL operations with row format binlog

This utility scans the binlog from specialized position to end and produces another binlog file that undos the operations in it. You can use mysqlbinlog to check and apply the produced binlog file.

Only supports row format binlog.

# build
## requirement
 * MySQL source code version >= 5.7
 * boost_program_options

Edit the Makefile, change the MYSQL_SRC to your MySQL source directory, then make.

# usage

Allowed options:

    -h [ --help ]                     print help message
    -f [ --file ] arg                 binlog file
    -p [ --position ] arg             position
    -o [ --output ] arg               output file
    -m [ --max-event-size ] arg (=16) max binlog event size (MB)
    -s [ --server-id ] arg            rewrite the specialized server id

e.g.

    ./binlog_undo -f /data/mysql/log-bin.000004 -p 3958 -o binlog.out
  
Produced binlog will be written to "binlog.out"


project home: https://github.com/xiezhenye/binlog-undo/
