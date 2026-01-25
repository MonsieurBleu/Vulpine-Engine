#pragma once

#include <Scripting/LuaBindings.hpp>
#include <Timer.hpp>
#include <Utils.hpp>
#include <Filewatcher.hpp>
#include <utility>

// typedef std::shared_ptr<sol::state> LuaStateRef;

// inline void my_panic(sol::optional<std::string> maybe_msg) {
//     std::cerr << "Lua panic: ";
//     if(maybe_msg) {
//         std::cerr << maybe_msg->c_str();
//     }
//     std::cerr << std::endl;
// }

// inline int my_exception_handler(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description) {
// 	// L is the lua state, which you can wrap in a state_view if necessary
// 	// maybe_exception will contain exception, if it exists
// 	// description will either be the what() of the exception or a description saying that we hit the general-case catch(...)
// 	std::cout << "An exception occurred in a function, here's what it says ";
// 	if (maybe_exception) {
// 		std::cout << "(straight from the exception): ";
// 		const std::exception& ex = *maybe_exception;
// 		std::cout << ex.what() << std::endl;
// 	}
// 	else {
// 		std::cout << "(from the description parameter): ";
// 		std::cout.write(description.data(), static_cast<std::streamsize>(description.size()));
// 		std::cout << std::endl;
// 	}

// 	// you must push 1 element onto the stack to be
// 	// transported through as the error object in Lua
// 	// note that Lua -- and 99.5% of all Lua users and libraries -- expects a string
// 	// so we push a single string (in our case, the description of the error)
// 	return sol::stack::push(L, description);
// }


inline thread_local std::string threadStateName;
// inline thread_local sol::state threadState(sol::c_call<decltype(&my_panic), &my_panic>);
inline thread_local sol::state threadState;

class ScriptInstance : public sol::load_result 
{
    private : 

        std::string file;
        BenchTimer instanceTimer;        
        bool built = false;
        TickTimer compileTimer;
        TickTimer errorTimer;
        bool firstCompileTry = true;
        
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
        
        template<typename Ret, typename ... Args>
        std::optional<Ret> runAndReturn(Args&& ... args)
        {
            if(built && threadState != this->lua_state() )
            {
                ERROR_MESSAGE(
                    "Script \'" 
                    ,  file 
                    ,  "\' called from the thread \'" 
                    ,   threadStateName 
                    ,  "\', which is not the script's original creator. Execution won't proceed."
                )
                return std::nullopt;
            }

            // NOTIF_MESSAGE(
            //     "compile timer last time" , 
            //     compileTimer.timeSinceLastTickMS() ,  "ms " ,  
            //     compileTimer.lasTickTime.time_since_epoch().count()
            // )
            if(!built && (compileTimer.timeSinceLastTick() > 1.0 || firstCompileTry))
                compile();

            std::optional<Ret> result = std::nullopt;
            if(built)
            {
                instanceTimer.start();
                getGlobalTimer().start();
                

                try {
                    Ret v = (*this)(args...);
                    result = v;
                }
                catch (const sol::error &e) {
                    if (errorTimer.timeSinceLastTick() > 1.0) {
                        errorTimer.tick();
                        ERROR_MESSAGE("Error while running script \'" ,  file ,  "\': " ,  e.what())
                    }
                }
                catch (const std::exception &e) {
                    if (errorTimer.timeSinceLastTick() > 1.0) {
                        errorTimer.tick();
                        ERROR_MESSAGE("Exception while running script \'" ,  file ,  "\': " ,  e.what())
                    }
                }
                catch (...) {
                    if (errorTimer.timeSinceLastTick() > 1.0) {
                        errorTimer.tick();
                        ERROR_MESSAGE("Unknown exception while running script \'" ,  file ,  "\'")
                    }
                }

                // sol::protected_function func = get<sol::protected_function>();
                // func(args...);

                

        
                instanceTimer.hold();
                getGlobalTimer().hold();
            }

            return result;
        };

    template<typename ... Args>
    void run(Args&& ... args)
    {
        if(built && threadState != this->lua_state() )
        {
            ERROR_MESSAGE(
                "Script \'" 
                ,  file 
                ,  "\' called from the thread \'" 
                ,   threadStateName 
                ,  "\', which is not the script's original creator. Execution won't proceed."
            )
            return;
        }

        // NOTIF_MESSAGE(
        //     "compile timer last time" , 
        //     compileTimer.timeSinceLastTickMS() ,  "ms " ,  
        //     compileTimer.lasTickTime.time_since_epoch().count()
        // )
        if(!built && (compileTimer.timeSinceLastTick() > 1.0 || firstCompileTry))
            compile();

        if(built)
        {
            instanceTimer.start();
            getGlobalTimer().start();
            

            try {
                (*this)(args...);
            }
            catch (const sol::error &e) {
                if (errorTimer.timeSinceLastTick() > 1.0) {
                    errorTimer.tick();
                    ERROR_MESSAGE("Error while running script \'" ,  file ,  "\': " ,  e.what())
                }
            }
            catch (const std::exception &e) {
                if (errorTimer.timeSinceLastTick() > 1.0) {
                    errorTimer.tick();
                    ERROR_MESSAGE("Exception while running script \'" ,  file ,  "\': " ,  e.what())
                }
            }
            catch (...) {
                if (errorTimer.timeSinceLastTick() > 1.0) {
                    errorTimer.tick();
                    ERROR_MESSAGE("Unknown exception while running script \'" ,  file ,  "\'")
                }
            }

            // sol::protected_function func = get<sol::protected_function>();
            // func(args...);

            

    
            instanceTimer.hold();
            getGlobalTimer().hold();
        }
    }
    
};