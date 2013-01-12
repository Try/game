#ifndef LOCALSERVER_H
#define LOCALSERVER_H

#include <vector>
#include "threads/async.h"

#include <vector>
#include "netuser.h"

class LocalServer : public NetUser {
  public:
    LocalServer();
    ~LocalServer();

    void start();
    void sendMsg( const std::vector<char> & v );

    bool isServer() const;
    bool isConnected() const;
  private:
    struct Client{
      SOCKET sock;
      sockaddr_in addr;

      Future thread;
      };

    std::vector<Client*> clients;
    SOCKET listenSocket;
    sockaddr_in listenAddress;

    bool isRunning;
    Future listner;

    int  clientRecieve( void* data  );

    int serverListen( void* );
    int sendToClients(const void *data);
  };

#endif // LOCALSERVER_H
