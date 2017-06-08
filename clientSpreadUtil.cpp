#include "clientSpreadUtil.h"
#include "header.h"
#include "mystring.h"
#include <netdb.h>
#include <climits>

// private:

ClientSpreadUtil::ClientSpreadUtil(){
    User = CLIENT_USER + to_string(getpid());
}

void ClientSpreadUtil::kicked_from_lobby() {
    leave_group(server_lobby);
    
    observer->notifyServerInformation("Server is unavailable. Kicked from " + server_lobby);
    observer->notifyDisconnect();
    server_lobby = "";
}

void ClientSpreadUtil::connection_from_lobby_to_chamber(const uint& server_id) {
    leave_group(server_lobby);
    server_lobby = "";
    
    server_chamber = "Server_" + to_string(server_id);
    join_group(server_chamber);
    
    observer->notifyServerInformation("Welcome to " + server_chamber);
} 


// public:

void ClientSpreadUtil::set_observer(ClientObserver * const ob){
    observer = ob;
    init_connect();
    attach_events();
}

void ClientSpreadUtil::unicast_server(const MessageInterface * const msg) {
    multicast_message(msg, server_chamber);
}

void ClientSpreadUtil::attach_events() const{
    E_attach_fd( 0, READ_FD, invoke_user_command, PASSCODE, NULL, LOW_PRIORITY );
    E_attach_fd( Mbox, READ_FD, invoke_client_read_message, PASSCODE, NULL, HIGH_PRIORITY );
}

void ClientSpreadUtil::connect_new_server(const uint& server_id){
    if (server_id <= 0 || server_id > 5) {
        printf("Server id must be from 1 to 5. %u is invalid.\n", server_id);
        return;
    }
    
    if (! server_chamber.empty()) { // disconnect from previous server if any
        leave_group(server_chamber);
        server_chamber = "";
    }
    
    server_lobby = "Lobby_" + to_string(server_id);
    join_group(server_lobby);
    
    observer->notifyServerInformation("Welcome to " + server_lobby);
}

void ClientSpreadUtil::user_command(string cmd) {
    assert(observer);
    observer->notifyUserCommand(cmd);
}

