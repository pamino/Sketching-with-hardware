#include <SFML/Graphics.hpp>
#include <vector>
#include <ranges>
#include <tuple>
#include <chrono>

using namespace sf;
using namespace std::chrono_literals;

struct Car {

};

struct Clickable : Drawable {
  bool isMouseOver(RenderWindow* pWindow) {
    Vector2f mousePos = pWindow->mapPixelToCoords(Mouse::getPosition(*pWindow));
    FloatRect buttonRect = pShape_->getGlobalBounds();
    return buttonRect.contains(mousePos);
  }

  Clickable* isClicked(RenderWindow* pWindow) {
    if (isMouseOver(pWindow)) {
      if (Mouse::isButtonPressed(Mouse::Left)) {
        return invoke();
      }
    }
    return nullptr;
  }

  virtual Clickable* invoke() = 0;

  void move(Vector2f pos) {
    pShape_->setPosition(pos);
  }

protected:
  std::unique_ptr<Shape> pShape_{nullptr};
};

template <typename TLambda>
struct _Clickable : Clickable {
  _Clickable(TLambda onClick) : onClick_(onClick) {}

  void draw(RenderTarget& target, RenderStates states) const override {
    target.draw(*pShape_, states);
    this->drawMore(target);
  }

  virtual void drawMore(RenderTarget& target) const {}

  Clickable* invoke() override {
    static _Clickable* mutex = this;
    static auto timeSinceLastInvoke = 0ms;
    static auto lastTime = std::chrono::high_resolution_clock::now();

    timeSinceLastInvoke = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - lastTime);
    if (mutex != this && timeSinceLastInvoke < 100ms)
      return nullptr;

    timeSinceLastInvoke = 0ms;
    lastTime = std::chrono::high_resolution_clock::now();

    if (!delay()) {
      mutex = this;
      return onClick_(this);
    }

    return nullptr;
  }

  virtual bool delay() const                            { return false; }

private:
  TLambda onClick_;
};

template <typename TLambda>
struct Wall : public _Clickable<TLambda> {
  Wall(Vector2f pos, bool horizontal, TLambda lambda) : _Clickable<TLambda>(lambda) {
    if (horizontal)
      _Clickable<TLambda>::pShape_ = std::make_unique<RectangleShape>(Vector2f{ 20, 200 });
    else
      _Clickable<TLambda>::pShape_ = std::make_unique<RectangleShape>(Vector2f{ 200, 20 });

    _Clickable<TLambda>::pShape_->setFillColor(Color::Black);
    _Clickable<TLambda>::pShape_->setOrigin(_Clickable<TLambda>::pShape_->getLocalBounds().width / 2.0f,
      _Clickable<TLambda>::pShape_->getLocalBounds().height / 2.0f);
    _Clickable<TLambda>::pShape_->setPosition(pos);
  }
};

template <typename TLambda>
struct GenerateDrawable : public _Clickable<TLambda> {
  GenerateDrawable(Vector2f pos, Vector2f size, TLambda lambda, const std::string& text)
    : _Clickable<TLambda>(lambda) {
    _Clickable<TLambda>::pShape_ = std::make_unique<CircleShape>(50);
    _Clickable<TLambda>::pShape_->setPosition(pos);
    _Clickable<TLambda>::pShape_->setFillColor(Color::Red);

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

  void drawMore(RenderTarget& target) const override {
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

void eventLoop() {
  RenderWindow window(sf::VideoMode(1600, 1200), "Labyrinth");

  auto moveWall = [&window](Clickable* pWall) -> Clickable* {
    pWall->move(window.mapPixelToCoords(Mouse::getPosition(window)));
    return nullptr; };
  auto createHWall = [moveWall](Clickable*) { return new Wall(Vector2f{400, 300}, false, moveWall); };
  auto createVWall = [moveWall](Clickable*) { return new Wall(Vector2f{400, 300}, true, moveWall); };

  auto horizontalGenerator = new GenerateDrawable(Vector2f{ 50, 50 }, Vector2f{ 50, 50 }, createHWall,
      "   generate \nhorizontal wall");
  auto verticalGenerator = new GenerateDrawable(Vector2f{ 50, 150 }, Vector2f{ 50, 50 }, createVWall,
    "   generate \nvertical wall");

  std::vector<std::unique_ptr<Drawable>> drawables;
  std::vector<Drawable*> clickables;

  drawables.push_back(std::unique_ptr<Drawable>((Drawable*)horizontalGenerator));
  clickables.push_back(drawables.back().get());
  drawables.push_back(std::unique_ptr<Drawable>((Drawable*)verticalGenerator));
  clickables.push_back(drawables.back().get());

  while (window.isOpen()) {
    Event event;
    while (window.pollEvent(event)) {
      if (event.type == Event::Closed)
        window.close();
    }

    window.clear(Color::White);

    for (auto& pDrawable : drawables)
      window.draw(*pDrawable);

    std::vector<Drawable*> appendClickables;
    for (auto& clicked : clickables) {
      if (auto draw = (((Clickable*)clicked)->isClicked(&window));
          draw != nullptr) {
        drawables.push_back(std::unique_ptr<Drawable>((Drawable*)draw));
        appendClickables.push_back(drawables.back().get());
      }
    }
    clickables.append_range(appendClickables);

    window.display();
  }
}

int main() {
  eventLoop();

  return 0;
}
