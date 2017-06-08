#include "header.h"
#include "clientSpreadUtil.h"

using namespace std;


class Client : public ClientObserver {
private:
    string user_name = "";
    int server_id = -1;
    ClientSpreadUtil *spreadUtil;
    vector<CommonMail *> myMails;

public:
    Client(ClientSpreadUtil *sp);

    /**
        Log in as a user
        @param user_name_ The user name
    */
    void login(const string user_name_);

    /**
        Connect to a specific mail server.
        @param server_id Server id
    */
    void connect_server(const uint server_id);

    void request_all_mails();

    void print_all_mails();

    void view_partition();

    void read_mail(size_t idx);

    void delete_mail(size_t idx);

    void sendMail();

    void notifyServerInformation(const string info) override ;

    void notifyMessage(MessageInterface *msg) override ;
    void notifyDisconnect() override ;

    void notifyUserCommand(string cmd) override ;
};

