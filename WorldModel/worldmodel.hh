#ifndef BOLD_WORLD_MODEL_HH
#define BOLD_WORLD_MODEL_HH

#include <iostream>

namespace bold
{
  class WorldModel
  {
  private:
    WorldModel()
    {};

    WorldModel(WorldModel const&);
    void operator=(WorldModel const&);

  public:
    void initialise(minIni const& ini)
    {
      std::cout << "[WorldModel::WorldModel] Initialising WorldModel" << std::endl;
    };

    /** Gets the singleton instance of the WorldModel. */
    static WorldModel& getInstance()
    {
      static WorldModel instance;
      return instance;
    }
  };
}

#endif
