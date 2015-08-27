# binlog-undo
undo data with row format binlog

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
