#include "Simulator.h"
#include "AdjacencyMatrix.h"

#include <algorithm>
#include <stack>

using namespace sf;

//--------------------------------------------------------------------------------------------------------------
// Graph
//--------------------------------------------------------------------------------------------------------------

struct Graph {
  enum Direction {
    TOP,
    TOPRIGHT,
    TOPLEFT,
    BOTTOMRIGHT,
    BOTTOMLEFT
  };

  Graph(const DistanceSensor& top, const DistanceSensor& topLeft, const DistanceSensor& topRight, const DistanceSensor& bottomRight, const DistanceSensor& bottomLeft, Car& car)
    : car_(car) {
    sensors_.at(Direction::TOP) = &top;
    sensors_.at(Direction::TOPRIGHT) = &topRight;
    sensors_.at(Direction::TOPLEFT) = &topLeft;
    sensors_.at(Direction::BOTTOMRIGHT) = &bottomRight;
    sensors_.at(Direction::BOTTOMLEFT) = &bottomLeft;

    adjacencyMatrix_.pushNode({0, 0});
  }

  Node detectNewNode();

  void move() {
    car_.move(car_.front);
    currentPos.x += car_.front.x;
    currentPos.y += car_.front.y;
  }

  void turnRight() {
    car_.turn90(true);
  }

  void turnLeft() {
    car_.turn90(false);
  }

  float measureDistance(Direction direction) { return sensors_[direction]->measureDistance(); }

  bool detectWall(Direction direction) { return measureDistance(direction) < wallDist_; }

private:
  std::array<const DistanceSensor*, 5> sensors_;
  AdjacencyMatrix adjacencyMatrix_;
  Car& car_;

  Node currentPos{0};
  std::stack<Node> nodeStack_;
  std::list<Node> visitedNodes_;

  int wallDist_;
};

Node Graph::detectNewNode() {
  static bool currentlyOnNode = false;
  static Node prevNode{0, 0};
  static bool onStart = true;

  if (onStart) {
    wallDist_ = sensors_[TOPRIGHT]->measureDistance() * 1.3;
    // also positionCorrection
  }

  if (!detectWall(TOPRIGHT) && !detectWall(BOTTOMRIGHT) ) {
    adjacencyMatrix_.pushNode(currentPos);
    if (measureDistance(TOP) < wallDist_ * 1.5)
      turnRight();
    while (!detectWall(BOTTOMRIGHT) && !detectWall(TOP))
      move();
  }
  if (measureDistance(TOPLEFT) >= wallDist || measureDistance(BOTTOMLEFT) >= wallDist) {
    adjacencyMatrix_.pushNode(currentPos);
  }

}