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

public:
    // Vertices has specific order:
    // 0 - TOP LEFT
    // 1 - BOTTOM LEFT
    // 2 - BOTTOM RIGHT
    // 3 - TOP RIGHT
    Vertex2D mPoints[4];
};

//////////////////////////////////////////////////////////////////////////
