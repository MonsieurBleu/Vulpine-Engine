#include "Scripting/ScriptInstance.hpp"

ScriptInstance::ScriptInstance(const std::string& file) : file(file), filewatcher(file){
    errorTimer.tick();
    compileTimer.tick();
}

void ScriptInstance::compile()
{
    firstCompileTry = false;
    try {   
        *(sol::load_result*)(this) = threadState.load_file(file);
        if(this->valid())
        {
            built = true;
        }
        else {
            compileTimer.tick();
            FILE_ERROR_MESSAGE(file, "Failed to compile script \'" ,  "\': " ,  this->get<sol::error>().what())
            
            built = false;
        }
    }
    catch (const sol::error &e) {
        compileTimer.tick();
        FILE_ERROR_MESSAGE(file, "Error while compiling script \'" ,  "\': " ,  e.what())
        built = false;
    }
    catch (const std::exception &e) {
        compileTimer.tick();
        FILE_ERROR_MESSAGE(file, "Exception while compiling script \'" ,  "\': " ,  e.what())
        built = false;
    }
    catch (...) {
        compileTimer.tick();
        FILE_ERROR_MESSAGE(file, "Unknown exception while compiling script \'" ,  "\'")
        built = false;
    }
}

BenchTimer& ScriptInstance::getTimer(){return instanceTimer;}
BenchTimer& ScriptInstance::getGlobalTimer(){return globalTimers[threadStateName];}

const std::string & ScriptInstance::getFileName() const {return file;}

void ScriptInstance::triggerRecompileOnNextRun(){built = false;}