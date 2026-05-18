#pragma once

#include "GameDefs.h"

namespace MapUtils
{

    // compute block bounding box within world
    inline cxx::aabbox ComputeBlockBounds(const MapPoint2D& location)
    {
        cxx::aabbox outputBounds;
        // min
        outputBounds.mMin.x = (location.x * MAP_TILE_SIZE) - MAP_TILE_HALF_SIZE;
        outputBounds.mMin.y = MAP_FLOOR_LEVEL;
        outputBounds.mMin.z = (location.y * MAP_TILE_SIZE) - MAP_TILE_HALF_SIZE;
        // max
        outputBounds.mMax.x = outputBounds.mMin.x + MAP_TILE_SIZE;
        outputBounds.mMax.y = MAP_FLOOR_LEVEL + MAP_BLOCK_HEIGHT;
        outputBounds.mMax.z = outputBounds.mMin.z + MAP_TILE_SIZE;

        return outputBounds;
    }

    // compute blocks area bounding box within world
    inline cxx::aabbox ComputeBlocksAreaBounds(const MapArea2D& area)
    {
        cxx::aabbox outputBounds;
        // min
        outputBounds.mMin.x = (area.x * MAP_TILE_SIZE) - MAP_TILE_HALF_SIZE;
        outputBounds.mMin.y = MAP_FLOOR_LEVEL;
        outputBounds.mMin.z = (area.y * MAP_TILE_SIZE) - MAP_TILE_HALF_SIZE;
        // max
        outputBounds.mMax.x = outputBounds.mMin.x + (area.w * MAP_TILE_SIZE);
        outputBounds.mMax.y = MAP_FLOOR_LEVEL + MAP_BLOCK_HEIGHT;
        outputBounds.mMax.z = outputBounds.mMin.z + (area.h * MAP_TILE_SIZE);

        return outputBounds;
    }

    // compute map block center in world coordinates
    inline glm::vec3 ComputeBlockCenter(const MapPoint2D& blockLocation)
    {
        return glm::vec3 
        { 
            blockLocation.x * MAP_TILE_SIZE,
            MAP_FLOOR_LEVEL + MAP_TILE_SIZE,
            blockLocation.y * MAP_TILE_SIZE
        };
    }

    // compute game map block position in world coordinates
    // corner
    inline glm::vec3 ComputeBlockCoordinate(const MapPoint2D& blockLocation)
    {
        return glm::vec3
        {
            (blockLocation.x * MAP_TILE_SIZE) - MAP_TILE_HALF_SIZE,
            MAP_FLOOR_LEVEL + MAP_TILE_SIZE,
            (blockLocation.y * MAP_TILE_SIZE) - MAP_TILE_HALF_SIZE
        };
    }

    // compute tile center coord 3d
    inline glm::vec3 ComputeTileCenter(const MapPoint2D& tileLocation)
    {
        return glm::vec3
        {
            tileLocation.x * MAP_TILE_SIZE,
            MAP_FLOOR_LEVEL,
            tileLocation.y * MAP_TILE_SIZE
        };
    }

    inline glm::vec2 ComputeTileCenter2d(const MapPoint2D& tileLocation)
    {
        return glm::vec2
        {
            tileLocation.x * MAP_TILE_SIZE,
            tileLocation.y * MAP_TILE_SIZE
        };
    }

    // convert world coordinate to game map block logical location
    // @param coordinate: World space
    // @param blockLocation: Logical x,y
    inline MapPoint2D ComputeTileFromPosition(const glm::vec3& coordinate)
    {
        return MapPoint2D
        {
            static_cast<int>((coordinate.x + MAP_TILE_HALF_SIZE) / MAP_TILE_SIZE),
            static_cast<int>((coordinate.z + MAP_TILE_HALF_SIZE) / MAP_TILE_SIZE)
        };
    }

    // convert world coordinate to game map block logical location
    // @param coordinate: World space
    // @param blockLocation: Logical x,y
    inline MapPoint2D ComputeTileFromPosition(const glm::vec2& coordinate)
    {
        return MapPoint2D
        {
            static_cast<int>((coordinate.x + MAP_TILE_HALF_SIZE) / MAP_TILE_SIZE),
            static_cast<int>((coordinate.y + MAP_TILE_HALF_SIZE) / MAP_TILE_SIZE)
        };
    }

    inline glm::vec3 WrapPositionWithinBlock(const glm::vec3& coordinate)
    {
        return glm::vec3
        {
            glm::mod(coordinate.x + MAP_TILE_HALF_SIZE, MAP_TILE_SIZE),
            coordinate.y,
            glm::mod(coordinate.z + MAP_TILE_HALF_SIZE, MAP_TILE_SIZE)
        };
    }

    inline glm::vec2 WrapPositionWithinBlock(const glm::vec2& coordinate)
    {
        return glm::vec2
        {
            glm::mod(coordinate.x + MAP_TILE_HALF_SIZE, MAP_TILE_SIZE),
            glm::mod(coordinate.y + MAP_TILE_HALF_SIZE, MAP_TILE_SIZE)
        };
    }

} // namespace MapUtils
