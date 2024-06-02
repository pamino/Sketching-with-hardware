#include "Simulator.h"

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
void Object::keyBoardMove(std::vector<std::shared_ptr<Drawable>>* pDrawables) {
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

//--------------------------------------------------------------------------------------------------------------
// DistanceSensor
//--------------------------------------------------------------------------------------------------------------

//------ DistanceSensor ------
DistanceSensor::DistanceSensor(Vector2f direct, Vector2f pos, int radius) {
  Object::pShape_ = std::make_shared<CircleShape>(radius);
  Object::pShape_->setFillColor(Color::Red);
  Object::pShape_->setPosition(pos);

  Object::front_ = direct;
}

//------ operator = ------
DistanceSensor& DistanceSensor::operator = (const DistanceSensor& rhs) {
  this->pShape_ = rhs.pShape_;
  this->front_ = rhs.front_;
  return *this;
}

//------ measureDistance ------
std::optional<float> DistanceSensor::measureDistance(std::vector<std::shared_ptr<Drawable>>* pWalls) {
  for (auto pWall : *pWalls) {
    if (auto distance = rectangleDistance(*(RectangleShape*)((Object*)pWall.get())->shape().get());
      distance.has_value())
      return distance.value();
  }

  return std::nullopt;
}

//------ rectangleDistance ------
std::optional<float> DistanceSensor::rectangleDistance(const RectangleShape& rectangle) {
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

  Vector2f direction = Object::front_;

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

static Simulator sim{};

static const auto moveObj = [&](Object* pClickable) -> Object* {
  pClickable->move2(sim.window.mapPixelToCoords(Mouse::getPosition(sim.window)));
  return nullptr; };

void eventLoop() {
  while (sim.window.isOpen()) {
    Event event;
    while (sim.window.pollEvent(event)) {
      if (event.type == Event::Closed)
        sim.window.close();
    }

    sim.window.clear(Color::White);

    ((Car<decltype(moveObj)>*)(sim.car.get()))->update(&sim.walls);

    for (auto& pDrawable : sim.clickables)
      sim.window.draw(*pDrawable);

    std::vector<std::shared_ptr<Drawable>> appendClickables;
    for (auto& pClicked : sim.clickables) {
      if (auto&& draw = (((Object*)pClicked.get())->isClicked(&sim.window));
          draw != nullptr) {

        appendClickables.push_back(std::shared_ptr<Drawable>(draw));
      }
    }
    sim.clickables.append_range(appendClickables);
    sim.walls.append_range(appendClickables);

    ((Object*)sim.car.get())->keyBoardMove(&sim.clickables);

    sim.window.display();
  }
}

int main() {
  if (!font_.loadFromFile("arial.ttf")) {
    throw std::runtime_error("Failed to load font");
  }
  eventLoop();

  return 0;
}
