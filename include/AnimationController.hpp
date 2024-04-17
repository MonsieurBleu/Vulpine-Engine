#include "Animation.hpp"
#include "Utils.hpp"

#include <iostream>
#include <fstream>

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

auto falseFunction = []()
{ return false; };

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
                                  std::function<bool()> _conditionFunction = falseFunction)
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
    // DFA automaton;
    std::vector<AnimationRef> animations;
    std::vector<AnimationControllerTransition> transitions;
    bool transitioning = false; // insert trans flag emoji
    AnimationControllerTransition *currentTransition = nullptr;
    AnimationRef currentAnimation;
    std::chrono::time_point<std::chrono::high_resolution_clock> animationStart;
    std::chrono::time_point<std::chrono::high_resolution_clock> transitionStart;
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
            transitions.push_back(AnimationControllerTransition(currentAnimation, currentAnimation, COND_ANIMATION_FINISHED, epsilon<float>()));
        }
    }

public:
    AnimationController(int32_t initialState, std::vector<AnimationControllerTransition> &_transitions, std::vector<AnimationRef> &_animations)
    {
        transitions = _transitions;
        animations = _animations;

        currentAnimation = animations[initialState];
        animationStart = std::chrono::high_resolution_clock::now();
        getTransitionsFromCurrentState();
    }

    void update()
    {
        float time = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - animationStart).count();

        if (!transitioning)
        {
            for (auto &t : transitionsFromCurrentState)
            {
                switch (t->condition)
                {
                case COND_ANIMATION_FINISHED:
                    if (currentAnimation->isFinished(time + t->transitionLength))
                    {
                        currentTransition = t;
                        transitioning = true;
                        transitionStart = std::chrono::high_resolution_clock::now();
                    }
                    break;
                case COND_CUSTOM:
                    if (t->conditionFunction())
                    {
                        currentTransition = t;
                        transitioning = true;
                        transitionStart = std::chrono::high_resolution_clock::now();
                    }
                    break;
                }
            }

            currentKeyframes = currentAnimation->getCurrentFrames(time);
        }
        else
        {
            float transitionTime = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - transitionStart).count();
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

            currentKeyframes = interpolateKeyframes(currentTransition->from, currentTransition->to, fmod(time, currentAnimation->getLength()), transitionTime, a);

            if (transitionTime >= currentTransition->transitionLength)
            {
                // std::cout << "a: " << a << "\n";
                // std::cout << "time: " << time << "\n";
                // std::cout << "transitionTime: " << transitionTime << "\n";
                currentAnimation = currentTransition->to;
                animationStart = transitionStart;
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