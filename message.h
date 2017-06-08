#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <vector>

using namespace std;


enum MessageType {
    CommonMailType,
    MarkMailType,
    ACKsMatrixType,
    MemberListType,
    ClientCommandType
};

class MessageInterface {
    // The super class used as an message interface
public:
    MessageType type = CommonMailType;

    MessageInterface();

    virtual string to_string() const;

    virtual string to_preview() const;
};

class MailInterface : public MessageInterface {
public:
    MailInterface();

    uint byServer = 0;
    uint index = 0;
    uint timestamp = 0;
};

class ClientCommand : public MessageInterface {
    // The command send by a client to a server
public:
    string cmd;
    string data;

    ClientCommand(string);

    ClientCommand(char);

    ClientCommand(char, string);

    string to_string() const;

    string to_preview() const;
};

class CommonMail : public MailInterface {
public:
    string toUser = "toUser";
    string fromUser = "fromUser";
    string subject = "subject";
    string body = "body\t of the\n mail";
    bool isRead = false;
    bool isDeleted = false;

    CommonMail(string);

    string to_string() const;

    string to_preview() const;
};


class MemberList : public MessageInterface {
public:
    vector<uint> members;

    MemberList(string);

    MemberList(const vector<uint> &mb);

    string to_string() const;

    string to_preview() const;
};

class MarkMail : public MailInterface {
public:
    uint targetByServer = 0;
    uint targetIndex = 0;
    bool toDelete = false;
    bool toRead = false;

    MarkMail(uint server, uint idx, bool del, bool rd);

    MarkMail(string str);

    string to_string() const;

    string to_preview() const;
};

class ACKMessage : public MessageInterface {
public:
    array<array<uint, N_SERVER>, N_SERVER> ACK;
    uint fromServer;

    ACKMessage(array<array<uint, N_SERVER>, N_SERVER> ack, uint id);

    ACKMessage(string str);

    string to_string() const;

    string to_preview() const;
};

MessageInterface *makeMessage(MessageType, string);

MessageInterface *makeMessage(string);

#endif
