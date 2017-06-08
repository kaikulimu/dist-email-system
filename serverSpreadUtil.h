#ifndef __SERVER_SPREAD_UTIL_H__
#define __SERVER_SPREAD_UTIL_H__

#include "spreadUtil.h"
#include "serverObserver.h"
#include <vector>

using namespace std;


/**
    Spread Utility for the server
*/
class ServerSpreadUtil : public SpreadUtil {
private:
    string self_lobby_name;
    string self_chamber_name;
    ServerObserver * observer = 0;
    
    vector<uint> partitionServers; // Servers in the same partition
    string recent_client; // The most recent client communicating to this server
        
    /**
        Determine whether the sender is me, the server myself
        @param sender The sender
        @return True if sender is myself, false else.
    */    
    bool isMyself(const string) const;

    /**
        Determine whether the sender is the server cluster Spread group
        @param sender The sender
        @return True if it's the server cluster, false else.
    */
    bool isServerCluster(const string) const;
    
    /**
        Determine whether the sender is a server or a client. Errs if it is neither.
        @param sender The sender
        @return True if it's server, false if it's client.
    */
    bool determine_server_or_client(const string) const;

    /**
     * Private constructor to ensure singleton
     */
    ServerSpreadUtil();
public:

    static ServerSpreadUtil & getInstance(){
        static ServerSpreadUtil instance;
        return instance;
    }

    void set_name(string user_name);

    void set_observer(ServerObserver * ob);

    void multicast_server_cluster(const MessageInterface * const msg);

    /**
        Reply a message to the most recent client
        @param msg The message
    */
    void reply_client(const MessageInterface * const msg);

    vector<uint> get_members() const;
    
    void attach_events() const override;

    /**
        Join the public group used for communication between servers
    */
    void join_server_cluster();

    /**
        Join the self lobby Spread group and the self chamber Spread group
    */
    void join_self_groups();

    void read_message() override;
};


/**
  Invoke server to read message from Spread and react accordingly
  @param fd Ignored
  @param passcode Passcode to verify that messages are genuine
  @param date Ignored
*/
void invoke_server_read_message(int fd, int passcode, void *data);

#endif
