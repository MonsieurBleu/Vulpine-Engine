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
    std::function<bool()> conditionFunction;

    AnimationControllerTransition(AnimationRef _from,
                                  AnimationRef _to,
                                  AnimationControllerTransitionCondition _condition,
                                  float _transitionLength,
                                  TransitionType _type = TRANSITION_LINEAR,
                                  std::function<bool()> _conditionFunction = [](){return false;})
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
    std::vector<AnimationRef> animations;
    std::vector<AnimationControllerTransition> transitions;
    bool transitioning = false; // 🏳️‍⚧️ 
    AnimationControllerTransition *currentTransition = nullptr;
    AnimationRef currentAnimation;
    std::chrono::time_point<std::chrono::high_resolution_clock> animationStart;
    std::chrono::time_point<std::chrono::high_resolution_clock> transitionStart;
    float animationTime = 0;
    float transitionTime = 0;

    std::vector<keyframeData> currentKeyframes;

    std::vector<AnimationControllerTransition *> transitionsFromCurrentState;

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

        if (transitionsFromCurrentState.size() == 0)
        {
            // failsafe if no transition is found, loop back to the same animation
            transitions.push_back(AnimationControllerTransition(currentAnimation, currentAnimation,
                                                                COND_ANIMATION_FINISHED, epsilon<float>()));
        }
    }

public:
    AnimationController(int32_t initialState, const std::vector<AnimationControllerTransition> &_transitions, const std::vector<AnimationRef> &_animations)
    {
        transitions = _transitions;
        animations = _animations;

        currentAnimation = animations[initialState];
        animationStart = std::chrono::high_resolution_clock::now();
        currentAnimation->onEnterAnimation();
        getTransitionsFromCurrentState();
    }

    void update(float dt)
    {
        animationTime += dt * currentAnimation->speedCallback();

        if (!transitioning)
        {
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
                        t->to->onEnterAnimation();
                        transitionTime = 0;
                    }
                    break;
                case COND_CUSTOM:
                    if (t->conditionFunction())
                    {
                        currentTransition = t;
                        transitioning = true;
                        transitionStart = std::chrono::high_resolution_clock::now();
                        t->to->onEnterAnimation();
                        transitionTime = 0;
                    }
                    break;
                }
            }

            currentKeyframes = currentAnimation->getCurrentFrames(animationTime);
        }
        else
        {
            transitionTime += dt * currentTransition->to->speedCallback();
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
                                     fmod(animationTime, currentAnimation->getLength()), transitionTime, a);

            if (transitionTime >= currentTransition->transitionLength)
            {
                // std::cout << "a: " << a << "\n";
                // std::cout << "time: " << time << "\n";
                // std::cout << "transitionTime: " << transitionTime << "\n";
                currentAnimation->onExitAnimation();
                currentAnimation = currentTransition->to;
                animationStart = transitionStart;
                animationTime = transitionTime;
                transitioning = false;
                getTransitionsFromCurrentState();
            }
        }
    }

    void applyKeyframes(SkeletonAnimationState &skeleton)
    {
        skeleton.applyKeyframes(currentKeyframes);
    }
};

typedef std::shared_ptr<AnimationController> AnimationControllerRef;