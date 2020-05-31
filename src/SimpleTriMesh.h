#pragma once

// simple triangle mesh

template<typename TVertexFormat>
class SimpleTriMesh
{
public:
    using VertexType = typename TVertexFormat::TVertexType;

    // ctor
    SimpleTriMesh() = default;
    SimpleTriMesh(int vertexCount, int triangleCount)
    {
        mTriangles.resize(triangleCount);
        mVertices.resize(vertexCount);
    }

    // enlarge allocated vertices count to the required size
    // @param vertexCount: Required number of vertices
    inline void EnsureVertexCount(int vertexCount)
    {
        int currentCount = (int) mVertices.size();
        if (currentCount < vertexCount)
        {
            mVertices.resize{vertexCount};
        }
    }

    // enlarge allocated triangles count to the required size
    // @param triangleCount: Required number of triangles
    inline void EnsureTriangleCount(int triangleCount)
    {
        int triangleCount = (int) mTriangles.size();
        if (triangleCount < triangleCount)
        {
            mTriangles.resize{triangleCount};
        }
    }

    // set vertex data
    // @param vertexData: Source
    // @param vertexCount: Number of vertices in source array
    inline void SetVertices(const VertexType* vertexData, int vertexCount)
    {
        debug_assert(vertexData);
        debug_assert(vertexCount > 0);

        mVertices.assign(vertexData, vertexData + vertexCount);
    }

    // set triangle data
    // @param triangleData: Source
    // @param triangleCount: Number of triangles in source array
    inline void SetTriangles(const glm::ivec3* triangleData, int triangleCount)
    {
        debug_assert(triangleData);
        debug_assert(triangleCount > 0);

        mTriangles.assign(triangleData, triangleData + triangleCount);
    }

    // change number of allocated vertices
    // @param vertexCount: New vertex count
    inline void SetVertexCount(int vertexCount)
    {
        mVertices.resize(vertexCount);
    }

    // change number of allocated triangles
    // @param triangleCount: New triangle count
    inline void SetTriangleCount(int triangleCount)
    {
        mTriangles.resize(triangleCount);
    }

    // refresh bounding box of triangle mesh
    inline void ComputeBounds()
    {
        mBoundingBox.clear();
        for (const VertexType& currVertex: mVertices)
        {
            mBoundingBox.extend(currVertex.mPosition);
        }
    }

    // clear all vertices and triangles
    inline void Clear()
    {
        mTriangles.clear();
        mVertices.clear();
        mBoundingBox.clear();
    }

public:
    using TrianglesArray = std::vector<glm::ivec3>;
    TrianglesArray mTriangles;

    using VerticesArray = std::vector<VertexType>;
    VerticesArray mVertices;

    cxx::aabbox mBoundingBox;
};