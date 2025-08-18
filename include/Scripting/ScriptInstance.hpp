#include <Scripting/LuaBindings.hpp>
#include <Timer.hpp>
#include <Utils.hpp>
#include <Filewatcher.hpp>

// typedef std::shared_ptr<sol::state> LuaStateRef;

inline thread_local std::string threadStateName;
inline thread_local sol::state threadState;

class ScriptInstance : sol::load_result 
{
    private : 

        std::string file;
        BenchTimer instanceTimer;        
        bool built = false;
        
    public :

        static inline std::unordered_map<std::string, BenchTimer> globalTimers;

        Filewatcher filewatcher;

        ScriptInstance(){};
        ScriptInstance(const std::string& file);

        void compile();
        void triggerRecompileOnNextRun();

        BenchTimer& getTimer();
        static BenchTimer& getGlobalTimer();

        const std::string &getFileName() const;
        
        template<typename ... Args>
        void run(Args&& ... args)
        {
            if(built && threadState != this->lua_state() )
            {
                ERROR_MESSAGE(
                    "Script \'" 
                    << file 
                    << "\' called from the thread \'" 
                    <<  threadStateName 
                    << "\', wich is not the script's original creator. Execution won't proceed."
                )
                return;
            }

            if(!built)
                compile();

            if(built)
            {
                instanceTimer.start();
                getGlobalTimer().start();
        
                (*this)(args...);
        
                instanceTimer.hold();
                getGlobalTimer().hold();
            }
        };
};

#ifdef SCRIPT_INSTANCE_IMPL

    ScriptInstance::ScriptInstance(const std::string& file) : file(file), filewatcher(file){}

    void ScriptInstance::compile()
    {
        *(sol::load_result*)(this) = threadState.load_file(file);
        built = true;
    }

    BenchTimer& ScriptInstance::getTimer(){return instanceTimer;}
    BenchTimer& ScriptInstance::getGlobalTimer(){return globalTimers[threadStateName];}

    const std::string & ScriptInstance::getFileName() const {return file;}

    void ScriptInstance::triggerRecompileOnNextRun(){built = false;}

#endif