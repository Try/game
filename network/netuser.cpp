#include "netuser.h"

NetUser::NetUser()
{
}


NetUser::~NetUser()
{
}

int NetUser::recvStr( SOCKET sock, std::vector<char> & str ) {
  const size_t blockMaxSZ = 255*255;
  unsigned sz = 0;//htonl( str.size() );

  int recieved = recv(sock, (char*)(&sz), 4, 0);

  if( recieved<=0 )
    return recieved;

  str.resize( ntohl(sz) );

  for( size_t i=0; i<str.size(); i+=blockMaxSZ ){
    size_t sz = blockMaxSZ;
    if( i+blockMaxSZ>=str.size() )
      sz = str.size()-i;

    recieved = recv(sock, &str[i], sz, 0);

    if( recieved<=0 )
      return recieved;
    }

  return 1;//str.size();
  }

int NetUser::sendStr(SOCKET sock, const std::vector<char> &str) {
  const size_t blockMaxSZ = 255*255;

  unsigned sz = htonl( str.size() );

  if( send(sock, (char*)(&sz), sizeof(sz), 0) == -1 )
    return -1;

  for( size_t i=0; i<str.size(); i+=blockMaxSZ ){
    size_t sz = blockMaxSZ;
    if( i+blockMaxSZ>=str.size() )
      sz = str.size()-i;

    if( send(sock, &str[i], sz, 0)== -1 )
      return -1;
    }

  return 0;
  }
