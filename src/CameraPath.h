#pragma once

//////////////////////////////////////////////////////////////////////////

using CameraPathId = unsigned int;

//////////////////////////////////////////////////////////////////////////

enum : CameraPathId
{
    // frontend

    CameraPathId_Frontend_Intro                 = 250,
    CameraPathId_Frontend_EntryToTable          = 251,
    CameraPathId_Frontend_TableToEntry          = 252,
    CameraPathId_Frontend_TableTo1stRight       = 253,
    CameraPathId_Frontend_1stRightToTable       = 254,
    CameraPathId_Frontend_Spare                 = 255,
    CameraPathId_Frontend_EntryTo2ndRight       = 256,
    CameraPathId_Frontend_2ndRightToEntry       = 257,
    CameraPathId_Frontend_Static2ndRight        = 258,
    CameraPathId_Frontend_EntryTo1stLeft        = 259,
    CameraPathId_Frontend_1stLeftToEntry        = 260,
    CameraPathId_Frontend_Static1stLeft         = 261,
    CameraPathId_Frontend_EntryTo2ndLeft        = 262,
    CameraPathId_Frontend_2ndLeftToEntry        = 263,
    CameraPathId_Frontend_Static2ndLeft         = 264,
    CameraPathId_Frontend_1stLeftToCreditView   = 265,
    CameraPathId_Frontend_CreditViewTo1stLeft   = 266,
    CameraPathId_Frontend_EntryToMPDTable       = 267,
    CameraPathId_Frontend_MPDTableToEntry       = 268,
    CameraPathId_Frontend_MPDTableTo1stRight    = 269,
    CameraPathId_Frontend_1stRightToMPDTable    = 270,
    CameraPathId_Frontend_EntryTo1stRight       = 271,
    CameraPathId_Frontend_1stRightToEntry       = 272,
    CameraPathId_Frontend_Static1stRight        = 273,
    CameraPathId_Frontend_StaticEntry           = 274,
};

//////////////////////////////////////////////////////////////////////////

class CameraPath final
{
public:

    //////////////////////////////////////////////////////////////////////////
    struct Frame
    {
        glm::vec3 mPosition;
        glm::vec3 mForward;
        glm::vec3 mRight;
        glm::vec3 mUp;
        cxx::angle_t mFov;
    };
    //////////////////////////////////////////////////////////////////////////

public:
    // common info
    int GetFramesCount() const;
    int GetPathLength() const
    {
        return GetFramesCount();
    }

    // accessing path frames by index
    const Frame& GetFrame(int frameIndex) const;
    const Frame& GetFirstFrame() const;
    const Frame& GetLastFrame() const;

    // gets an interpolated path frame by fractional index
    Frame SampleFrame(float frameIndexf) const;
    Frame SampleFrameT(float t) const; // [0..1]

public:
    std::vector<Frame> mFrames;
};

//////////////////////////////////////////////////////////////////////////