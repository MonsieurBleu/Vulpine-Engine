#pragma once

#include "AssetManager.hpp"
#include "Globals.hpp"
#include "MappedEnum.hpp"
#include "Scripting/ScriptInstance.hpp"
#include "Utils.hpp"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct ScriptNameWrapper {
    std::string scriptName;
};

struct Flag : std::enable_shared_from_this<Flag> {
    GENERATE_ENUM_FAST_REVERSE(Type,
        INT,
        FLOAT,
        STRING,
        BOOL,
        NONE);
    Type type;
    bool isScripted = false;

    Flag(Type t, bool scripted = false)
        : type(t)
        , isScripted(scripted)
    {
    }
    Flag()
        : type(NONE)
        , isScripted(false) { };

    static std::shared_ptr<Flag> MakeFlag(int value);
    static std::shared_ptr<Flag> MakeFlag(float value);
    static std::shared_ptr<Flag> MakeFlag(const std::string& value);
    static std::shared_ptr<Flag> MakeFlag(const char* value);
    static std::shared_ptr<Flag> MakeFlag(bool value);
    template <typename T>
    static std::shared_ptr<Flag> MakeFlagFromScript(const std::string& scriptName);

    template <>
    std::shared_ptr<Flag> MakeFlagFromScript<int>(const std::string& scriptName);
    template <>
    std::shared_ptr<Flag> MakeFlagFromScript<float>(const std::string& scriptName);
    template <>
    std::shared_ptr<Flag> MakeFlagFromScript<std::string>(const std::string& scriptName);
    template <>
    std::shared_ptr<Flag> MakeFlagFromScript<bool>(const std::string& scriptName);

    Flag& operator=(int v);
    Flag& operator=(float v);
    Flag& operator=(const std::string& v);
    Flag& operator=(bool v);
    template <typename T>
    Flag& operator=(const ScriptNameWrapper& v);

    template <>
    Flag& operator= <int>(const ScriptNameWrapper& v);
    template <>
    Flag& operator= <float>(const ScriptNameWrapper& v);
    template <>
    Flag& operator= <std::string>(const ScriptNameWrapper& v);
    template <>
    Flag& operator= <bool>(const ScriptNameWrapper& v);

    virtual int as_int() = 0;
    virtual float as_float() = 0;
    virtual std::string as_string() = 0;
    virtual bool as_bool() = 0;

    std::string typeToString();
    typedef std::shared_ptr<Flag> FlagPtr;

    static bool equals(const FlagPtr a, const FlagPtr b);
    static bool notEquals(const FlagPtr a, const FlagPtr b);
    static bool lessThan(const FlagPtr a, const FlagPtr b);
    static bool greaterThan(const FlagPtr a, const FlagPtr b);
    static bool lessThanOrEqual(const FlagPtr a, const FlagPtr b);
    static bool greaterThanOrEqual(const FlagPtr a, const FlagPtr b);
    static bool logicalAnd(const FlagPtr a, const FlagPtr b);
    static bool logicalOr(const FlagPtr a, const FlagPtr b);
    static bool logicalNot(const FlagPtr a);
    static FlagPtr add(const FlagPtr a, const FlagPtr b);
    static FlagPtr subtract(const FlagPtr a, const FlagPtr b);
    static FlagPtr multiply(const FlagPtr a, const FlagPtr b);
    static FlagPtr divide(const FlagPtr a, const FlagPtr b);

    FlagPtr clone();

    operator struct FlagWrapper();
};
typedef std::shared_ptr<Flag> FlagPtr;

struct ScriptFlagBase : Flag {
    std::string luaScriptName;
    uint64_t lastUpdate = (uint64_t)(-1); // Force update on first get
    ScriptFlagBase(Type t, const std::string& scriptName)
        : Flag(t, true)
        , luaScriptName(scriptName)
    {
    }
};

template <typename T>
struct ScriptFlag : ScriptFlagBase {
    T lastValue;

