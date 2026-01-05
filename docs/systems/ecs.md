# Entity Component System

Tako has an archetype based ECS.

```cpp
import Tako.ECS;

tako::ECS::World world;
void Update(float dt)
{
    for (auto& [transform, player] : world.Iterate<Transform, Player>())
    {
        if (!player.grounded)
        {
            transform.position.y -= 9.81f * dt;
        }
    }
}
```
