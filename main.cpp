#include <SFML/Graphics.hpp>
#include <vector>
#include <ranges>
#include <tuple>

using namespace sf;

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
    else {
      return nullptr;
    }
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

  void draw(RenderTarget& target, RenderStates states) const {
    target.draw(*pShape_, states);
  }

  Clickable* invoke()                               { return onClick_(this); }

private:
  TLambda onClick_;
};

template <typename TLambda>
struct Wall : public _Clickable<TLambda> {
  Wall(Vector2f pos, bool horizontal, TLambda lambda) : _Clickable<TLambda>(lambda) {
    if (horizontal)
      _Clickable<TLambda>::pShape_ =  std::make_unique<RectangleShape>(Vector2f{ 10, 100 });
    else
      _Clickable<TLambda>::pShape_ = std::make_unique<RectangleShape>(Vector2f{ 100, 10 });

    _Clickable<TLambda>::pShape_->setFillColor(Color::Black);
    _Clickable<TLambda>::pShape_->setOrigin(_Clickable<TLambda>::pShape_->getLocalBounds().width / 2.0f,
      _Clickable<TLambda>::pShape_->getLocalBounds().height / 2.0f);
    _Clickable<TLambda>::pShape_->setPosition(pos);
  }
};

template <typename TLambda>
struct GenerateDrawable : public _Clickable<TLambda> {
  GenerateDrawable(Vector2f pos, Vector2f size, TLambda lambda)
    : _Clickable<TLambda>(lambda) {
    _Clickable<TLambda>::pShape_ = std::make_unique<CircleShape>(50);
    _Clickable<TLambda>::pShape_->setPosition(pos);
    _Clickable<TLambda>::pShape_->setFillColor(Color::Red);
  }
};

void eventLoop() {
  RenderWindow window(sf::VideoMode(800, 600), "Labyrinth");

  auto moveWall = [&window](Clickable* pWall) -> Clickable* { 
    pWall->move(window.mapPixelToCoords(Mouse::getPosition(window)));
    return nullptr; };
  auto createWall = [moveWall](Clickable*) { return new Wall(Vector2f{400, 300}, true, moveWall); };

  auto horizontalGenerator = new GenerateDrawable(Vector2f{ 50, 50 }, Vector2f{ 50, 50 }, createWall);

  std::vector<std::unique_ptr<Drawable>> drawables;
  std::vector<Drawable*> clickables;

  drawables.push_back(std::unique_ptr<Drawable>((Drawable*)horizontalGenerator));
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