    ScriptFlag(Type t, const std::string& scriptName)
        : ScriptFlagBase(t, scriptName)
    {
    }
    T get(T defaultValue)
    {
        if (lastUpdate == globals.appTime.getUpdateCounter())
            return lastValue;

        lastUpdate = globals.appTime.getUpdateCounter();

        if (Loader<ScriptInstance>::loadingInfos.find(luaScriptName) != Loader<ScriptInstance>::loadingInfos.end()) {
            std::optional<T> r = Loader<ScriptInstance>::get(luaScriptName).template runAndReturn<T>();
            if (r.has_value()) {
                lastValue = r.value();
                return lastValue;
            }
        }
        lastValue = defaultValue;
        return lastValue;
    }
};

struct IntFlag : Flag {
    int value;

    IntFlag(int v)
        : Flag(INT)
        , value(v)
    {
    }

    int as_int() override { return value; }

    float as_float() override { return static_cast<float>(value); }

    std::string as_string() override { return std::to_string(value); }

    bool as_bool() override { return value != 0; }
};

struct IntScriptFlag : ScriptFlag<int> {
    IntScriptFlag(const std::string& scriptName)
        : ScriptFlag(INT, scriptName)
    {
    }

    int as_int() override { return get(0); }
    float as_float() override { return static_cast<float>(as_int()); }
    std::string as_string() override { return std::to_string(as_int()); }
    bool as_bool() override { return as_int() != 0; }
};

struct FloatFlag : Flag {
    float value;

    FloatFlag(float v)
        : Flag(FLOAT)
        , value(v)
    {
    }

    int as_int() override { return static_cast<int>(value); }
    float as_float() override { return value; }
    std::string as_string() override { return std::to_string(value); }
    bool as_bool() override { return value != 0.0f; }
};

struct FloatScriptFlag : ScriptFlag<float> {
    FloatScriptFlag(const std::string& scriptName)
        : ScriptFlag(FLOAT, scriptName)
    {
    }

    float as_float() override { return get(0.0f); }
    int as_int() override { return static_cast<int>(as_float()); }
    std::string as_string() override { return std::to_string(as_float()); }
    bool as_bool() override { return as_float() != 0.0f; }
};

struct StrFlag : Flag {
    std::string value;

    StrFlag(const std::string& v)
        : Flag(STRING)
        , value(v)
    {
    }

    int as_int() override
    {
        try {
            return std::stoi(value);
        } catch (...) {
            WARNING_MESSAGE("Failed to convert string to int: " + value);
            return 0;
        }
    }

    float as_float() override
    {
        try {
            return std::stof(value);
        } catch (...) {
            WARNING_MESSAGE("Failed to convert string to float: " + value);
            return 0.0f;
        }
    }

    std::string as_string() override { return value; }

    bool as_bool() override
    {
        WARNING_MESSAGE("Converting string to bool, this is probably not what you want, returning true if not empty");
        return !value.empty();
    }
};

struct StrScriptFlag : ScriptFlag<std::string> {
    StrScriptFlag(const std::string& scriptName)
        : ScriptFlag(STRING, scriptName)
    {
    }

    std::string as_string() override { return get(""); }

    int as_int() override
    {
        try {
            return std::stoi(as_string());
        } catch (...) {
            WARNING_MESSAGE("Failed to convert string to int: " + as_string());
            return 0;
        }
    }

    float as_float() override
    {
        try {
            return std::stof(as_string());
        } catch (...) {
            WARNING_MESSAGE("Failed to convert string to float: " + as_string());
            return 0.0f;
        }
    }

    bool as_bool() override { return !as_string().empty(); }
};

struct BoolFlag : Flag {
    bool value;

    BoolFlag(bool v)
        : Flag(BOOL)
        , value(v)
    {
    }

    int as_int() override { return value ? 1 : 0; }
    float as_float() override { return value ? 1.0f : 0.0f; }
    std::string as_string() override { return value ? "true" : "false"; }
    bool as_bool() override { return value; }
};

struct BoolScriptFlag : ScriptFlag<bool> {
    std::string luaScriptName;

    BoolScriptFlag(const std::string& scriptName)
        : ScriptFlag(BOOL, scriptName)
    {
    }

    bool as_bool() override { return get(false); }
    int as_int() override { return as_bool() ? 1 : 0; }
    float as_float() override { return as_bool() ? 1.0f : 0.0f; }
    std::string as_string() override { return as_bool() ? "true" : "false"; }
};



