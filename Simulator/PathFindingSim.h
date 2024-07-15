#pragma once

#include <array>
#include "Simulator.h"
#include "PathFinding.h"
#include "AdjacencyMatrix.h"

namespace sf {
struct PathFinderSim : public PathFinder {
  PathFinderSim(Car* car);

  float move() override;
  void move(float dist);


  bool turn90RightImpl() override;
  bool turn90LeftImpl() override;
  float measureDistance(SensorDirection direction) override { return sensors_[direction]->measureDistance(); }

  float travelledDist() override { return pCar_->getTravelledDistance(); }

  bool touchWallTop() override { return sensors_[SensorDirection::TOP]->measureDistance() < 20.0f; }
  void uploadNode(const Node& node) override;

private:
  std::array<const DistanceSensor*, 5> sensors_;
  Car* pCar_;
};

} // end of namespace sf