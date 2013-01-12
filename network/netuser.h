#ifndef NETUSER_H
#define NETUSER_H

#include <MyWidget/signal>
#include <string>

#ifdef _WIN32
    #include "winsock.h"
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

class NetUser {
  public:
    NetUser();
    virtual ~NetUser();

    MyWidget::signal< const std::vector<char> & > onRecv;
    MyWidget::signal< const std::string& > onError;

    virtual void start() = 0;
    virtual void sendMsg( const std::vector<char> & v ) = 0;

    virtual bool isServer() const = 0;
    virtual bool isConnected() const = 0;
  protected:
    int sendStr(SOCKET sock, const std::vector<char> &str);
    int recvStr(SOCKET sock, std::vector<char> &str);
  };

#endif // NETUSER_H
