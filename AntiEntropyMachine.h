#ifndef __ANTI_ENTROPY_MACHINE_H__
#define __ANTI_ENTROPY_MACHINE_H__

class Server;

#include "serverObserver.h"
#include "message.h"
#include <vector>
#include <array>
#include <bitset>

enum AntiEntropyState {
    REST,
    PREPARE,
    SELF_KNOWLEDGE_EXCHANGE,
    KNOWLEDGE_AWAIT,
    INFORMER_NARRATION,
    RECONCILE_MESSAGES
};

/**
    State machine for carrying out Anti-Entropy Method between servers
*/
class AntiEntropyMachine {
private:
    ServerObserver * const parentServer;
    const uint my_server_id;

    array<array<uint, N_SERVER>, N_SERVER> ACK = {}; // The matrix of ACKs
    bitset<N_SERVER> acknowledged_servers; // Servers that I have acknowledged during KNOWLEDGE_EXCHANGE or that are not in the same partition
    array<uint, N_SERVER> min_knowledge; // Minimum knowledge ACK about each server's messages amongst servers in my partition
    array<uint, N_SERVER> max_knowledge; // Maximum knowledge ACK about each server's messages amongst servers in my partition
    array<uint, N_SERVER> informer; // Informer of each server's messages amongst servers in my partition; 0 if none
    array<uint, N_SERVER> informer_ACK; // ACK of the informer of each server's messages amongst servers in my partition
    bool merging; // Is the parent server in the middle of merging
    bool pre_reconcile; // Is the current situation before message reconciliation has started?
    AntiEntropyState state; // The machine's current state

    /**
        Is the parent server isolated from other servers in the cluster?
        @return True if yes, false else
    */
    bool is_isolated() const;
    
    /**
        Check if target server is in the same partition as our parent server
        @param server_id Target server id
        @return True if same partition, false else.
    */
    bool in_same_partition(uint server_id) const;
    
    /**
        Print out my ACK matrix to stdout
    */
    void print_ACK_matrix() const;
     
    /**
        Right before Knowledge Exchange, assume that I am the informer and my ACKs are both the min knowledge and the max knowledge
    */
    void assume_myself_informer_min_max_knowledge();
    
    /**
        During Knowledge Exchange, process knowledge of the received exchange server
        @param xchg_server_id   Exchange server id
        @param exchange_ACK     Exchange server's ACK matrix 
    */
    void processKnowledgeExchange(const uint xchg_server_id, const array<array<uint, N_SERVER>, N_SERVER> exchange_ACK);
    
    /**
        After processing knowledge of exchange server, verify that my ACK matrix is still integrous
        It uses assertion that can potentially crash the program
    */
    void verify_ACK_matrix_integrity();
    
    /**
        After knowledge exchange, given that I am the informer of server X's messages, I will multicast every message
        by X in the range from min_X_ACK + 1 to max_X_ACK
        @param X The server whose messages are to be informed
        @param min_X_ACK Minimum knowledge ACK about server X's messages
        @param max_X_ACK Maximum knowledge ACK about server X's messages
    */
    void informer_narrative(uint X, uint min_X_ACK, uint max_X_ACK);

    
public:
    /**
        Constrcutor
        @param server_id Id of the parent server
        @param mcast_ACK_matrix Function to mulitcast my ACK matrix through parent server
    */
    AntiEntropyMachine(ServerObserver * const parent);
     
    /**
        Is this server in the middle of merging?
        @return True if yes, false else
    */
    bool is_merging() const;
    
    /**
        Get the minimum ACK of each server's messages amongst all five servers
    */
    array<uint, N_SERVER> get_min_ACKs() const;

    uint get_my_ACK_of(uint server_id) const;
    
    /**
        Notification of a server-side join, merge or partition during merge
    */
    void notifyMerge();
    
    /**
        Notification of the parent server sending a message
        @param msg The message
    */
    void notify_send(MailInterface* msg);
    
    /**
        Notification of the parent server receiving a MailInterface type message
        @param msg The message
    */
    void notify_receive_mail(MailInterface* msg);
             
    /**
        Clean up after myself after Anti-Entropy steps are completed
    */
    void cleanup();
    
    /**
        Carry out a step in the Anti-Entropy method
        @param exchangeServer       Exchange server id, or -1 if none
        @param exchange_ACK         Exchange server's ACK matrix
        @param msg                  The received messaged during RECONCILE_MESSAGES state
        @param reconcile_only       This call is handled if and if only the machine is in RECONCILE_MESSAGES state
        @return                     True if Anti-Entropy is completed, false else
    */
    bool do_Anti_Entropy_step(const uint exchangeServer, const array<array<uint, N_SERVER>, N_SERVER> exchange_ACK, MailInterface* msg, 
            const bool reconcile_only);
};

#endif