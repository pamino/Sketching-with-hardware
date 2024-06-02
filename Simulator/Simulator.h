#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <ranges>
#include <tuple>
#include <chrono>
#include <numeric>

using namespace sf;
using namespace std::chrono_literals;

static Font font_;

//------ rotateVector ------
Vector2f rotateVector(const Vector2f& vector, float angleDegrees) {
  Transform rotation;
  rotation.rotate(angleDegrees);
  return rotation.transformPoint(vector);
}

//------ normalizeVector ------
sf::Vector2f normalizeVector(const sf::Vector2f& vector) {
  float magnitude = std::sqrt(vector.x * vector.x + vector.y * vector.y);
  if (magnitude == 0) {
    return sf::Vector2f(0.f, 0.f);
  }
  return sf::Vector2f(vector.x / magnitude, vector.y / magnitude);
}

//--------------------------------------------------------------------------------------------------------------
// Object
//--------------------------------------------------------------------------------------------------------------

struct Object : Drawable {
public:
  bool isMouseOver(RenderWindow* pWindow);
  Object* isClicked(RenderWindow* pWindow);

  virtual Object* invoke()                              { return nullptr; }

  std::shared_ptr<Shape> shape()                        { return pShape_; }

  virtual void move2(Vector2f pos)                      { pShape_->setPosition(pos); }
  virtual void move(Vector2f pos)                       { pShape_->setPosition(pShape_->getPosition() + pos); }
  virtual void turn(bool right) {
    front_ = right ? rotateVector(front_, turnSpeed_) : rotateVector(front_, -turnSpeed_);
    front_ = normalizeVector(front_);
  }

  void keyBoardMove(std::vector<std::shared_ptr<Drawable>>* pDrawables);

  void draw(RenderTarget& target, RenderStates states) const override { target.draw(*pShape_, states); }

  virtual bool collides(FloatRect rect)                 { return pShape_->getGlobalBounds().intersects(rect); }

  float x() const                                       { return pShape_->getPosition().x; }
  float y() const                                       { return pShape_->getPosition().y; }

protected:
  std::shared_ptr<Shape> pShape_{nullptr};
  Vector2f front_{0, -1};
  float turnSpeed_{0.03f};
  float speed_{0.1f};
};

//--------------------------------------------------------------------------------------------------------------
// Clickable
//--------------------------------------------------------------------------------------------------------------

template <typename TLambda>
struct Clickable : Object {
  Clickable(TLambda onClick) : onClick_(onClick) {}

  Object* invoke() override;
  virtual bool delay() const                            { return false; }

private:
  TLambda onClick_;
};

//------ invoke ------
template <typename TLambda>
Object* Clickable<TLambda>::invoke() {
  static Clickable* mutex = this;
  static auto timeSinceLastInvoke = 0ms;
  static auto lastTime = std::chrono::high_resolution_clock::now();

  timeSinceLastInvoke = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - lastTime);
  if (mutex != this && timeSinceLastInvoke < 100ms)
    return nullptr;
  mutex = this;

  timeSinceLastInvoke = 0ms;
  lastTime = std::chrono::high_resolution_clock::now();

  if (!delay())
    return onClick_(this);

  return nullptr;
}

//--------------------------------------------------------------------------------------------------------------
// Wall
//--------------------------------------------------------------------------------------------------------------

template <typename TLambda>
struct Wall : public Clickable<TLambda> {
  Wall(Vector2f pos, bool horizontal, TLambda lambda) : Clickable<TLambda>(lambda) {
    if (horizontal)
      Clickable<TLambda>::pShape_ = std::make_shared<RectangleShape>(Vector2f{ 20, 200 });
    else
      Clickable<TLambda>::pShape_ = std::make_shared<RectangleShape>(Vector2f{ 200, 20 });

    Clickable<TLambda>::pShape_->setFillColor(Color::Black);
    Clickable<TLambda>::pShape_->setOrigin(Clickable<TLambda>::pShape_->getLocalBounds().width / 2.0f,
      Clickable<TLambda>::pShape_->getLocalBounds().height / 2.0f);
    Clickable<TLambda>::pShape_->setPosition(pos);
  }
};

//--------------------------------------------------------------------------------------------------------------
// GenerateDrawable
//--------------------------------------------------------------------------------------------------------------

template <typename TLambda>
struct GenerateDrawable : public Clickable<TLambda> {
  GenerateDrawable(Vector2f pos, Vector2f size, TLambda lambda, const std::string& text);

  void draw(RenderTarget& target, RenderStates states) const override {
    Object::draw(target, states);
    target.draw(text_);
  }

