#ifndef CLIENT_H
#define CLIENT_H

#include <memory>

#include "threads/async.h"
#include "netuser.h"

class Client : public NetUser {
  public:
    Client();
    ~Client();

    void start();
    bool isConnected() const;

    void connect( const std::string & addr );
    void sendMsg(const std::vector<char> &str );

    bool isServer() const;
  private:
    void startClient();
    void clientConnect( const char* in );
    int  reciever(void *);

    #ifdef _WIN32
    WSADATA wsaData;
    #endif
    SOCKET serverSocket;
    sockaddr_in serverAddress;

    Future connection;
    bool   connected;
  };

#endif // CLIENT_H
