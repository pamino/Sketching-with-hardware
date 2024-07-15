#pragma once

#include <array>
#include <memory>
#include <stack>
#include <cassert>
#include <unordered_set>
#include <algorithm>
#include "AdjacencyMatrix.h"
#include "Sensor.h"

//--------------------------------------------------------------------------------------------------------------
// PathFinder
//--------------------------------------------------------------------------------------------------------------

using NodeSet = std::set<std::shared_ptr<Node>, decltype([](const std::shared_ptr<Node>& l, const std::shared_ptr<Node>& r)
    { return l.get() < r.get(); })>;

struct PathFinder {
  enum class State {
    BEGIN,
    CHECK_WALLS,
    MOVE_TO_JUNCTION,
    MOVE_ONTO_JUNCTION,
    HANDLE_JUNCTION,
    HANDLE_OUT_OF_JUNCTION_RIGHT,
    HANDLE_OUT_OF_JUNCTION_LEFT,
    MOVE,
    WAIT,
  };


  PathFinder() : currentNode_(Node(0,0)), currentOrientation_(NORTH) {}

  //------------------------------------------------------------------------------------------------------------
  // TODO implement

  virtual float move() = 0; // moves front a fixed distance, gets regularely called
  virtual bool turn90RightImpl() = 0; // turns Right exactly 90 degrees without covering distance
  virtual bool turn90LeftImpl() = 0; // turns LEFT exactly 90 degrees without covering distance
  virtual float measureDistance(SensorDirection direction) = 0; // measures distance of each Sensor
  virtual bool touchWallTop() = 0; // detects if the car is touching a wall or is close to
  virtual float travelledDist() = 0; // returns an absolute value of the travalled

  virtual void uploadNode(const Node& node) = 0; // uploads a Node somewhere, whenever it is created

  //------------------------------------------------------------------------------------------------------------

  bool turn(Orientation orientation);
  bool detectWall(SensorDirection direction);

  float getTravelDist()                                 { return travelledDist() - travelledDist_; }
  void updateTravelDist()                               { travelledDist_ = travelledDist(); }

  bool detectWallRight()                                { return detectWall(TOPRIGHT) || detectWall(BOTTOMRIGHT); }
  bool detectWallLeft()                                 { return detectWall(TOPLEFT) || detectWall(BOTTOMLEFT); }

  bool createNode();
  std::optional<Node> setGoal(const std::optional<Node>& node = std::nullopt);
  void search();
  std::shared_ptr<Node> newNode() {
    // returns Node if a new Node was found
    static NodeSet nodes = visitedNodes_;
    if (visitedNodes_.size() > nodes.size()) {
      NodeSet ret;
      std::set_difference(visitedNodes_.begin(), visitedNodes_.end(),nodes.begin(), nodes.end(), std::inserter(ret, ret.begin()));
      nodes = visitedNodes_;
      return *ret.begin();
    }
    return nullptr;
  }


protected:
  AdjacencyMatrix adjacencyMatrix_;

  Node currentNode_;
  std::stack<std::shared_ptr<Node>> nodeStack_;
  NodeSet visitedNodes_;
  Orientation currentOrientation_;

  float travelledDist_{0};
  float wallDist_{0};

private:
  State currentState_ = State::BEGIN;

  void initialize();
  void moveToJunction();
  void handleJunction();
  void moveOntoJunction();
  void handleOutOfJunctionLeft();
  void handleOutOfJunctionRight();
  void backtrack();
  void wait();
  void goToNeighbor(const Node& goal);

  bool backtrack_ = false;
  bool begin_ = true;
  bool freePlay_ = false;
  std::optional<Node> goal_;
  float move_ = 0;
};


