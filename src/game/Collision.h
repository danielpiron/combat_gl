#include <glm/glm.hpp>
#include <glm/vec2.hpp>

#include <utility>
#include <vector>

struct Quad
{
    glm::vec2 points[4];
};

struct AABB
{
    glm::vec2 min, max;

    glm::vec2 center() const
    {
        return (min + max) * 0.5f;
    }
};

struct TileMap
{
    struct Tile
    {
        bool isCollidable = false;
    };

    bool isCollidable(size_t x, size_t y)
    {
        return tiles[y][x].isCollidable;
    }

    AABB tileAABB(size_t x, size_t y)
    {
        y = (tiles.size() - 1) - y;
        glm::vec2 min = glm::vec2{static_cast<float>(x * tileSize) - tileSize * 0.5f, static_cast<float>(y * tileSize) - tileSize * 0.5f} - center;
        glm::vec2 max = glm::vec2{min.x + tileSize, min.y + tileSize};
        return {min, max};
    }

    // Given a point, return tile coordinates
    std::pair<size_t, size_t> pointToTileCoordinates(glm::vec2 point) const
    {
        glm::vec2 adjustedPoint = point + center + glm::vec2{0.5f, 0.5f};

        return {static_cast<size_t>(adjustedPoint.x), static_cast<size_t>(tiles.size() - adjustedPoint.y)};
    }

    bool checkCollision(const Quad &quad, glm::vec2 &ejectionVector);

    std::vector<std::vector<Tile>> tiles;
    glm::vec2 center;
    int tileSize = 1.0f;
};

void prepareTileMap(const char *playField, TileMap &tm);
bool checkCollision(const AABB &lhs, const AABB &rhs);