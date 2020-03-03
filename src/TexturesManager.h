#pragma once

class Texture2D;

// render textures manager class
class TexturesManager: public cxx::noncopyable
{
public:

    // setup texture manager internal resources, returns false on error
    bool Initialize();
    void Deinit();


};

extern TexturesManager gTexturesManager;