#ifndef __SPREAD_UTIL_H__
#define __SPREAD_UTIL_H__

#include "include/sp.h"
#include "params.h"
#include <string>

class MessageInterface;
void exit_with(const std::string& msg);
using namespace std;

/**
    Utility class for using Spread for network connections
*/
class SpreadUtil {
    
protected:
    string User;
    string Spread_name;
    char Private_group[32];
    mailbox Mbox;

public:
    /**
        Default constructor
    */
    SpreadUtil();
    
    /**
        Initialize the connection to Spread and its Events Interface
    */
    void init_connect();
    
    /**
        Attach Spread events that the program needs
    */
    virtual void attach_events() const = 0;
    
    /**
        Begin handling Spread events
    */
    void handle_events() const;
    
    /**
        Join a public group in Spread
        @param group Name of group
    */
    void join_group(const string group);
    
        
    /**
        Leave a public group in Spread
        @param group Name of group to join
    */
    void leave_group(const string group);
   
    /**
        Multicast a message to a public group in Spread
        @param msg Pointer to the message
        @param group The group
    */
    void multicast_message(const MessageInterface * const msg, const string);

    /**
        Read a message from Spread and react accordingly
    */
    virtual void read_message() = 0;
};

#endif
