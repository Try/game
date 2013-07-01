#ifndef NETUSERCLIENT_H
#define NETUSERCLIENT_H

#include "netuser.h"

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

class NetUser::Client{
  SOCKET sock;
  sockaddr_in addr;

  Future thread;
  friend class LocalServer;
  };

#endif // NETUSERCLIENT_H
