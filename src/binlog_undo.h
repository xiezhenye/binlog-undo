#include <vector>
#include "binary_log.h"

using namespace binary_log;

#define MAX_EVENT_SIZE 1048576

typedef struct Event {
  size_t pos;
  size_t size;
} Event;

typedef struct Trans {
  Event begin;
  std::vector<Event> rows;
  Event xid;
} Trans;

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

  Result scan(size_t pos);

  Result scan_begin();
  Result scan_table_map_or_xid();
  Result scan_row();

  Result output();
  Result copy_event(Event e);
  Result revert_row_data();
  Result write_reverted_row(Event e);
  void rewrite_checksum();
};


