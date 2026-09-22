#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiDefs.h"
#include "GraphicsDefs.h"
#include "VertexFormat.h"

//////////////////////////////////////////////////////////////////////////

// Simple quad of four 2d vertices
struct Quad2D
{
public:
    // Build simple quad vertices
    // @param texDims: Texture size
    // @param rcSrc, rcDest: Source and destination rectangles
    // @param theColor: Quad vertices color
    inline void BuildTextureQuad(const Point2D& texDims, const Rect2D& rcSrc, const Rect2D& rcDest, Color32 theColor)
    {
        const float invx = 1.0f / texDims.x;
        const float invy = 1.0f / texDims.y;

        // setup quad vertices in specific order
        mPoints[0].mColor       = theColor;
        mPoints[0].mTexcoord[0] = rcSrc.x * invx;
        mPoints[0].mTexcoord[1] = rcSrc.y * invy;
        mPoints[0].mPosition.x  = rcDest.x * 1.0f;
        mPoints[0].mPosition.y  = rcDest.y * 1.0f;
        mPoints[1].mColor       = theColor;
        mPoints[1].mTexcoord[0] = mPoints[0].mTexcoord[0];
        mPoints[1].mTexcoord[1] = (rcSrc.y + rcSrc.h) * invy;
        mPoints[1].mPosition.x  = mPoints[0].mPosition.x;
        mPoints[1].mPosition.y  = (rcDest.y + rcDest.h) * 1.0f;
        mPoints[2].mColor       = theColor;
        mPoints[2].mTexcoord[0] = (rcSrc.x + rcSrc.w) * invx;
        mPoints[2].mTexcoord[1] = mPoints[1].mTexcoord[1];
        mPoints[2].mPosition.x  = (rcDest.x + rcDest.w) * 1.0f;
        mPoints[2].mPosition.y  = mPoints[1].mPosition.y;
        mPoints[3].mColor       = theColor;
        mPoints[3].mTexcoord[0] = mPoints[2].mTexcoord[0];
        mPoints[3].mTexcoord[1] = mPoints[0].mTexcoord[1];
        mPoints[3].mPosition.x  = mPoints[2].mPosition.x;
        mPoints[3].mPosition.y  = mPoints[0].mPosition.y;
    }

    inline void BuildTextureQuad(const TextureRegion& parentRegion, const Rect2D& rcSrc, const Rect2D& rcDest, Color32 theColor)
    {
        // setup quad vertices in specific order
        mPoints[0].mColor       = theColor;
        mPoints[0].mPosition.x  = rcDest.x * 1.0f;
        mPoints[0].mPosition.y  = rcDest.y * 1.0f;
        mPoints[1].mColor       = theColor;
        mPoints[1].mPosition.x  = mPoints[0].mPosition.x;
        mPoints[1].mPosition.y  = (rcDest.y + rcDest.h) * 1.0f;
        mPoints[2].mColor       = theColor;
        mPoints[2].mPosition.x  = (rcDest.x + rcDest.w) * 1.0f;
        mPoints[2].mPosition.y  = mPoints[1].mPosition.y;
        mPoints[3].mColor       = theColor;
        mPoints[3].mPosition.x  = mPoints[2].mPosition.x;
        mPoints[3].mPosition.y  = mPoints[0].mPosition.y;

        // texcoords

        cxx_assert(parentRegion.mRect.w > 0);
        cxx_assert(parentRegion.mRect.h > 0);

        const float invW = 1.0f / parentRegion.mRect.w;
        const float invH = 1.0f / parentRegion.mRect.h;

        const glm::vec2 rel0 = {rcSrc.x * invW, rcSrc.y * invH};
        const glm::vec2 rel1 {
            (rcSrc.x + rcSrc.w) * invW,
            (rcSrc.y + rcSrc.h) * invH
        };
        // remap
        const glm::vec2 uv0 = parentRegion.mUvMin + rel0 * (parentRegion.mUvMax - parentRegion.mUvMin);
        const glm::vec2 uv1 = parentRegion.mUvMin + rel1 * (parentRegion.mUvMax - parentRegion.mUvMin);

        mPoints[0].mTexcoord    = uv0;
        mPoints[1].mTexcoord.x  = uv0.x;
        mPoints[1].mTexcoord.y  = uv1.y;
        mPoints[2].mTexcoord    = uv1;
        mPoints[3].mTexcoord.x  = uv1.x;
        mPoints[3].mTexcoord.y  = uv0.y;
    }

    inline void BuildTextureQuad(const TextureRegion& parentRegion, const Rect2D& rcDest, Color32 theColor)
    {
        // setup quad vertices in specific order
        mPoints[0].mColor       = theColor;
        mPoints[0].mPosition.x  = rcDest.x * 1.0f;
        mPoints[0].mPosition.y  = rcDest.y * 1.0f;
        mPoints[1].mColor       = theColor;
        mPoints[1].mPosition.x  = mPoints[0].mPosition.x;
        mPoints[1].mPosition.y  = (rcDest.y + rcDest.h) * 1.0f;
        mPoints[2].mColor       = theColor;
        mPoints[2].mPosition.x  = (rcDest.x + rcDest.w) * 1.0f;
        mPoints[2].mPosition.y  = mPoints[1].mPosition.y;
        mPoints[3].mColor       = theColor;
        mPoints[3].mPosition.x  = mPoints[2].mPosition.x;
        mPoints[3].mPosition.y  = mPoints[0].mPosition.y;

        // texcoords
        mPoints[0].mTexcoord    = parentRegion.mUvMin;
        mPoints[1].mTexcoord.x  = parentRegion.mUvMin.x;
        mPoints[1].mTexcoord.y  = parentRegion.mUvMax.y;
        mPoints[2].mTexcoord    = parentRegion.mUvMax;
        mPoints[3].mTexcoord.x  = parentRegion.mUvMax.x;
        mPoints[3].mTexcoord.y  = parentRegion.mUvMin.y;
    }

    inline void RotateAroundCenter(cxx::angle_t rotationAngle)
    {
        glm::vec2 center {0.0f, 0.0f};
        for (const Vertex2D& roller: mPoints)
        {
            center += roller.mPosition;
        }
        center /= 4.0f;

        float cosA;
        float sinA;
        rotationAngle.get_sin_cos(sinA, cosA);

        for (Vertex2D& roller: mPoints)
        {
            const glm::vec2 p = roller.mPosition - center;
            const glm::vec2 new_p
            {
                p.x * cosA - p.y * sinA,
                p.x * sinA + p.y * cosA
            };
            roller.mPosition = new_p + center;
        }
    }

public:
    // Vertices has specific order:
    // 0 - TOP LEFT
    // 1 - BOTTOM LEFT
    // 2 - BOTTOM RIGHT
    // 3 - TOP RIGHT
    Vertex2D mPoints[4];
};

//////////////////////////////////////////////////////////////////////////
