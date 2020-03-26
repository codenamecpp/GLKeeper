#pragma once

class RoomsManager: public cxx::noncopyable
{
public:

    // setup rooms manager internal resources
    bool Initialize();
    void Deinit();


};

extern RoomsManager gRoomsManager;