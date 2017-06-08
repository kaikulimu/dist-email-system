#include "header.h"

using namespace std;


Logger::Logger(string path_) : path(path_) {
    // Create log files if not exists
    mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

void Logger::append(uint server_id, MessageInterface *msg) {
    // Open a log file, append a message, and then close it
    string filename = path + "/" + to_string(server_id) + ".log";
    ofstream outfile;
    outfile.open(filename, ios_base::app);
    outfile << msg->type << msg->to_string() << endl;
    outfile.close();
}

void Logger::backup(const array<vector<MailInterface *>, N_SERVER>& Mails){
    for(size_t i = 0; i < N_SERVER; ++i){
        string filename = path + "/" + to_string(i) + ".log_backup";
        string newfilename = path + "/" + to_string(i) + ".log";
        ofstream outfile;
        outfile.open(filename, ios_base::out);
        for(auto m : Mails[i]){
            outfile << m->type << m->to_string() << endl;
        }
        outfile.close();
        if(0 == rename(filename.c_str(), newfilename.c_str())){
            cout << "==> Successfully saved log file " << newfilename << endl;
        }
    }
}