#pragma once 

#include "Animation.hpp"
#include "Skeleton.hpp"
#include "Utils.hpp"

#include <fstream>
#include <Skeleton.hpp>
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
    float animationTime = 0;
    float transitionTime = 0;
    float animationLTime = 0;
    float transitionLTime = 0;

    void* usr = nullptr;

    std::vector<keyframeData> currentKeyframes;

    std::vector<AnimationControllerTransition *> transitionsFromCurrentState;

    std::vector<int16> currentKeyframeIndexA;
    std::vector<int16> currentKeyframeIndexB;

    void getTransitionsFromCurrentState()
    {
        transitionsFromCurrentState.clear();
        for (auto &t : transitions)
        {
            if (t.from == currentAnimation)
            {
                transitionsFromCurrentState.push_back(&t);
            }
        }

        // if (transitionsFromCurrentState.size() == 0)
        // {
        //     // failsafe if no transition is found, loop back to the same animation
        //     transitions.push_back(AnimationControllerTransition(currentAnimation, currentAnimation,
        //                                                         COND_ANIMATION_FINISHED, epsilon<float>()));
        // }
    }

public:
    AnimationController(
        const std::vector<AnimationControllerTransition> &_transitions, 
        AnimationRef &initialState, 
        void *usr = nullptr) : usr(usr)
    {
        transitions = _transitions;

        currentAnimation = initialState;

        currentKeyframeIndexA.resize(currentAnimation->getKeyframeNumber());
        currentKeyframeIndexB.resize(currentAnimation->getKeyframeNumber());

        std::fill(currentKeyframeIndexA.begin(), currentKeyframeIndexA.end(), 0);
        std::fill(currentKeyframeIndexB.begin(), currentKeyframeIndexB.end(), 0);

        animationStart = std::chrono::high_resolution_clock::now();
        currentAnimation->onEnterAnimation(usr);
        getTransitionsFromCurrentState();
    }

    void update(float dt)
    {
        float prct = 100.f * animationTime / currentAnimation->getLength();

        animationTime += dt * currentAnimation->speedCallback(prct, usr);

        if (!transitioning)
        {
            currentKeyframes = currentAnimation->getCurrentFrames(animationTime, animationLTime, currentKeyframeIndexA);

            for (auto &t : transitionsFromCurrentState)
            {
                switch (t->condition)
                {
                case COND_ANIMATION_FINISHED:
                    if (currentAnimation->isFinished(animationTime + t->transitionLength))
                    {
                        currentTransition = t;
                        transitioning = true;
                        transitionStart = std::chrono::high_resolution_clock::now();
                        t->to->onEnterAnimation(usr);
                        transitionTime = 0;
                        transitionLTime = 0;

                        // std::fill(currentKeyframeIndexA.begin(), currentKeyframeIndexA.end(), 0);
                        std::fill(currentKeyframeIndexB.begin(), currentKeyframeIndexB.end(), 0);

                        return;
                    }
                    break;
                case COND_CUSTOM:
                    if (t->conditionFunction(usr))
                    {
                        currentTransition = t;
                        transitioning = true;
                        transitionStart = std::chrono::high_resolution_clock::now();
                        t->to->onEnterAnimation(usr);
                        transitionTime = 0;
                        transitionLTime = 0;

                        // std::fill(currentKeyframeIndexA.begin(), currentKeyframeIndexA.end(), 0);
                        std::fill(currentKeyframeIndexB.begin(), currentKeyframeIndexB.end(), 0);

                        return;
                    }
                    break;
                }
            }
        }
        else
        {
            // std::cout << "transitionning omg " << usr << " " << transitionTime << "\t" << currentTransition->transitionLength << "\n";
            transitionTime += dt * currentTransition->to->speedCallback(prct, usr);
            float a = 0;
            switch (currentTransition->type)
            {
                case TRANSITION_LINEAR:
                    a = transitionTime / (currentTransition->transitionLength);
                    break;
                case TRANSITION_SMOOTH:
                    a = smoothstep(0.0f, 1.0f, transitionTime / (currentTransition->transitionLength));
                    break;
            }

            currentKeyframes =
                interpolateKeyframes(currentTransition->from, currentTransition->to,
                                     fmod(animationTime, currentAnimation->getLength()), transitionTime, 
                                     animationTime, transitionLTime , a, currentKeyframeIndexA, currentKeyframeIndexB);

            if (transitionTime >= currentTransition->transitionLength)
            {
                // std::cout << "a: " << a << "\n";
                // std::cout << "time: " << time << "\n";
                // std::cout << "transitionTime: " << transitionTime << "\n";
                currentAnimation->onExitAnimation(usr);
                currentAnimation = currentTransition->to;
                animationStart = transitionStart;
                animationTime = transitionTime;
                animationLTime = transitionLTime;
                transitioning = false;
                getTransitionsFromCurrentState();
                currentKeyframeIndexA = currentKeyframeIndexB;
                std::fill(currentKeyframeIndexB.begin(), currentKeyframeIndexB.end(), 0);
            }
        }
    }

    void applyKeyframes(SkeletonAnimationState &skeleton)
    {
        skeleton.applyKeyframes(currentKeyframes);
    }
};

typedef std::shared_ptr<AnimationController> AnimationControllerRef;