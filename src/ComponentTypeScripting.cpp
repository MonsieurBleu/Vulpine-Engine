#include "ECS/ComponentTypeScripting.hpp"
#include "Scripting/ScriptInstance.hpp"

Script::Script()
{
}

Script::Script(std::string scriptAssetName, ScriptHook hook) {
    scripts[hook].push_back(scriptAssetName);
}

#define RUN_FOR_HOOK(hook, ...) \
    for (std::string s : scripts[hook]) { \
        if (Loader<ScriptInstance>::loadingInfos.find(s) != Loader<ScriptInstance>::loadingInfos.end()) { \
            Loader<ScriptInstance>::get(s).run(__VA_ARGS__); \
        } \
    }

void Script::run_OnInit() {
    RUN_FOR_HOOK(ScriptHook::ON_INIT)
    initialized = true;
}

void Script::run_OnUpdate() {
    RUN_FOR_HOOK(ScriptHook::ON_UPDATE)
}

void Script::run_OnCollisionEnter(EntityRef other) {
    RUN_FOR_HOOK(ScriptHook::ON_COLLISION_ENTER, other)
}

void Script::run_OnCollisionExit(EntityRef other) {
    RUN_FOR_HOOK(ScriptHook::ON_COLLISION_EXIT, other)
}


void Script::addScript(std::string scriptAssetName, ScriptHook hook) {
    scripts[hook].push_back(scriptAssetName);
    if (initialized && hook == ScriptHook::ON_INIT) {
        if (Loader<ScriptInstance>::loadingInfos.find(scriptAssetName) != Loader<ScriptInstance>::loadingInfos.end()) {
            Loader<ScriptInstance>::get(scriptAssetName).run();
        }
    }
}