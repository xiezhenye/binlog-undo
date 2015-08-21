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

  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "print help message")
    ("file,f", po::value<std::string>(&in_path)->required(), "binlog file")
    ("position,p", po::value<size_t>(&pos)->required(), "position")
    ("output,o", po::value<std::string>(&out_path)->required(), "output file")
    ("max-event-size,m", po::value<size_t>(&max_event_size)->default_value(16), "max binlog event size (MB)")
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

  FILE* in_fd = fopen(in_path.c_str(), "r");
  if (!in_fd) {
    return 2;
  }
  FILE* out_fd = fopen(out_path.c_str(), "w");
  if (!out_fd) {
    return 3;
  }

  //printf("%s %ld > %s\n", in_path.c_str(), pos, out_path.c_str());
  BinlogUndo undo(in_fd, out_fd, max_event_size);
  Result ret = undo.scan(pos);
  if (ret) {
    print_error_msg(ret);
    return ret + 10;
  }
  ret = undo.output();
  if (ret) {
    print_error_msg(ret);
    return ret + 10;
  }

  fclose(in_fd);
  fclose(out_fd);  
  return ret;
}

