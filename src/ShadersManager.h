#pragma once

//////////////////////////////////////////////////////////////////////////

#include "ShaderProgram.h"

//////////////////////////////////////////////////////////////////////////

class ShadersManager : public cxx::noncopyable
{
public:
    void Initialize();
    void Shutdown();

    //////////////////////////////////////////////////////////////////////////

    struct IShaderFactory
    {
    public:
        virtual std::unique_ptr<ShaderProgram> CreateInstance(const std::string& name) = 0;
    };

    //////////////////////////////////////////////////////////////////////////

    template<typename TShaderProgram>
    struct DefaultShaderFactory: public IShaderFactory
    {
    public:
        std::unique_ptr<ShaderProgram> CreateInstance(const std::string& name) override
        {
            return std::make_unique<TShaderProgram>(name); 
        }
    };

    //////////////////////////////////////////////////////////////////////////

    struct LoadParams
    {
        IShaderFactory* mFactory = nullptr;
        // will create fallback program if loading fails for some reason
        bool mInitDefaultOnFail = true;
    };

    //////////////////////////////////////////////////////////////////////////

    // loads new program if it is not loaded yet
    // will always return true if mInitFallbackOnFailure is set to true (operation never fails)
    // will not init render data automatically
    // returns nullptr if name is empty
    ShaderProgram* LoadProgram(const std::string& name, const LoadParams& params = {});

    // returns loaded program or nullptr if it not found
    ShaderProgram* FindProgram(const std::string& name) const;

    // returns already loaded program (params are ignored) or loads new program otherwise
    // returns nullptr if name is empty
    ShaderProgram* GetProgram(const std::string& name, const LoadParams& params = {});

    //////////////////////////////////////////////////////////////////////////
    template<typename TShaderProgram>
    inline TShaderProgram* GetProgramOfType(const std::string& name, LoadParams params = {})
    {
        if (params.mFactory == nullptr)
        {// constuct with default factory
            static DefaultShaderFactory<TShaderProgram> defaultFactory;
            params.mFactory = &defaultFactory;
        }
        ShaderProgram* shaderProgram = GetProgram(name, params);
        cxx_assert(typeid(*shaderProgram) == typeid(TShaderProgram));
        return static_cast<TShaderProgram*>(shaderProgram);
    }
    //////////////////////////////////////////////////////////////////////////

private:
    using ResourcesMap = std::map<std::string, std::unique_ptr<ShaderProgram>, cxx::icase_string_less>;
    ResourcesMap mResourcesMap;
};

//////////////////////////////////////////////////////////////////////////

extern ShadersManager gShadersManager;

//////////////////////////////////////////////////////////////////////////