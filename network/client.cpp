#include "client.h"

#include <string>

Client::Client() {
  connected = false;
  }

Client::~Client() {

  }

void Client::start() {
  onError("Enter an IP address to connect to:");
  startClient();
  }

bool Client::isConnected() const {
  return connected;
  }

void Client::connect(const std::string &addr) {
  connection.join();
  clientConnect( addr.data() );
  }

void Client::sendMsg(const std::vector<char> &str) {
  sendStr(serverSocket, str);
  }

bool Client::isServer() const {
  return false;
  }

void Client::startClient(){
  //consolePrintInput = 1;

  #ifdef _WIN32
  WSAStartup(MAKEWORD(2,0), &wsaData);
  #endif

  serverSocket = socket(PF_INET, SOCK_STREAM, 0);
  }

void Client::clientConnect( const char* in ){
  unsigned long ip;
  char str[128] = "Connecting to ";
  ip = inet_addr(in);

  if (ip == INADDR_ANY || ip == INADDR_NONE){
    onError("You have entered an invalid IP. Example of a valid IP: 83.72.95.125");
    }else{
    strcat(str, in);
    onError(str);

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(1313);
    serverAddress.sin_addr.S_un.S_addr = ip;

    if( ::connect( serverSocket,
                 (sockaddr*)(&serverAddress),
                 sizeof(struct sockaddr_in)) != 0){
      onError("Error connecting");
      }else{
      onError("Connected successfully!");
      //ReciverThread *r = new ReciverThread(serverSocket);
      //r->start();

      //connection.reset(r);
      connection = async(this, &Client::reciever, 0 );
      connected  = true;
      }
    }
  }

int Client::reciever(void *){
  std::vector<char> buffer;
  int recieved;

  while(1){
    recieved = recvStr( serverSocket, buffer );

    if (recieved == 0){
      onError("Server closed");
      break;
      }else
    if (recieved == SOCKET_ERROR){
      onError("Recieve error");
      break;
      }

    onRecv(buffer);
    buffer.clear();
    }

  closesocket(serverSocket);

  //removeConsoleListener(sender);
  connected = false;
  startClient();
  return 0;
  }
