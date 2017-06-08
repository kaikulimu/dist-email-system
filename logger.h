#ifndef __LOGGER_H__
#define __LOGGER_H__
using namespace std;


class Logger {
    // The persistent layer of mail server
private:
    const string path;

public:
    /** Constructor
     *  @param path_ Path of logging directory
     */
    Logger(string);

    void append(uint server_id, MessageInterface *msg);

    void backup(const array<vector<MailInterface *>, N_SERVER> &);

};

#endif
