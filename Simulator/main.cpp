#include "Simulator.h"
#include "PathfindingSim.h"

using namespace sf;

//--------------------------------------------------------------------------------------------------------------
// main
//--------------------------------------------------------------------------------------------------------------

void eventLoop() {
  Simulator sim{};
  bool automatic = false;
  PathFinderSim pathFind(((Car*)sim.car.get()));

  while (sim.window.isOpen()) {
    Event event;
    while (sim.window.pollEvent(event)) {
      if (event.type == Event::Closed)
        sim.window.close();
    }

    sim.window.clear(Color::White);

    ((Car*)(sim.car.get()))->update();

    for (auto& pDrawable : sim.clickables)
      sim.window.draw(*pDrawable);

    if (Keyboard::isKeyPressed(Keyboard::Space))
      automatic = true;
    if (Keyboard::isKeyPressed(Keyboard::BackSpace))
      automatic = false;

    if (automatic) {
      pathFind.search();
      if (Keyboard::isKeyPressed(Keyboard::F10))
        pathFind.setGoal(Node(sim.window.mapPixelToCoords(Mouse::getPosition()).x, sim.window.mapPixelToCoords(Mouse::getPosition()).y));
    }
    else {
      std::vector<std::shared_ptr<Drawable>> appendClickables;
      for (auto& pClicked : sim.clickables) {
        if (auto&& draw = (((Object*)pClicked.get())->isClicked(&sim.window));
            draw != nullptr) {
          appendClickables.push_back(std::shared_ptr<Drawable>(draw));
        }
      }
      sim.clickables.append_range(appendClickables);
      sim.walls.append_range(appendClickables);

      ((Object*)sim.car.get())->keyBoardMove();
    }
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