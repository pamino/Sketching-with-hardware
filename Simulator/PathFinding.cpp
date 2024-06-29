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
    case State::BACKTRACK:
      backtrack();
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
      currentState_ = State::WAIT;
      return;
    }
    if (backtrack_) {
      if (nodeStack_.size() == 1 && *currentNode_ == Node(0, 0)) {
        currentState_ = State::HANDLE_JUNCTION;
        return;
      }
    }
  }
  if (!detectWallRight() || !detectWallLeft()) {
    currentState_ = State::MOVE_ONTO_JUNCTION;
    return;
  }
  else if (touchWallTop()) {
    turn90Right();
    turn90Right();
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

//------ handleJunction ------
void PathFinder::handleJunction() {
  if (freePlay_) {
    auto nodes = adjacencyMatrix_.goTo(*currentNode_, goal_.value());
    if (!nodes.empty())
      goTo(nodes[0]);
    else {
      goal_ = std::nullopt;
      currentState_ = State::WAIT;
      return;
    }
    return;
  }
  if (backtrack_) {
    currentState_ = State::BACKTRACK;
    return;
  }

  bool unvisitedRight = false;
  bool unvisitedLeft = false;
  bool unvisitedFront = false;
  bool newNodeCreated = false;

  adjacencyMatrix_.pushNode(*currentNode_);
  if (!begin_)
    adjacencyMatrix_.addDistance(*nodeStack_.top(), *currentNode_, getTravelDist());
  else
    begin_ = false;
  updateTravelDist();

  auto pNodeIt = std::ranges::find_if(visitedNodes_, [this](std::shared_ptr<Node> n) { return *currentNode_ == *n; });
  Node* pCurrentNode;
  if (pNodeIt == visitedNodes_.end()) {
    createNode(); pCurrentNode = nodeStack_.top().get();
  }
  else
    pCurrentNode = pNodeIt->get();

  // adds path to where it came from
  pCurrentNode->junction.erase({ currentOrientation_.turnBack(), false});
  pCurrentNode->junction.insert({ currentOrientation_.turnBack(), true });

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

  for (auto j : pNode->junction) {
    if (std::get<1>(j) == false) {
      backtrack_ = false;
      currentState_ = State::HANDLE_JUNCTION;
      return;
    }
  }
  nodeStack_.pop();

  if (nodeStack_.empty()) {
    currentState_ = State::WAIT;
    return;
  }

  pNode = nodeStack_.top();
  assert(pNode != currentNode_);

  goTo(*pNode);
  return;
}

//------ wait ------
void PathFinder::wait() {
  if (!freePlay_)
    adjacencyMatrix_.floydWarshall();
  freePlay_ = true;
  backtrack_ = false;
  goal_ = setGoal(adjacencyMatrix_.nodes_[4]);
  if (!goal_.has_value()) {
    return;
  }
  else {
    currentState_ = State::HANDLE_JUNCTION;
  }
}

//------ goTo ------
void PathFinder::goTo(const Node& goal) {
  if (currentNode_->x < goal.x)
    turn(EAST);
  else if (currentNode_->x > goal.x)
    turn(WEST);
  else if (currentNode_->y < goal.y)
    turn(SOUTH);
  else if (currentNode_->y > goal.y)
    turn(NORTH);
  else
    assert(false);

  if (!detectWallRight()) {
    currentState_ = State::HANDLE_OUT_OF_JUNCTION_RIGHT;
  }
  else {
    currentState_ = State::HANDLE_OUT_OF_JUNCTION_LEFT;
  }
}

//------ turn ------
void PathFinder::turn(Orientation orientation) {
  if (currentOrientation_.turnRight() == orientation)
    turn90Right();
  else if (currentOrientation_.turnLeft() == orientation)
    turn90Left();
  else if (currentOrientation_.turnBack() == orientation) {
    turn90Right();turn90Right();
  }
}

//------ createNode ------
bool PathFinder::createNode() {
  assert(visitedNodes_.find(currentNode_) == visitedNodes_.end());

  auto node = std::make_shared<Node>(*currentNode_);
  nodeStack_.push(node);

  visitedNodes_.insert(node);

  return true;
}

//------ setGoal ------
std::optional<Node> PathFinder::setGoal(const std::optional<Node>& node) {
  static std::optional<Node> ret = std::nullopt;
  auto temp = ret;
  ret = node;
  return temp;
}

//------ detectWall ------
bool PathFinder::detectWall(SensorDirection direction) {
  auto x = measureDistance(direction);
  return measureDistance(direction) < wallDist_ * 1.5;
}