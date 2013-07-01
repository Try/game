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

    Tempest::signal< NetUser&, Client& > onConnected;
    Tempest::signal< NetUser&, Client& > onDisConnected;

  private:
    Mutex clientsMut;
    std::vector<Client*> clients;

    struct Data;
    std::unique_ptr<Data> data;

    bool isRunning;
    Future listner;

    int  clientRecieve( void* data  );

    int serverListen( void* );
    int sendToClients(const void *data);
  };

#endif // LOCALSERVER_H
