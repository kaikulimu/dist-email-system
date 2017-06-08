#include "AntiEntropyMachine.h"
#include <iostream>
#include "params.h"
#include <assert.h>

// private:

bool AntiEntropyMachine::is_isolated() const { 
    return parentServer->get_partition_servers().size() == 1;
}

bool AntiEntropyMachine::in_same_partition(uint server_id) const {
    vector<uint> partitionServers = parentServer->get_partition_servers();
    for (uint i : partitionServers) {
        if (i == server_id) return true;
    }
    return false;
}

void AntiEntropyMachine::print_ACK_matrix() const {
    cout << (ACKMessage (ACK, my_server_id)).to_string() << endl;
}

void AntiEntropyMachine::assume_myself_informer_min_max_knowledge() {    
    for (uint j = 0; j < N_SERVER; j++) {
        informer.at(j) = my_server_id;
        informer_ACK.at(j) = ACK.at(my_server_id).at(j);
        min_knowledge.at(j) = ACK.at(my_server_id).at(j);
        max_knowledge.at(j) = ACK.at(my_server_id).at(j);
    }
}

void AntiEntropyMachine::processKnowledgeExchange(const uint xchg_server_id, const array<array<uint, N_SERVER>, N_SERVER> exchange_ACK) {   
    assert(xchg_server_id <= 4);
    assert(xchg_server_id != my_server_id);
    
    for (uint j = 0; j < N_SERVER; j++) {
        // Verify that no one claims I know more than I do
        assert(exchange_ACK.at(my_server_id).at(j) <= ACK.at(my_server_id).at(j));
        
        // Verify that I do not claim he knows more than he does
        assert(ACK.at(xchg_server_id).at(j) <= exchange_ACK.at(xchg_server_id).at(j));
        
        // Check if need to update the informer and its ACK
        if ( (exchange_ACK.at(xchg_server_id).at(j) > informer_ACK.at(j)) ||
            ( (exchange_ACK.at(xchg_server_id).at(j) == informer_ACK.at(j)) 
                    && (xchg_server_id < informer.at(j))
                    && (exchange_ACK.at(xchg_server_id).at(j) > min_knowledge.at(j)) ) 
            ) {
            informer.at(j) = xchg_server_id;
            informer_ACK.at(j) = exchange_ACK.at(xchg_server_id).at(j);
        }
    }
    
    // Update other's ACK on all rows except my row
    for (uint i = 0; i < N_SERVER; i++) {
        if (i == my_server_id) { 
            continue;
        }
        
        for (uint j = 0; j < N_SERVER; j++) {
            ACK.at(i).at(j) = max(ACK.at(i).at(j), exchange_ACK.at(i).at(j));
        }
    }
  
    for (uint j = 0; j < N_SERVER; j++) {
        min_knowledge.at(j) = min(min_knowledge.at(j), exchange_ACK.at(xchg_server_id).at(j));
        max_knowledge.at(j) = max(max_knowledge.at(j), exchange_ACK.at(xchg_server_id).at(j)); 
        
        // min.know =< self.know =< max.know
        assert(min_knowledge.at(j) <= ACK.at(my_server_id).at(j) && ACK.at(my_server_id).at(j) <= max_knowledge.at(j));
        // assert informer has max knowledge
        assert(max_knowledge.at(j) == informer_ACK.at(j));
    }
     
    verify_ACK_matrix_integrity();
    parentServer->log_ACK_matrix(ACK);
}

void AntiEntropyMachine::verify_ACK_matrix_integrity() {    
    // Verify that a diagonal is always the largest of the column
    for (uint j = 0; j < N_SERVER; j++) {
        for (uint i = 0; i < N_SERVER; i++) {
            assert(ACK.at(i).at(j) <= ACK.at(j).at(j));
        }
    }
}

void AntiEntropyMachine::informer_narrative(uint X, uint min_X_ACK, uint max_X_ACK) {
    assert(min_X_ACK == min_knowledge.at(X));
    assert(max_X_ACK == max_knowledge.at(X) && max_X_ACK == informer_ACK.at(X) && max_X_ACK == ACK.at(my_server_id).at(X));

    cout << "Informing narrative of Server " << X << endl;
    for (uint idx = min_X_ACK + 1; idx <= max_X_ACK; idx++) {
        cout << "Informing Msg " << idx << " of Server " << X << endl;
        parentServer->multicast_NACK_mail(X, idx);
        ++min_knowledge.at(X);
    }
    assert(min_knowledge.at(X) == max_knowledge.at(X));
}


