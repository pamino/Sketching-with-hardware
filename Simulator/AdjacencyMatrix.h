#include <vector>
#include <optional>

//--------------------------------------------------------------------------------------------------------------
// Node
//--------------------------------------------------------------------------------------------------------------

struct Node {
  int x;
  int y;

  bool operator == (const Node&) const = default;
};

//--------------------------------------------------------------------------------------------------------------
// AdjacencyMatrix
//--------------------------------------------------------------------------------------------------------------

struct AdjacencyMatrix {
  void pushNode(const Node& node);

  void addDistance(const Node& nodeFrom, const Node& nodeTo, float dist);
  std::optional<std::vector<float>> getFrom(const Node& node) const;
  std::optional<float> getFromTo(const Node& nodeFrom, const Node& nodeTo) const;
  std::optional<std::vector<float>> getTo(const Node& node) const;
  std::optional<float> getToFrom(const Node& nodeTo, const Node& nodeFrom) const;

  std::optional<int> find(Node node) const;

  bool contains(Node node) const { return find(node).has_value(); }

private:
  std::vector<std::vector<float>> data_;
  int length_;
  std::vector<Node> nodes_;
};