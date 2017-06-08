#include "header.h"
#include "serverSpreadUtil.h"
#include "serverObserver.h"
#include "AntiEntropyMachine.h"
#include <bitset>

using namespace std;


class Server : public ServerObserver {
private:
    array<vector<MailInterface *>, N_SERVER> Mails; // The array of vectors of Mails
    uint localtime = 0; // Local Lamport timestamp
    unordered_multimap<string, CommonMail *> lookup; // The lookup table used to find all mails received by an user

    Logger *logger;
    ServerSpreadUtil *spreadUtil;
    AntiEntropyMachine *antiEntropyMachine;
    uint mail_cnt = 0;

    void create_dummy_mails();
    MailInterface *get_mail(uint server, uint idx_target);

    /**
     * Save a mail to local memory and hard disk
     * @param mail
     */
    void save_mail(MailInterface *mail);
    /**
     * Print state of server for debugging purpose
     */
    void print_state();

    /**
     * Load messages from log files
     */
    void recover();

    /**
     * Run garbage collection (romove all mark mails)
     */
    void garbage_collection();

    /**
     * Save server state in to log file
     */
    void save_server_state();

    /**
     * Recover server state from log file
     */
    void recover_server_state();

public:
    const uint id;

    uint getServerId() const override;

    Server(const uint &id_, ServerSpreadUtil *sp);


    bool is_merging() override;
    
    vector<uint> get_partition_servers() override;

    void multicast_ACK_matrix(const array<array<uint, N_SERVER>, N_SERVER>& ACK) override;
    
    void multicast_NACK_mail(uint server_id, uint index) override;

    void log_ACK_matrix(const array<array<uint, N_SERVER>, N_SERVER>& ACK) override;
    
    void recover_ACK_matrix(array<array<uint, N_SERVER>, N_SERVER>& ACK) override;
    
    /**
     * Add a new mail that a client requests to create
     * @param mail: the mail received from a client
     */
    void add_new_mail(MailInterface *mail);

    /**
     * Add an old mail that is processed by other servers
     * @param mail: the mail received from other servers
     */
    void add_others_mail(MailInterface *mail);

    void list_all_mails(string user);

    void view_partition();

    /**
     * Mark a mail as read or deleted
     * @param mm : the mark mail containing the instruction
     * @return true if the target mail changed state after marking
     */
    bool mark_mail(MarkMail *mm);


    void notifyMerge() override;

    /**
     * The Spread Utility will call this function when it receives a message
     * @param msg : received message by Spread
     * @param isFromServer : true if the message is from a server
     */
    void notifyMessage(MessageInterface *msg, const bool isFromServer) override;
};