void ClientSpreadUtil::read_message(){
    assert(observer);
    static	char		mess[MAX_MESSLEN];
    char		        sender_buf[MAX_GROUP_NAME];
    string              sender;
    char		        target_groups[N_SERVER][MAX_GROUP_NAME];
    membership_info     memb_info;
    vs_set_info         vssets[MAX_VSSETS];
    uint                my_vsset_index;
    int                 num_vs_sets;
    char                members[N_SERVER][MAX_GROUP_NAME];
    int		            num_groups;
    int		            service_type;
    int16		        mess_type;
    int		            endian_mismatch;
    int                 ret;

    service_type = 0;

    ret = SP_receive( Mbox, &service_type, sender_buf, INT_MAX, &num_groups, target_groups, &mess_type, &endian_mismatch, sizeof(mess), mess );
    sender = string(sender_buf);
    
    // if error in SP_receive
    if (ret < 0){
        SP_error( ret );
        SP_disconnect( Mbox );
        exit_with("");
    }

    if ( Is_regular_mess( service_type ) ){
        MessageInterface* msg;
        
        assert(is_server(sender) || is_client(sender));
        // only care about Server sender
        if (! is_server(sender)) {
            return;
        }
        
        mess[ret] = 0;
        assert (Is_agreed_mess(service_type));
        /*printf("received AGREED message from %s, of type %d, (endian %d) to %d groups \n(%d bytes):\n", sender.c_str(),
               mess_type, endian_mismatch, num_groups, ret);   */        
        
        msg = makeMessage((MessageType) mess_type, string(mess));
        observer->notifyMessage(msg);
        
    } else if(Is_membership_mess( service_type )){
        ret = SP_get_memb_info( mess, service_type, &memb_info );
        if (ret < 0) {
            SP_error( ret );
            SP_disconnect( Mbox );
            exit_with("BUG: membership message does not have valid body\n");
        }
        
        if (Is_reg_memb_mess(service_type)){
            ///printf("Received REGULAR membership for group %s with %d members, where I am member %d:\n", sender, num_groups, mess_type );
            assert (num_groups >= 0);
            
            for(int i = 0; i < num_groups; i++ ){
                ///printf("\t%s\n", &target_groups[i][0]);
            }

            //printf("grp id is %d %d %d\n",memb_info.gid.id[0], memb_info.gid.id[1], memb_info.gid.id[2] );
            
            if (Is_caused_join_mess(service_type)){
                ///printf("Due to the JOIN of %s\n", memb_info.changed_member );
            } else if (Is_caused_leave_mess(service_type)){
                ///printf("Due to the LEAVE of %s\n", memb_info.changed_member );
            } else if (Is_caused_disconnect_mess( service_type)){
                string disconnected_member = string(memb_info.changed_member);
                if ( is_server(disconnected_member) ) {
                    observer->notifyServerInformation("Server_" + to_string(get_server_face_id(disconnected_member)) + " has disconnected");
                    observer->notifyDisconnect();
                    leave_group(sender);
                }
                ///printf("Due to the DISCONNECT of %s\n", memb_info.changed_member );
            } else if (Is_caused_network_mess(service_type)){
                ///printf("Due to NETWORK change with %u VS sets\n", memb_info.num_vs_sets);
                num_vs_sets = SP_get_vs_sets_info( mess, &vssets[0], MAX_VSSETS, &my_vsset_index );
                if(num_vs_sets < 0){
                    printf("BUG: membership message has more then %d vs sets. Recompile with larger MAX_VSSETS\n", MAX_VSSETS);
                    SP_error( num_vs_sets );
                    SP_disconnect( Mbox );
                    exit_with("");
                }
                for (uint i = 0; i < (uint) num_vs_sets; i++){
                    ///printf("%s VS set %d has %u members:\n",
                            ///(i  == my_vsset_index) ?
                            ///("LOCAL") : ("OTHER"), i, vssets[i].num_members );
                    ret = SP_get_vs_set_members(mess, &vssets[i], members, INT_MAX);
                    if (ret < 0){
                        ///printf("VS Set has more then %d members.");
                        SP_error( ret );
                        SP_disconnect( Mbox );
                        exit_with("");
                    }
                    
                    bool server_still_active = false;
                    for (uint j = 0; j < vssets[i].num_members; j++ ) {
                        if ( is_server(string(members[j])) ) {
                            server_still_active = true;
                        }
                        ///printf("\t%s\n", members[j] );
                    }
                    if (! server_still_active) {
                        observer->notifyServerInformation("The server has disconnected due to network partition");
                        observer->notifyDisconnect();
                    }
                }
            }
            
            // special case when you are waiting in the lobby
            if (is_lobby(sender)) {
                uint server_id = 0;
                for(int i = 0; i < num_groups; i++ ){
                    if ( is_server(string(target_groups[i])) ) {
                        server_id = get_server_face_id(string(target_groups[i]));
                    }
                }
                if (server_id == 0) {
                    kicked_from_lobby();
                } else {
                    connection_from_lobby_to_chamber(server_id);
                }    
            }
        }
        else if ( Is_transition_mess( service_type ) ) {
            //printf("received TRANSITIONAL membership for group %s\n", sender );
        } else if ( Is_caused_leave_mess( service_type ) ){
            //printf("received membership message that left group %s\n", sender );
        } else{
            printf("received incorrecty membership message of type 0x%x\n", service_type );
        }
    } else if(Is_reject_mess(service_type)){
        printf("REJECTED message from %s, of servicetype 0x%x messtype %d, (endian %d) to %d groups \n(%d bytes): %s\n",
                sender.c_str(), service_type, mess_type, endian_mismatch, num_groups, ret, mess );
    } else{
        printf("received message of unknown message type 0x%x with ret %d\n", service_type, ret);
    }
}



void invoke_client_read_message(int fd, int passcode, void *data){
    if (passcode != PASSCODE) {
        exit_with("Received message with unknown passcode, potentially malicious.\n");
    }
    ClientSpreadUtil::getInstance().read_message();
}


void invoke_user_command(int fd, int passcode, void *data) {
    if (passcode != PASSCODE) {
        exit_with("Received message with unknown passcode, potentially malicious");
    }
    string command = "";
    getline(cin, command);
    ClientSpreadUtil::getInstance().user_command(command);
}