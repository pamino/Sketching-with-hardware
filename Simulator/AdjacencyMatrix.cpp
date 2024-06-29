#include "AdjacencyMatrix.h"
#include <cassert>

//--------------------------------------------------------------------------------------------------------------
// AdjacencyMatrix
//--------------------------------------------------------------------------------------------------------------

//------ pushNode ------
bool AdjacencyMatrix::pushNode(const Node& node) {
  if (contains(node))
    return false;

  ++length_;
  for (auto& row : distanceData_)
    row.push_back(std::numeric_limits<float>::infinity());
  distanceData_.push_back(std::vector<float>(length_, std::numeric_limits<float>::infinity()));
  nodes_.push_back(node);

  // Initialize the predecessor matrix
  for (auto& row : predecessor_)
    row.push_back(-1);
  predecessor_.push_back(std::vector<int>(length_, -1));

  // Distance to itself is 0
  distanceData_.back()[length_ - 1] = 0;

  return true;
}

//------ addDistance ------
void AdjacencyMatrix::addDistance(const Node& nodeFrom, const Node& nodeTo, float dist) {
  assert(contains(nodeTo) && contains(nodeFrom));

  int fromIndex = find(nodeFrom).value();
  int toIndex = find(nodeTo).value();

  distanceData_[fromIndex][toIndex] = dist;
  distanceData_[toIndex][fromIndex] = dist;
  predecessor_[fromIndex][toIndex] = fromIndex;
  predecessor_[toIndex][fromIndex] = toIndex;
}

//------ goTo ------
std::vector<Node> const AdjacencyMatrix::goTo(const Node& nodeFrom, const Node nodeTo) {
  assert(contains(nodeTo) && contains(nodeFrom));

  int fromIndex = find(nodeFrom).value();
  int toIndex = find(nodeTo).value();

  if (distanceData_[fromIndex][toIndex] == std::numeric_limits<float>::infinity() || fromIndex == toIndex)
    return {};

  int from = fromIndex;
  int to = toIndex;

  std::vector<Node> ret;

  ret.push_back(nodes_[to]);
  while (true) {
    to = predecessor_[from][to];
    if (from == to)
      break;
    ret.push_back(nodes_[to]);
  }
  std::reverse(ret.begin(), ret.end());
  return ret;
}

//------ find ------
std::optional<int> AdjacencyMatrix::find(Node node) const {
  auto pIt = std::ranges::find_if(nodes_, [&node](Node rhs) { return node == rhs; });
  return pIt == nodes_.end() ? std::nullopt : std::optional(std::distance(nodes_.begin(), pIt));
}


//------ floydWarshall ------
void AdjacencyMatrix::floydWarshall() {
  std::vector<std::vector<float>> dist = distanceData_;

  // Floyd-Warshall algorithm
  for (int k = 0; k < length_; ++k) {
    for (int i = 0; i < length_; ++i) {
      for (int j = 0; j < length_; ++j) {
        if (dist[i][k] < std::numeric_limits<float>::infinity() && dist[k][j] < std::numeric_limits<float>::infinity()) {
          if (dist[i][j] > dist[i][k] + dist[k][j]) {
            dist[i][j] = dist[i][k] + dist[k][j];
            predecessor_[i][j] = predecessor_[k][j];
          }
        }
      }
    }
  }

  // Update the data_ matrix with the shortest paths found
  distanceData_ = dist;
}