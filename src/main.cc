#include <stdio.h>
#include <iostream>
#include "binlog_undo.h"

#include <boost/program_options.hpp>
namespace po = boost::program_options;

using namespace binary_log;

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

  printf("%s %ld > %s\n", in_path.c_str(), pos, out_path.c_str());
  BinlogUndo undo(in_fd, out_fd, max_event_size);
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

