#ifndef __PARAMS_H__
#define __PARAMS_H__

#define NDEBUG

typedef unsigned int uint;
#include<string>
using namespace std;

const string SPREAD_NAME = "10600";
const uint N_SERVER = 5;            // total number of mail servers
const string SERVER_CLUSTER_NAME = "DisSys_Servers";
const string CLIENT_USER = "Cli";

const uint MAX_MESSLEN = 1400;      // maximum length of a message
const uint MAX_USERNAME_LEN = 40;   // maximum length of user name
const uint MAX_VSSETS = 10;
const uint PASSCODE = 3456543;      // unique passcode to fight intrusions

const char Delimiter = 29;
const char NewLine = 30;

#endif
