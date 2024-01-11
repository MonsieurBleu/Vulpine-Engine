#include <Launcher.hpp>

int main()
{
    App *engine = nullptr;
    std::string winname =  "Vulpine Engine";
    int ret = launchGame(&engine, winname);
    if(engine) delete engine;
    return ret;
}