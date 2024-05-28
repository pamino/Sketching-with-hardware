#include <SFML/Graphics.hpp>
#include <vector>
#include <ranges>
#include <tuple>
#include <chrono>
#include <numeric>

using namespace sf;
using namespace std::chrono_literals;

static Font font_;

Vector2f rotateVector(const Vector2f& vector, float angleDegrees) {
  Transform rotation;
  rotation.rotate(angleDegrees);  // Rotate transform by angle in degrees
  return rotation.transformPoint(vector);  // Apply rotation to the vector
}

sf::Vector2f normalizeVector(const sf::Vector2f& vector) {
  float magnitude = std::sqrt(vector.x * vector.x + vector.y * vector.y);
  if (magnitude == 0) {
    return sf::Vector2f(0.f, 0.f); // Return zero vector if original vector is zero vector
  }
  return sf::Vector2f(vector.x / magnitude, vector.y / magnitude);
}

// ------ Object ------
struct Object : Drawable {
  bool isMouseOver(RenderWindow* pWindow) {
    Vector2f mousePos = pWindow->mapPixelToCoords(Mouse::getPosition(*pWindow));
    FloatRect buttonRect = pShape_->getGlobalBounds();
    return buttonRect.contains(mousePos);
  }

  Object* isClicked(RenderWindow* pWindow) {
    if (isMouseOver(pWindow)) {
      if (Mouse::isButtonPressed(Mouse::Left))
        return invoke();
    }
    return nullptr;
  }

  virtual Object* invoke()                              { return nullptr; }

  std::shared_ptr<Shape> shape()                        { return pShape_; }

  virtual void move2(Vector2f pos)                      { pShape_->setPosition(pos); }
  virtual void move(Vector2f pos)                       { pShape_->setPosition(pShape_->getPosition() + pos); }
  virtual void turn(bool right) {
    if (right)
      front_ = rotateVector(front_, turnSpeed_);
    else
      front_ = rotateVector(front_, -turnSpeed_);

    front_ = normalizeVector(front_);
  }

  void draw(RenderTarget& target, RenderStates states) const override { target.draw(*pShape_, states); }


  void keyBoardMove(std::vector<std::shared_ptr<Drawable>>* pDrawables) {
    Vector2f savedPos = pShape_->getPosition();

    if (Keyboard::isKeyPressed(Keyboard::Up))
      move(front_ * speed_);
    if (Keyboard::isKeyPressed(Keyboard::Down))
      move(-front_ * speed_);
    if (Keyboard::isKeyPressed(Keyboard::Right))
      turn(true);
    if (Keyboard::isKeyPressed(Keyboard::Left))
      turn(false);

    for (auto& pDrawable : *pDrawables) {
      if (pDrawable.get() == this)
        continue;
      if (collides(((Object*)(pDrawable.get()))->pShape_->getGlobalBounds())) {
        move2(savedPos);
        if (Keyboard::isKeyPressed(Keyboard::Right))
          turn(false);
        if (Keyboard::isKeyPressed(Keyboard::Left))
          turn(true);
      }
    }
  }

  virtual bool collides(FloatRect rect) { return pShape_->getGlobalBounds().intersects(rect); }

  float x() { return pShape_->getPosition().x; }
  float y() { return pShape_->getPosition().y; }

protected:
  std::shared_ptr<Shape> pShape_{nullptr};
  Vector2f front_{0, -1};
  float turnSpeed_{0.03f};
  float speed_{0.1f};
};

// ------ Clickable ------
template <typename TLambda>
struct Clickable : Object {
  Clickable(TLambda onClick) : onClick_(onClick) {}

  Object* invoke() override {
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

  virtual bool delay() const                            { return false; }

private:
  TLambda onClick_;
};

// ------ Wall ------
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

// ------ GenerateDrawable ------
template <typename TLambda>
struct GenerateDrawable : public Clickable<TLambda> {
  GenerateDrawable(Vector2f pos, Vector2f size, TLambda lambda, const std::string& text)
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
    text_.setPosition(pos + Vector2f(50, 50));
  }

