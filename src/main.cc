#include <stdio.h>
#include <iostream>
#include "binlog_undo.h"

#include <boost/program_options.hpp>
namespace po = boost::program_options;

using namespace binary_log;

void print_error_msg(Result result)
{
  switch (result) {
  case BU_IO_ERROR:
    fprintf(stderr, "io error\n");
    break;
  case BU_CORRUPT_EVENT:
    fprintf(stderr, "binlog event is corrupt\n");
    break;
  case BU_UNEXCEPTED_EVENT_TYPE:
    fprintf(stderr, "meet unexepted binlog event type\n");
    break;
  case BU_EVENT_TOO_BIG:
    fprintf(stderr, "binlog event too big\n");
    break;
  case BU_NOT_FULL_ROW_IMAGE:
    fprintf(stderr, "not full row image\n");
    break;
  case BU_NO_TRANSACTIONS:
    fprintf(stderr, "no transactions to undo\n");
    break;
  default:
    fprintf(stderr, "unknown error\n");
    break;
  }
}


int main(int argc, char** argv) {
  std::string in_path;
  std::string out_path;
  size_t pos;
  size_t max_event_size;
  uint32_t server_id = UINT_MAX;
  bool quiet = false;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "print help message")
    ("file,f", po::value<std::string>(&in_path)->required(), "binlog file (required)")
    ("position,p", po::value<size_t>(&pos)->required(), "position (required)")
    ("output,o", po::value<std::string>(&out_path)->required(), "output file (required)")
    ("quiet,q", po::bool_switch()->default_value(false), "be quiet")
    ("max-event-size,m", po::value<size_t>(&max_event_size)->default_value(16), "max binlog event size (MB)")
    ("server-id,s", po::value<uint32_t>(&server_id), "rewrite server id");
    ;
	
  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  } catch (po::error&) {
    std::cout << desc << std::endl;
    return 1;
  }
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }
  quiet = vm["quiet"].as<bool>();
  FILE* in_fd = fopen(in_path.c_str(), "r");
  if (!in_fd) {
    if (!quiet) {
      fprintf(stderr, "can not open input file\n");
    }
    return 2;
  }
  FILE* out_fd = fopen(out_path.c_str(), "w");
  if (!out_fd) {
    if (!quiet) {
      fprintf(stderr, "can not open output file\n");
    }
    return 3;
  }

  //printf("%s %ld > %s\n", in_path.c_str(), pos, out_path.c_str());
  BinlogUndo undo(in_fd, out_fd, max_event_size);
  undo.set_server_id(server_id);
  undo.set_quiet(quiet);
  if (!quiet) {
    printf("scanning events\n");
  }
  Result result = undo.scan(pos);
  int ret = 0;
  if (result) {
    if (!quiet) {
      print_error_msg(result);
    }
    ret = result + 10;
  } else {
    result = undo.output();
    if (result) {
      if (!quiet) {
        print_error_msg(result);
      }
      ret = result + 10;
    }
  }
  fclose(in_fd);
  fclose(out_fd);  
  return ret;
}

