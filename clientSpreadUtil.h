#ifndef __CLIENT_SPREAD_UTIL_H__
#define __CLIENT_SPREAD_UTIL_H__

#include "spreadUtil.h"
#include "clientObserver.h"

using namespace std;

/**
    Spread Utility for the client
*/
class ClientSpreadUtil : public SpreadUtil {
private:
    string server_chamber; // Chamber group for the server that the client has currently joined
    string server_lobby; // Lobby group for the server that the client has currently joined
    ClientObserver * observer = 0;

    /**
     * Private constructor using Singleton pattern
     */
    ClientSpreadUtil();
    
    /**
        Forcefully disconnect (kicked) from the server lobby
    */
    void kicked_from_lobby();
    
    /**
        Move the connection from server lobby to server chamber
        @param server_id The connected server id
    */
    void connection_from_lobby_to_chamber(const uint& server_id);


public:

    static ClientSpreadUtil & getInstance(){
        static ClientSpreadUtil instance;
        return instance;
    }


    void set_observer(ClientObserver * const ob);

    // Unicast a message to the mail server
    void unicast_server(const MessageInterface * const msg);
    
    void attach_events() const override;
    
    /**
       Connect to a new mail server
       @param server_id Server id
    */
    void connect_new_server(const uint& server_id);

    void read_message() override;

    void user_command(string);

};


/**
  Invoke server to read message from Spread and react accordingly
  @param fd Ignored
  @param passcode Passcode to verify that messages are genuine
  @param date MessageInterface pointer
*/
void invoke_client_read_message(int fd, int passcode, void *data);

/**
  Parse User command and take action
  @param fd Ignored
  @param passcode Passcode to verify that messages are genuine
  @param date Ignored
*/
void invoke_user_command(int fd, int passcode, void *data);

#endif