// public:

AntiEntropyMachine::AntiEntropyMachine(ServerObserver * const parent)
        : parentServer(parent), my_server_id(parentServer->getServerId()) {
    // Reset all variables
    for (uint i = 0; i < N_SERVER; ++i) {
        for (uint &x : ACK.at(i)) x = 0;
        informer.at(i) = 0;
        informer_ACK.at(i) = 0;
    }

    // Initilize ACK
    for(auto & row : ACK){
        for(uint & x : row){
            x = 0;
        }
    }

    acknowledged_servers.reset();
    merging = false;
    pre_reconcile = true;
    state = REST;
    
    parentServer->recover_ACK_matrix(ACK);
}

bool AntiEntropyMachine::is_merging() const {
    return merging;
}

array<uint, N_SERVER> AntiEntropyMachine::get_min_ACKs() const {
    array<uint, N_SERVER> min_ACKs;

    for (uint j = 0; j < N_SERVER; j++) {
        uint theMin = ~0U;
        for(uint i = 0; i < N_SERVER; ++i){
            theMin = min(theMin, ACK.at(i).at(j));
        }
        min_ACKs.at(j) = theMin;
    }

    return min_ACKs;
}

uint AntiEntropyMachine::get_my_ACK_of(uint server_id) const{
    return ACK.at(my_server_id).at(server_id);
}

void AntiEntropyMachine::notifyMerge() {
    state = PREPARE;
    merging = true;
}

void AntiEntropyMachine::notify_send(MailInterface* msg) {
    cout << "notify_send(): should be equal: " << msg->index << " " << ACK.at(my_server_id).at(my_server_id) + 1 << endl;
    
    assert(msg->index == ACK.at(my_server_id).at(my_server_id) + 1);
    ++ACK.at(my_server_id).at(my_server_id);
    parentServer->log_ACK_matrix(ACK);
}

void AntiEntropyMachine::notify_receive_mail(MailInterface* msg) {
    // if receiving very new mail while still merging old mails
    if ((msg->index > ACK.at(my_server_id).at(msg->byServer) + 1) && in_same_partition(msg->byServer)) {
        return;
    }
    
    assert(msg->index <= ACK.at(my_server_id).at(msg->byServer) + 1);
    assert(msg->index <= ACK.at(msg->byServer).at(msg->byServer) + 1);
    
    if (msg->index > ACK.at(my_server_id).at(msg->byServer)) {
        ++ACK.at(my_server_id).at(msg->byServer);
        parentServer->log_ACK_matrix(ACK);
        if (msg->index > ACK.at(msg->byServer).at(msg->byServer)) {
            ++ACK.at(msg->byServer).at(msg->byServer);
        }
        
        do_Anti_Entropy_step(msg->byServer, {}, msg, true);
    }
}

void AntiEntropyMachine::cleanup() {
    cout << "\nCLEANUP called()" << endl;

    // reset informer and min_max knowledge info
    for (uint j = 0; j < N_SERVER; ++j) {
        informer.at(j) = 0;
        informer_ACK.at(j) = 0;
        min_knowledge.at(j) = 0;
        max_knowledge.at(j) = 0;
    }

    state = REST;
    cout << "\ngo to REST" << endl;
    cout << "Cleaned ACK matrix is :\n" << ACKMessage(ACK, my_server_id).to_string() << endl;
    merging = false;
    pre_reconcile = true;
}

