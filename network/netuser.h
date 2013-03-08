#ifndef NETUSER_H
#define NETUSER_H

#include <Tempest/signal>
#include <string>

#ifdef _WIN32
    #include "winsock.h"
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

#include "threads/async.h"

class NetUser {
  public:
    class Client{
      SOCKET sock;
      sockaddr_in addr;

      Future thread;
      friend class LocalServer;
      };

    NetUser();
    virtual ~NetUser();

    Tempest::signal< const std::vector<char> & > onRecv;
    Tempest::signal< const std::string& > onError;

    virtual void start() = 0;
    virtual void sendMsg( const std::vector<char> & v ) = 0;
    virtual void sendMsg( const std::vector<char> &,
                          Client & ){}

    virtual bool isServer() const = 0;
    virtual bool isConnected() const = 0;
  protected:
    int sendStr(SOCKET sock, const std::vector<char> &str);
    int recvStr(SOCKET sock, std::vector<char> &str);
  };

#endif // NETUSER_H
