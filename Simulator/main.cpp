#include <SFML/Graphics.hpp>
#include <vector>
#include <ranges>
#include <tuple>
#include <chrono>

using namespace sf;
using namespace std::chrono_literals;

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

  void draw(RenderTarget& target, RenderStates states) const override { target.draw(*pShape_, states); }


  void keyBoardMove(std::vector<std::shared_ptr<Drawable>>* pDrawables) {
    Vector2f savedPos = pShape_->getPosition();

    if (Keyboard::isKeyPressed(Keyboard::Up))
      move(Vector2f{0, -0.1f});
    if (Keyboard::isKeyPressed(Keyboard::Down))
      move(Vector2f{0, 0.1f});
    if (Keyboard::isKeyPressed(Keyboard::Right))
      move(Vector2f{0.1f, 0});
    if (Keyboard::isKeyPressed(Keyboard::Left))
      move(Vector2f{-0.1f, 0});

    for (auto pDrawable : *pDrawables) {
      if (pDrawable.get() == this)
        continue;
      if (pShape_->getGlobalBounds().intersects(((Object*)(pDrawable.get()))->pShape_->getGlobalBounds()))
        move2(savedPos);
    }
  }

protected:
  std::shared_ptr<Shape> pShape_{nullptr};
};

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

template <typename TLambda>
struct GenerateDrawable : public Clickable<TLambda> {
  GenerateDrawable(Vector2f pos, Vector2f size, TLambda lambda, const std::string& text)
    : Clickable<TLambda>(lambda) {
    Clickable<TLambda>::pShape_ = std::make_shared<CircleShape>(50);
    Clickable<TLambda>::pShape_->setPosition(pos);
    Clickable<TLambda>::pShape_->setFillColor(Color::Red);

    if (!font_.loadFromFile("arial.ttf")) {
      throw std::runtime_error("Failed to load font");
    }

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
  Font font_;
};

struct DistanceSensor : public Object {
  DistanceSensor() = default;

  DistanceSensor(Vector2f direct, Vector2f pos, int radius)
    : direction(direct) {
    Object::pShape_ = std::make_shared<CircleShape>(radius);
    Object::pShape_->setFillColor(Color::Red);
    Object::pShape_->setPosition(pos);
  }

  DistanceSensor& operator = (const DistanceSensor& rhs) {
    this->pShape_ = rhs.pShape_;
    this->direction = rhs.direction;
    return *this;
  }

  std::optional<float> measureDistance(std::vector<std::shared_ptr<Object>>* pWalls) {
    for (auto pWall : *pWalls) {
      if (auto distance = rectangleDistance(*(RectangleShape*)pWall->shape().get()); distance.has_value())
        return distance.value();
    }

    return std::nullopt;
  }

  std::optional<float> rectangleDistance(const RectangleShape& rectangle) {
    // These are the bounds of the rectangle
    float rectLeft = rectangle.getPosition().x;
    float rectTop = rectangle.getPosition().y;
    float rectRight = rectLeft + rectangle.getSize().x;
    float rectBottom = rectTop + rectangle.getSize().y;

    std::vector<float> distances;

    auto pos = pShape_->getPosition();
    pos.x += ((CircleShape*)(pShape_.get()))->getRadius();
    pos.y += ((CircleShape*)(pShape_.get()))->getRadius();

    // Check intersection with each side of the rectangle
    // Left side
    if (direction.x != 0) { // Avoid division by zero
      float t = (rectLeft - pos.x) / direction.x;
      float y = pos.y + t * direction.y;
      if (t >= 0 && y >= rectTop && y <= rectBottom) distances.push_back(t);
    }

    // Right side
    if (direction.x != 0) {
      float t = (rectRight - pos.x) / direction.x;
      float y = pos.y + t * direction.y;
      if (t >= 0 && y >= rectTop && y <= rectBottom) distances.push_back(t);
    }

    // Top side
    if (direction.y != 0) {
      float t = (rectTop - pos.y) / direction.y;
      float x = pos.x + t * direction.x;
      if (t >= 0 && x >= rectLeft && x <= rectRight) distances.push_back(t);
    }

    // Bottom side
    if (direction.y != 0) {
      float t = (rectBottom - pos.y) / direction.y;
      float x = pos.x + t * direction.x;
      if (t >= 0 && x >= rectLeft && x <= rectRight) distances.push_back(t);
    }

    if (distances.empty()) return std::nullopt;

    // Return the smallest positive distance
    return *std::min_element(distances.begin(), distances.end());
  }

