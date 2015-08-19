#include <stdio.h>
#include <string.h>
#include <iostream>

#include "binlog_undo.h"

using namespace binary_log;

#define MAX_EVENT_SIZE 1048576

int main(int argc, char** argv) {
  if (argc < 2) {
    return 1;
  }
  long pos;
  pos = atol(argv[2]);
  if (pos <= 0) {
    return 1;
  }
  FILE* in_fd = fopen(argv[1], "r");
  if (!in_fd) {
    return 2;
  }
  FILE* out_fd = fopen(argv[3], "w");
  if (!out_fd) {
    return 3;
  }

  printf("%s %ld > %s\n", argv[1], pos, argv[3]);
  BinlogUndo undo(in_fd, out_fd);
  int ret = undo.scan(pos);
  if (ret) {
    return ret + 10;
  }
  ret = undo.output();
  if (ret) {
    return ret + 10;
  }

  fclose(in_fd);
  
  return ret;
}

