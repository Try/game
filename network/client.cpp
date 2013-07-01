#include "client.h"

#include <string>

#ifdef _WIN32
    #include "winsock.h"
#else
    #include <sys/socket.h>
    #include <netinet/in.h>

    #include <string.h>
    #include <sys/types.h>
    #include <arpa/inet.h>

    typedef int SOCKET;
#endif

struct Client::Data{
  #ifdef _WIN32
  WSADATA wsaData;
  #endif
  SOCKET serverSocket;
  sockaddr_in serverAddress;
  };

Client::Client() {
  data.reset( new Data() );
  connected = false;
  }

Client::~Client() {
  connected = false;
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
  sendStr(data->serverSocket, str);
  }

bool Client::isServer() const {
  return false;
  }

void Client::startClient(){
  //consolePrintInput = 1;

  #ifdef _WIN32
  WSAStartup(MAKEWORD(2,0), &data->wsaData);
  #endif

  data->serverSocket = socket(PF_INET, SOCK_STREAM, 0);
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

    data->serverAddress.sin_family = AF_INET;
    data->serverAddress.sin_port = htons(1313);
#ifdef _WIN32
    data->serverAddress.sin_addr.S_un.S_addr = ip;
#else
    data->serverAddress.sin_addr.s_addr = ip;
#endif

    if( ::connect( data->serverSocket,
                   (sockaddr*)(&data->serverAddress),
                   sizeof(sockaddr_in) ) != 0){
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

  while( connected ){
    recieved = recvStr( data->serverSocket, buffer );

    if (recieved == 0){
      onError("Server closed");
      break;
      }else
    if (recieved == -1){
      onError("Recieve error");
      break;
      }

    onRecv(buffer);
    buffer.clear();
    }

#ifdef _WIN32
  closesocket(data->serverSocket);
#else
  close(data->serverSocket);
#endif

  //removeConsoleListener(sender);
  connected = false;
  startClient();
  return 0;
  }
