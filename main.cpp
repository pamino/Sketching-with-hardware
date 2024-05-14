#include <SFML/Graphics.hpp>
#include <vector>
#include <ranges>
#include <tuple>

using namespace sf;
template <typename TLambda>
struct Object : Drawable {
  Object(TLambda onClick) : onClick_(onClick) {}

  bool isMouseOver(RenderWindow* pWindow) {
    Vector2f mousePos = pWindow->mapPixelToCoords(Mouse::getPosition(*pWindow));
    FloatRect buttonRect = pShape_->getGlobalBounds();
    return buttonRect.contains(mousePos);
  }

  template <typename... T>
  std::unique_ptr<Drawable> isClicked(RenderWindow* pWindow, T... args) {
    if (isMouseOver(pWindow)) {
      if (Mouse::isButtonPressed(Mouse::Left)) {
        return std::unique_ptr<Drawable>((Drawable*)onClick_(args...));
      }
    }
    else {
      return nullptr;
    }
  }

  void draw(RenderTarget& target, RenderStates states) const {
    target.draw(*pShape_, states);
  }

  std::unique_ptr<Shape> pShape_;

private:
  TLambda onClick_;
};

template <typename TLambda>
struct Wall : public Object<TLambda> {
  Wall(Vector2f pos, bool horizontal, TLambda lambda) : Object<TLambda>(lambda) {
    if (horizontal)
      Object<TLambda>::pShape_ =  std::make_unique<RectangleShape>(Vector2f{ 10, 100 });
    else
      Object<TLambda>::pShape_ = std::make_unique<RectangleShape>(Vector2f{ 100, 10 });

    Object<TLambda>::pShape_->setFillColor(Color::Black);
    Object<TLambda>::pShape_->setOrigin(Object<TLambda>::pShape_->getLocalBounds().width / 2.0f,
        Object<TLambda>::pShape_->getLocalBounds().height / 2.0f);
    Object<TLambda>::pShape_->setPosition(pos);
  }

  void move(Vector2f pos) {
    Object::pShape_->setPosition(pos);
  }
};

template <typename TLambda>
struct GenerateDrawable : public Object<TLambda> {
  GenerateDrawable(Vector2f pos, Vector2f size, TLambda lambda)
    : Object<TLambda>(lambda) {
    Object<TLambda>::pShape_ = std::make_unique<CircleShape>(50);
    Object<TLambda>::pShape_->setPosition(pos);
    Object<TLambda>::pShape_->setFillColor(Color::Red);
  }
};

void eventLoop() {
  auto moveWall = []() {};
  auto createWall = [&moveWall]() { return new Wall(Vector2f{400, 300}, true, moveWall); };

  auto horizontalGenerator = new GenerateDrawable(Vector2f{ 50, 50 }, Vector2f{ 50, 50 },
      createWall);

  std::vector<std::unique_ptr<Drawable>> objects;
  objects.push_back(std::unique_ptr<Drawable>((Drawable*)(horizontalGenerator)));

  RenderWindow window(sf::VideoMode(800, 600), "Labyrinth");
  while (window.isOpen()) {
    Event event;
    while (window.pollEvent(event)) {
      if (event.type == Event::Closed)
        window.close();
    }

    window.clear(Color::White);
    for (auto& pObject : objects)
      window.draw(*pObject);
    if (auto draw = horizontalGenerator->isClicked(&window);
        draw != nullptr)
      objects.push_back(std::move(draw));
    window.display();
  }
}

int main() {
  eventLoop();

  return 0;
}
