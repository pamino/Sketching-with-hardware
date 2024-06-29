#pragma once

#include <array>
#include "Simulator.h"
#include "PathFinding.h"
#include "AdjacencyMatrix.h"

namespace sf {
struct PathFinderSim : public PathFinder {
  PathFinderSim(Car* car);

  void move() override;
  void move(float dist);

  void turn90Right() override;
  void turn90Left() override;
  float measureDistance(SensorDirection direction) override { return sensors_[direction]->measureDistance(); }

  float travelledDist() override { return pCar_->getTravelledDistance(); }

  bool touchWallTop() override { return sensors_[SensorDirection::TOP]->measureDistance() < 20.0f; }

private:
  std::array<const DistanceSensor*, 5> sensors_;
  Car* pCar_;
};

} // end of namespace sf