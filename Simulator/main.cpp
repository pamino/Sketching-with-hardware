#include "Simulator.h"

using namespace sf;

//--------------------------------------------------------------------------------------------------------------
// main
//--------------------------------------------------------------------------------------------------------------

void eventLoop() {
  Simulator sim{};
  while (sim.window.isOpen()) {
    Event event;
    while (sim.window.pollEvent(event)) {
      if (event.type == Event::Closed)
        sim.window.close();
    }

    sim.window.clear(Color::White);

    ((Car*)(sim.car.get()))->update(&sim.walls);

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