  void draw(RenderTarget& target, RenderStates states) const override {
    Object::draw(target, states);
    // target.draw(*Object::pShape_);
    target.draw(text_);
  }

  bool delay() const override {
    static auto start = std::chrono::high_resolution_clock::now();
    static auto end = std::chrono::high_resolution_clock::now() + 500ms;

    end = std::chrono::high_resolution_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start) > 500ms) {
      start = std::chrono::high_resolution_clock::now();
      return false;
    }
    return true;
  }

private:
  Text text_;
};

// ------ DistanceSensor ------
struct DistanceSensor : public Object {
  DistanceSensor() = default;

  DistanceSensor(Vector2f direct, Vector2f pos, int radius) {
    Object::pShape_ = std::make_shared<CircleShape>(radius);
    Object::pShape_->setFillColor(Color::Red);
    Object::pShape_->setPosition(pos);

    Object::front_ = direct;
  }

  DistanceSensor& operator = (const DistanceSensor& rhs) {
    this->pShape_ = rhs.pShape_;
    this->front_ = rhs.front_;
    return *this;
  }

  std::optional<float> measureDistance(std::vector<std::shared_ptr<Drawable>>* pWalls) {
    for (auto pWall : *pWalls) {
      if (auto distance = rectangleDistance(*(RectangleShape*)((Object*)pWall.get())->shape().get());
          distance.has_value())
        return distance.value();
    }

    return std::nullopt;
  }

  void draw(RenderTarget& target, RenderStates states) const override {
    Object::draw(target, states);

  }

  std::optional<float> rectangleDistance(const RectangleShape& rectangle) {
    // Bounds of the rectangle
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

    // Direction of the sensor
    Vector2f direction = Object::front_;

    // Check each side of the rectangle for intersections
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

    // Return the smallest distance if any were found
    if (!distances.empty()) {
      return *std::min_element(distances.begin(), distances.end());
    }

    return std::nullopt; // No intersection found
  }

};

