#include "Simulator.h"
#include "PathfindingSim.h"

#include <fstream>

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
    if (Keyboard::isKeyPressed(Keyboard::F2)) {
      std::ofstream file;
      file.open("C:/Users/Admin/Documents/Sketching with hardware/labyrinth.txt");
      for (auto wall : sim.walls) {
        file << "{{ " << ((Object*)(wall.get()))->x() << ", " << ((Object*)(wall.get()))->y() << " }, " << ((Wall*)(wall.get()))->horizontal() << " }," << "\n";
      }
      file.close();
    }

    if (automatic) {
      pathFind.search();
      if (auto pNode = pathFind.newNode()) {
        sim.clickables.push_back(std::shared_ptr<Drawable>(new DrawNode(((Object*)sim.car.get())->shape()->getPosition(),
          [&](Object* pThis) -> Object* {
             pathFind.setGoal(*((DrawNode*)(pThis))->pNode);
             return nullptr; },
          pNode)));
      }
    }
    std::vector<std::shared_ptr<Drawable>> appendClickables;
    for (auto& pClicked : sim.clickables) {
      if (auto&& draw = (((Object*)pClicked.get())->isClicked(&sim.window));
          draw != nullptr) {
        appendClickables.push_back(std::shared_ptr<Drawable>(draw));
      }
    }
    sim.clickables.append_range(appendClickables);
    sim.walls.append_range(appendClickables);

    if (!automatic)
      ((Object*)sim.car.get())->keyBoardMove();
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