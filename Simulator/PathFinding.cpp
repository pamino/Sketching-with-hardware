#include "PathFinding.h"

void PathFinder::depthFirstSearch() {
  switch (currentState_) {
    case State::BEGIN:
      initialize();
      break;
    case State::MOVE_TO_JUNCTION:
      moveToJunction();
      break;
    case State::MOVE_ONTO_JUNCTION:
      moveOntoJunction();
      break;
    case State::HANDLE_JUNCTION:
      handleJunction();
      break;
    case State::HANDLE_OUT_OF_JUNCTION_LEFT:
      handleOutOfJunctionLeft();
      break;
    case State::HANDLE_OUT_OF_JUNCTION_RIGHT:
      handleOutOfJunctionRight();
      break;
    case State::BACKTRACK:
      backtrack();
    case State::FINISHED:
      return;
  }
}

//------ initialize ------
void PathFinder::initialize() {
  wallDist_ = (measureDistance(TOPLEFT) + measureDistance(TOPRIGHT));
  wallDist_ *= 1; // deduct size of car
  // also positionCorrection

  Pos::setTolerance(wallDist_ / 2);
  currentState_ = State::HANDLE_JUNCTION;
}

//------ moveToJunction ------
void PathFinder::moveToJunction() {
  if (nodeStack_.empty()) {
    currentState_ = State::FINISHED;
    return;
  }
  if (backtrack_) {
    if (nodeStack_.size() == 1 && *currentNode_ == Node(0, 0)) {
      currentState_ = State::HANDLE_JUNCTION;
      return;
    }
  }
  if (!detectWallRight() || !detectWallLeft()) {
    currentState_ = State::MOVE_ONTO_JUNCTION;
    return;
  }
  else if (touchWallTop()) {
    turn90Right();
    turn90Right();
    return;
  }
  move();
}

//------ handleJunction ------
void PathFinder::handleJunction() {
  if (backtrack_) {
    currentState_ = State::BACKTRACK;
    return;
  }

  bool unvisitedRight = false;
  bool unvisitedLeft = false;
  bool unvisitedFront = false;
  bool newNodeCreated = false;

  auto pNodeIt = std::ranges::find_if(visitedNodes_, [this](std::shared_ptr<Node> n) { return *currentNode_ == *n; });
  Node* pCurrentNode;
  if (pNodeIt == visitedNodes_.end()) {
    createNode(); pCurrentNode = nodeStack_.top().get(); newNodeCreated = true;
  }
  else
    pCurrentNode = pNodeIt->get();
  if (!detectWallRight()) {
    auto pIt = pCurrentNode->junction.find({ currentOrientation_.turnRight(), false });
    if (pIt == pCurrentNode->junction.end()) {
      pCurrentNode->junction.insert({ currentOrientation_.turnRight(), false });
      unvisitedRight = true;
    }
    else
      unvisitedRight = !std::get<1>(*pIt);
  }
  if (!detectWallLeft()) {
    auto pIt = pCurrentNode->junction.find({ currentOrientation_.turnLeft(), false });
    if (pIt == pCurrentNode->junction.end()) {
      pCurrentNode->junction.insert({ currentOrientation_.turnLeft(), false });
      unvisitedLeft = true;
    }
    else
      unvisitedLeft = !std::get<1>(*pIt);
  }
  if (!detectWall(TOP)) {
    auto pIt = pCurrentNode->junction.find({ currentOrientation_, false });
    if (pIt == pCurrentNode->junction.end()) {
      pCurrentNode->junction.insert({ currentOrientation_, false });
      unvisitedFront = true;
    }
    else
      unvisitedFront = !std::get<1>(*pIt);
  }

  // decides which path to take
  if (unvisitedFront) {
    pCurrentNode->junction.erase({ currentOrientation_, false });
    pCurrentNode->junction.insert({ currentOrientation_, true });

    if (!detectWallRight()) {
      currentState_ = State::HANDLE_OUT_OF_JUNCTION_RIGHT;
      return;
    }
    else {
      currentState_ = State::HANDLE_OUT_OF_JUNCTION_LEFT;
      return;
    }
  }
  else if (unvisitedRight) {
    turn90Right();
    pCurrentNode->junction.erase({ currentOrientation_, false });
    pCurrentNode->junction.insert({ currentOrientation_, true });

    currentState_ = State::HANDLE_OUT_OF_JUNCTION_RIGHT;
    return;
  }
  else if (unvisitedLeft) {
    turn90Left();
    pCurrentNode->junction.erase({ currentOrientation_, false });
    pCurrentNode->junction.insert({ currentOrientation_, true });

    currentState_ = State::HANDLE_OUT_OF_JUNCTION_LEFT;
    return;
  }
  else {
    pCurrentNode->junction.erase({ currentOrientation_.turnBack(), false});
    pCurrentNode->junction.insert({ currentOrientation_.turnBack(), true });

    backtrack_ = true;
    if(newNodeCreated)
      nodeStack_.pop();
    turn90Right();
    turn90Right();
    if (!detectWallRight()) {
      currentState_ = State::HANDLE_OUT_OF_JUNCTION_RIGHT;
    }
    else {
      currentState_ = State::HANDLE_OUT_OF_JUNCTION_LEFT;
    }
    return;
  }
}

//------ moveOntoJunction ------
void PathFinder::moveOntoJunction() {
  static bool begin = true;

  static float travelDistStart = 0;

  if (begin) {
    travelDistStart = getTravelDist();
    begin = false;
  }

  if (getTravelDist() - travelDistStart < (wallDist_ / 4))
    move();
  else {
    begin = true;
    currentState_ = State::HANDLE_JUNCTION;
  }
}

//------ handleUnvisitedRight ------
void PathFinder::handleOutOfJunctionRight() {
  if (detectWall(BOTTOMRIGHT))
    currentState_ = State::MOVE_TO_JUNCTION;
  move();
}

//------ handleUnvisitedLeft ------
void PathFinder::handleOutOfJunctionLeft() {
  if (detectWall(BOTTOMLEFT))
    currentState_ = State::MOVE_TO_JUNCTION;
  move();
}

//------ backtrack ------
void PathFinder::backtrack() {
  auto pNode = nodeStack_.top();
  nodeStack_.pop();

  if (nodeStack_.empty()) {
    currentState_ = State::FINISHED;
    return;
  }

  for (auto j : pNode->junction) {
    if (std::get<1>(j) == false) {
      backtrack_ = false;
      currentState_ = State::HANDLE_JUNCTION;
      return;
    }
  }
  pNode = nodeStack_.top();
  assert(pNode != currentNode_);

  if (currentNode_->x < pNode->x)
    turn(EAST);
  else if (currentNode_->x > pNode->x)
    turn(WEST);
  else if (currentNode_->y < pNode->y)
    turn(SOUTH);
  else if (currentNode_->y > pNode->y)
    turn(NORTH);
  else
    my_assert(false);

  if (!detectWallRight()) {
    currentState_ = State::HANDLE_OUT_OF_JUNCTION_RIGHT;
  }
  else {
    currentState_ = State::HANDLE_OUT_OF_JUNCTION_LEFT;
  }
}