#include <SFML/Graphics.hpp>
#include <vector>
#include <ranges>
#include <tuple>

using namespace sf;

struct Car {

};

struct ClickableBase : Drawable {
  bool isMouseOver(RenderWindow* pWindow) {
    Vector2f mousePos = pWindow->mapPixelToCoords(Mouse::getPosition(*pWindow));
    FloatRect buttonRect = pShape_->getGlobalBounds();
    return buttonRect.contains(mousePos);
  }

  ClickableBase* isClicked(RenderWindow* pWindow) {
    if (isMouseOver(pWindow)) {
      if (Mouse::isButtonPressed(Mouse::Left)) {
        return invoke();
      }
    }
    else {
      return nullptr;
    }
  }

  virtual ClickableBase* invoke() = 0;

protected:
  std::unique_ptr<Shape> pShape_{nullptr};
};

template <typename TLambda>
struct Clickable : ClickableBase {
  Clickable(TLambda onClick) : onClick_(onClick) {}

  void draw(RenderTarget& target, RenderStates states) const {
    target.draw(*pShape_, states);
  }

  ClickableBase* invoke()                               { return onClick_(); }

private:
  TLambda onClick_;
};

template <typename TLambda>
struct Wall : public Clickable<TLambda> {
  Wall(Vector2f pos, bool horizontal, TLambda lambda) : Clickable<TLambda>(lambda) {
    if (horizontal)
      Clickable<TLambda>::pShape_ =  std::make_unique<RectangleShape>(Vector2f{ 10, 100 });
    else
      Clickable<TLambda>::pShape_ = std::make_unique<RectangleShape>(Vector2f{ 100, 10 });

    Clickable<TLambda>::pShape_->setFillColor(Color::Black);
    Clickable<TLambda>::pShape_->setOrigin(Clickable<TLambda>::pShape_->getLocalBounds().width / 2.0f,
      Clickable<TLambda>::pShape_->getLocalBounds().height / 2.0f);
    Clickable<TLambda>::pShape_->setPosition(pos);
  }

  void move(Vector2f pos) {
    Clickable<TLambda>::pShape_->setPosition(pos);
  }
};

template <typename TLambda>
struct GenerateDrawable : public Clickable<TLambda> {
  GenerateDrawable(Vector2f pos, Vector2f size, TLambda lambda)
    : Clickable<TLambda>(lambda) {
    Clickable<TLambda>::pShape_ = std::make_unique<CircleShape>(50);
    Clickable<TLambda>::pShape_->setPosition(pos);
    Clickable<TLambda>::pShape_->setFillColor(Color::Red);
  }
};

void eventLoop() {
  auto moveWall = []() -> ClickableBase* { return nullptr; };
  auto createWall = [moveWall]() { return new Wall(Vector2f{400, 300}, true, moveWall); };

  auto horizontalGenerator = new GenerateDrawable(Vector2f{ 50, 50 }, Vector2f{ 50, 50 }, createWall);

  std::vector<std::unique_ptr<Drawable>> drawables;
  std::vector<Drawable*> clickables;

  drawables.push_back(std::unique_ptr<Drawable>((Drawable*)horizontalGenerator));
  clickables.push_back(drawables.back().get());

  RenderWindow window(sf::VideoMode(800, 600), "Labyrinth");
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
      if (auto draw = (((ClickableBase*)clicked)->isClicked(&window));
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
