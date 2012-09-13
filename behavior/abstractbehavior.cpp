#include "abstractbehavior.h"

AbstractBehavior::~AbstractBehavior()
{
}

bool AbstractBehavior::message(AbstractBehavior::Message ,
                               int , int , AbstractBehavior::Modifers )
{
  return 0;
}

bool AbstractBehavior::message(AbstractBehavior::Message , const std::string &,
                               AbstractBehavior::Modifers ) {
  return 0;
  }
