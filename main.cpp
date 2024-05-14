#include <SFML/Graphics.hpp>
#include <vector>
#include <ranges>
#include <tuple>

using namespace sf;
template <typename TLambda>
struct Object : Drawable {
  Object(TLambda onClick) : onClick_(onClick) {}

  static bool isMouseOver(RenderWindow* pWindow, const Shape& shape) {
    Vector2f mousePos = pWindow->mapPixelToCoords(Mouse::getPosition(*pWindow));
    FloatRect buttonRect = shape.getGlobalBounds();
    return buttonRect.contains(mousePos);
  }

  template <typename... T>
  std::optional<Drawable> isClicked(RenderWindow* pWindow, const Shape& shape, T... args) {
    if (isMouseOver(pWindow, shape)) {
      if (Mouse::isButtonPressed(Mouse::Left)) {
        return onClick_(args...);
      }
    }
    else {
      return std::nullopt;
    }
  }

  TLambda onClick_;
};
