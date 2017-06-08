#ifndef __SERVER_OBSERVER_H__
#define __SERVER_OBSERVER_H__

class MessageInterface;

#include "observer.h"
#include "params.h"
#include <array>
#include <vector>

using namespace std;


class ServerObserver : public Observer {
public:
    /**
        Get my server id
        @return The id
    */
    virtual uint getServerId() const = 0;

    /**
        Notify the server when a server-side join, merge or partition during merge occurs
    */
    virtual void notifyMerge() = 0;
    
    /**
        Notify the server when Spread receives a regular message
        Not all regular messages will trigger this notification
        @param msg The received message
        @param isFromServer Is the sender a server?
    */
    virtual void notifyMessage(MessageInterface * msg, const bool isFromServer) = 0;
    
    /**
        Is the server observer in the middle of merging?
        @return True if it's merging, false else.
    */
    virtual bool is_merging() = 0;
    
    /**
        Get the vector of servers in the same partition as our parent server
        @return The vector of servers
    */
    virtual vector<uint> get_partition_servers() = 0;
    
    /**
        Mulitcast my ACK matrix to all servers in my partition
        @param ACK The matrix of ACKs
    */
    virtual void multicast_ACK_matrix(const array<array<uint, N_SERVER>, N_SERVER>& ACK) = 0;
    
    /**
        Mulitcast a NACK mail to all servers in my partition
        @param server_id The server id of the NACK mail
        @param index The index of the NACK mail
    */
    virtual void multicast_NACK_mail(uint server_id, uint index) = 0;
    
    /**
        Write my ACK matrix to log
        @param ACK The matrix of ACKs
    */
    virtual void log_ACK_matrix(const array<array<uint, N_SERVER>, N_SERVER>& ACK) = 0;
    
    /**
        Recover my ACK matrix from log
        @param ACK The matrix of ACKs
    */
    virtual void recover_ACK_matrix(array<array<uint, N_SERVER>, N_SERVER>& ACK) = 0;
};

#endif
