#ifndef SCENARIOFACTORY_H
#define SCENARIOFACTORY_H

#include "factory.h"
#include "game/scenario.h"
typedef Factory<Scenario, Game&, MainGui &, BehaviorMSGQueue &> ScenarioFactory;

#endif // SCENARIOFACTORY_H
