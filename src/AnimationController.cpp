#include <Graphics/AnimationController.hpp>

void AnimationController::getTransitionsFromCurrentState()
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

AnimationController::AnimationController(
    const std::vector<AnimationControllerTransition> &_transitions, 
    AnimationRef &initialState, 
    void *usr
) : usr(usr)
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

void AnimationController::update(float dt)
{
    float prct = 100.f * animationTime / currentAnimation->getLength();
    prct = clamp(prct, 0.f, 100.f);
    float prctTransition = 100.f * transitionTime / currentAnimation->getLength();

    float currentSpeed = currentAnimation->speedCallback(prct, usr);
    animationTime += dt * currentSpeed;

    if(!currentAnimation->repeat)
        animationTime = clamp(animationTime, 0.f, currentAnimation->getLength());

    auto switchToTransition = [&](AnimationControllerTransition *t)
    {
        // WARNING_MESSAGE(t->from->getName() << " to" << t->to->getName())

        currentAnimation->onExitAnimation(usr);
        currentTransition = t;
        currentAnimation = currentTransition->to;
        currentAnimation->onEnterAnimation(usr);

        transitioning = true;
        transitionStart = std::chrono::high_resolution_clock::now();
        transitionTimeSinceStart = 0;
        transitionTime = animationTime;
        transitionLTime = animationLTime;


        animationStart = std::chrono::high_resolution_clock::now();
        // animationTimeB = animationTime;
        animationTime = 0;
        animationLTime = 0;

        getTransitionsFromCurrentState();

        currentKeyframeIndexB = currentKeyframeIndexA;
        std::fill(currentKeyframeIndexA.begin(), currentKeyframeIndexA.end(), 0);
        // std::fill(currentKeyframeIndexB.begin(), currentKeyframeIndexB.end(), 0);

    };

    // if (!transitioning)
    {
        [&](){
            for (auto &t : transitionsFromCurrentState)
                if(currentTransition != t)
                {
                    switch (t->condition)
                    {
                    case COND_ANIMATION_FINISHED:
                        // if (currentAnimation->isFinished(animationTime + t->transitionLength))
                        if (currentAnimation->isFinished(animationTime))
                        {
                            switchToTransition(t);
                            return;
                        }
                        break;
                    case COND_CUSTOM:
                        if (t->conditionFunction(usr))
                        {
                            switchToTransition(t);
                            return;
                        }
                        break;
                    }
                }
        }();
    }
    // else
    if(transitioning)
    {
        // std::cout << "transitionning omg " << usr << " " << transitionTime << "\t" << currentTransition->transitionLength << "\n";
        // transitionTime += dt * currentTransition->to->speedCallback(prctTransition, usr);
        transitionTime += dt * currentSpeed;
        transitionTimeSinceStart += dt;

        float a = 0;
        switch (currentTransition->type)
        {
            case TRANSITION_LINEAR:
                a = transitionTimeSinceStart / (currentTransition->transitionLength);
                break;
            case TRANSITION_SMOOTH:
                a = smoothstep(0.0f, 1.0f, transitionTimeSinceStart / (currentTransition->transitionLength));
                break;
        }

        // interpolateKeyframes(currentTransition->from, currentTransition->to,
        //                     fmod(animationTime, currentAnimation->getLength()), transitionTime, 
        //                     animationLTime, transitionLTime , 
        //                     a, currentKeyframeIndexA, currentKeyframeIndexB, currentKeyframes);

        float timeA = currentTransition->to->repeat ? 
            fmod(animationTime, currentTransition->to->getLength()) : min(animationTime, currentTransition->to->getLength());

        float timeB = currentTransition->from->repeat ? 
            fmod(transitionTime, currentTransition->from->getLength()) : min(transitionTime, currentTransition->from->getLength());

        // timeA = animationTime;
        // timeB = transitionLTime;

        interpolateKeyframes(currentTransition->to, currentTransition->from,
                            // fmod(animationTime, currentAnimation->getLength()), transitionTime, 
                            timeA, timeB,
                            animationLTime, transitionLTime, 
                            (1.f-a)
                            // a
                            , currentKeyframeIndexA, currentKeyframeIndexB, currentKeyframes);

        if (transitionTimeSinceStart >= currentTransition->transitionLength)
        {
            // std::cout << "a: " << a << "\n";
            // std::cout << "time: " << time << "\n";
            // std::cout << "transitionTime: " << transitionTime << "\n";

            // currentAnimation->onExitAnimation(usr);
            // currentAnimation = currentTransition->to;
            // animationStart = transitionStart;
            // animationTime = transitionTime;
            // animationLTime = transitionLTime;
            transitioning = false;
            // getTransitionsFromCurrentState();
            // currentKeyframeIndexA = currentKeyframeIndexB;

            std::fill(currentKeyframeIndexB.begin(), currentKeyframeIndexB.end(), 0);
        }
    }
    else
    {
        currentAnimation->getCurrentFrames(animationTime, animationLTime, currentKeyframeIndexA, currentKeyframes);
    }
}

void AnimationController::applyKeyframes(SkeletonAnimationState &skeleton)
{
    skeleton.applyKeyframes(currentKeyframes);
}