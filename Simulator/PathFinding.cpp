#include "PathFinding.h"

void PathFinder::search() {
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
    case State::WAIT:
      wait();
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
  if (!freePlay_) {
    if (nodeStack_.empty()) {
      if (currentNode_.x == 0 && currentNode_.y != 0)
        move();
      else
        currentState_ = State::WAIT;
      return;
    }
    if (backtrack_) {
      if (nodeStack_.size() == 1 && currentNode_ == Node(0, 0)) {
        currentState_ = State::HANDLE_JUNCTION;
        return;
      }
    }
  }
  if (goal_ == currentNode_)
    currentState_ = State::WAIT;

  if (!detectWallRight() || !detectWallLeft()) {
    currentState_ = State::MOVE_ONTO_JUNCTION;
    return;
  }
  else if (touchWallTop()) {
    turn(currentOrientation_.turnBack());
    if (!freePlay_)
      backtrack_ = true;
    else
      assert(false);
    return;
  }
  move();
}

//------ moveOntoJunction ------
void PathFinder::moveOntoJunction() {
  static bool begin = true;

  static float travelDistStart = 0;

  if (move_ != 0) {
    move_ -= move();
    if (move_ <= 0)
      move_ = 0;
  }

  if (begin) {
    travelDistStart = getTravelDist();
    begin = false;
  }

  if (move_ == 0 && getTravelDist() - travelDistStart < (wallDist_ / 3.7f)) {
    if (measureDistance(TOPRIGHT) < wallDist_ * 1.5 && measureDistance(TOPLEFT) < wallDist_ * 1.5) {
      turn(currentOrientation_.turnBack());
      move_ = (wallDist_ / 8);
      return;
    }
    move();
  }
  else {
    if (measureDistance(BOTTOMLEFT) < wallDist_ * 1.5 && measureDistance(BOTTOMRIGHT) < wallDist_ * 1.5)
      begin = true;
    else {
      if (freePlay_ && !goal_.has_value()) {
        currentState_ = State::WAIT;
        return;
      }
      begin = true;
      currentState_ = State::HANDLE_JUNCTION;
    }
  }
}

//------ handleJunction ------
void PathFinder::handleJunction() {
  static std::shared_ptr<Node> prevNode = nullptr;

  if (freePlay_) {
    if (!adjacencyMatrix_.contains(currentNode_)) {
      currentState_ = State::MOVE_ONTO_JUNCTION;
      return;
    }
    auto nodes = adjacencyMatrix_.goTo(currentNode_, goal_.value());
    if (!nodes.empty())
      goToNeighbor(nodes[0]);
    else {
      goal_ = std::nullopt;
      currentState_ = State::MOVE_ONTO_JUNCTION;
      return;
    }
    return;
  }

  adjacencyMatrix_.pushNode(currentNode_);

  auto pNodeIt = std::ranges::find_if(visitedNodes_, [this](std::shared_ptr<Node> n) { return currentNode_ == *n; });
  std::shared_ptr<Node> pCurrentNode;
  if (pNodeIt == visitedNodes_.end()) {
    auto node = std::make_shared<Node>(currentNode_);
    nodeStack_.push(node);
    visitedNodes_.insert(node);
    pCurrentNode = nodeStack_.top();
  }
  else
    pCurrentNode = *pNodeIt;

  if (prevNode) {
    if (prevNode != pCurrentNode)
      adjacencyMatrix_.addDistance(*prevNode, currentNode_, getTravelDist());
    prevNode->junction.erase({ currentOrientation_, false});
    prevNode->junction.insert({ currentOrientation_, true });
    pCurrentNode->junction.erase({ currentOrientation_.turnBack(), false});
    pCurrentNode->junction.insert({ currentOrientation_.turnBack(), true });
  }
  else
    begin_ = false;
  updateTravelDist();

  prevNode = pCurrentNode;

  bool unvisitedRight = false;
  bool unvisitedLeft = false;
  bool unvisitedFront = false;
  bool newNodeCreated = false;

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
    turn(currentOrientation_.turnRight());

    currentState_ = State::HANDLE_OUT_OF_JUNCTION_RIGHT;
    return;
  }
  else if (unvisitedLeft) {
    turn(currentOrientation_.turnLeft());

    currentState_ = State::HANDLE_OUT_OF_JUNCTION_LEFT;
    return;
  }
  else {
    if (backtrack_) {
      backtrack();
      return;
    }
    backtrack_ = true;
    if(newNodeCreated)
      nodeStack_.pop();
    turn(currentOrientation_.turnBack());
    if (!detectWallRight()) {
      currentState_ = State::HANDLE_OUT_OF_JUNCTION_RIGHT;
    }
    else {
      currentState_ = State::HANDLE_OUT_OF_JUNCTION_LEFT;
    }
    return;
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
  static std::vector<Node> backtrackStack;
   if (nodeStack_.empty()) {
    currentState_ = State::WAIT;
    return;
  }

  if (backtrackStack.empty()) {
    auto pNodeIt = std::ranges::find_if(visitedNodes_, [this](std::shared_ptr<Node> n) { return currentNode_ == *n; });
    std::shared_ptr<Node> pCurrentNode = *pNodeIt;

    adjacencyMatrix_.floydWarshall();
    if (*nodeStack_.top() == currentNode_)
      nodeStack_.pop();

    if (nodeStack_.empty()) {
      currentState_ = State::WAIT;
      return;
    }

    backtrackStack = adjacencyMatrix_.goTo(currentNode_, *nodeStack_.top());
  }
  if (!backtrackStack.empty()) {
    goToNeighbor(backtrackStack[0]);
    backtrackStack.erase(backtrackStack.begin());
  }
  else {
    goal_ = std::nullopt;
    currentState_ = State::WAIT;
    return;
  }

  return;
}

