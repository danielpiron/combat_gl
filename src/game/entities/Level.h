#include <applesauce/Entity.h>

class Wall : public applesauce::Entity
{
    void init(applesauce::ResourceManager &rm)
    {
        mesh = rm.getMesh("Wall");
    }
    void update(float)
    {
    }
};