  bool delay() const override;

private:
  Text text_;
};

//------ GenerateDrawable ------
template <typename TLambda>
GenerateDrawable<TLambda>::GenerateDrawable(Vector2f pos, Vector2f size, TLambda lambda, const std::string& text)
  : Clickable<TLambda>(lambda) {
  Clickable<TLambda>::pShape_ = std::make_shared<CircleShape>(50);
  Clickable<TLambda>::pShape_->setPosition(pos);
  Clickable<TLambda>::pShape_->setFillColor(Color::Red);

  text_.setFont(font_);
  text_.setCharacterSize(15); // Adjust the size if necessary
  text_.setFillColor(Color::Black);
  text_.setString(text);

  // Center the text in the circle
  FloatRect textRect = text_.getLocalBounds();
  text_.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
  text_.setPosition(pos + Vector2f(10, 40));
}

//------ delay ------
template <typename TLambda>
bool GenerateDrawable<TLambda>::delay() const {
  static auto start = std::chrono::high_resolution_clock::now();
  static auto end = std::chrono::high_resolution_clock::now() + 500ms;

  end = std::chrono::high_resolution_clock::now();
  if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start) > 500ms) {
    start = std::chrono::high_resolution_clock::now();
    return false;
  }
  return true;
}

//--------------------------------------------------------------------------------------------------------------
// DistanceSensor
//--------------------------------------------------------------------------------------------------------------

struct DistanceSensor : public Object {
  DistanceSensor() = default;

  DistanceSensor(Vector2f direct, Vector2f pos, int radius);

  DistanceSensor& operator = (const DistanceSensor& rhs);

  std::optional<float> measureDistance(std::vector<std::shared_ptr<Drawable>>* pWalls);

  void draw(RenderTarget& target, RenderStates states) const override { Object::draw(target, states); }

  std::optional<float> rectangleDistance(const RectangleShape& rectangle);
};

//--------------------------------------------------------------------------------------------------------------
// Car
//--------------------------------------------------------------------------------------------------------------

template <typename TLambda>
struct Car : public Clickable<TLambda> {
  Car(Vector2f pos, TLambda lambda);

  void update(std::vector<std::shared_ptr<Drawable>>* pWalls);

  void draw(RenderTarget& target, RenderStates states) const override;

  void move2(Vector2f pos) override { Object::move2(pos); updateSensorPositions(); }
  void move(Vector2f pos) override { Object::move(pos); for (auto& sensor : sensors_) sensor.move(pos);}

  bool collides(FloatRect rect) override;

  std::vector<Vector2f> edges();

  void updateSensorPositions();

  void turn(bool right) override;

private:
  std::vector<DistanceSensor> sensors_; // topLeft, topRight, bottomLeft, bottomRight, topMiddle
  std::vector<Text> sensorsText_;

  int radiusSensors = 10;
};

//------ Car ------
template <typename TLambda>
Car<TLambda>::Car(Vector2f pos, TLambda lambda) : Clickable<TLambda>(lambda), sensorsText_(5) {
  Clickable<TLambda>::pShape_ = std::make_shared<RectangleShape>(Vector2f{ 75, 100 });

  Clickable<TLambda>::pShape_->setFillColor(Color::Blue);
  Clickable<TLambda>::pShape_->setOrigin(Clickable<TLambda>::pShape_->getLocalBounds().width / 2.0f,
    Clickable<TLambda>::pShape_->getLocalBounds().height / 2.0f);
  Clickable<TLambda>::pShape_->setPosition(pos);

  auto bounds = Clickable<TLambda>::pShape_->getGlobalBounds();
  int radius = 10;
  sensors_.push_back(DistanceSensor(Vector2f(1, 0), Vector2f(0, 0), radiusSensors));
  sensors_.push_back(DistanceSensor(Vector2f(-1, 0), Vector2f(0, 0), radiusSensors));
  sensors_.push_back(DistanceSensor(Vector2f(1, 0), Vector2f(0, 0), radiusSensors));
  sensors_.push_back(DistanceSensor(Vector2f(-1, 0), Vector2f(0, 0), radiusSensors));
  sensors_.push_back(DistanceSensor(Vector2f(0, -1), Vector2f(0, 0), radiusSensors));

  int yPos = 300;
  for (auto& text : sensorsText_) {
    text.setFont(font_);
    text.setCharacterSize(24);
    text.setFillColor(Color::Black);
    text.setPosition(Vector2f(1200, yPos));
    yPos -= 50;
  }

  updateSensorPositions();
}

