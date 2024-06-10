#include "AdjacencyMatrix.h"
#include <cassert>

//--------------------------------------------------------------------------------------------------------------
// AdjacencyMatrix
//--------------------------------------------------------------------------------------------------------------

//------ pushNode ------
bool AdjacencyMatrix::pushNode(const Node& node) {
  if (contains(node)) return false;
  ++length_;
  for (auto& row : data_)
    row.push_back(0);
  data_.push_back(std::vector<float>(length_, 0));
  nodes_.push_back(node);

  return true;
}

//------ addDistance ------
void AdjacencyMatrix::addDistance(const Node& nodeFrom, const Node& nodeTo, float dist) {
  my_assert(contains(nodeTo) && contains(nodeFrom));
  data_[find(nodeFrom).value()][find(nodeTo).value()] = dist;
  data_[find(nodeTo).value()][find(nodeFrom).value()] = dist;
}

//------ getFrom ------
std::optional<std::vector<float>> AdjacencyMatrix::getFrom (const Node& node) const {
  if (!contains(node)) return std::nullopt;
  return data_[find(node).value()];
}

//------ getFromTo ------
std::optional<float> AdjacencyMatrix::getFromTo(const Node& nodeFrom, const Node& nodeTo) const {
  if (!contains(nodeTo) || !contains(nodeFrom)) return std::nullopt;
  return getFrom(nodeFrom).value()[find(nodeTo).value()];
}

//------ getTo ------
std::optional<std::vector<float>> AdjacencyMatrix::getTo (const Node& node) const {
  if (!contains(node)) return std::nullopt;
  std::vector<float> ret;
  for (auto& dist : data_)
    ret.push_back(dist[find(node).value()]);
  return ret;
}

//------ getToFrom ------
std::optional<float> AdjacencyMatrix::getToFrom (const Node& nodeTo, const Node& nodeFrom) const {
  if (!contains(nodeTo) || !contains(nodeFrom)) return std::nullopt;
  return getTo(nodeTo).value()[find(nodeFrom).value()];
}

//------ find ------
std::optional<int> AdjacencyMatrix::find(Node node) const {
  auto pIt = std::ranges::find_if(nodes_, [&node](Node rhs) { return node == rhs; });
  return pIt == nodes_.end() ? std::nullopt : std::optional(std::distance(nodes_.begin(), pIt));
}