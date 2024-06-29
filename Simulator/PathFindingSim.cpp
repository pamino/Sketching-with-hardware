#include "PathFindingSim.h"

#include <algorithm>
#include <stack>
#include <cassert>
#include <set>

using namespace sf;

//--------------------------------------------------------------------------------------------------------------
// Graph
//--------------------------------------------------------------------------------------------------------------

//------ PathFinderSim ------
PathFinderSim::PathFinderSim(Car* pCar)
    : PathFinder(), pCar_(pCar) {
  sensors_.at(SensorDirection::TOPRIGHT) = &pCar->sensors[SensorDirection::TOPRIGHT];
  sensors_.at(SensorDirection::TOPLEFT) = &pCar->sensors[SensorDirection::TOPLEFT];
  sensors_.at(SensorDirection::BOTTOMRIGHT) = &pCar->sensors[SensorDirection::BOTTOMRIGHT];
  sensors_.at(SensorDirection::BOTTOMLEFT) = &pCar->sensors[SensorDirection::BOTTOMLEFT];
  sensors_.at(SensorDirection::TOP) = &pCar->sensors[SensorDirection::TOP];
}

//------ move ------
void PathFinderSim::move() {
  return move(0.1f);
}

//------ move ------
void PathFinderSim::move(float dist) {
  if (!((Object*)pCar_)->backTrackedMove(pCar_->front * dist))
    return;

  currentNode_->x += pCar_->front.x * dist;
  currentNode_->y += pCar_->front.y * dist;
}

//------ turn90Right ------
void PathFinderSim::turn90Right() {
  if (!((Object*)pCar_)->backTrackedTurn90(true))
    return;
  currentOrientation_ = currentOrientation_.turnRight();
}

//------ turn90Left ------
void PathFinderSim::turn90Left() {
  if (!((Object*)pCar_)->backTrackedTurn90(false))
    return;
  currentOrientation_ = currentOrientation_.turnLeft();
}