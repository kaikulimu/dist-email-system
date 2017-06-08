#include "serverSpreadUtil.h"
#include "header.h"
#include "mystring.h"
#include <climits>

using namespace std;


// private:

bool ServerSpreadUtil::isMyself(const string sender) const {
    if (! is_server(sender)) {
        return false;
    }
    return get_server_face_id(sender) == observer->getServerId() + 1;
}

bool ServerSpreadUtil::isServerCluster(const string sender) const {
    return sender == SERVER_CLUSTER_NAME;
}

ServerSpreadUtil::ServerSpreadUtil(){
    partitionServers.clear();
}

// public:


void ServerSpreadUtil::set_name(string user_name){
    User = user_name;
    self_lobby_name = "Lobby_" + string(1, user_name.back());
    self_chamber_name = user_name.c_str();
}

void ServerSpreadUtil::set_observer(ServerObserver * ob){
    observer = ob;
}

void ServerSpreadUtil::multicast_server_cluster(const MessageInterface * const msg) {
    multicast_message(msg, SERVER_CLUSTER_NAME);
}

void ServerSpreadUtil::reply_client(const MessageInterface * const msg) {       
    multicast_message(msg, recent_client);
}

vector<uint> ServerSpreadUtil::get_members() const {
    // Get server id of all members within the group
    assert(1 <= partitionServers.size() && partitionServers.size() <= 5);
    for (uint i : partitionServers) {
        assert(i <= 4);
    }
    
    return partitionServers;
}

void ServerSpreadUtil::attach_events() const {
    E_attach_fd( Mbox, READ_FD, invoke_server_read_message, PASSCODE, NULL, HIGH_PRIORITY );
}

void ServerSpreadUtil::join_server_cluster() {        
    join_group(SERVER_CLUSTER_NAME);
}

void ServerSpreadUtil::join_self_groups() {
    join_group(self_lobby_name);
    join_group(self_chamber_name);
}

