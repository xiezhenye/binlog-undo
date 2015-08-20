#include <vector>
#include "binary_log.h"

using namespace binary_log;

#define MAX_EVENT_SIZE 1048576
#define MAX_TABLE_MAP_SIZE 8192

typedef struct Event {
  size_t pos;
  size_t size;
} Event;

typedef struct Trans {
  Event begin;
  std::vector<Event> rows;
  Event xid;
} Trans;

typedef struct Slice {
  char   *p;
  size_t size;
} Slice;

static char magic[]= {'\xfe', '\x62', '\x69', '\x6e', '\x00'};

typedef enum Result {
  BU_OK = 0,
  BU_EOF = 1,
  BU_END_TRANSACTION = 2,
  BU_IO_ERROR = 3,
  BU_CORRUPT_EVENT = 4,
  BU_UNEXCEPTED_EVENT_TYPE = 5,
  BU_EVENT_TOO_BIG = 6
} Result;


class BinlogUndo 
{
public:
  FILE *in_fd;
  FILE *out_fd;
  char *event_buffer;
  Format_description_event *fde = NULL;
  int max_event_size;
  bool has_checksum;
  Log_event_header current_header;
  size_t current_event_pos;
  size_t current_event_len;
  std::vector<Trans> transactions;

public:  
  BinlogUndo(FILE *in_fd, FILE *out_fd);
  ~BinlogUndo();

  Result read_fde();
  Result read_event_header();
  Result read_event_body();
  Result read_event_at(size_t pos);
  Result read_event_header_at(size_t pos); 
  Result scan(size_t pos);

  Result scan_begin();
  Result scan_table_map_or_xid();
  Result scan_row();

  Result read_event_data(Event e);
  Result write_event_data(Event e);

  Result output();
  Result copy_event_data(Event e);
  void revert_row_data(Table_map_event *table_map);
  Result write_reverted_row(size_t row_pos, Table_map_event *table_map);
  void rewrite_checksum();
  Slice calc_rows_body_slice();
  Result calc_update_data(Slice body, uint32_t *number_of_fields, Slice *slice);
  void calc_update_row(Slice data, uint32_t num_col, Table_map_event *table_map); 
};

#define ASSERT_BU_OK(r) if((r)!=BU_OK){return r;}


