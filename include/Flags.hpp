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

struct FlagData : std::enable_shared_from_this<FlagData> {
    GENERATE_ENUM_FAST_REVERSE(Type,
        INT,
        FLOAT,
        STRING,
        BOOL,
        NONE);
    Type type;
    bool isScripted = false;
    bool isLogicBlock = false;

    FlagData(Type t, bool scripted = false, bool logicBlock = false)
        : type(t)
        , isScripted(scripted)
        , isLogicBlock(logicBlock)
    {
    }
    FlagData()
        : type(NONE)
        , isScripted(false) 
        , isLogicBlock(false) { };

    static std::shared_ptr<FlagData> MakeFlag(int value);
    static std::shared_ptr<FlagData> MakeFlag(float value);
    static std::shared_ptr<FlagData> MakeFlag(const std::string& value);
    static std::shared_ptr<FlagData> MakeFlag(const char* value);
    static std::shared_ptr<FlagData> MakeFlag(bool value);

    template <typename T>
    static std::shared_ptr<FlagData> MakeFlagFromScript(const std::string& scriptName);


    template <typename T>
    static std::shared_ptr<FlagData> MakeFlagFromLogicBlock(const std::string& logicBlock);


    FlagData& operator=(int v);
    FlagData& operator=(float v);
    FlagData& operator=(const std::string& v);
    FlagData& operator=(bool v);
    template <typename T>
    FlagData& operator=(const ScriptNameWrapper& v);



    virtual int as_int() = 0;
    virtual float as_float() = 0;
    virtual std::string as_string() = 0;
    virtual bool as_bool() = 0;

    std::string typeToString();
    typedef std::shared_ptr<FlagData> FlagDataPtr;

    static bool equals(const FlagDataPtr a, const FlagDataPtr b);
    static bool notEquals(const FlagDataPtr a, const FlagDataPtr b);
    static bool lessThan(const FlagDataPtr a, const FlagDataPtr b);
    static bool greaterThan(const FlagDataPtr a, const FlagDataPtr b);
    static bool lessThanOrEqual(const FlagDataPtr a, const FlagDataPtr b);
    static bool greaterThanOrEqual(const FlagDataPtr a, const FlagDataPtr b);
    static bool logicalAnd(const FlagDataPtr a, const FlagDataPtr b);
    static bool logicalOr(const FlagDataPtr a, const FlagDataPtr b);
    static bool logicalNot(const FlagDataPtr a);
    static FlagDataPtr add(const FlagDataPtr a, const FlagDataPtr b);
    static FlagDataPtr subtract(const FlagDataPtr a, const FlagDataPtr b);
    static FlagDataPtr multiply(const FlagDataPtr a, const FlagDataPtr b);
    static FlagDataPtr divide(const FlagDataPtr a, const FlagDataPtr b);

    FlagDataPtr clone();

    operator struct Flag();
};
typedef std::shared_ptr<FlagData> FlagDataPtr;


struct ScriptFlagBase : FlagData {
    std::string luaScriptName;
    // uint64_t lastUpdate = (uint64_t)(-1); // Force update on first get
    ScriptFlagBase(Type t, const std::string& scriptName)
        : FlagData(t, true, false)
        , luaScriptName(scriptName)
    {
    }
};

template <typename T>
struct ScriptFlag : ScriptFlagBase {
    // T lastValue;

    ScriptFlag(Type t, const std::string& scriptName)
        : ScriptFlagBase(t, scriptName)
    {
    }
    // T get(T defaultValue)
    T get()
    {
        // if (lastUpdate == globals.appTime.getUpdateCounter())
        //     return lastValue;

        // lastUpdate = globals.appTime.getUpdateCounter();

        if (Loader<ScriptInstance>::loadingInfos.find(luaScriptName) != Loader<ScriptInstance>::loadingInfos.end()) {
            std::optional<T> r = Loader<ScriptInstance>::get(luaScriptName).template runAndReturn<T>();
            if (r.has_value()) {
                // lastValue = r.value();
                // return lastValue;
                return r.value();
            }
        }
        // lastValue = defaultValue;
        // return lastValue;
        return T();
    }
};

struct IntFlag : FlagData {
    int value;

    IntFlag(int v)
        : FlagData(INT)
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

    int as_int() override { return get(); }
    float as_float() override { return static_cast<float>(as_int()); }
    std::string as_string() override { return std::to_string(as_int()); }
    bool as_bool() override { return as_int() != 0; }
};

struct FloatFlag : FlagData {
    float value;