// ------ Car ------
template <typename TLambda>
struct Car : public Clickable<TLambda> {
  Car(Vector2f pos, TLambda lambda) : Clickable<TLambda>(lambda), sensorsText_(5) {
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

  void update(std::vector<std::shared_ptr<Drawable>>* pWalls) {
    int i = 0;
    std::vector<std::string> names{"Top: ", "Left Top: ", "Right Top: ", "Left Bottom: ", "Right Bottom: "};
    for (auto&& [sensor, name] : std::views::zip(sensors_, std::views::reverse(names))) {
      float distance = sensor.measureDistance(pWalls).value_or(10000.0f);
      sensorsText_[i].setString(name + std::to_string(distance));
      ++i;
    }
  }

  void draw(RenderTarget& target, RenderStates states) const override {
    Object::draw(target, states);
    for (auto& sensor : sensors_)
      sensor.draw(target, states);

    for (auto& text : sensorsText_)
      target.draw(text);
  }

  void move2(Vector2f pos) override {
    Object::move2(pos);
    updateSensorPositions();
  }

  void move(Vector2f pos) override {
    Object::move(pos);
    for (auto sensor : sensors_)
      sensor.move(pos);
  }

  bool collides(FloatRect rect) override {
    for (auto& edge : edges()) {
      if (rect.contains(edge))
        return true;
    }
    return false;

  }

  std::vector<Vector2f> edges() {
    RectangleShape car = *((RectangleShape*)Object::pShape_.get());
    return {
      car.getTransform().transformPoint(Vector2f(0, 0)), // top left
      car.getTransform().transformPoint(Vector2f(car.getSize().x, 0)), // top right
      car.getTransform().transformPoint(Vector2f(0, car.getSize().y)), // bottom left
      car.getTransform().transformPoint(Vector2f(car.getSize().x, car.getSize().y)), // bottom right
      car.getTransform().transformPoint(sf::Vector2f(car.getSize().x / 2, 0)) // top middle
    };
  }

  void updateSensorPositions() {
    auto bounds = Clickable<TLambda>::pShape_->getGlobalBounds();
    sensors_[0].move2(Vector2f(edges()[0].x - radiusSensors, edges()[0].y - radiusSensors));
    sensors_[1].move2(Vector2f(edges()[1].x - radiusSensors, edges()[1].y - radiusSensors));
    sensors_[2].move2(Vector2f(edges()[2].x - radiusSensors, edges()[2].y - radiusSensors));
    sensors_[3].move2(Vector2f(edges()[3].x - radiusSensors, edges()[3].y - radiusSensors));
    sensors_[4].move2(Vector2f(edges()[4].x - radiusSensors, edges()[4].y - radiusSensors));
  }

  void turn(bool right) override {
    Object::turn(right);
    if (right)
      ((RectangleShape*)(Object::pShape_.get()))->rotate(Object::turnSpeed_);
    else
      ((RectangleShape*)(Object::pShape_.get()))->rotate(-Object::turnSpeed_);

    updateSensorPositions();
    for (auto& sensor : sensors_)
      sensor.turn(right);
  }

private:
  std::vector<DistanceSensor> sensors_; // topLeft, topRight, bottomLeft, bottomRight, topMiddle
  std::vector<Text> sensorsText_;

  int radiusSensors = 10;
};

void eventLoop() {
  RenderWindow window(sf::VideoMode(1600, 1200), "Labyrinth");

  auto moveObj = [&window](Object* pClickable) -> Object* {
    pClickable->move2(window.mapPixelToCoords(Mouse::getPosition(window)));
    return nullptr; };

  auto createHWall = [moveObj](Object*) { return new Wall(Vector2f{ 400, 300 }, false, moveObj); };
  auto createVWall = [moveObj](Object*) { return new Wall(Vector2f{ 400, 300 }, true, moveObj); };

  std::shared_ptr<Drawable> horizontalGenerator{new GenerateDrawable(Vector2f{ 50, 50 }, Vector2f{ 50, 50 }, createHWall,
      "   generate \nhorizontal wall")};
  std::shared_ptr<Drawable> verticalGenerator{new GenerateDrawable(Vector2f{ 50, 150 }, Vector2f{ 50, 50 }, createVWall,
    "   generate \nvertical wall")};
  std::shared_ptr<Drawable> car{new Car(Vector2f{500, 500}, moveObj)};

  std::vector<std::shared_ptr<Drawable>> clickables;
  std::vector<std::shared_ptr<Drawable>> walls;

  clickables.push_back(std::move(horizontalGenerator));
  clickables.push_back(std::move(verticalGenerator));
  clickables.push_back(car);

  while (window.isOpen()) {
    Event event;
    while (window.pollEvent(event)) {
      if (event.type == Event::Closed)
        window.close();
    }

    window.clear(Color::White);

    ((Car<decltype(moveObj)>*)(car.get()))->update(&walls);

    for (auto& pDrawable : clickables)
      window.draw(*pDrawable);

    std::vector<std::shared_ptr<Drawable>> appendClickables;
    for (auto& pClicked : clickables) {
      if (auto&& draw = (((Object*)pClicked.get())->isClicked(&window));
          draw != nullptr) {

        appendClickables.push_back(std::shared_ptr<Drawable>(draw));
      }
    }
    clickables.append_range(appendClickables);
    walls.append_range(appendClickables);

    ((Object*)car.get())->keyBoardMove(&clickables);

    window.display();
  }
}

int main() {
  if (!font_.loadFromFile("arial.ttf")) {
    throw std::runtime_error("Failed to load font");
  }
  eventLoop();

  return 0;
}