    Vector2f direction;
};

template <typename TLambda>
struct Car : public Clickable<TLambda> {
  Car(Vector2f pos, TLambda lambda) : Clickable<TLambda>(lambda) {
    Clickable<TLambda>::pShape_ = std::make_shared<RectangleShape>(Vector2f{ 75, 100 });

    Clickable<TLambda>::pShape_->setFillColor(Color::Blue);
    Clickable<TLambda>::pShape_->setOrigin(Clickable<TLambda>::pShape_->getLocalBounds().width / 2.0f,
        Clickable<TLambda>::pShape_->getLocalBounds().height / 2.0f);
    Clickable<TLambda>::pShape_->setPosition(pos);

    auto bounds = Clickable<TLambda>::pShape_->getGlobalBounds();
    int radius = 10;
    topLeft_ = DistanceSensor(Vector2f(-1, 0), Vector2f(0, 0), radiusSensors);
    topRight_ = DistanceSensor(Vector2f(1, 0), Vector2f(0, 0), radiusSensors);
    bottomLeft_ = DistanceSensor(Vector2f(-1, 0), Vector2f(0, 0), radiusSensors);
    bottomRight_ = DistanceSensor(Vector2f(1, 0), Vector2f(0, 0), radiusSensors);
    front_ = DistanceSensor(Vector2f(0, 1), Vector2f(0, 0), radiusSensors);

    moveSensors();
  }

  void draw(RenderTarget& target, RenderStates states) const override {
    Object::draw(target, states);
    target.draw(topRight_);
    target.draw(topLeft_);
    target.draw(bottomRight_);
    target.draw(bottomLeft_);
    target.draw(front_);
  }

  void move2(Vector2f pos) override {
    Object::move2(pos);
    moveSensors();
  }

  void move(Vector2f pos) override {
    Object::move(pos);
    topRight_.move(pos);
    topLeft_.move(pos);
    bottomRight_.move(pos);
    bottomLeft_.move(pos);
    front_.move(pos);
  }

  void moveSensors() {
    auto bounds = Clickable<TLambda>::pShape_->getGlobalBounds();
    topLeft_.move2(Vector2f(bounds.left - radiusSensors, bounds.top - radiusSensors));
    topRight_.move2(Vector2f(bounds.left - radiusSensors + bounds.width, bounds.top - radiusSensors));
    bottomLeft_.move2(Vector2f(bounds.left - radiusSensors, bounds.top - radiusSensors + bounds.height));
    bottomRight_.move2(Vector2f(bounds.left - radiusSensors + bounds.width, bounds.top - radiusSensors + bounds.height));
    front_.move2(Vector2f(bounds.left + bounds.width / 2 - radiusSensors, bounds.top - radiusSensors));
  }

private:
  DistanceSensor topRight_;
  DistanceSensor topLeft_;
  DistanceSensor bottomRight_;
  DistanceSensor bottomLeft_;
  DistanceSensor front_;

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

  std::vector<std::shared_ptr<Drawable>> drawables;
  std::vector<std::shared_ptr<Drawable>> clickables;
  std::vector<std::shared_ptr<Drawable>> walls;

  drawables.push_back(std::move(horizontalGenerator));
  drawables.push_back(std::move(verticalGenerator));
  drawables.push_back(car);

  clickables = drawables;

  while (window.isOpen()) {
    Event event;
    while (window.pollEvent(event)) {
      if (event.type == Event::Closed)
        window.close();
    }

    window.clear(Color::White);

    for (auto& pDrawable : drawables)
      window.draw(*pDrawable);

    std::vector<std::shared_ptr<Drawable>> appendClickables;
    for (auto& pClicked : clickables) {
      if (auto&& draw = (((Object*)pClicked.get())->isClicked(&window));
          draw != nullptr) {
        drawables.push_back(std::shared_ptr<Drawable>(draw));
        appendClickables.push_back(drawables.back());
      }
    }
    clickables.append_range(appendClickables);
    walls.append_range(appendClickables);

    ((Object*)car.get())->keyBoardMove(&drawables);

    window.display();
  }
}

int main() {
  eventLoop();

  return 0;
}