bool AntiEntropyMachine::do_Anti_Entropy_step(const uint exchangeServer, const array<array<uint, N_SERVER>, N_SERVER> exchange_ACK,
        MailInterface* msg, const bool reconcile_only) {
    assert( ((! merging) && state == REST)
            || (merging && state != REST) );
            
    if ( (reconcile_only && state != RECONCILE_MESSAGES)  ||
            ((! reconcile_only) && state == RECONCILE_MESSAGES) ) {
        return (! is_merging());
    }

    switch (state){
        case REST:
            cout << "\nREST" << endl;
            assert(! is_merging() && pre_reconcile);
            return true;
            break;
            
        case PREPARE:
            cout << "\nPREPARE" << endl;
            if (is_isolated()) {
                cout << "isolated cuz partition during merge" << endl;
                cleanup();
                return true;
            }
            pre_reconcile = true;
            state = SELF_KNOWLEDGE_EXCHANGE;
            
        case SELF_KNOWLEDGE_EXCHANGE:
            cout << "\nSELF_KNOWLEDGE_EXCHANGE" << endl;
            
            acknowledged_servers.set();
            //debug cout << "bitset set()" << endl;
            for (uint server_id : parentServer->get_partition_servers()) {
                assert(server_id <= 4);
                if (server_id != my_server_id) {
                    acknowledged_servers.reset( server_id );
                    //debug cout << "bitset reset(" << server_id << ")" << endl;
                }
            }
            
            parentServer->multicast_ACK_matrix(ACK);
            assume_myself_informer_min_max_knowledge();
            state = KNOWLEDGE_AWAIT;
            return false;
            
        case KNOWLEDGE_AWAIT: {
            cout << "\nKNOWLEDGE_AWAIT" << endl;
            assert(! exchange_ACK.empty());
            acknowledged_servers.set(exchangeServer);
            //debug cout << "bitset set(" << exchangeServer << ")" << endl;
            
            // Process received exchange server id and ACK
            processKnowledgeExchange(exchangeServer, exchange_ACK);
            
            if (! acknowledged_servers.all()) {
                //debug cout << "NOT bitset.all()" << endl;
                return false;
            }
            //debug cout << "YAY bitset.all()" << endl;
            cout << "YAY! All exchange knowledge received" << endl;
            if (! pre_reconcile) {
                cleanup();
                return true;
            }
            
            // Do I need to reconcile messages?
            bool need_reconcile = false;
            for (uint j = 0; j < N_SERVER; j++) {
                assert(min_knowledge.at(j) <= max_knowledge.at(j));
                if (min_knowledge.at(j) < max_knowledge.at(j)) { // still waiting for more knowledge to reconcile
                    need_reconcile = true;
                }
            }
            
            if (! need_reconcile) {
                cleanup();
                return true;
            }
            state = INFORMER_NARRATION;
        }
        
        case INFORMER_NARRATION: {
            cout << "\nINFORMER_NARRATION" << endl;
            cout << "My ACK matrix is now: " << endl;
            print_ACK_matrix();
            
            cout << "The informers are: " << endl << "\t";
            for (uint j = 0; j < N_SERVER; j++) {
                cout << informer.at(j) << ", ";
            }
            cout << endl;
            
            cout << "The min knowledges are: " << endl << "\t";
            for (uint j = 0; j < N_SERVER; j++) {
                cout << min_knowledge.at(j) << ", ";
            }
            cout << endl;
            cout << "The max knowledges are: " << endl << "\t";
            for (uint j = 0; j < N_SERVER; j++) {
                cout << max_knowledge.at(j) << ", ";
            }
            cout << endl;
            
            cout << endl;
            
            bool has_other_informers = false;
            // tell others of my narrative
            for (uint j = 0; j < N_SERVER; j++) {
                if (informer.at(j) == my_server_id) {
                    informer_narrative(j, min_knowledge.at(j), max_knowledge.at(j) );
                } else {
                    has_other_informers = true;
                }
            }

            if (has_other_informers) {
                state = RECONCILE_MESSAGES;
                pre_reconcile = false;
                return false;
            } else {
                cleanup();
                return true;
            }
            break;
        }
        
        case RECONCILE_MESSAGES:  
            cout << "\nRECONCILE_MESSAGES" << endl;
            assert( reconcile_only && ! pre_reconcile);
            assert(exchangeServer == msg->byServer);
            assert(msg->index == min_knowledge.at(msg->byServer) + 1);
            ++min_knowledge.at(msg->byServer);
            
            for (uint j = 0; j < N_SERVER; j++) {
                print_ACK_matrix();
                assert(min_knowledge.at(j) <= max_knowledge.at(j));
                if (min_knowledge.at(j) < max_knowledge.at(j)) { // still waiting for more knowledge to reconcile
                    return false;
                }
            }
            
            // reconcile messages all received
            state = SELF_KNOWLEDGE_EXCHANGE;
            return do_Anti_Entropy_step(my_server_id, {}, nullptr, false);
            break;

        default:
            throw std::runtime_error("Unknown Anti-Entropy state reached");
    }
    
    throw std::runtime_error("do_Anti_Entropy_step() should never reach here");
}
