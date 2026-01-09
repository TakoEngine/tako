# Reflection system

## Imports
```cpp
#include "Reflection.hpp"

import Tako.Reflection;
```

## Annotate structs
```cpp
struct Player
{
    Vector3 position;
    bool grounded;
    float hp;

    REFLECT(Player, position, grounded, hp)
};
```

## Annotate enums
```cpp
namespace Game
{
    enum class DamageType
    {
        Physical,
        Magical,
        Poison
	};
}
// Outside any namespace
REFLECT_ENUM(Game::DamageType, DamageType, Physical, Magical, Poison)
```