    FloatFlag(float v)
        : FlagData(FLOAT)
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

    float as_float() override { return get(); }
    int as_int() override { return static_cast<int>(as_float()); }
    std::string as_string() override { return std::to_string(as_float()); }
    bool as_bool() override { return as_float() != 0.0f; }
};

struct StrFlag : FlagData {
    std::string value;

    StrFlag(const std::string& v)
        : FlagData(STRING)
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

    std::string as_string() override { return get(); }

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

struct BoolFlag : FlagData {
    bool value;

    BoolFlag(bool v)
        : FlagData(BOOL)
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

    bool as_bool() override { return get(); }
    int as_int() override { return as_bool() ? 1 : 0; }
    float as_float() override { return as_bool() ? 1.0f : 0.0f; }
    std::string as_string() override { return as_bool() ? "true" : "false"; }
};



struct Flag {
    FlagDataPtr flag;
    Flag()
        : flag(nullptr)
    {
    }
    Flag(FlagDataPtr f)
        : flag(f)
    {
    }

    Flag& operator=(int v)
    {
        if (flag) {
            if (flag->type == FlagData::Type::INT) {
                *flag = v;
                return *this;
            }
        }
        flag = FlagData::MakeFlag(v);
        return *this;
    }

    Flag& operator=(float v)
    {
        if (flag) {
            if (flag->type == FlagData::Type::FLOAT) {
                *flag = v;
                return *this;
            }
        }
        flag = FlagData::MakeFlag(v);
        return *this;
    }

    Flag& operator=(const std::string& v)
    {
        if (flag) {
            if (flag->type == FlagData::Type::STRING) {
                *flag = v;
                return *this;
            }
        }
        flag = FlagData::MakeFlag(v);
        return *this;
    }

    Flag& operator=(const char* v)
    {
        if (flag) {
            if (flag->type == FlagData::Type::STRING) {
                *flag = std::string(v);
            }
        }
        flag = FlagData::MakeFlag(std::string(v));
        return *this;
    }

    Flag& operator=(bool v)
    {
        if (flag) {
            if (flag->type == FlagData::Type::BOOL) {
                *flag = v;
                return *this;
            }
        }
        flag = FlagData::MakeFlag(v);
        return *this;
    }

    template <typename T>
    Flag& operator=(const ScriptNameWrapper& v);

    FlagData& operator*() const
    {
        return *flag;
    }

    FlagData* operator->() const
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
    std::unordered_map<std::string, Flag> flags;

public:

    bool writeAsSaveFileMode = false;

    static Flag& getFlag(const std::string& name);

    template <typename T>
    void setFlag(const std::string& flag, T value)
    {
        flags[flag] = FlagData::MakeFlag(value);
    }

    template<typename T>
    void setFlagFromLogicBlock(const std::string& flag, const std::string& logicBlock)
    {
        flags[flag] = FlagData::MakeFlagFromLogicBlock<T>(logicBlock);
    }
    

    template <typename T>
    void setFlagFromScript(const std::string& flag, const std::string& scriptName)
    {
        flags[flag] = FlagData::MakeFlagFromScript<T>(scriptName);
    }

    template <typename T>
    void setFlag(const std::string& flag, const ScriptNameWrapper& value)
    {
        flags[flag] = FlagData::MakeFlagFromScript<T>(value.scriptName);
    }

    void setFlag(const std::string& flag, FlagDataPtr value)
    {
        flags[flag] = value;
    }

    std::vector<std::pair<std::string, Flag>> getAllFlags() const
    {
        std::vector<std::pair<std::string, Flag>> result;
        for (const auto& pair : flags) {
            result.push_back(pair);
        }
        return result;
    }

    const std::unordered_map<std::string, Flag>& getAllFlagsMap() const
    {
        return flags;
    }

    std::vector<std::pair<std::string, Flag>> getAllByValueFlags() const
    {
        std::vector<std::pair<std::string, Flag>> result;
        for (const auto& pair : flags) {
            if (!pair.second->isScripted)
                result.push_back(pair);
        }
        return result;
    }

    std::vector<std::pair<std::string, Flag>> getAllScriptedFlags() const
    {
        std::vector<std::pair<std::string, Flag>> result;
        for (const auto& pair : flags) {
            if (pair.second->isScripted)
                result.push_back(pair);
        }
        return result;
    }

    Flag& operator[](const std::string& name)
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
        std::vector<FlagData::Type> expectedArgTypes;
        FlagData::Type expectedReturnType;
        typedef std::function<FlagDataPtr(const std::vector<FlagDataPtr>&)> FlagFuncType;
        FlagFuncType func;

