#include"header.h"

using namespace std;

MessageInterface::MessageInterface() {}

MailInterface::MailInterface() {}


string MessageInterface::to_string() const {
    return "Please overload to_string()";
}

string MessageInterface::to_preview() const {
    return "Please overload to_preview()";
}


ClientCommand::ClientCommand(string str) {
    type = MessageType::ClientCommandType;
    auto fields = split(str);
    assert(fields.size() == 2);
    cmd = fields[0];
    data = fields[1];
}

ClientCommand::ClientCommand(char c) {
    type = MessageType::ClientCommandType;
    cmd = string(1, c);
    data = "_";
}

ClientCommand::ClientCommand(char c, string d) {
    type = MessageType::ClientCommandType;
    cmd = string(1, c);
    data = d;
}

string ClientCommand::to_string() const {
    stringstream ss;
    ss << cmd << " " << data;
    return ss.str();
}

string ClientCommand::to_preview() const {
    return "Client ClientCommandType: " + to_string();
}


CommonMail::CommonMail(string str) {
    type = MessageType::CommonMailType;
    // Constructor from a string.
    if (str.empty()) return;
    auto fields = split(str, Delimiter);
    assert(fields.size() == 9);
    byServer = stoul(fields[0]);
    index = stoul(fields[1]);
    timestamp = stoul(fields[2]);
    toUser = fields[3];
    fromUser = fields[4];
    subject = fields[5];
    body = fields[6];
    for (char &c : body) if (c == NewLine) c = '\n';
    isRead = (fields[7] != "0");
    isDeleted = (fields[8] != "0");
}

string CommonMail::to_string() const {
    string body2 = body;
    for (char &c : body2) if (c == '\n') c = NewLine;
    stringstream ss;
    ss << byServer << Delimiter
       << index << Delimiter
       << timestamp << Delimiter
       << toUser << Delimiter
       << fromUser << Delimiter
       << subject << Delimiter
       << body2 << Delimiter
       << isRead << Delimiter
       << isDeleted;
    return ss.str();
}

string CommonMail::to_preview() const {
    stringstream ss;
    ss << fromUser << "\t| ";
    if (!isRead) { ss << "* "; }
    ss << subject;
    return ss.str();
}


MemberList::MemberList(string str) {
    // Constructor from a string representation
    type = MessageType::MemberListType;
    if (str.empty()) return;
    members.clear();
    for (string s : split(str)) {
        members.push_back(stoul(s));
    }
    sort(members.begin(), members.end());
}

MemberList::MemberList(const vector<uint> &mb) {
    // Constructor from a vector
    type = MessageType::MemberListType;
    members = mb;
    sort(members.begin(), members.end());
}

string MemberList::to_string() const {
    assert(!members.empty()); // the member list should at least contains the server itself
    stringstream ss;
    for (size_t i = 0; i + 1 < members.size(); ++i) {
        ss << members[i] << " ";
    }
    ss << members.back();
    return ss.str();
}

string MemberList::to_preview() const {
    stringstream ss;
    for (uint m : members) ss << m + 1 << " ";
    return ss.str();
}


MarkMail::MarkMail(uint server, uint idx, bool del, bool rd) {
    type = MarkMailType;
    targetByServer = server;
    targetIndex = idx;
    toDelete = del;
    toRead = rd;
    assert(toRead != toDelete);
}

MarkMail::MarkMail(string str) {
    type = MarkMailType;
    auto fields = split(str);
    assert(fields.size() == 6);
    byServer = stoul(fields[0]);
    index = stoul(fields[1]);
    targetByServer = stoul(fields[2]);
    targetIndex = stoul(fields[3]);
    toDelete = (fields[4] != "0");
    toRead = (fields[5] != "0");
    assert(toRead != toDelete);
}

string MarkMail::to_string() const {
    assert(toRead != toDelete);
    stringstream ss;
    ss << byServer << " "
       << index << " "
       << targetByServer << " "
       << targetIndex << " "
       << toDelete << " "
       << toRead;
    return ss.str();
}

string MarkMail::to_preview() const {
    stringstream ss;
    ss << "MarkMailType (" << targetByServer << "|" << targetIndex << ") as";
    if (toDelete) ss << " deleted";
    if (toRead) ss << " read";
    return ss.str();
}


ACKMessage::ACKMessage(array<array<uint, N_SERVER>, N_SERVER> ack, uint id) : ACK(ack), fromServer(id) {
    type = ACKsMatrixType;
}

ACKMessage::ACKMessage(string str) {
    type = ACKsMatrixType;
    if (str.empty()) {
        for (auto &row : ACK) {
            for (uint &x : row) {
                x = 0;
            }
        }
        fromServer = 0;
    } else {
        auto rows = split(str, '\n');
        for (uint i = 0; i < N_SERVER; ++i) {
            auto aa = split(rows[i], ' ');
            assert(aa.size() == 5);
            for (uint j = 0; j < N_SERVER; ++j) {
                ACK[i][j] = stoul(aa[j]);
            }
        }
        fromServer = stoul(rows.at(N_SERVER));
    }
}

string ACKMessage::to_string() const {
    stringstream ss;
    for (auto row : ACK) {
        for (uint x : row) {
            ss << x << " ";
        }
        ss << '\n';
    }
    ss << fromServer;
    return ss.str();
}

string ACKMessage::to_preview() const {
    return to_string();
}


MessageInterface *makeMessage(MessageType type, string str) {
    switch (type) {
        case CommonMailType:
            return new CommonMail(str);
        case MarkMailType:
            return new MarkMail(str);
        case ACKsMatrixType:
            return new ACKMessage(str);
        case MemberListType:
            return new MemberList(str);
        case ClientCommandType:
            return new ClientCommand(str);
        default:
            throw runtime_error("Not implemented message type");
    }
}

MessageInterface *makeMessage(string type_and_msg) {
    // Make a message object from a string with its first char representing the type
    string typestr(1, type_and_msg.front());
    MessageType type = static_cast<MessageType>(stoul(typestr));
    type_and_msg.erase(0, 1);
    return makeMessage(type, type_and_msg);
}