//------ update ------
template <typename TLambda>
void Car<TLambda>::update(std::vector<std::shared_ptr<Drawable>>* pWalls) {
  int i = 0;
  std::vector<std::string> names{"Top: ", "Left Bottom: ", "Right Bottom: ", "Left Top: ", "Right Top: " };
  for (auto&& [sensor, name] : std::views::zip(sensors_, std::views::reverse(names))) {
    float distance = sensor.measureDistance(pWalls).value_or(10000.0f);
    sensorsText_[i].setString(name + std::to_string(distance));
    ++i;
  }
}

//------ draw ------
template <typename TLambda>
void Car<TLambda>::draw(RenderTarget& target, RenderStates states) const {
  Object::draw(target, states);
  for (auto& sensor : sensors_)
    sensor.draw(target, states);

  for (auto& text : sensorsText_)
    target.draw(text);
}

//------ collides ------
template <typename TLambda>
bool Car<TLambda>::collides(FloatRect rect) {
  for (auto& edge : edges()) {
    if (rect.contains(edge))
      return true;
  }
  return false;
}

//------ edges ------
template <typename TLambda>
std::vector<Vector2f> Car<TLambda>::edges() {
  RectangleShape car = *((RectangleShape*)Object::pShape_.get());
  return {
    car.getTransform().transformPoint(Vector2f(0, 0)), // top left
    car.getTransform().transformPoint(Vector2f(car.getSize().x, 0)), // top right
    car.getTransform().transformPoint(Vector2f(0, car.getSize().y)), // bottom left
    car.getTransform().transformPoint(Vector2f(car.getSize().x, car.getSize().y)), // bottom right
    car.getTransform().transformPoint(sf::Vector2f(car.getSize().x / 2, 0)) // top middle
  };
}

//------ updateSensorPositions ------
template <typename TLambda>
void Car<TLambda>::updateSensorPositions() {
  auto bounds = Clickable<TLambda>::pShape_->getGlobalBounds();
  sensors_[0].move2(Vector2f(edges()[0].x - radiusSensors, edges()[0].y - radiusSensors));
  sensors_[1].move2(Vector2f(edges()[1].x - radiusSensors, edges()[1].y - radiusSensors));
  sensors_[2].move2(Vector2f(edges()[2].x - radiusSensors, edges()[2].y - radiusSensors));
  sensors_[3].move2(Vector2f(edges()[3].x - radiusSensors, edges()[3].y - radiusSensors));
  sensors_[4].move2(Vector2f(edges()[4].x - radiusSensors, edges()[4].y - radiusSensors));
}

//------ turn ------
template <typename TLambda>
void Car<TLambda>::turn(bool right) {
  Object::turn(right);
  if (right)
    ((RectangleShape*)(Object::pShape_.get()))->rotate(Object::turnSpeed_);
  else
    ((RectangleShape*)(Object::pShape_.get()))->rotate(-Object::turnSpeed_);

  updateSensorPositions();
  for (auto& sensor : sensors_)
    sensor.turn(right);
}

//--------------------------------------------------------------------------------------------------------------
// Simulator
//--------------------------------------------------------------------------------------------------------------

struct Simulator {
  Simulator();

  RenderWindow window;

  std::shared_ptr<Drawable> car;

  std::vector<std::shared_ptr<Drawable>> clickables;
  std::vector<std::shared_ptr<Drawable>> walls;
};

//------ Simulator ------
Simulator::Simulator() : window(sf::VideoMode(1600, 1200), "Labyrinth") {
  auto moveObj = [&](Object* pClickable) -> Object* {
    pClickable->move2(window.mapPixelToCoords(Mouse::getPosition(window)));
    return nullptr; };

  auto createHWall = [moveObj](Object*) { return new Wall(Vector2f{ 400, 300 }, false, moveObj); };
  auto createVWall = [moveObj](Object*) { return new Wall(Vector2f{ 400, 300 }, true, moveObj); };

  std::shared_ptr<Drawable> horizontalGenerator(new GenerateDrawable(Vector2f{ 50, 50 }, Vector2f{ 50, 50 }, createHWall,
    "   generate \nhorizontal wall"));
  std::shared_ptr<Drawable> verticalGenerator(new GenerateDrawable(Vector2f{ 50, 150 }, Vector2f{ 50, 50 }, createVWall,
    "   generate \nvertical wall"));
  car = std::shared_ptr<Drawable>(new Car(Vector2f{500, 500}, moveObj));

  clickables.push_back(std::move(horizontalGenerator));
  clickables.push_back(std::move(verticalGenerator));
  clickables.push_back(car);
}