struct FlagWrapper {
    FlagPtr flag;
    FlagWrapper()
        : flag(nullptr)
    {
    }
    FlagWrapper(FlagPtr f)
        : flag(f)
    {
    }

    FlagWrapper& operator=(int v)
    {
        if (flag) {
            if (flag->type == Flag::Type::INT) {
                *flag = v;
                return *this;
            }
        }
        flag = Flag::MakeFlag(v);
        return *this;
    }

    FlagWrapper& operator=(float v)
    {
        if (flag) {
            if (flag->type == Flag::Type::FLOAT) {
                *flag = v;
                return *this;
            }
        }
        flag = Flag::MakeFlag(v);
        return *this;
    }

    FlagWrapper& operator=(const std::string& v)
    {
        if (flag) {
            if (flag->type == Flag::Type::STRING) {
                *flag = v;
                return *this;
            }
        }
        flag = Flag::MakeFlag(v);
        return *this;
    }

    FlagWrapper& operator=(const char* v)
    {
        if (flag) {
            if (flag->type == Flag::Type::STRING) {
                *flag = std::string(v);
            }
        }
        flag = Flag::MakeFlag(std::string(v));
        return *this;
    }

    FlagWrapper& operator=(bool v)
    {
        if (flag) {
            if (flag->type == Flag::Type::BOOL) {
                *flag = v;
                return *this;
            }
        }
        flag = Flag::MakeFlag(v);
        return *this;
    }

    template <typename T>
    FlagWrapper& operator=(const ScriptNameWrapper& v);

    template <>
    FlagWrapper& operator= <int>(const ScriptNameWrapper& v)
    {
        flag = Flag::MakeFlagFromScript<int>(v.scriptName);

        return *this;
    }

    template <>
    FlagWrapper& operator= <float>(const ScriptNameWrapper& v)
    {
        flag = Flag::MakeFlagFromScript<float>(v.scriptName);
        return *this;
    }

    template <>
    FlagWrapper& operator= <std::string>(const ScriptNameWrapper& v)
    {
        flag = Flag::MakeFlagFromScript<std::string>(v.scriptName);
        return *this;
    }

    template <>
    FlagWrapper& operator= <bool>(const ScriptNameWrapper& v)
    {
        flag = Flag::MakeFlagFromScript<bool>(v.scriptName);
        return *this;
    }

    Flag& operator*() const
    {
        return *flag;
    }

    Flag* operator->() const
    {
        return flag.get();
    }

    operator bool() const
    {
        return flag != nullptr && flag->as_bool();
    }

    operator int() const
    {
        return flag ? flag->as_int() : 0;
    }

    operator float() const
    {
        return flag ? flag->as_float() : 0.0f;
    }

    operator std::string() const
    {
        return flag ? flag->as_string() : "";
    }
};


class Flags {
private:
    std::unordered_map<std::string, FlagWrapper> flags;

public:

    bool writeAsSaveFileMode = false;

    static FlagWrapper& getFlag(const std::string& name);

    template <typename T>
    void setFlag(const std::string& flag, T value)
    {
        flags[flag] = Flag::MakeFlag(value);
    }

    template <typename T>
    void setFlagFromScript(const std::string& flag, const std::string& scriptName)
    {
        flags[flag] = Flag::MakeFlagFromScript<T>(scriptName);
    }

    template <typename T>
    void setFlag(const std::string& flag, const ScriptNameWrapper& value)
    {
        flags[flag] = Flag::MakeFlagFromScript<T>(value.scriptName);
    }

    void setFlag(const std::string& flag, FlagPtr value)
    {
        flags[flag] = value;
    }

    std::vector<std::pair<std::string, FlagWrapper>> getAllFlags() const
    {
        std::vector<std::pair<std::string, FlagWrapper>> result;
        for (const auto& pair : flags) {
            result.push_back(pair);
        }
        return result;
    }

    const std::unordered_map<std::string, FlagWrapper>& getAllFlagsMap() const
    {
        return flags;
    }

    std::vector<std::pair<std::string, FlagWrapper>> getAllByValueFlags() const
    {
        std::vector<std::pair<std::string, FlagWrapper>> result;
        for (const auto& pair : flags) {
            if (!pair.second->isScripted)
                result.push_back(pair);
        }
        return result;
    }

