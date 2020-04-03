#pragma once

// shader program
class RenderProgram: public cxx::noncopyable
{
public:
    // readonly
    std::string mProgramName;
    
    GpuProgram* mGpuProgram = nullptr;

public:
    // @param srcFileName: File name of shader source
    RenderProgram(const std::string& srcFilePath);
    virtual ~RenderProgram();

    // load shader program from source file, compile and create gpu object
    // @returns false on error
    bool LoadProgram();
    void FreeProgram();

    // try to reload from source file but not destroy current gpu object on fail
    // @returns false on error
    bool ReloadProgram();

    // test whether program is initialized and currently active
    bool IsProgramLoaded() const;
    bool IsProgramActive() const;

    // activate or deactivate render program
    void ActivateProgram();
    void DeactivateProgram();

protected:
    // overridable
    virtual void OnProgramActivated();
    virtual void OnProgramDeactivated();
    virtual void OnProgramLoad();
    virtual void OnProgramFree();
};