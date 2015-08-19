#include <vector>
#include <iostream>
#include <stdio.h>
#include <string.h>
//#include <endian.h>

#include "binary_log.h"
#include "binlog_undo.h"
using namespace binary_log;

BinlogUndo::BinlogUndo(FILE *in_fd, FILE *out_fd):
  in_fd(in_fd),out_fd(out_fd),
  max_event_size(MAX_EVENT_SIZE),
  has_checksum(false),
  current_event_pos(0),
  current_event_len(0)
{
  event_buffer = new char[max_event_size];
  fde = new Format_description_event(3, "");
} 

BinlogUndo::~BinlogUndo()
{
  delete event_buffer;
  delete fde;
}

Result BinlogUndo::read_event_header()
{
  int n = fread(event_buffer, sizeof(char), LOG_EVENT_HEADER_LEN, in_fd);
  if (n != LOG_EVENT_HEADER_LEN) {
    if (n == 0 && feof(in_fd)) {
      return BU_EOF;
    } 
    return BU_IO_ERROR;
  }
  new (&current_header)Log_event_header(event_buffer, BINLOG_VERSION);
  if (current_header.log_pos - current_header.data_written != current_event_pos) {
    return BU_CORRUPT_EVENT; 
  }
  current_event_len = current_header.data_written;
  if (has_checksum) {
    current_event_len-= BINLOG_CHECKSUM_LEN;
  }
  //printf("%d %lu %lld\n", current_header.type_code, current_header.data_written, current_header.log_pos);
  return BU_OK;
}

Result BinlogUndo::read_fde()
{
  fseek(in_fd, BIN_LOG_HEADER_SIZE, SEEK_SET); 
  read_event_header();
  if (current_header.type_code != FORMAT_DESCRIPTION_EVENT) {
    return BU_UNEXCEPTED_EVENT_TYPE;
  } 
  //printf("%d %lu %lld\n", current_header.type_code, current_header.data_written, current_header.log_pos);
  Result result = read_event_body();
  if (result != BU_OK) {
    return result;
  }
  Format_description_event *tmp = fde;
  fde = new Format_description_event(event_buffer, current_header.data_written, tmp);
  delete tmp;
  fde->footer()->checksum_alg = fde->footer()->get_checksum_alg(event_buffer, current_header.data_written);
  has_checksum = (fde->footer()->checksum_alg == BINLOG_CHECKSUM_ALG_CRC32);
  return BU_OK;
}

Result BinlogUndo::read_event_body()
{
  if (current_header.data_written > MAX_EVENT_SIZE) {
    return BU_EVENT_TOO_BIG;
  }
  int rest = current_header.data_written - LOG_EVENT_HEADER_LEN; 
  int n = fread(event_buffer + LOG_EVENT_HEADER_LEN, sizeof(char), rest, in_fd);
  if (n != rest) {
    return BU_IO_ERROR;
  }
  current_event_pos = current_header.log_pos;
  return BU_OK;
}

Result BinlogUndo::scan_begin()
{
  Result result;
  result = read_event_header();
  if (result != BU_OK) {
    return result;
  }
  if (current_header.type_code != QUERY_EVENT || current_header.data_written > 100) { // much bigger than begin event(79)
    return BU_UNEXCEPTED_EVENT_TYPE;
  }
  result = read_event_body();
  if (result != BU_OK) {
    return result;
  }
  Query_event begin(event_buffer, current_event_len, fde, QUERY_EVENT);
  if (begin.q_len != 5 || memcmp(begin.query, "BEGIN", 5)) { 
    //printf("%ld %s\n", begin.q_len, begin.query);
    return BU_UNEXCEPTED_EVENT_TYPE;
  }
  //begin.print_event_info(std::cout);
  //printf("BEGIN\n");
  transactions.resize(transactions.size()+1);
  transactions.back().begin = {
    current_header.log_pos - current_header.data_written,
    current_header.data_written
  };
  return BU_OK;
}

Result BinlogUndo::scan_table_map_or_xid()
{
  Result result = read_event_header();
  if (result != BU_OK) {
    return result;
  }
  if (current_header.type_code == TABLE_MAP_EVENT) {
    result = BU_OK;
    //printf("TABLE_MAP\n");
    transactions.back().rows.push_back({
      current_event_pos,
      current_header.data_written
    });
  } else if (current_header.type_code == XID_EVENT) {
    result = BU_END_TRANSACTION;
    transactions.back().xid = {
      current_event_pos,
      current_header.data_written
    };
    //printf("XID\n");
  } else {
    return BU_UNEXCEPTED_EVENT_TYPE;
  }
  current_event_pos = current_header.log_pos;
  fseek(in_fd, current_event_pos, SEEK_SET); 
  return result;
}

