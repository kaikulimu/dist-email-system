#ifndef __CLIENT_OBSERVER_H__
#define __CLIENT_OBSERVER_H__

#include "observer.h"

using namespace std;


class ClientObserver : public Observer {
public:
    /**
        Notify the client about server's information
    */
    virtual void notifyServerInformation(const string info) = 0;
    
    /**
        Notify the client when Spread receives a regular message
        Not all regular messages will trigger this notification
        @param msg The received message
    */
    virtual void notifyMessage(MessageInterface * msg) = 0;

    /**
     * Notify the client when the user type in a command
     * @param command User command string
     */
    virtual void notifyUserCommand(const string command) = 0;

    virtual void notifyDisconnect() = 0;

};

#endif
