#include "header.h"
//#include "message.cpp"
using namespace std;

void test_message() {
  // Test message data structure
  CommonMail msg("");
  assert(msg.to_string() == "000toUserfromUsersubjectbody\t of the mail00");

  msg.timestamp = ~0U;
  assert(msg.to_string() == "004294967295toUserfromUsersubjectbody\t of the mail00");

  CommonMail msg2(msg.to_string());
  assert(msg2.type == msg.type);
  assert(msg2.timestamp == msg.timestamp);
  assert(msg2.body == msg.body);
  assert(msg2.byServer == msg.byServer);

  // Test Instruction
  MarkMail del ("4 6 7 8 1 0");
  MarkMail del2 (7, 8, true, false);
  assert(del.to_preview() == del2.to_preview());
  cout << del.to_preview() << endl;
}

void test_logger() {

  // Construct a test message
  CommonMail msg("");
  msg.byServer = 1;
  msg.index = 6;
  msg.timestamp = 3;

  // Test writing to files
  Logger lg("1");
  lg.append(2, &msg);
  lg.append(1, &msg);

  MarkMail del (1, 2, true, false);
  lg.append(1, &del);

}

void test_member_list(){
  vector<uint> ml0 = {1, 2, 3};
  MemberList ml1 (ml0);
  MemberList ml2 (ml1.to_string());
  assert(ml1.type = MessageType::MemberListType);
  assert(ml2.type = MessageType::MemberListType);
  assert(ml1.to_string() == ml2.to_string());
  assert(ml1.members == ml2.members);
}



void test_ack_message(){
  array<array<uint, N_SERVER>, N_SERVER> ACK;
  for(auto& row : ACK) for(uint & x : row) x = rand() % 10;
  ACKMessage msg(ACK, 3);
  assert(msg.to_string() == ACKMessage(msg.to_string()).to_string());
}




void test_client_command(){
  ClientCommand cc('v');
  assert(cc.cmd == "v");
  assert(ClientCommand(cc.to_string()).cmd == "v");

  ClientCommand ls('l', "user");
  assert(ls.cmd == "l");
  assert(ls.data == "user");
  assert(ClientCommand(ls.to_string()).cmd == "l");
  assert(ClientCommand(ls.to_string()).data == "user");

}

ostream & operator<<(ostream& os, vector<string> xx){
  for(string x : xx) os << "|" << x << "| ";
  return os;
}

void test_split(){
  assert(split("  hello  world  ") == vector<string> ({"hello", "world"}));
  assert(split("user name  Sen Lin ", Delimiter) == vector<string>({"user name ", " Sen Lin "}));
}

void test_regex(){
    assert(2 == get_server_face_id("lsh#Server_2Client3"));
    assert(2 == get_server_face_id("#Server_2"));
    assert(is_server("#Server_2"));
    assert(2 == get_server_face_id("#Server_2ugrad8"));

    assert(3729 == get_client_face_id("#Cli3729ugrad8"));
    assert(3 == get_lobby_face_id("#Lobby_3ugrad8"));
}

void test_file(){
    assert(readfile("notexists") == "");
}

int main() {
  test_split();
  test_regex();
  test_message();
  test_logger();
  test_member_list();
  test_ack_message();
  test_client_command();
  test_file();
  std::cout << "\nAll test cases passed!\n";

}

