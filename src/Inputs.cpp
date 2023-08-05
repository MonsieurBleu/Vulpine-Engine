#include <Inputs.hpp>

void giveCallbackToApp(GLFWKeyInfo input)
{
    inputs.add(input);
}

void InputBuffer::add(GLFWKeyInfo input)
{
    inputMutex.lock();
    push_back(input);
    inputMutex.unlock();
};

bool InputBuffer::pull(GLFWKeyInfo& input)
{
    inputMutex.lock();

    bool eventPulled = this->size() != 0;

    if(eventPulled)
    {
        input = front();
        pop_front();
    }

    inputMutex.unlock();

    return eventPulled;
};
