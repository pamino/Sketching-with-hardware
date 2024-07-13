#pragma once

#include <array>
#include <memory>
#include <stack>
#include <cassert>
#include <unordered_set>
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
    BACKTRACK,
    WAIT,
  };


  PathFinder() : currentNode_(std::shared_ptr<Node>(new Node(0,0))), currentOrientation_(NORTH) {}

  //------------------------------------------------------------------------------------------------------------
  // TODO implement

  virtual void move() = 0; // moves front a fixed distance, gets regularely called
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
  std::optional<Node> setGoal(const std::optional<Node>& node);
  void search();

protected:
  AdjacencyMatrix adjacencyMatrix_;

  std::shared_ptr<Node> currentNode_;
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
  void goTo(const Node& goal);

  bool backtrack_ = false;
  bool begin_ = true;
  bool freePlay_ = false;
  std::optional<Node> goal_;
};


