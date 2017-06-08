#ifndef __HEADER_H__
#define __HEADER_H__

#include "include/sp.h"
#include "params.h"
#include <iostream>
#include <vector>
#include <array>
#include <assert.h>
#include <unordered_map>
#include <stdio.h>
#include <stdlib.h> // General purpose standard libary
#include <string.h>
#include <sys/types.h> // various C types
#include <unistd.h>
#include <sys/socket.h> // Open and close a socket, which is used to receive and send messages
#include <sys/select.h> // monitor multiple file descriptors
#include <netinet/in.h> // Internet Protocol family
#include <netdb.h> /* definitions for network database operations */
#include <errno.h> /* macros for integer error numbers */
#include <iostream>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <regex>
#include <sys/stat.h>

#include "mystring.h"
#include "message.h"
#include "logger.h"
#include "spreadUtil.h"

/**
   Exit the program with EXIT_FAILURE flag and an optional error message
   @param msg The error message, or "" if none
*/
void exit_with(const std::string &msg);

#endif
