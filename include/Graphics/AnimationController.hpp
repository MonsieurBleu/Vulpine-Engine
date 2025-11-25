#pragma once 

#include "Animation.hpp"
#include "Skeleton.hpp"
#include "Utils.hpp"

#include <fstream>
#include <Graphics/Skeleton.hpp>
#include <iostream>

enum AnimationControllerTransitionCondition
{
    COND_ANIMATION_FINISHED = 0x1,
    COND_CUSTOM = 0x2,
};

enum TransitionType
{
    TRANSITION_LINEAR = 0x1,
    TRANSITION_SMOOTH = 0x2,
};

struct AnimationControllerTransition
{
    AnimationRef from;
    AnimationRef to;
    AnimationControllerTransitionCondition condition;
    float transitionLength; // length of the transition
    TransitionType type;
    std::function<bool(void *)> conditionFunction;

    AnimationControllerTransition(AnimationRef _from,
                                  AnimationRef _to,
                                  AnimationControllerTransitionCondition _condition,
                                  float _transitionLength,
                                  TransitionType _type = TRANSITION_LINEAR,
                                  std::function<bool(void *)> _conditionFunction = [](void *){return false;})
        : from(_from),
          to(_to),
          condition(_condition),
          transitionLength(_transitionLength),
          type(_type),
          conditionFunction(_conditionFunction)
    {
    }
};

class AnimationController
{
  private:
    std::vector<AnimationControllerTransition> transitions;
    bool transitioning = false; // 🏳️‍⚧️ 
    AnimationControllerTransition *currentTransition = nullptr;
    AnimationRef currentAnimation;
    std::chrono::time_point<std::chrono::high_resolution_clock> animationStart;
    std::chrono::time_point<std::chrono::high_resolution_clock> transitionStart;
    float animationTime = 0;    // animation time of the current animation
    // float animationTimeB = 0;   // animation time of the "from" animation when transitionning
    float animationLTime = 0;
    
    float transitionTimeSinceStart = 0;
    float transitionTime = 0;
    float transitionLTime = 0;

    void* usr = nullptr;

    std::vector<keyframeData> currentKeyframes;

    std::vector<AnimationControllerTransition *> transitionsFromCurrentState;

    std::vector<int16> currentKeyframeIndexA;
    std::vector<int16> currentKeyframeIndexB;

    void getTransitionsFromCurrentState();
public:
    AnimationController(){};

     AnimationController(
        const std::vector<AnimationControllerTransition> &_transitions, 
        AnimationRef &initialState, 
        void *usr = nullptr);

    void update(float dt);

    void applyKeyframes(SkeletonAnimationState &skeleton);
};

typedef std::shared_ptr<AnimationController> AnimationControllerRef;