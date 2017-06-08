#include <string>
#include <vector>
#include <sstream>

using namespace std;

vector<string> split(string, char);

vector<string> split(string);
string readfile(string);

uint extract1(string ptn, string s);

uint get_server_face_id(string s);
uint get_client_face_id(string s);
uint get_lobby_face_id(string s);

bool is_server(string s);
bool is_lobby(string s);
bool is_client(string s);