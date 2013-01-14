#include "localserver.h"

#include <string>

LocalServer::LocalServer() {
  isRunning = false;
  }

LocalServer::~LocalServer() {
  isRunning = false;
  listner.join();
  }

void LocalServer::start() {
  if( isRunning )
    return;

  isRunning = true;
#ifdef _WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2,0), &wsaData);
#endif

  listenSocket = socket(PF_INET, SOCK_STREAM, 0);
  listenAddress.sin_family = AF_INET;
  listenAddress.sin_port = htons(1313);
  listenAddress.sin_addr.S_un.S_addr = INADDR_ANY;

  if (bind( listenSocket,
            (sockaddr*)(&listenAddress),
            sizeof(sockaddr_in)) != 0){
    onError("Binding Error");
    }

  listner = async( this, &LocalServer::serverListen, (void*)0 );
  }

void LocalServer::sendMsg(const std::vector<char> &v) {
  sendToClients( &v );
  }

void LocalServer::sendMsg( const std::vector<char> &v,
                           LocalServer::Client &c ) {
  if (sendStr(c.sock, v) == SOCKET_ERROR){
    onError("Send error");
    }
  }

bool LocalServer::isServer() const {
  return true;
  }

bool LocalServer::isConnected() const {
  return true;
  }

int LocalServer::serverListen( void * ){
  SOCKET newSocket;
  struct sockaddr_in newAddress;
  int addrSize = sizeof(struct sockaddr_in);
  onError("Server listening for connections..");

  if( listen(listenSocket, SOMAXCONN) != 0 ){
    onError("Listen error");
    }

  while ( isRunning ){
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(listenSocket, &readSet);
    timeval timeout;
    timeout.tv_sec = 0;  // Zero timeout (poll)
    timeout.tv_usec = 0;

    if( select(listenSocket, &readSet, NULL, NULL, &timeout) == 1 ) {
      newSocket = accept(listenSocket, (sockaddr*)&newAddress, &addrSize);

      if (newSocket == INVALID_SOCKET){
        onError("Accept Error!");
        }else{
        onError("Connection! Client connected:");
        onError(inet_ntoa(newAddress.sin_addr));

        Client *c = new Client();
        c->sock = newSocket;
        c->addr = newAddress;

        clientsMut.lock();
        clients.push_back(c);
        clientsMut.unlock();

        c->thread = async( this, &LocalServer::clientRecieve, (void*)c);
        }
      }
    }

  clientsMut.lock();

  for( size_t i=0; i<clients.size(); ++i ){
    Client &c = *clients[i];
    c.thread.cancel();
    closesocket(c.sock);
    delete clients[i];
    }
  clients.clear();

  clientsMut.unlock();

  return 0;
  }

int LocalServer::clientRecieve( void* data ){
  std::vector<char> buffer;
  Client* client = (Client*)data;

  onConnected(*this, *client);

  while( 1 ){
    int recieved = recvStr( client->sock, buffer );

    if (recieved == 0){
      onError("Client quit");
      break;
      }else
    if (recieved == SOCKET_ERROR){
      onError("Recieve error");
      break;
      }

    //onError("Data recieved;");
    onRecv(buffer);
    //fsend = async(this, &LocalServer::sendToClients, (void*)&buffer);

    //sendToClients( (void*)&buffer );
    }  

  clientsMut.lock();
  for( size_t i = 0; i<clients.size(); ){
    if (clients[i] == client){
      clients[i] = clients.back();
      clients.pop_back();
      } else {
      ++i;
      }
    }
  clientsMut.unlock();

  onDisConnected(*this, *client);
  closesocket(client->sock);

  delete client;
  return 0;
  }

int LocalServer::sendToClients( const void* data ){
  clientsMut.lock();
  const std::vector<char>  &s = *(( const std::vector<char> *)data);

  for ( size_t i = 0; i<clients.size(); ++i){
    sendMsg( s, *clients[i] );
    }

  clientsMut.unlock();
  return 0;
  }