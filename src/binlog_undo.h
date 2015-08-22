#include <vector>
#define STANDALONE_BINLOG
#include "binary_log.h"

namespace binary_log 
{
  unsigned long get_field_length(unsigned char **packet);
}

using namespace binary_log;

#define MAX_TABLE_MAP_SIZE 65536

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

//unsigned long get_field_length(unsigned char **packet);
size_t get_type_size(char type);

class Bitset
{
  char *p;
public:
  Bitset(char *ptr);
  bool get(size_t n);
};

class BinlogUndo 
{
public:
  FILE *in_fd;
  FILE *out_fd;
  size_t max_event_size;
  
  char *event_buffer;
  //char *swap_buffer;
  Format_description_event *fde = NULL;
  bool has_checksum;
  bool is_rewrite_server_id = false;
  uint32_t server_id = 0;
  size_t current_event_pos;
  size_t current_event_len;
  Log_event_header current_header;

  std::vector<Trans> transactions;

public:  
  BinlogUndo(FILE *in_fd, FILE *out_fd, size_t max_event_size);
  ~BinlogUndo();

  Result read_fde();
  Result read_event_header();
  Result read_event_body();
  Result read_event_at(size_t pos);
  Result read_event_header_at(size_t pos); 
  Result scan(size_t pos);

  Result scan_begin();
  Result scan_table_map();
  Result scan_row_or_xid();

  Result read_event_data(Event e);
  Result write_event_data(Event e);

  Result output();
  Result copy_event_data(Event e);
  void revert_row_data(Table_map_event *table_map);
  void rewrite_checksum();
  Slice calc_rows_body_slice();
  Result calc_update_data(Slice body, uint32_t *number_of_fields, Slice *slice);
  void swap_update_row(Slice data, uint32_t num_col, Table_map_event *table_map); 
  void swap(char *str, size_t first, size_t second);

  void set_server_id(uint32_t server_id);
  void rewrite_server_id();
};

#define ASSERT_BU_OK(r) if((r)!=BU_OK){return r;}


