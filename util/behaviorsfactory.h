#ifndef BEHAVIORSFACTORY_H
#define BEHAVIORSFACTORY_H

#include "factory.h"
#include "behavior/behavior.h"
typedef Factory<AbstractBehavior, GameObject&, Behavior::Closure&> BehaviorsFactory;

#endif // BEHAVIORSFACTORY_H
