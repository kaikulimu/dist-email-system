#include "mystring.h"
#include <regex>
#include <fstream>

using namespace std;


vector<string> split(string input, char dem) {
    stringstream ss(input);
    string word;
    vector<string> result;
    while (getline(ss, word, dem)) {
        result.push_back(word);
    }
    return result;
}

vector<string> split(string input) {
    // Split string with blank chars
    stringstream ss(input);
    string word;
    vector<string> result;
    while(ss >> word){
        result.push_back(word);
    }
    return result;
}


string readfile(string filename) {
// Return the content of the file as a string
    ifstream ifs(filename);
    string content((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
    return content;
}


uint extract1(string ptn, string s){
    auto pattern = regex(ptn);
    smatch matchs;
    if(regex_match(s, matchs, pattern)){
        return stoul(matchs[1]);
    }else{
        throw runtime_error("Failed to match |" + s + "| with pattern |" + ptn + "|");
    }
}

uint get_server_face_id(string s){
    return extract1(".*Server_(\\d+).*", s);
}

bool is_server(string s){
    size_t found = s.find("Server_");
    return found != std::string::npos;
}

uint get_client_face_id(string s){
    return extract1(".*Cli(\\d+).*", s);
}

bool is_client(string s){
    size_t found = s.find("Cli");
    return found != std::string::npos;
}

uint get_lobby_face_id(string s) {
    return extract1(".*Lobby_(\\d+).*", s);
}

bool is_lobby(string s){
    size_t found = s.find("Lobby_");
    return found != std::string::npos;
}

