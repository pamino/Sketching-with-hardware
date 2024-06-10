#include "Simulator.h"

using namespace sf;

//--------------------------------------------------------------------------------------------------------------
// Object
//--------------------------------------------------------------------------------------------------------------

//------ isMouseOver ------
bool Object::isMouseOver(RenderWindow* pWindow) {
  Vector2f mousePos = pWindow->mapPixelToCoords(Mouse::getPosition(*pWindow));
  FloatRect buttonRect = pShape_->getGlobalBounds();
  return buttonRect.contains(mousePos);
}

//------ isClicked ------
Object* Object::isClicked(RenderWindow* pWindow) {
  if (isMouseOver(pWindow)) {
    if (Mouse::isButtonPressed(Mouse::Left))
      return invoke();
  }
  return nullptr;
}

//------ keyBoardMove ------
void Object::keyBoardMove() {
  if (Keyboard::isKeyPressed(Keyboard::Up))
    backTrackedMove(front * speed_);
  if (Keyboard::isKeyPressed(Keyboard::Down))
    backTrackedMove(-front * speed_);
  if (Keyboard::isKeyPressed(Keyboard::Right))
    backTrackedTurn(true);
  if (Keyboard::isKeyPressed(Keyboard::Left))
    backTrackedTurn(false);
}

bool Object::backTrackedMove(Vector2f direct) {
  Vector2f savedPos = pShape_->getPosition();
  move(direct);

  for (auto& pDrawable : *pWalls) {
    if (pDrawable.get() == this)
      continue;
    if (collides(((Object*)(pDrawable.get()))->pShape_->getGlobalBounds())) {
      move2(savedPos);
      return false;
    }
  }
  return true;
}

bool Object::backTrackedTurn(bool right) {
  if (right)
    turn(true);
  else
    turn(false);

  for (auto& pDrawable : *pWalls) {
    if (pDrawable.get() == this)
      continue;
    if (collides(((Object*)(pDrawable.get()))->pShape_->getGlobalBounds())) {
      if (right)
        turn(false);
      else
        turn(true);
      return false;
    }
  }
  return true;
}

bool Object::backTrackedTurn90(bool right) {
  if (right)
    turn90(true);
  else
    turn90(false);

  for (auto& pDrawable : *pWalls) {
    if (pDrawable.get() == this)
      continue;
    if (collides(((Object*)(pDrawable.get()))->pShape_->getGlobalBounds())) {
      if (right)
        turn90(false);
      else
        turn90(true);
      return false;
    }
  }
  return true;
}

//--------------------------------------------------------------------------------------------------------------
// Clickable
//--------------------------------------------------------------------------------------------------------------