void ServerSpreadUtil::read_message() {
    assert(observer);
    static char mess[MAX_MESSLEN];
    char sender_buf[MAX_GROUP_NAME];
    string sender;
    char target_groups[N_SERVER][MAX_GROUP_NAME];
    membership_info memb_info;
    vs_set_info vssets[MAX_VSSETS];
    uint my_vsset_index;
    int num_vs_sets;
    char members[N_SERVER][MAX_GROUP_NAME];
    int num_groups;
    int service_type;
    int16 mess_type;
    int endian_mismatch;
    int ret;
    
    service_type = 0;

    ret = SP_receive(Mbox, &service_type, sender_buf, INT_MAX, &num_groups, target_groups, &mess_type,
                     &endian_mismatch, sizeof(mess), mess);
    sender = string(sender_buf);
                     
    // if error in SP_receive
    if (ret < 0) {
        SP_error(ret);
        SP_disconnect(Mbox);
        exit_with("");
    }

    if (Is_regular_mess(service_type)) {
        assert(is_server(sender) || is_client(sender));
        
        bool isServer;
        MessageInterface* msg;
        
        if (isMyself(sender)) {
            return;
        }
        
        isServer = is_server(sender);
        if (! isServer) { // sender is client
            recent_client = sender;
        }
        
        mess[ret] = 0;
        assert (Is_agreed_mess(service_type));
        printf("received AGREED message from %s, of type %d, (endian %d) to %d groups \n(%d bytes):\n", sender.c_str(),
               mess_type, endian_mismatch, num_groups, ret);

        msg = makeMessage((MessageType) mess_type, string(mess));
        if (mess_type != ACKsMatrixType)
            cout << "// Regular message preview:\n" << msg->to_preview() << endl;
        observer->notifyMessage(msg, isServer);

    } else if (Is_membership_mess(service_type)) {
        ret = SP_get_memb_info(mess, service_type, &memb_info);
        if (ret < 0) {
            SP_error(ret);
            SP_disconnect(Mbox);
            exit_with("BUG: membership message does not have valid body\n");
        }

        if (Is_reg_memb_mess(service_type)) {
            printf("Received REGULAR membership for group %s with %d members, where I am member %d:\n", sender.c_str(),
                   num_groups, mess_type);
            assert (num_groups >= 0);

            for (int i = 0; i < num_groups; i++) {
                string server_name(&target_groups[i][0]);
                printf("\t%s\n", server_name.c_str());
            }

            // Extract server partition when server cluster network change occurs
            if(isServerCluster(sender)){
                partitionServers.clear();
                for (int i = 0; i < num_groups; i++) {
                    string server_name(&target_groups[i][0]);
                    partitionServers.push_back(get_server_face_id(string(target_groups[i])) - 1);
                }
            }
            printf("grp id is %d %d %d\n", memb_info.gid.id[0], memb_info.gid.id[1], memb_info.gid.id[2]);

            if (sender == SERVER_CLUSTER_NAME && (uint) num_groups > N_SERVER) {
                exit_with("Error! At most 5 servers can be in the server cluster.\n");
            }

            if (Is_caused_join_mess(service_type)) {
                printf("Due to the JOIN of %s\n", memb_info.changed_member);
                if (isServerCluster(sender)) {
                    observer->notifyMerge();
                }                 
            } else if (Is_caused_leave_mess(service_type)) {
                printf("Due to the LEAVE of %s\n", memb_info.changed_member);
            } else if (Is_caused_disconnect_mess(service_type)) {
                printf("Due to the DISCONNECT of %s\n", memb_info.changed_member);
            } else if (Is_caused_network_mess(service_type)) {
                printf("Due to NETWORK change with %u VS sets\n", memb_info.num_vs_sets);
                num_vs_sets = SP_get_vs_sets_info(mess, &vssets[0], MAX_VSSETS, &my_vsset_index);
                if (num_vs_sets < 0) {
                    printf("BUG: membership message has more then %d vs sets. Recompile with larger MAX_VSSETS\n",
                           MAX_VSSETS);
                    SP_error(num_vs_sets);
                    SP_disconnect(Mbox);
                    exit_with("");
                }                   
                for (uint i = 0; i < (uint) num_vs_sets; i++) {
                    printf("%s VS set %d has %u members:\n",
                           (i == my_vsset_index) ?
                           ("LOCAL") : ("OTHER"), i, vssets[i].num_members);
                    ret = SP_get_vs_set_members(mess, &vssets[i], members, INT_MAX);
                    if (ret < 0) {
                        printf("VS Set has too many members.");
                        SP_error(ret);
                        SP_disconnect(Mbox);
                        exit_with("");
                    }
                    for (uint j = 0; j < vssets[i].num_members; j++)
                        printf("\t%s\n", members[j]);
                }
                  
                if (isServerCluster(sender)) {
                    if ( num_vs_sets > 1                                        // if this is a merge event
                            || (num_vs_sets == 1 && observer->is_merging()) ) { // if partition during merge
                        observer->notifyMerge();
                    }
                }
            }
        }
        else if (Is_transition_mess(service_type)) {
            printf("received TRANSITIONAL membership for group %s\n", sender.c_str());
        } else if (Is_caused_leave_mess(service_type)) {
            printf("received membership message that left group %s\n", sender.c_str());
        } else {
            printf("received incorrecty membership message of type 0x%x\n", service_type);
        }
    } else if (Is_reject_mess(service_type)) {
        printf("REJECTED message from %s, of servicetype 0x%x messtype %d, (endian %d) to %d groups \n(%d bytes): %s\n",
               sender.c_str(), service_type, mess_type, endian_mismatch, num_groups, ret, mess);
    } else {
        printf("received message of unknown message type 0x%x with ret %d\n", service_type, ret);
    }
}


void invoke_server_read_message(int fd, int passcode, void *data) {
    if (passcode != PASSCODE) {
        exit_with("Received message with unknown passcode, potentially malicious");
    }
    ServerSpreadUtil::getInstance().read_message();
}