    std::vector<std::pair<std::string, FlagWrapper>> getAllScriptedFlags() const
    {
        std::vector<std::pair<std::string, FlagWrapper>> result;
        for (const auto& pair : flags) {
            if (pair.second->isScripted)
                result.push_back(pair);
        }
        return result;
    }

    FlagWrapper& operator[](const std::string& name)
    {
        // return getFlag(name);
        return this->flags[name];
    }
};

// static inline Flags flags;

class LogicBlock {
public:
    class Function {
    private:
        std::string name;
        std::vector<Flag::Type> expectedArgTypes;
        Flag::Type expectedReturnType;
        typedef std::function<FlagPtr(const std::vector<FlagPtr>&)> FlagFuncType;
        FlagFuncType func;

    public:
        Function(const std::string& name, FlagFuncType func)
            : name(name)
            , func(func)
        {
            WARNING_MESSAGE(
                "Function " + name + " created without argument type checking. This may lead to runtime errors.");
        }

        Function(const std::string& name, Flag::Type expectedReturnType, const std::vector<Flag::Type>& argTypes, FlagFuncType func)
            : name(name)
            , expectedArgTypes(argTypes)
            , expectedReturnType(expectedReturnType)
            , func(func)
        {
        }

        Function() : name(""), func(nullptr) {}

        const std::string& getName() const
        {
            return name;
        }

        Flag::Type getReturnType() const
        {
            return expectedReturnType;
        }

        Flag::Type getArgType(size_t index) const
        {
            if (index >= expectedArgTypes.size()) {
                WARNING_MESSAGE("Function " + name + " argument index out of range");
                return Flag::Type::NONE;
            }
            return expectedArgTypes[index];
        }

        std::optional<FlagPtr> call(const std::vector<FlagPtr>& args) const
        {
            if (expectedArgTypes.size() == 0) // No type checking
                return func(args);

            if (expectedArgTypes.size() != args.size()) {
                WARNING_MESSAGE("Function " + name + " called with incorrect number of arguments. Expected " +
                                std::to_string(expectedArgTypes.size()) + ", got " + std::to_string(args.size()) + ".");
                return std::nullopt;
            }

            for (size_t i = 0; i < expectedArgTypes.size(); i++) {
                if (expectedArgTypes[i] != args[i]->type) {
                    // check if cast is possible
                    if ((expectedArgTypes[i] == Flag::INT && args[i]->type == Flag::FLOAT) ||
                        (expectedArgTypes[i] == Flag::FLOAT && args[i]->type == Flag::INT)) {
                        continue; // allow int<->float casting
                    }

                    WARNING_MESSAGE("Function " + name + " called with incorrect argument type for argument " +
                                    std::to_string(i) + ". Expected " +
                                    Flag::TypeReverseMap[expectedArgTypes[i]] + ", got " +
                                    Flag::TypeReverseMap[args[i]->type] + ".");
                    return std::nullopt;
                }
            }

            return func(args);
        }
    };

private:
    static inline std::unordered_map<std::string, Function> functions;

    struct OperationNode : std::enable_shared_from_this<OperationNode> {
        enum type {
            VALUE, // all the stuff that's not an operator
            LOGICAL_NOT,
            MULTIPLICATION, // and division
            ADDITION, // and subtraction
            COMPARISON,
            EQUALITY,
            LOGICAL_AND,
            LOGICAL_OR
        } nodeType;

        std::vector<std::shared_ptr<OperationNode>> children;
        int expectedChildren = 0;
        std::shared_ptr<OperationNode> parent;
        virtual FlagPtr evaluate() = 0;

        void addChild(std::shared_ptr<OperationNode> child)
        {
            if (children.size() + 1 > expectedChildren) {
                WARNING_MESSAGE("Trying to add too many children to OperationNode");
                return;
            }
            child->parent = shared_from_this();
            children.push_back(child);
        }
    };

    typedef std::shared_ptr<OperationNode> OperationNodePtr;

    struct FlagValueNode : OperationNode {
        FlagPtr value;

        FlagValueNode(FlagPtr v)
            : value(v)
        {
            nodeType = VALUE;
            expectedChildren = 0;
        }