Result BinlogUndo::scan_row()
{
  Result result = read_event_header();
  if (result != BU_OK) {
    return result;
  }
  switch (current_header.type_code) {
  case WRITE_ROWS_EVENT:
  case UPDATE_ROWS_EVENT:
  case DELETE_ROWS_EVENT:
    //printf("ROW\n");
    break;
  default:
    return BU_UNEXCEPTED_EVENT_TYPE;
  }
  current_event_pos = current_header.log_pos;
  fseek(in_fd, current_event_pos, SEEK_SET); 
  return BU_OK; 
}

Result BinlogUndo::scan(size_t pos)
{
  Result result;
  result = read_fde();
  if (result != BU_OK) {
    return result;
  }
  //fde->print_event_info(std::cout);
  //printf("\n");
  current_event_pos = pos;
  fseek(in_fd, pos, SEEK_SET);
  
  while (true) {
    result = scan_begin();
    if (result != BU_OK) {
      if (result == BU_EOF) {
        break;
      }
      return result;
    }
    while (true) {
      result = scan_table_map_or_xid();
      if (result == BU_END_TRANSACTION) {
        break;
      } 
      if (result != BU_OK) {
        return result;
      }
      result = scan_row();
      if (result != BU_OK) {
        return result;
      }
    } // rows
  } // transactions
  /*
  for (std::vector<Trans>::iterator trans_itr = transactions.begin(); 
       trans_itr != transactions.end();
       ++trans_itr) {
    printf("begin: %ld\n", trans_itr->begin);
    for (std::vector<size_t>::iterator row_itr = trans_itr->rows.begin();
         row_itr != trans_itr->rows.end();
         ++row_itr) {
      printf("row_tm: %ld\n", *row_itr);
    }
    printf("xid: %ld\n", trans_itr->xid);
  }
  */
  return BU_OK;
}

Result BinlogUndo::output()
{
  //ftruncate(fd_out, 0); 
  Result result;
  fwrite(magic, sizeof(char), BIN_LOG_HEADER_SIZE, out_fd);
  result = copy_event({
    BIN_LOG_HEADER_SIZE, fde->header()->data_written
  });
  if (result != BU_OK) {
    return result;
  }
  for (std::vector<Trans>::reverse_iterator trans_itr = transactions.rbegin();
       trans_itr != transactions.rend();
       ++trans_itr) {
    printf("begin: %ld %ld\n", trans_itr->begin.pos, trans_itr->begin.size);
    result = copy_event(trans_itr->begin);
    if (result != BU_OK) {
      return result;
    }
    for (std::vector<Event>::reverse_iterator row_itr = trans_itr->rows.rbegin();
         row_itr != trans_itr->rows.rend();
         ++row_itr) {
      printf("row_tm: %ld %ld\n", row_itr->pos, row_itr->size);
      result = copy_event(*row_itr);
      if (result != BU_OK) {
        return result;
      }
      result = write_reverted_row(*row_itr);
      if (result != BU_OK) {
        return result;
      }
    }
    printf("xid: %ld %ld\n", trans_itr->xid.pos, trans_itr->xid.size);
    result = copy_event(trans_itr->xid);
    if (result != BU_OK) {
      return result;
    }

  }
  return BU_OK;
}

Result BinlogUndo::copy_event(Event e)
{
  fseek(in_fd, e.pos, SEEK_SET);
  int n = fread(event_buffer, sizeof(char), e.size, in_fd);
  if (n != e.size) {
    return BU_IO_ERROR;
  }
  n = fwrite(event_buffer, sizeof(char), e.size, out_fd);
  if (n != e.size) {
    return BU_IO_ERROR;
  }
  return BU_OK;
}

/**
 * Event e is the Table_map event before the row event
 */
Result BinlogUndo::write_reverted_row(Event e)
{
  Result result;
  current_event_pos = e.pos + e.size;
  fseek(in_fd, current_event_pos, SEEK_SET);
  result = read_event_header();
  if (result != BU_OK) {
    return result;
  }
  result = read_event_body();
  result = revert_row_data();

  int n = fwrite(event_buffer, sizeof(char), current_header.data_written, out_fd);
  if (n != current_header.data_written) {
    return BU_IO_ERROR;
  }
  return BU_OK;
}

Result BinlogUndo::revert_row_data()
{
  if (current_header.type_code == WRITE_ROWS_EVENT) {
    event_buffer[EVENT_TYPE_OFFSET] = DELETE_ROWS_EVENT;
    rewrite_checksum(); 
  }

  if (current_header.type_code == DELETE_ROWS_EVENT) {
    event_buffer[EVENT_TYPE_OFFSET] = WRITE_ROWS_EVENT;
    rewrite_checksum(); 
  }
  return BU_OK;
}

void BinlogUndo::rewrite_checksum() 
{
  uint32_t checksum;
  checksum = checksum_crc32(0L, NULL, 0);
  checksum = checksum_crc32(checksum,
                       (const unsigned char*) event_buffer,
                       current_header.data_written - BINLOG_CHECKSUM_LEN);
  checksum = htole32(checksum);  
  *(uint32_t*)(&event_buffer[current_header.data_written - BINLOG_CHECKSUM_LEN]) = checksum;
}

