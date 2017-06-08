#include "header.h"

void exit_with(const std::string& msg){
  if (! msg.empty()) {
    perror(msg.c_str());
  }
  printf("\nBye.\n");
  exit(EXIT_FAILURE);
}
