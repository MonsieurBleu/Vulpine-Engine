#pragma once

#include "Entity.hpp"
#include "MappedEnum.hpp"
#include <vector>
#include "AssetManager.hpp"

GENERATE_ENUM_FAST_REVERSE(
    ScriptHook,
    ON_INIT,
    ON_UPDATE,
    ON_COLLISION_ENTER,
    ON_COLLISION_EXIT,
    HOOK_END
);



class Script {
private:
    std::vector<std::string> scripts[ScriptHook::HOOK_END];
    bool initialized = false;
public:
    Script();
    Script(std::string scriptAssetName, ScriptHook hook);
    
    template <typename... Args>
    Script(std::string scriptAssetName, ScriptHook hook, Args... args)
    {
        addScript(scriptAssetName, hook);
        addScript(args...);
    }

    void run_OnInit();
    void run_OnUpdate();
    void run_OnCollisionEnter(EntityRef other);
    void run_OnCollisionExit(EntityRef other);

    void addScript(std::string scriptAssetName, ScriptHook hook);

    bool isInitialized() const { return initialized; }
    void setInitialized(bool val) { initialized = val; }

    friend class DataLoader<Script>;
};