#include "header.h"
#include "serverSpreadUtil.h"
#include "serverObserver.h"
#include "AntiEntropyMachine.h"
#include <bitset>
#include "server.h"

using namespace std;


void Server::save_server_state() {
    string filename = to_string(id) + "/server.log";
    ofstream outfile;
    outfile.open(filename, ios_base::out);
    outfile << localtime << endl
            << mail_cnt;
    outfile.close();
}

void Server::recover_server_state() {
    string filename = to_string(id) + "/server.log";
    string buffer = readfile(filename);
    if (buffer.empty()) {
        localtime = 0;
        mail_cnt = 0;
    } else {
        auto fields = split(buffer);
        localtime = stoul(fields.at(0));
        mail_cnt = stoul(fields.at(1));
    }

}

void Server::create_dummy_mails() {
    // Create some dummy mails for debugging
    CommonMail *m1 = new CommonMail("");
    m1->toUser = "Sen";
    m1->fromUser = "Vincent";
    m1->subject = "Arnold's concert";

    CommonMail *m2 = new CommonMail("");
    m2->toUser = "Sen";
    m2->fromUser = "Tom";
    m2->subject = "Hello world";

    add_new_mail(m1);
    add_new_mail(m2);
}

MailInterface *Server::get_mail(uint server, uint idx_target) {
    // Find a message by idx using binary search
    vector<MailInterface *> &row = Mails[server];
    if (row.empty()) return nullptr;

    size_t left = 0, right = row.size() - 1;
    while (left <= right) {
        size_t mid = left + (right - left) / 2;

        if (row[mid]->index == idx_target) {
            return row[mid];
        } else if (row[mid]->index < idx_target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    cout << "Cannot find Msg " << idx_target << " of Server " << server << endl;
    return nullptr;
}


void Server::save_mail(MailInterface *mail) {
    logger->append(mail->byServer, mail);
    Mails[mail->byServer].push_back(mail);
    if (mail->type == CommonMailType) {
        CommonMail *cm = static_cast<CommonMail *>(mail);
        lookup.insert({cm->toUser, cm});
    }
}


void Server::print_state() {
    cout << "\n\n_________Server " << id << "________________\n";
    cout << "==> Local time: " << localtime << endl;
    cout << "==> Mail count: " << mail_cnt << endl;
    cout << "==> Mail matrix:\n";
    for (size_t i = 0; i < N_SERVER; ++i) {
        cout << i + 1 << "(" << Mails[i].size() << "): ";
        for (auto m : Mails[i]) {
            // Only print out common mails that are not deleted.
            if (m->type == CommonMailType) {
                CommonMail *cm = static_cast<CommonMail *>(m);
                if (!cm->isDeleted) {
                    cout << "(" << m->byServer + 1 << "," << m->index;
                    if (!cm->isRead) cout << "*";
                    cout << ") ";
                }
            }
        }
        cout << endl;
    }
    cout << "\n==> All messages:\n";
    for (size_t i = 0; i < N_SERVER; ++i) {
        cout << i + 1 << "(" << Mails[i].size() << "): ";
        for (auto m : Mails[i]) {
            cout << "(";
            if (m->type == CommonMailType) {
                cout << "M";
            } else {
                if (static_cast<MarkMail *>(m)->toRead) {
                    cout << "R";
                } else {
                    cout << "D";
                }
            }
            cout << "," << m->index << ")";
        }
        cout << endl;
    }

    cout << "==> Lookup hashtable:\n";
    for (auto it : lookup) {
        if (!it.second->isDeleted) {
            cout << it.first << " <- " << it.second->to_preview() << endl;
        }
    }
}

void Server::recover() {
    for (uint i = 0; i < N_SERVER; ++i) {
        Mails[i].clear();
        string filename = to_string(id) + "/" + to_string(i) + ".log";
        ifstream infile(filename);
        if (infile.is_open()) {
            string line;
            while (getline(infile, line, '\n')) {
                MessageInterface *m = makeMessage(line);
                Mails[i].push_back(static_cast<MailInterface *>(m));
                if (m->type == MarkMailType) {
                    mark_mail(static_cast<MarkMail *>(m));
                } else if (m->type == CommonMailType) {
                    CommonMail *cm = static_cast<CommonMail *>(m);
                    lookup.insert({cm->toUser, cm});
                } else {
                    throw runtime_error("Should not have this type of mail: " + m->to_preview());
                }
            }
            infile.close();
        }
    }
}


void Server::garbage_collection() {
    array<uint, N_SERVER> acks = antiEntropyMachine->get_min_ACKs();
    cout << "Min ACK is : ";
    for (uint a : acks) cout << a << " ";
    cout << endl;

    // Compact mail vectors
    for (size_t i = 0; i < N_SERVER; ++i) {
        vector<MailInterface *> row2;
        for (auto &m : Mails[i]) {
            // Only collect message that are acknowledged by all others.
            if (m->index < acks[i]) {
                if (m->type == CommonMailType && !static_cast<CommonMail *>(m)->isDeleted) {
                    row2.push_back(m);
                }
            } else {
                row2.push_back(m);
            }
        }
        Mails[i] = row2;
    }
    // Update the lookup table
    lookup.clear();
    for (auto &row : Mails) {
        for (auto &m : row) {
            if (m->type == CommonMailType) {
                CommonMail *cm = static_cast<CommonMail *>(m);
                lookup.insert({cm->toUser, cm});
            }
        }
    }

    // Write to files
    logger->backup(Mails);
}


uint Server::getServerId() const {
    return id;
}

Server::Server(const uint &id_, ServerSpreadUtil *sp) : id(id_) {
    spreadUtil = sp;

    // Reset all variables
    for (uint i = 0; i < N_SERVER; ++i) {
        Mails[i].clear();
    }
    logger = new Logger(to_string(id));

    // Recover from hard disk files
    recover();
    recover_server_state();
    antiEntropyMachine = new AntiEntropyMachine(this);

    spreadUtil->set_observer(this);
    string server_name = "Server_" + to_string(id + 1);
    spreadUtil->set_name(server_name);

    spreadUtil->init_connect();
    spreadUtil->attach_events();
    spreadUtil->join_server_cluster();
    spreadUtil->join_self_groups();
    spreadUtil->handle_events();
    garbage_collection();
}


bool Server::is_merging() {
    return antiEntropyMachine->is_merging();
}

vector<uint> Server::get_partition_servers() {
    return spreadUtil->get_members();
}

void Server::multicast_ACK_matrix(const array<array<uint, N_SERVER>, N_SERVER> &ACK) {
    assert(ACK.size() == 5);
    for (uint j = 0; j < N_SERVER; j++) {
        assert(ACK.at(j).size() == 5);
    }

    ACKMessage *ACK_msg = new ACKMessage(ACK, id);
    spreadUtil->multicast_server_cluster(ACK_msg);
}

void Server::multicast_NACK_mail(uint server_id, uint index) {
    // Share with other servers in the partition
    spreadUtil->multicast_server_cluster(get_mail(server_id, index));
}

void Server::log_ACK_matrix(const array<array<uint, N_SERVER>, N_SERVER> &ACK) {
    string filename = to_string(id) + "/ack.log";
    ofstream outfile;
    outfile.open(filename, ios_base::out);
    ACKMessage ack(ACK, id);
    outfile << ack.to_string();
    outfile.close();
}

void Server::recover_ACK_matrix(array<array<uint, N_SERVER>, N_SERVER> &ACK) {
    string filename = to_string(id) + "/ack.log";
    ACKMessage ack(readfile(filename));
    ACK = ack.ACK;
}


void Server::add_new_mail(MailInterface *mail) {
    if (mail->type == MarkMailType) {
        if (!mark_mail(static_cast<MarkMail *>(mail))) {
            return;
        }
    }
    // Add in some details about the mail
    mail->byServer = id;
    mail->index = ++mail_cnt;
    mail->timestamp = ++localtime;

    save_mail(mail);
    save_server_state();

    antiEntropyMachine->notify_send(mail);

    // Share with other servers in the partition
    spreadUtil->multicast_server_cluster(mail);

}


void Server::add_others_mail(MailInterface *mail) {
    // Update my local time
    localtime = max(localtime, mail->timestamp);

    if (mail->type == MarkMailType) {
        if (!mark_mail(static_cast<MarkMail *>(mail))) {
            // Still keep going if the mark mail is from other servers.
        }
    }

    // Check if already exists
    if (mail->index <= antiEntropyMachine->get_my_ACK_of(mail->byServer)
        || get_mail(mail->byServer, mail->index)) {
        cout << "Already received: " << mail->to_preview() << endl;
        // Already received
    } else {
        cout << "Newly received: " << mail->to_preview() << endl;
        save_mail(mail);
        save_server_state();
    }
}

void Server::list_all_mails(string user) {
    // data is the user name
    auto range = lookup.equal_range(user);
    for (auto it = range.first; it != range.second; ++it) {
        CommonMail *mail = it->second;
        if (!mail->isDeleted) {
            spreadUtil->reply_client(mail);
        }
    }
    ClientCommand finished('0', "finished");
    spreadUtil->reply_client(&finished);
}

void Server::view_partition() {
    MemberList *ml = new MemberList(spreadUtil->get_members());
    spreadUtil->reply_client(ml);
}

bool Server::mark_mail(MarkMail *mm) {
    MailInterface *mi = get_mail(mm->targetByServer, mm->targetIndex);
    if (mi) {
        assert(mi->type == CommonMailType);
        CommonMail *mail = static_cast<CommonMail *>(mi);
        if (mail->isDeleted == mm->toDelete && mail->isRead == mm->toRead) {
            return false;
        } else {
            if(mail->isDeleted){
                return false;
            }else{
                mail->isDeleted = mm->toDelete;
                mail->isRead = mm->toRead;
                return true;
            }
        }
    } else {
        // the mail is already deleted; just ignore
        return false;
    }
}


void Server::notifyMerge() {
    antiEntropyMachine->notifyMerge();
    antiEntropyMachine->do_Anti_Entropy_step(-1, {}, nullptr, false);
}


void Server::notifyMessage(MessageInterface *msg, const bool isFromServer) {
    if (isFromServer) {
        ACKMessage *ACK_msg;
        switch (msg->type) {
            case MarkMailType:
            case CommonMailType:
                add_others_mail(static_cast<MailInterface *>(msg));
                antiEntropyMachine->notify_receive_mail(static_cast<MailInterface *>(msg));
                break;
            case ACKsMatrixType:
                cout << "ACKs:\n" << msg->to_preview() << endl;
                ACK_msg = static_cast<ACKMessage *>(msg);
                antiEntropyMachine->do_Anti_Entropy_step(ACK_msg->fromServer, ACK_msg->ACK,
                                                         nullptr, false);
                break;
            case MemberListType:
            case ClientCommandType:
                throw runtime_error("Server should not receive this: " + msg->to_preview());
        }
    } else { // is from client
        switch (msg->type) {
            case MarkMailType:
            case CommonMailType:
                add_new_mail(static_cast<MailInterface *>(msg));
                break;

            case ACKsMatrixType:
            case MemberListType:
                throw runtime_error("Client should never send ACKsMatrixType or MemberListType");

            case ClientCommandType: {
                string cmd = static_cast<ClientCommand *>(msg)->cmd;
                string data = static_cast<ClientCommand *>(msg)->data;
                if (cmd == "v") {
                    view_partition();
                } else if (cmd == "l") {
                    list_all_mails(data);
                } else if (cmd == "x") {
                    if (data == "dummy") {
                        create_dummy_mails();
                    } else if (data == "state") {
                        print_state();
                    } else {

                    }
                } else {

                }
                break;
            }
        }
        // Print out server state for debugging purpose
        print_state();
    }
}


int main(int argc, char *argv[]) {
    assert(sizeof(MessageInterface) < MAX_MESSLEN);

    if (argc != 2) exit_with("Usage: server <server_id>\n");
    uint id = stoul(argv[1]) - 1;
    // error check for command line args
    if (id >= 5) {
        exit_with("Usage error: invalid <server_id>\n");
    }

    ServerSpreadUtil &sp = ServerSpreadUtil::getInstance();
    Server server(id, &sp);

    return 0;
}