        FlagPtr evaluate() override
        {
            return value;
        }
    };

    struct FunctionNode : OperationNode {
        std::string functionName;

        std::vector<std::string> argumentStrings;

        FunctionNode(const std::string& fname, const std::vector<std::string>& argStrs)
            : functionName(fname)
            , argumentStrings(argStrs)
        {
            nodeType = VALUE;
            expectedChildren = 0;
        }

        FlagPtr evaluate() override;
    };

    struct NotOperatorNode : OperationNode {
        NotOperatorNode()
        {
            nodeType = LOGICAL_NOT;
            expectedChildren = 1;
        }

        FlagPtr evaluate() override;
    };

    struct AdditionOperatorNode : OperationNode {
        enum AdditionType {
            ADD,
            SUBTRACT
        } additionType;

        AdditionOperatorNode(AdditionType at)
            : additionType(at)
        {
            nodeType = ADDITION;
            expectedChildren = 2;
        }

        FlagPtr evaluate() override;
    };

    struct MultiplicationOperatorNode : OperationNode {
        enum MultiplicationType {
            MULTIPLY,
            DIVIDE
        } multiplicationType;

        MultiplicationOperatorNode(MultiplicationType mt)
            : multiplicationType(mt)
        {
            nodeType = MULTIPLICATION;
            expectedChildren = 2;
        }

        FlagPtr evaluate() override;
    };

    struct ComparisonOperatorNode : OperationNode {
        enum ComparisonType {
            LESS_THAN,
            GREATER_THAN,
            LESS_THAN_OR_EQUAL,
            GREATER_THAN_OR_EQUAL
        } comparisonType;

        ComparisonOperatorNode(ComparisonType ct)
            : comparisonType(ct)
        {
            nodeType = COMPARISON;
            expectedChildren = 2;
        }

        FlagPtr evaluate() override;
    };

    struct EqualityOperatorNode : OperationNode {
        enum EqualityType {
            EQUALS,
            NOT_EQUALS
        } equalityType;

        EqualityOperatorNode(EqualityType et)
            : equalityType(et)
        {
            nodeType = EQUALITY;
            expectedChildren = 2;
        }

        FlagPtr evaluate() override;
    };

    struct LogicalAndOperatorNode : OperationNode {
        LogicalAndOperatorNode()
        {
            nodeType = LOGICAL_AND;
            expectedChildren = 2;
        }

        FlagPtr evaluate() override;
    };

    struct LogicalOrOperatorNode : OperationNode {
        LogicalOrOperatorNode()
        {
            nodeType = LOGICAL_OR;
            expectedChildren = 2;
        }

        FlagPtr evaluate() override;
    };

    class Token {
    public:
        GENERATE_ENUM_FAST_REVERSE(TokenType,
            FLAG,
            FUNCTION,
            PARENS_OPEN,
            PARENS_CLOSE,
            LOGICAL_AND,
            LOGICAL_OR,
            NOT,
            ADD,
            SUBTRACT,
            MULTIPLY,
            DIVIDE,
            EQUALS,
            NOT_EQUALS,
            LESS_THAN,
            GREATER_THAN,
            LESS_THAN_OR_EQUAL,
            GREATER_THAN_OR_EQUAL
        );

        TokenType type;

        std::string value;
        
        Token(TokenType t, const std::string& v)
            : type(t)
            , value(v)
        { }

        OperationNodePtr toOperationNode() const;
    };


    thread_local static inline struct ErrorInfos
    {
        bool success;
        std::string message;
    } error;

public:
    static void registerFunction(const Function& function)
    {
        functions[function.getName()] = function;
    }

    static void parse_string(std::string& str);
    static FlagPtr parse_substring(const std::string& str, size_t idx_start, size_t idx_end);
    

    static const ErrorInfos& parse_string_cstr(char ** input, size_t &len, size_t Allocated);
    static FlagPtr parse_substring_cstr(const char* input, const size_t len, const size_t idx_start, const size_t idx_end);

    
private:
    static OperationNodePtr buildOperationTree(std::vector<Token> tokens);
    static void tokenize(const std::string& str, std::vector<Token> &tokens); 
};