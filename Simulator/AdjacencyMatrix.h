#pragma once

#include <vector>
#include <optional>
#include <set>
#include "Sensor.h"

enum CardinalOrientation {
  NORTH,
  EAST,
  SOUTH,
  WEST,
};

struct Orientation {
  using is_transparent = std::true_type;

  CardinalOrientation o;

  Orientation(Orientation::CardinalOrientation o) : o(o) {}

  Orientation turnRight() const { if (o == WEST) return NORTH; else return (CardinalOrientation)(o + 1); }
  Orientation turnLeft() const { if (o == NORTH) return WEST; else return (CardinalOrientation)(o - 1); }
  Orientation turnBack() const { return turnRight().turnRight(); }

  auto operator <=> (const Orientation& rhs) const = default;
};

struct OrientationComparator {
  bool operator () (const Orientation& lhs, const std::tuple<Orientation, bool>& rhs) const { return lhs.o < std::get<0>(rhs); }
  bool operator () (const std::tuple<Orientation, bool>& lhs, const Orientation& rhs) const { return std::get<0>(lhs) < rhs.o; }
  bool operator () (const std::tuple<Orientation, bool>& lhs, const std::tuple<Orientation, bool>& rhs) const { return std::get<0>(lhs) < std::get<0>(rhs).o; }
};

//--------------------------------------------------------------------------------------------------------------
// Pos
//--------------------------------------------------------------------------------------------------------------


struct Pos {
  static inline float tolerance_{ 0.0f };

  using is_transparent = std::true_type;

  Pos(float f) : val(f) {}
  Pos() = default;

  bool operator == (const Pos& rhs) const { return val < rhs.val + Pos::tolerance_ && val > rhs.val - Pos::tolerance_; }
  bool operator < (const Pos& rhs) const { return val < rhs.val - Pos::tolerance_; }
  bool operator > (const Pos& rhs) const { return val > rhs.val + Pos::tolerance_; }

  float operator + (const Pos& rhs) const { return val + rhs.val; }
  Pos& operator += (const Pos& rhs) { val += rhs.val; return *this; }

  static void setTolerance(float tolerance) { tolerance_ = tolerance; }

  float val;
};

//--------------------------------------------------------------------------------------------------------------
// Node
//--------------------------------------------------------------------------------------------------------------

struct Node {
  Node(Pos x, Pos y) : x(x), y(y) {}
  Node() = default;

  std::set<std::tuple<Orientation, /*visited*/ bool>, OrientationComparator> junction;

  bool operator == (const Node& rhs) const { return x == rhs.x && y == rhs.y; };

  Pos x;
  Pos y;
};

//--------------------------------------------------------------------------------------------------------------
// AdjacencyMatrix
//--------------------------------------------------------------------------------------------------------------

struct AdjacencyMatrix {
  bool pushNode(const Node& node);

  void addDistance(const Node& nodeFrom, const Node& nodeTo, float dist);

  std::optional<int> find(const Node& node) const;

  bool contains(Node node) const {
    return find(node).has_value();
  }

  std::vector<Node> const goTo(const Node& from, const Node& to);

  void floydWarshall();

  std::vector<Node> nodes_; // order of Nodes

private:
  std::vector<std::vector<float>> distanceData_;
  int length_{0};
  std::vector<std::vector<int>> predecessor_;
};