    public:
        Function(const std::string& name, FlagFuncType func)
            : name(name)
            , func(func)
        {
            WARNING_MESSAGE(
                "Function " + name + " created without argument type checking. This may lead to runtime errors.");
        }

        Function(const std::string& name, FlagData::Type expectedReturnType, const std::vector<FlagData::Type>& argTypes, FlagFuncType func)
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

        FlagData::Type getReturnType() const
        {
            return expectedReturnType;
        }

        FlagData::Type getArgType(size_t index) const
        {
            if (index >= expectedArgTypes.size()) {
                WARNING_MESSAGE("Function " + name + " argument index out of range");
                return FlagData::Type::NONE;
            }
            return expectedArgTypes[index];
        }

        std::optional<FlagDataPtr> call(const std::vector<FlagDataPtr>& args, std::vector<std::pair<int, int>> argumentStringOffsets) const
        {
            if (expectedArgTypes.size() == 0) // No type checking
                return func(args);

            if (expectedArgTypes.size() != args.size()) {
                // WARNING_MESSAGE("Function " + name + " called with incorrect number of arguments. Expected " +
                //                 std::to_string(expectedArgTypes.size()) + ", got " + std::to_string(args.size()) + ".");
                error.success = false;
                error.errorType = ErrorInfos::ErrorType::FUNCTION_CALL_ERROR_COUNT;
                static std::string errorMsg = "In function " + name + ", Expected " +
                    std::to_string(expectedArgTypes.size()) + ", got " + std::to_string(args.size()) + ".";
                error.messageExtra = &errorMsg;
                error.printError();
                return std::nullopt;
            }

            for (size_t i = 0; i < expectedArgTypes.size(); i++) {
                if (expectedArgTypes[i] != args[i]->type) {
                    // check if cast is possible
                    if ((expectedArgTypes[i] == FlagData::INT && args[i]->type == FlagData::FLOAT) ||
                        (expectedArgTypes[i] == FlagData::FLOAT && args[i]->type == FlagData::INT)) {
                        continue; // allow int<->float casting
                    }

                    // WARNING_MESSAGE("Function " + name + " called with incorrect argument type for argument " +
                    //                 std::to_string(i) + ". Expected " +
                    //                 FlagData::TypeReverseMap[expectedArgTypes[i]] + ", got " +
                    //                 FlagData::TypeReverseMap[args[i]->type] + ".");
                    error.success = false;
                    error.errorType = ErrorInfos::ErrorType::FUNCTION_CALL_ERROR_TYPE;
                    static std::string errorMsg = "In function " + name + ", argument " +
                        std::to_string(i) + ": Expected " +
                        FlagData::TypeReverseMap[expectedArgTypes[i]] + ", got " +
                        FlagData::TypeReverseMap[args[i]->type] + ".";
                    error.messageExtra = &errorMsg;
                    std::cout << "Offsets: " << argumentStringOffsets[i].first << ", " << argumentStringOffsets[i].second << std::endl;
                    error.column_end = error.column_start + argumentStringOffsets[i].second;
                    error.column_start += argumentStringOffsets[i].first;
                    error.printError();
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
        virtual FlagDataPtr evaluate() = 0;

        void addChild(std::shared_ptr<OperationNode> child)
        {
            if (children.size() + 1 > expectedChildren) {
                // WARNING_MESSAGE("Trying to add too many children to OperationNode");
                error.success = false;
                error.errorType = ErrorInfos::ErrorType::SYNTAX_ERROR;
                error.printError();
                return;
            }
            child->parent = shared_from_this();
            children.push_back(child);
        }
    };

    typedef std::shared_ptr<OperationNode> OperationNodePtr;

    struct FlagValueNode : OperationNode {
        FlagDataPtr value;

        FlagValueNode(FlagDataPtr v)
            : value(v)
        {
            nodeType = VALUE;
            expectedChildren = 0;
        }

        FlagDataPtr evaluate() override
        {
            return value;
        }
    };

    struct FunctionNode : OperationNode {
        std::string functionName;

        std::vector<std::string> argumentStrings;
        std::vector<std::pair<int, int>> argumentStringOffsets;

        FunctionNode(const std::string& fname, const std::vector<std::string>& argStrs, const std::vector<std::pair<int, int>>& argOffsets)
            : functionName(fname)
            , argumentStrings(argStrs)
            , argumentStringOffsets(argOffsets)
        {
            nodeType = VALUE;
            expectedChildren = 0;
        }

        FlagDataPtr evaluate() override;
    };

    struct NotOperatorNode : OperationNode {
        NotOperatorNode()
        {
            nodeType = LOGICAL_NOT;
            expectedChildren = 1;
        }

        FlagDataPtr evaluate() override;
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

        FlagDataPtr evaluate() override;
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

        FlagDataPtr evaluate() override;
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

        FlagDataPtr evaluate() override;
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

        FlagDataPtr evaluate() override;
    };

    struct LogicalAndOperatorNode : OperationNode {
        LogicalAndOperatorNode()
        {
            nodeType = LOGICAL_AND;
            expectedChildren = 2;
        }

        FlagDataPtr evaluate() override;
    };

    struct LogicalOrOperatorNode : OperationNode {
        LogicalOrOperatorNode()
        {
            nodeType = LOGICAL_OR;
            expectedChildren = 2;
        }

        FlagDataPtr evaluate() override;
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
        enum class ErrorType {
            NONE,
            UNKNOWN_FLAG,
            UNSUPPORTED_OPERATION,
            SYNTAX_ERROR,
            UNMATCHED_PARENS,
            UNKNOWN_FUNCTION,
            FUNCTION_CALL_FAILED,
            INVALID_ARGUMENTS,
            MALFORMED_IF_STATEMENT,
            FUNCTION_CALL_ERROR_COUNT,
            FUNCTION_CALL_ERROR_TYPE
        } errorType;

        std::string* fileName;
        const char* LineContent;
        int lineNumber;
        size_t column_start;
        size_t column_end;
        std::string* messageExtra;

        ErrorInfos() : success(true), errorType(ErrorType::NONE), lineNumber(-1), column_start(0), column_end(0) {clear();}

        void clear()
        {
            success = true;
            errorType = ErrorType::NONE;
            fileName = nullptr;
            LineContent = nullptr;
            messageExtra = nullptr;
            lineNumber = -1;
            column_start = 0;
            column_end = 0;
        }
        void printError() const 
        {
            if (success)
                return;

            std::string errorMsg = "Error: ";
            switch (errorType) {
                case ErrorType::UNKNOWN_FLAG:
                    errorMsg += "Unknown flag: " + *messageExtra;
                    break;
                case ErrorType::UNSUPPORTED_OPERATION:
                    errorMsg += "Unsupported operation";
                    break;
                case ErrorType::SYNTAX_ERROR:
                    errorMsg += "Syntax error";
                    break;
                case ErrorType::UNMATCHED_PARENS:
                    errorMsg += "Unmatched parentheses";
                    if (messageExtra && !(*messageExtra).empty()) {
                        errorMsg += ": " + *messageExtra;
                    }
                    break;
                case ErrorType::UNKNOWN_FUNCTION:
                    errorMsg += "Unknown function: " + *messageExtra;
                    break;
                case ErrorType::FUNCTION_CALL_FAILED:
                    errorMsg += "Function call failed";
                    break;
                case ErrorType::INVALID_ARGUMENTS:
                    errorMsg += "Invalid argument: " + *messageExtra;
                    break;
                case ErrorType::MALFORMED_IF_STATEMENT:
                    errorMsg += "Malformed if statement:" + *messageExtra;
                    break;
                case ErrorType::FUNCTION_CALL_ERROR_COUNT:
                    errorMsg += "Wrong number of arguments in function call. ";
                    errorMsg += *messageExtra;
                    break;
                case ErrorType::FUNCTION_CALL_ERROR_TYPE:
                    errorMsg += "Wrong argument type in function call. ";
                    errorMsg += *messageExtra;
                    break;
                default:
                    errorMsg += "Unknown error";
                    break;
            }

            if (error.column_start > 0 && LineContent != nullptr)
            {   
                std::string line = getLineFromString(LineContent, lineNumber);
                // count the number of \t before column_start to adjust the column position
                size_t tabCount = 0;
                for (size_t i = 0; i < error.column_start - 1 && i < line.length(); i++) {
                    if (line[i] == '\t') {
                        tabCount++;
                    }
                }
                // replace tabs with 4 spaces for error display
                std::string adjustedLine;
                for (char c : line) {
                    if (c == '\t') {
                        adjustedLine += "    ";
                    } else {
                        adjustedLine += c;
                    }
                }
                line = adjustedLine;
                size_t adjustedColumnStart = error.column_start + tabCount * 3; // each tab is replaced by 4 spaces, so we add 3 extra spaces
                error.column_start = adjustedColumnStart;
                size_t adjustedColumnEnd = error.column_end + tabCount * 3;
                error.column_end = adjustedColumnEnd;

                std::cerr << TERMINAL_ERROR << "ERROR   |" << TERMINAL_RESET << " While Parsing Logic Block: " << TERMINAL_INFO << TERMINAL_BOLD << "\"" << line << "\"";
                if (fileName != nullptr) {
                    std::cerr << TERMINAL_RESET << " in file " << TERMINAL_FILENAME << *fileName;
                    if (lineNumber > 0) {
                        std::cerr << ":" << lineNumber;
                    }
                }
                std::cerr << "\n";
                std::cerr << TERMINAL_ERROR << "        |> " << errorMsg << "\n";
                std::cerr << "           \"" << line << "\"\n            ";
                for (size_t i = 0; i < error.column_start - 1; i++) {
                    std::cerr << " ";
                }
                std::cerr << "^";

                if (error.column_end > error.column_start)
                {
                    for (size_t i = error.column_start + 1; i < error.column_end; i++) {
                        std::cerr << "~";
                    }
                }
                std::cerr << "\n\n";

                std::cerr << TERMINAL_RESET;
            }
        }
    } error;

    static OperationNodePtr buildOperationTree(std::vector<Token> tokens, const std::vector<std::pair<int, int>> &tokenPositions);
    static bool tokenize(const std::string& str, std::vector<Token> &tokens, std::vector<std::pair<int, int>> &tokenPositions); 

public:
    static void registerFunction(const Function& function)
    {
        functions[function.getName()] = function;
    }

    static void registerAllFunctions();

    
    static void parse_string(std::string& str, std::string* filename = nullptr);
    static FlagDataPtr parse_substring(const std::string& str, size_t idx_start, size_t idx_end);
    
    static void parse_string_cstr(char ** input, size_t &len, size_t Allocated, std::string* filename = nullptr);
    static FlagDataPtr parse_substring_cstr(const char* input, const size_t len, const size_t idx_start, const size_t idx_end);
};

struct LogicFlag : FlagData {
    std::string logicBlock;
    LogicFlag(Type t, const std::string& logic)
        : FlagData(t, false, true)
        , logicBlock(logic)
    {
    } 
};

struct IntLogicFlag : LogicFlag 
{
    IntLogicFlag(const std::string& logic)
        : LogicFlag(INT, logic)
    {
    }

    int as_int() override { 
        return LogicBlock::parse_substring(logicBlock, 0, logicBlock.size())->as_int();
    }
    float as_float() override { return static_cast<float>(as_int()); }
    std::string as_string() override { return std::to_string(as_int()); }
    bool as_bool() override { return as_int() != 0; }
};

struct FloatLogicFlag : LogicFlag 
{
    FloatLogicFlag(const std::string& logic)
        : LogicFlag(FLOAT, logic)
    {
    }

    float as_float() override { 
        return LogicBlock::parse_substring(logicBlock, 0, logicBlock.size())->as_float();
    }
    int as_int() override { return static_cast<int>(as_float()); }
    std::string as_string() override { return std::to_string(as_float()); }
    bool as_bool() override { return as_float() != 0.0f; }
};

struct StrLogicFlag : LogicFlag 
{
    StrLogicFlag(const std::string& logic)
        : LogicFlag(STRING, logic)
    {
    }

    std::string as_string() override {
        // std::cout << "Parsing: " << logicBlock << std::endl;
        return LogicBlock::parse_substring(logicBlock, 0, logicBlock.size())->as_string();
    }
    int as_int() override { 
        try {
            return std::stoi(as_string());
        } catch (...) {
            WARNING_MESSAGE("Failed to convert string to int: " + as_string());
            return 0;
        }
    }
    float as_float() override { 
        try {
            return std::stof(as_string());
        } catch (...) {
            WARNING_MESSAGE("Failed to convert string to float: " + as_string());
            return 0.0f;
        }
    }
    bool as_bool() override { return !as_string().empty(); }
};

struct BoolLogicFlag : LogicFlag 
{
    BoolLogicFlag(const std::string& logic)
        : LogicFlag(BOOL, logic)
    {
    }

    bool as_bool() override { 
        return LogicBlock::parse_substring(logicBlock, 0, logicBlock.size())->as_bool();
    }
    int as_int() override { return as_bool() ? 1 : 0; }
    float as_float() override { return as_bool() ? 1.0f : 0.0f; }
    std::string as_string() override { return as_bool() ? "true" : "false"; }
};