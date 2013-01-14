#ifndef LOCALSERVER_H
#define LOCALSERVER_H

#include <vector>
#include "threads/async.h"

#include <vector>
#include "netuser.h"
#include "threads/mutex.h"

class LocalServer : public NetUser {
  public:
    LocalServer();
    ~LocalServer();

    void start();
    void sendMsg( const std::vector<char> & v );
    void sendMsg( const std::vector<char> & v,
                  Client & c );

    bool isServer() const;
    bool isConnected() const;

    MyWidget::signal< NetUser&, Client& > onConnected;
    MyWidget::signal< NetUser&, Client& > onDisConnected;

  private:
    Mutex clientsMut;
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
