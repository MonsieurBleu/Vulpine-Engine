#include <vector>
#include <unordered_map>
#include <set>
#include <cstdint>
#include <functional>
#include <string>
#include <iostream>

struct DFATransition
{
    int32_t from;
    int32_t symbol;
    int32_t to;
};

// deterministic finite automaton state
// ACCEPTING: state is accepting
// REJECTING: state is rejecting
// ERROR: state is error
enum DFAState
{
    ACCEPTING,
    REJECTING,
    ERROR
};

// read type
// READ_NORMAL: read normal, valid symbol
// READ_EPSILON: read epsilon, no symbol
// READ_INVALID: read invalid, invalid symbol
// it is important to distinguish between epsilon and valid symbols
// since epsilon transitions do not change the read symbol
enum ReadType
{
    // Invalid symbol
    READ_INVALID,

    // Normal symbol
    READ_NORMAL,

    // Epsilon symbol
    READ_EPSILON
};

// template <typename State>
// struct DFAStepInfo {
//     ReadType readType;
//     State* from;
//     State* to;

// };

#define e 0

// deterministic finite automaton
class DFA
{
private:
    int32_t currentState;
    std::vector<int32_t> acceptingStates;
    std::vector<DFATransition *> transitions;
    std::unordered_map<uint64_t, DFATransition *> transitionMap;

    std::function<std::string(int32_t)> state_to_string = [](int32_t state)
    { return std::to_string(state); };
    std::function<std::string(int32_t)> input_to_string = [](int32_t input)
    {
        if (input == e)
        {
            return std::string("e");
        }
        return std::to_string(input);
    };

    DFATransition nullTransition = {-1, -1, -1};

    bool printDebugInfo = false;

    // get the transition from a state on a symbol
    // since the DFA is deterministic, there is only one transition for each state and symbol
    // if there is no transition, return transition with from = -1
    DFATransition *getTransition(int32_t from, int32_t symbol)
    {
        // check if valid non-epsilon transition is present
        for (DFATransition *transition : transitions)
        {
            if (transition->from == from && transition->symbol == symbol)
            {
                return transition;
            }

            if (transition->from == from && transition->symbol == e)
            {
                return transition;
            }
        }

        return &nullTransition;
    }

    std::set<int32_t> getStates()
    {
        std::set<int32_t> states;
        for (DFATransition *transition : transitions)
        {
            states.insert(transition->from);
            states.insert(transition->to);
        }

        return states;
    }

    std::set<int32_t> getSymbols()
    {
        std::set<int32_t> symbols;
        for (DFATransition *transition : transitions)
        {
            symbols.insert(transition->symbol);
        }

        return symbols;
    }

    void computeTransitionMap()
    {
        std::set<int32_t> states = getStates();
        std::set<int32_t> symbols = getSymbols();

        union
        {
            struct
            {
                int32_t state;
                int32_t symbol;
            };
            uint64_t key;
        } u;

        for (int32_t state : states)
        {
            for (int32_t symbol : symbols)
            {
                u.state = state;
                u.symbol = symbol;
                transitionMap[u.key] = getTransition(state, symbol);
            }
        }
    }

    bool isAccepting(int32_t state)
    {
        for (int32_t acceptingState : acceptingStates)
        {
            if (state == acceptingState)
            {
                return true;
            }
        }
        return false;
    }

    void printTransition(int32_t from, int32_t symbol, int32_t to)
    {
        std::cout << state_to_string(from) << " --(" << input_to_string(symbol) << ")--> " << state_to_string(to) << "\n";
    }

    bool checkDeterminism()
    {
        std::set<int32_t> states = getStates();
        std::set<int32_t> symbols = getSymbols();

        for (int32_t state : states)
        {
            for (int32_t symbol : symbols)
            {
                int32_t count = 0;
                for (DFATransition *transition : transitions)
                {
                    if (transition->from == state && transition->symbol == symbol)
                    {
                        count++;
                    }
                }

                if (count > 1)
                {
                    return false;
                }
            }
        }

        return true;
    }

public:
    DFA(int32_t initialState,
        std::vector<int32_t> acceptingStates,
        std::function<std::string(int32_t)> state_to_string,
        std::function<std::string(int32_t)> input_to_string,
        std::vector<DFATransition *> transitions) : currentState(initialState),
                                                    acceptingStates(acceptingStates),
                                                    state_to_string(state_to_string),
                                                    input_to_string(input_to_string),
                                                    transitions(transitions)
    {
        for (DFATransition *transition : transitions)
        {
            if (printDebugInfo)
            {
                printTransition(transition->from, transition->symbol, transition->to);
            }
        }

        if (!checkDeterminism())
        {
            std::cerr << "DFA is not deterministic\n";
            std::cerr << "Behavior is undefined\n";
        }

        computeTransitionMap();
    }

    DFA(int32_t initialState,
        std::vector<int32_t> acceptingStates,
        std::vector<DFATransition *> transitions) : currentState(initialState),
                                                    acceptingStates(acceptingStates),
                                                    transitions(transitions)
    {
        for (DFATransition *transition : transitions)
        {
            if (printDebugInfo)
            {
                printTransition(transition->from, transition->symbol, transition->to);
            }
        }

        if (!checkDeterminism())
        {
            std::cerr << "DFA is not deterministic\n";
            std::cerr << "Behavior is undefined\n";
        }

        computeTransitionMap();
    }

    DFA() : currentState(-1),
            acceptingStates({}),
            transitions({})
    {
    }

    bool addTransition(DFATransition *newTransition)
    {
        DFATransition *transition = getTransition(newTransition->from, newTransition->symbol);
        if (transition->from != -1)
        {
            return false;
        }

        transitions.push_back(newTransition);
        computeTransitionMap();
        return true;
    }

    ReadType step(int32_t symbol)
    {
        union
        {
            struct
            {
                int32_t state;
                int32_t symbol;
            };
            uint64_t key;
        } u = {currentState, symbol};

        DFATransition *transition = transitionMap[u.key];
        if (printDebugInfo)
        {
            printTransition(transition->from, transition->symbol, transition->to);
        }
        if (transition->to == -1)
        {
            return READ_INVALID;
        }

        currentState = transition->to;
        return transition->symbol == e ? READ_EPSILON : READ_NORMAL;
    }

    ReadType step(int32_t symbol, DFATransition *transition)
    {
        union
        {
            struct
            {
                int32_t state;
                int32_t symbol;
            };
            uint64_t key;
        } u = {currentState, symbol};

        transition = transitionMap[u.key];
        if (printDebugInfo)
        {
            printTransition(transition->from, transition->symbol, transition->to);
        }
        if (transition->to == -1)
        {
            return READ_INVALID;
        }

        currentState = transition->to;

        return transition->symbol == e ? READ_EPSILON : READ_NORMAL;
    }

    bool run(std::vector<int32_t> input)
    {
        // verify that the input is valid
        for (int32_t symbol : input)
        {
            if (symbol == e)
            {
                return false;
            }
        }
        uint32_t i = 0;
        while (i < input.size())
        {
            if (printDebugInfo)
            {
                std::cout << "Current state: " << state_to_string(currentState) << "\n";
                std::cout << "Current input: " << input_to_string(input[i]) << "\n";
            }
            ReadType readType = step(input[i]);
            if (readType == READ_INVALID)
            {
                return false;
            }
            else if (readType == READ_NORMAL)
            {
                i++;
            }
        }

        return isAccepting(currentState);
    }

    int32_t getCurrentState()
    {
        return currentState;
    }
};