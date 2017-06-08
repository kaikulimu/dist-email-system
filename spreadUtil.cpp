#include "spreadUtil.h"
#include "header.h"
#include <assert.h>

// public:

SpreadUtil::SpreadUtil(){
    Spread_name = SPREAD_NAME;
}

void SpreadUtil::init_connect() {     
    int mver, miver, pver, ret;
    
    E_init();      
    if (!SP_version( &mver, &miver, &pver)){
        exit_with("SpreadUtil: Illegal variables passed to SP_version()\n");
    }
    printf("Spread library version is %d.%d.%d\n", mver, miver, pver);

    ret = SP_connect( Spread_name.c_str(), User.c_str(), 0, 1, &Mbox, Private_group);
    if( ret != ACCEPT_SESSION ){
        SP_error( ret );
        SP_disconnect( Mbox );
        exit_with("");
    }
    printf("SpreadUtil: connected to %s with private group %s\n", Spread_name.c_str(), Private_group);

    E_init();      
}

void SpreadUtil::handle_events() const {
    E_handle_events();
}

void SpreadUtil::join_group(const string group) { 
    string error_msg;
    int ret;

    ret = SP_join( Mbox, group.c_str());
    if( ret < 0){ // failed to join the group
        SP_error(ret);
        SP_disconnect( Mbox );
        error_msg =  "Failed to join Group: " + group + "\n";
        exit_with(error_msg);
    }
}

void SpreadUtil::leave_group(const string group) {
    string error_msg;
    int ret;
    
    ret = SP_leave( Mbox, group.c_str());
    if(ret < 0){
        SP_error(ret);
        SP_disconnect( Mbox );
        error_msg = "Failed to leave Group: " + group + "\n";
        exit_with(error_msg);
    }
}

void SpreadUtil::multicast_message(const MessageInterface * const msg, const string group) {
    string error_msg;
    int ret;

    assert(sizeof(*msg) <= MAX_MESSLEN);
    assert(msg);

    char buffer[MAX_MESSLEN];
    strcpy(buffer, msg->to_string().c_str());
    ret = SP_multicast(Mbox, AGREED_MESS, group.c_str(), msg->type, sizeof(buffer), buffer);
    if (ret < 0) {
        SP_error(ret);
        SP_disconnect(Mbox);
        error_msg = "Multicast error to Group: " + group + "\n";
        exit_with(error_msg);
    }
}
