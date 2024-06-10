#pragma once

#include <array>
#include <memory>
#include <stack>
#include <cassert>
#include "AdjacencyMatrix.h"
#include "Sensor.h"

//--------------------------------------------------------------------------------------------------------------
// PathFinder
//--------------------------------------------------------------------------------------------------------------

using NodeSet = std::set < std::shared_ptr<Node>, decltype([](const std::shared_ptr<Node>& l, const std::shared_ptr<Node>& r) { return l.get() < r.get(); }) > ;

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
    FINISHED
  };


  PathFinder() : currentNode_(std::shared_ptr<Node>(new Node(0,0))), currentOrientation_(NORTH) {}

  void depthFirstSearch();

  virtual void move() = 0;
  virtual void move(float dist) = 0;

  virtual void turn90Right() = 0;

  virtual void turn90Left() = 0;
  virtual void turn(Orientation orientation) = 0;

  virtual float measureDistance(SensorDirection direction) = 0;

  bool detectWall(SensorDirection direction) {
    auto x = measureDistance(direction);
    return measureDistance(direction) < wallDist_;
  }

  float getTravelDist()                                 { return travelledDist() - travelledDist_; }
  void updateTravelDist()                               { travelledDist_ = travelledDist(); }

  bool detectWallRight()                                { return detectWall(TOPRIGHT) || detectWall(BOTTOMRIGHT); }
  bool detectWallLeft()                                 { return detectWall(TOPLEFT) || detectWall(BOTTOMLEFT); }

  virtual bool touchWallTop() = 0;

  bool createNode() {
    my_assert(visitedNodes_.find(currentNode_) == visitedNodes_.end());

    if (!adjacencyMatrix_.pushNode(*currentNode_))
      return false;
    if (!begin_)
      adjacencyMatrix_.addDistance(*nodeStack_.top(), *currentNode_, getTravelDist());
    else
      begin_ = false;
    updateTravelDist();

    auto node = std::make_shared<Node>(*currentNode_);
    nodeStack_.push(node);

    node->junction.insert({ currentOrientation_.turnBack(), true });
    visitedNodes_.insert(node);

    return true;
  }
  virtual float travelledDist() = 0;

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

  bool backtrack_ = false;
  bool begin_ = true;
};


