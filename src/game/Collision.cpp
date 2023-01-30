#include "Collision.h"

#include <glm/glm.hpp>
#include <algorithm>
#include <limits>
#include <sstream>
#include <string>

#include <iostream>

static Quad AABB2Quad(const AABB &aabb)
{
    glm::vec2 upperLeft{aabb.min.x, aabb.min.y};
    glm::vec2 lowerLeft{aabb.min.x, aabb.max.y};
    glm::vec2 lowerRight{aabb.max.x, aabb.max.y};
    glm::vec2 upperRight{aabb.max.x, aabb.min.y};

    return {upperLeft, lowerLeft, lowerRight, upperRight};
}

template <class InputIt>
static std::pair<float, float> projectedExtents(const glm::vec2 &axis, InputIt pointsBegin, InputIt pointsEnd)
{
    auto minDot = std::numeric_limits<float>::max();
    auto maxDot = -std::numeric_limits<float>::max();
    for (auto it = pointsBegin; it != pointsEnd; it++)
    {
        const auto &point = *it;
        const auto dot = glm::dot(point, axis);
        minDot = std::min(minDot, dot);
        maxDot = std::max(maxDot, dot);
    }
    return {minDot, maxDot};
}

static std::pair<float, float> projectedExtents(const glm::vec2 &axis, const Quad &quad)
{
    return projectedExtents(axis, std::begin(quad.points), std::end(quad.points));
}

static bool rangeOverlaps(float min1, float max1, float min2, float max2)
{
    return max1 >= min2 && max2 >= min1;
}

static bool checkCollision(const AABB &aabb, const Quad &quad, glm::vec2 &normal, float &minOverlap)
{
    // AABB's have two distinct projects, horizonal and veritcal
    glm::vec2 aabbAxes[] = {
        {1.0f, 0},
        {0, 1.0f},
    };

    Quad aabbQuad = AABB2Quad(aabb);

    minOverlap = std::numeric_limits<float>::max();
    for (size_t i = 0; i < 2; i++)
    {
        const auto &axis = aabbAxes[i];

        const auto quadExtents = projectedExtents(axis, quad);
        const auto aabbExtents = projectedExtents(axis, aabbQuad);

        if (!rangeOverlaps(quadExtents.first, quadExtents.second, aabbExtents.first, aabbExtents.second))
            return false;

        const auto overlap = std::min(aabbExtents.second - quadExtents.first, quadExtents.second - aabbExtents.first);
        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            normal = axis;
        }
    }

    // Optimization for Oriented-Bound-Box, and not a generalized quad.
    for (size_t i = 0; i < 4; i++)
    {
        const auto p1 = quad.points[i];
        const auto p2 = quad.points[(i + 1) % 4];
        const auto edgeVector = p2 - p1;
        const auto axis = glm::normalize(glm::vec2{edgeVector.y, -edgeVector.x});

        const auto quadExtents = projectedExtents(axis, quad);
        const auto aabbExtents = projectedExtents(axis, aabbQuad);

        if (!rangeOverlaps(quadExtents.first, quadExtents.second, aabbExtents.first, aabbExtents.second))
            return false;

        const auto overlap = std::min(aabbExtents.second - quadExtents.first, quadExtents.second - aabbExtents.first);
        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            normal = axis;
        }
    }
    return true;
}

bool TileMap::checkCollision(const Quad &boxQuad, glm::vec2 &ejectionVector)
{
    // Create an AABB from the boxQuad points
    glm::vec2 minExtents = boxQuad.points[0];
    glm::vec2 maxExtents = boxQuad.points[0];
    for (size_t i = 1; i < 4; i++)
    {
        minExtents.x = std::min(minExtents.x, boxQuad.points[i].x);
        minExtents.y = std::min(minExtents.y, boxQuad.points[i].y);
        maxExtents.x = std::max(maxExtents.x, boxQuad.points[i].x);
        maxExtents.y = std::max(maxExtents.y, boxQuad.points[i].y);
    }
    glm::vec2 boxCenter = (minExtents + maxExtents) / 2.0f;

    auto minCoords = pointToTileCoordinates(minExtents);
    auto maxCoords = pointToTileCoordinates(maxExtents);

    ejectionVector = glm::vec2{0};
    // Only check tiles overlapped

    bool collisionDetected = false;
    for (size_t i = maxCoords.second; i <= minCoords.second; i++)
    {
        for (size_t j = minCoords.first; j <= maxCoords.first; j++)
        {
            if (!isCollidable(j, i))
                continue;

            const auto tile = tileAABB(j, i);
            float overlapAmount = 0;
            glm::vec2 ejectionNormal;
            if (!::checkCollision(tile, boxQuad, ejectionNormal, overlapAmount))
                continue;
            ejectionVector += glm::normalize(boxCenter - tile.center()) * overlapAmount;
            collisionDetected = true;
        }
    }
    return collisionDetected;
}

static std::vector<std::string> split(const std::string &s, char delimiter = '\n')
{
    std::vector<std::string> tokens;

    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

void prepareTileMap(const char *playField, TileMap &tm)
{
    auto lines = split(playField);

    size_t maxColumns = 0;
    for (const auto &line : lines)
    {
        maxColumns = std::max(maxColumns, line.size());
    }
    tm.tiles.reserve(lines.size());
    for (const auto &line : lines)
    {
        std::vector<TileMap::Tile> row;
        row.reserve(maxColumns);
        for (const auto &character : line)
        {
            row.emplace_back(TileMap::Tile{character == '*'});
        }
        tm.tiles.emplace_back(std::move(row));
    }

    tm.center.x = static_cast<float>(tm.tiles[0].size()) * 0.5f * tm.tileSize;
    tm.center.y = static_cast<float>(tm.tiles.size()) * 0.5f * tm.tileSize;
}