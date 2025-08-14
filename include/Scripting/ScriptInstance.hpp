#include <Scripting/LuaBindings.hpp>
#include <Timer.hpp>
#include <Utils.hpp>

// typedef std::shared_ptr<sol::state> LuaStateRef;

inline thread_local std::string threadStateName;
inline thread_local sol::state threadState;

class ScriptInstance : sol::load_result 
{
    private : 

        BenchTimer instanceTimer;        
        bool built = false;
        
    public :

        static inline std::unordered_map<std::string, BenchTimer> globalTimers;

        std::string file;
        // LuaStateRef state;

        ScriptInstance(){};
        ScriptInstance(const std::string& file);
        // ScriptInstance(const std::string& file, LuaStateRef state);

        void compile();

        BenchTimer& getTimer();
        static BenchTimer& getGlobalTimer();
        
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

    ScriptInstance::ScriptInstance(const std::string& file) : file(file){}

    // ScriptInstance::ScriptInstance(const std::string& file, LuaStateRef state) : file(file), state(state)
    // {
    //     compile();
    // }

    void ScriptInstance::compile()
    {
        // *(sol::load_result*)(this) = state->load_file(file);
        *(sol::load_result*)(this) = threadState.load_file(file);

        built = true;
    }

    BenchTimer& ScriptInstance::getTimer(){return instanceTimer;}
    // BenchTimer& ScriptInstance::getGlobalTimer(){return globalTimer;}
    BenchTimer& ScriptInstance::getGlobalTimer(){return globalTimers[threadStateName];}


    // void ScriptInstance::run(... args)
    // {
    //     if(built && threadState != this->lua_state() )
    //     {
    //         ERROR_MESSAGE(
    //             "Script \'" 
    //             << file 
    //             << "\' called from the thread \'" 
    //             <<  threadStateName 
    //             << "\', wich is not the script's original creator. Execution won't proceed."
    //         )
    //         return;
    //     }

    //     if(!built)
    //         compile();

    //     if(built)
    //     {
    //         instanceTimer.start();
    //         globalTimer.start();
    
    //         (*this)(args);
    
    //         instanceTimer.hold();
    //         globalTimer.hold();
    //     }
    // }

#endif