//------ wait ------
void PathFinder::wait() {
  if (!freePlay_)
    adjacencyMatrix_.floydWarshall();
  freePlay_ = true;
  backtrack_ = false;
  goal_ = setGoal();
  if (!goal_.has_value()) {
    return;
  }
  else {
    currentState_ = State::HANDLE_JUNCTION;
  }
}

//------ goToNeighbor ------
void PathFinder::goToNeighbor(const Node& goal) {
  if (currentNode_.x < goal.x)
    turn(EAST);
  else if (currentNode_.x > goal.x)
    turn(WEST);
  else if (currentNode_.y < goal.y)
    turn(SOUTH);
  else if (currentNode_.y > goal.y)
    turn(NORTH);
  else
    assert(false);

  if (!detectWall(BOTTOMRIGHT)) {
    currentState_ = State::HANDLE_OUT_OF_JUNCTION_RIGHT;
  }
  else {
    currentState_ = State::HANDLE_OUT_OF_JUNCTION_LEFT;
  }
}

//------ turn ------
bool PathFinder::turn(Orientation orientation) {
  if (currentOrientation_.turnRight() == orientation) {
    if (turn90RightImpl())
      currentOrientation_ = currentOrientation_.turnRight();
    else return false;
  }
  else if (currentOrientation_.turnLeft() == orientation) {
    if (turn90LeftImpl())
      currentOrientation_ = currentOrientation_.turnLeft();
    else return false;
  }
  else if (currentOrientation_.turnBack() == orientation) {
    bool ret = turn(currentOrientation_.turnRight());
    return ret && turn(currentOrientation_.turnRight());
  }
  return true;
}

//------ setGoal ------
std::optional<Node> PathFinder::setGoal(const std::optional<Node>& goal) {
  static std::optional<Node> _goal = std::nullopt;
  std::optional<Node> ret = std::nullopt;
  if (!goal.has_value()) {
    ret = _goal;
    _goal = std::nullopt;
  }
  else if (!_goal.has_value())
    _goal = goal;
  return ret;
}

//------ detectWall ------
bool PathFinder::detectWall(SensorDirection direction) {
  auto x = measureDistance(direction);
  return measureDistance(direction) < wallDist_ * 1.5;
}