//------ invoke ------
Object* Clickable::invoke() {
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
// GenerateDrawable
//--------------------------------------------------------------------------------------------------------------

//------ GenerateDrawable ------
GenerateDrawable::GenerateDrawable(Vector2f pos, Vector2f size, std::function<Object*(Object*)> lambda, const std::string& text)
  : Clickable(lambda) {
  Clickable::pShape_ = std::shared_ptr<CircleShape>(new CircleShape(50));
  Clickable::pShape_->setPosition(pos);
  Clickable::pShape_->setFillColor(Color::Red);

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
bool GenerateDrawable::delay() const {
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

//------ DistanceSensor ------
DistanceSensor::DistanceSensor(Vector2f direct, Vector2f pos, int radius) {
  Object::pShape_ = std::shared_ptr<CircleShape>(new CircleShape((float)radius));
  Object::pShape_->setFillColor(Color::Red);
  Object::pShape_->setPosition(pos);

  Object::front = direct;
}

//------ operator = ------
DistanceSensor& DistanceSensor::operator = (const DistanceSensor& rhs) {
  this->pShape_ = rhs.pShape_;
  this->front = rhs.front;
  return *this;
}

//------ measureDistance ------
float DistanceSensor::measureDistance() const {
  float ret = 10000;
  for (auto pWall : *pWalls) {
    if (auto distance = rectangleDistance(*(RectangleShape*)((Object*)pWall.get())->shape().get());
        distance.has_value())
      ret = distance.value() < ret ? distance.value() : ret;
  }

  return ret;
}

//------ rectangleDistance ------
std::optional<float> DistanceSensor::rectangleDistance(const RectangleShape& rectangle) const {
  float width = rectangle.getSize().x;
  float height = rectangle.getSize().y;

  float centerX = rectangle.getPosition().x;
  float centerY = rectangle.getPosition().y;

  float rectLeft = centerX - width / 2.0f;
  float rectRight = centerX + width / 2.0f;
  float rectTop = centerY - height / 2.0f;
  float rectBottom = centerY + height / 2.0f;

  std::vector<float> distances;
  Vector2f pos = Object::pShape_->getPosition();
  pos += Vector2f(((CircleShape*)(Object::pShape_.get()))->getRadius(), ((CircleShape*)(Object::pShape_.get()))->getRadius());

  Vector2f direction = Object::front;

  if (direction.x != 0) {
    float tLeft = (rectLeft - pos.x) / direction.x;
    float yLeft = pos.y + tLeft * direction.y;
    if (tLeft >= 0 && yLeft >= rectTop && yLeft <= rectBottom) distances.push_back(tLeft);

    float tRight = (rectRight - pos.x) / direction.x;
    float yRight = pos.y + tRight * direction.y;
    if (tRight >= 0 && yRight >= rectTop && yRight <= rectBottom) distances.push_back(tRight);
  }

  if (direction.y != 0) {
    float tTop = (rectTop - pos.y) / direction.y;
    float xTop = pos.x + tTop * direction.x;
    if (tTop >= 0 && xTop >= rectLeft && xTop <= rectRight) distances.push_back(tTop);

    float tBottom = (rectBottom - pos.y) / direction.y;
    float xBottom = pos.x + tBottom * direction.x;
    if (tBottom >= 0 && xBottom >= rectLeft && xBottom <= rectRight) distances.push_back(tBottom);
  }

  if (!distances.empty()) {
    return *std::min_element(distances.begin(), distances.end());
  }

  return std::nullopt; // No intersection found
}

//--------------------------------------------------------------------------------------------------------------
// Car
//--------------------------------------------------------------------------------------------------------------

//------ Car ------
Car::Car(Vector2f pos, std::function<Object*(Object*)> lambda)
  : Clickable(lambda), sensorsText_(5) {
  Clickable::pShape_ = std::make_shared<RectangleShape>(Vector2f{75, 100});

  Clickable::pShape_->setFillColor(Color::Blue);
  Clickable::pShape_->setOrigin(Clickable::pShape_->getLocalBounds().width / 2.0f,
    Clickable::pShape_->getLocalBounds().height / 2.0f);
  Clickable::pShape_->setPosition(pos);

  auto bounds = Clickable::pShape_->getGlobalBounds();
  int radius = 10;
  sensors.push_back(DistanceSensor(Vector2f(1, 0), Vector2f(0, 0), radiusSensors));
  sensors.push_back(DistanceSensor(Vector2f(-1, 0), Vector2f(0, 0), radiusSensors));
  sensors.push_back(DistanceSensor(Vector2f(1, 0), Vector2f(0, 0), radiusSensors));
  sensors.push_back(DistanceSensor(Vector2f(-1, 0), Vector2f(0, 0), radiusSensors));
  sensors.push_back(DistanceSensor(Vector2f(0, -1), Vector2f(0, 0), radiusSensors));

  int yPos = 300;
  for (auto& text : sensorsText_) {
    text.setFont(font_);
    text.setCharacterSize(24);
    text.setFillColor(Color::Black);
    text.setPosition(Vector2f(1200, yPos));
    yPos -= 50;
  }
  Object::front = Vector2f(0, -1);

  updateSensorPositions();
}

//------ update ------
void Car::update() {
  int i = 0;
  std::vector<std::string> names{"Top: ", "Left Bottom: ", "Right Bottom: ", "Left Top: ", "Right Top: "};
  for (auto&& [sensor, name] : std::views::zip(sensors, std::views::reverse(names))) {
    sensorsText_[i].setString(name + std::to_string(sensor.measureDistance()));
    ++i;
  }
}

//------ draw ------
void Car::draw(RenderTarget& target, RenderStates states) const {
  Object::draw(target, states);
  for (auto& sensor : sensors)
    sensor.draw(target, states);

  for (auto& text : sensorsText_)
    target.draw(text);
}

//------ collides ------
bool Car::collides(FloatRect rect) {
  for (auto& edge : edges()) {
    if (rect.contains(edge))
      return true;
  }
  return false;
}

//------ edges ------
std::vector<Vector2f> Car::edges() {
  RectangleShape car = *((RectangleShape*)Object::pShape_.get());
  return {
    car.getTransform().transformPoint(Vector2f(car.getSize().x, 0)), // top right
    car.getTransform().transformPoint(Vector2f(0, 0)), // top left
    car.getTransform().transformPoint(Vector2f(car.getSize().x, car.getSize().y)), // bottom right
    car.getTransform().transformPoint(Vector2f(0, car.getSize().y)), // bottom left
    car.getTransform().transformPoint(sf::Vector2f(car.getSize().x / 2, 0)) // top middle
  };
}

//------ updateSensorPositions ------
void Car::updateSensorPositions() {
  auto bounds = Clickable::pShape_->getGlobalBounds();
  sensors[0].move2(Vector2f(edges()[0].x - radiusSensors, edges()[0].y - radiusSensors));
  sensors[1].move2(Vector2f(edges()[1].x - radiusSensors, edges()[1].y - radiusSensors));
  sensors[2].move2(Vector2f(edges()[2].x - radiusSensors, edges()[2].y - radiusSensors));
  sensors[3].move2(Vector2f(edges()[3].x - radiusSensors, edges()[3].y - radiusSensors));
  sensors[4].move2(Vector2f(edges()[4].x - radiusSensors, edges()[4].y - radiusSensors));
}

//------ turn ------
void Car::turn(bool right) {
  Object::turn(right);
  if (right)
    ((RectangleShape*)(Object::pShape_.get()))->rotate(Object::turnSpeed_);
  else
    ((RectangleShape*)(Object::pShape_.get()))->rotate(-Object::turnSpeed_);

  updateSensorPositions();
  for (auto& sensor : sensors)
    sensor.turn(right);
}

//------ turn90 ------
void Car::turn90(bool right) {
  Object::turn90(right);
  if (right)
    ((RectangleShape*)(Object::pShape_.get()))->rotate(90);
  else
    ((RectangleShape*)(Object::pShape_.get()))->rotate(-90);

  updateSensorPositions();
  for (auto& sensor : sensors)
    sensor.turn90(right);
}

//--------------------------------------------------------------------------------------------------------------
// Simulator
//--------------------------------------------------------------------------------------------------------------

//------ Simulator ------
Simulator::Simulator() : window(sf::VideoMode(1600, 1200), "Labyrinth") {
  pWalls = &walls;

  auto moveObj = [&](Object* pClickable) -> Object* {
    pClickable->move2(window.mapPixelToCoords(Mouse::getPosition(window)));
    return nullptr; };

  auto createHWall = [moveObj](Object*) { return new Wall(Vector2f{400, 300}, false, moveObj); };
  auto createVWall = [moveObj](Object*) { return new Wall(Vector2f{400, 300}, true, moveObj); };

  std::shared_ptr<Drawable> horizontalGenerator(new GenerateDrawable(Vector2f{50, 50}, Vector2f{50, 50}, createHWall,
    "   generate \nhorizontal wall"));
  std::shared_ptr<Drawable> verticalGenerator(new GenerateDrawable(Vector2f{50, 150}, Vector2f{50, 50}, createVWall,
    "   generate \nvertical wall"));
  car = std::shared_ptr<Drawable>(new Car(Vector2f{500, 500}, moveObj));

  clickables.push_back(std::move(horizontalGenerator));
  clickables.push_back(std::move(verticalGenerator));
  clickables.push_back(car);

  // maze
  std::vector<std::tuple<Vector2f, bool>> maze = {
    {{800, 1000}, false},
    {{600, 1000}, false},
    {{700, 1200}, true},
    {{800, 600}, true},
    {{1000, 800}, true},
    {{1200, 600}, false},
    {{400, 800}, true},
    {{1000, 400}, true},
    {{600, 400}, true},
    {{400, 600}, false},
  };

  for (auto it : maze) {
    clickables.push_back(std::shared_ptr<Drawable>(new Wall(std::get<0>(it), std::get<1>(it), moveObj)));
    walls.push_back(clickables.back());
  }
}
