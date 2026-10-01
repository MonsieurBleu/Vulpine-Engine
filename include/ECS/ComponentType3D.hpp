#pragma once

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <Globals.hpp>

#include <MathsUtils.hpp>
#include <MappedEnum.hpp>

using namespace glm;

template<typename T> 
struct Interpolated
{
    T last;
    T next;
};

struct State3D
{
    vec3 position = vec3(0);
    quat rotation = quat(1, 0, 0, 0);
    vec3 scale = vec3(1);
    ModelStatus isActive = ModelStatus::UNDEFINED;
};

bool operator==(const State3D &a, const State3D &b);


/*
    State3D for entites that uses physic interpolation
*/
typedef Interpolated<State3D> DynamicState3D;

struct HierarchyState3D : State3D
{
    GENERATE_ENUM_FAST_REVERSE(SynchType, 
        NOTHING        ,
        POS_ONLY       ,
        ROT_ONLY       ,
        POS_AND_ROT    ,
        SCALE_ONLY     ,
        SCALE_AND_POS  ,
        SCALE_AND_ROT  ,
        ALL            ,
    );
 
    SynchType type = ALL;

    HierarchyState3D(const State3D & s) : State3D(s), type(SynchType::ALL){};
    HierarchyState3D(const SynchType type = SynchType::ALL) : type(type){};
};

class Entity;

struct EntitySpawner
{
    struct info
    {
        std::string name = "";
        std::string cond = "";
        HierarchyState3D state;
        std::shared_ptr<Entity> child;
    };

    std::vector<info> onLoading;
    std::vector<info> onTrigger;
};

/*
    Soft Goals represent any physical or logical properties that an entity wants to adcheive.
*/
template<typename T> 
struct SoftGoal
{
    T current;
    T goal;
};

/*
    Hard goals are similar to soft goals, but they need tobe independlly accepted and every
    change is timed.
*/
template<typename T> 
class HardGoal
{
    private : 
        T current;
        T goal;
        float lastChangeTime = 0.f;
        float lastGoalMet = 0.f;

    public :

        HardGoal<T>(){};
        HardGoal<T>(const T & current) : current(current), goal(current){};

        HardGoal &setGoal(const T & t)
        {
            goal = t; 
            lastGoalMet = globals.simulationTime.getElapsedTime(); 
            return  *this;
        };

        HardGoal &acceptGoal()
        {
            current = goal; 
            lastChangeTime = globals.simulationTime.getElapsedTime(); 
            return  *this;
        };

        HardGoal &overrideCurrent(const T & t)
        {
            current = t; 
            return  *this;
        };

        const T &getGoal(){return goal;};
        const T &getCurrent(){return current;};

        float timeSinceGoalChange(){return globals.simulationTime.getElapsedTime() - lastGoalMet;}
        float timeSinceGoalMet(){return globals.simulationTime.getElapsedTime() - lastChangeTime;}

        bool isGoalMet(){return current == goal;}
};


template<typename T> 
class TimedValue
{
    private : 
        T v;
        float mPreviousTimeSinceChange = 0.f;
        float lastChangeTime = -1.f;

    public :
        TimedValue(){};
        
        TimedValue(const T &v) : v(v){};
        
        const T & get(){return v;};
        
        void set(const T & newV)
        {
            if(v != newV || lastChangeTime < 0.f)
            {
                mPreviousTimeSinceChange = timeSinceChange();
                lastChangeTime = globals.simulationTime.getElapsedTime();
                v = newV;
            }
        };

        float timeSinceChange(){return globals.simulationTime.getElapsedTime() - lastChangeTime;};
        float previousTimeSinceChange(){return mPreviousTimeSinceChange;}
};


struct Movement
{
    SoftGoal<vec3> look = {vec3(1, 0, 0), vec3(1, 0, 0)};
    SoftGoal<vec3> direction = {vec3(1, 0, 0), vec3(1, 0, 0)};
    SoftGoal<float> speed = {0.f, 0.f};
    TimedValue<bool> grounded = TimedValue<bool>(false);

    float walkSpeed   = 1.f;
    float jogSpeed    = 1.f;
    float sprintSpeed = 1.f;

    // Steps between walk and jog speed
    uint speedSteps   = 1;
    // Current step, from 0 to speedSteps, used to control speed
    uint currentSpeedStep  = 0;
    HardGoal<bool> sprint = HardGoal<bool>(false);
};

struct Gauge
{
    Gauge(float v = 100.f, float min = 0.f, float max = 100.f) : current(v), min(min), max(max) {};

    float current;
    float min;
    float max;

    Gauge& operator+=(float f){current = clamp(current + f, min, max); return *this;};
    Gauge& operator-=(float f){current = clamp(current - f, min, max); return *this;};
    Gauge& operator*=(float f){current = clamp(current * f, min, max); return *this;};
    Gauge& operator/=(float f){if(abs(f) > 0.f) current = clamp(current / f, min, max); return *this;};

    float getFraction(){return linearstep(min, max, current);};
};

struct Gauges
{
    Gauge fatigue = Gauge(0.f, 0.f, 100.f);
    Gauge stamina = Gauge(100.f, 0.f, 100.f);
};

struct ComplexMovements
{
    enum ClimbTypeEnum : uint8 { High, Low, Platform } climbType;

    float climbAnimSpeed = 1.f;

    /* Safe surface where the entity can climb */
    vec3 closestSurface = vec3(0);
    
    /* Edge position and normal where the entity can be placed for climbing-style animation*/
    vec3 closestEdge = vec3(0);
    vec3 closestEdgeNormal = vec3(0, 1, 0);

    /* Closest Wall position and normal where the entity can wall jump or can be placed do lean-style animations*/
    vec3 closestWall = vec3(0);
    vec3 closestWallNormal = vec3(0, 1, 0);
    
    /* Used for interpolation purpuses when doing an animation. Target final position will be closest edge/surface/wall/... */
    vec3 animationInitialPosition = vec3(0);
    
    /* Used to make sure the player can't infinite loop wall jumps on the same wall */
    vec3 lastWallJumpDirection = vec3(0);
    
    HardGoal<bool> climb = HardGoal<bool>(false);
    HardGoal<bool> jump  = HardGoal<bool>(false);
    HardGoal<bool> wallJump = HardGoal<bool>(false);
};

struct Actions
{
    HardGoal<bool> attack = HardGoal<bool>(false);
    HardGoal<bool> block  = HardGoal<bool>(false);
    HardGoal<bool> stun   = HardGoal<bool>(false);
};
