#include "Flags.hpp"
#include "AssetManager.hpp"
#include "AssetManagerUtils.hpp"
#include "Utils.hpp"

template <>
std::shared_ptr<FlagData> FlagData::MakeFlagFromScript<int>(const std::string& scriptName);
template <>
std::shared_ptr<FlagData> FlagData::MakeFlagFromScript<float>(const std::string& scriptName);
template <>
std::shared_ptr<FlagData> FlagData::MakeFlagFromScript<std::string>(const std::string& scriptName);
template <>
std::shared_ptr<FlagData> FlagData::MakeFlagFromScript<bool>(const std::string& scriptName);

template <>
std::shared_ptr<FlagData> FlagData::MakeFlagFromLogicBlock<int>(const std::string& logicBlock);
template <>
std::shared_ptr<FlagData> FlagData::MakeFlagFromLogicBlock<float>(const std::string& logicBlock);
template <>
std::shared_ptr<FlagData> FlagData::MakeFlagFromLogicBlock<std::string>(const std::string& logicBlock);
template <>
std::shared_ptr<FlagData> FlagData::MakeFlagFromLogicBlock<bool>(const std::string& logicBlock);

template <>
FlagData& FlagData::operator= <int>(const ScriptNameWrapper& v);
template <>
FlagData& FlagData::operator= <float>(const ScriptNameWrapper& v);
template <>
FlagData& FlagData::operator= <std::string>(const ScriptNameWrapper& v);
template <>
FlagData& FlagData::operator= <bool>(const ScriptNameWrapper& v);

template <>
Flag& Flag::operator= <int>(const ScriptNameWrapper& v)
{
    flag = FlagData::MakeFlagFromScript<int>(v.scriptName);

    return *this;
}

template <>
Flag& Flag::operator= <float>(const ScriptNameWrapper& v)
{
    flag = FlagData::MakeFlagFromScript<float>(v.scriptName);
    return *this;
}

template <>
Flag& Flag::operator= <std::string>(const ScriptNameWrapper& v)
{
    flag = FlagData::MakeFlagFromScript<std::string>(v.scriptName);
    return *this;
}

template <>
Flag& Flag::operator= <bool>(const ScriptNameWrapper& v)
{
    flag = FlagData::MakeFlagFromScript<bool>(v.scriptName);
    return *this;
}

FlagDataPtr FlagData::MakeFlag(int value) {
    return std::make_shared<IntFlag>(value);
}

FlagDataPtr FlagData::MakeFlag(float value) {
    return std::make_shared<FloatFlag>(value);
}

FlagDataPtr FlagData::MakeFlag(const std::string& value) {
    return std::make_shared<StrFlag>(value);
}

FlagDataPtr FlagData::MakeFlag(const char* value) {
    return std::make_shared<StrFlag>(std::string(value));
}

FlagDataPtr FlagData::MakeFlag(bool value) {
    return std::make_shared<BoolFlag>(value);
}

template <>
FlagDataPtr FlagData::MakeFlagFromScript<int>(const std::string& scriptName) {
    return std::make_shared<IntScriptFlag>(scriptName);
}

template <>
FlagDataPtr FlagData::MakeFlagFromScript<float>(const std::string& scriptName) {
    return std::make_shared<FloatScriptFlag>(scriptName);
}

template <>
FlagDataPtr FlagData::MakeFlagFromScript<std::string>(const std::string& scriptName) {
    return std::make_shared<StrScriptFlag>(scriptName);
}

template <>
FlagDataPtr FlagData::MakeFlagFromScript<bool>(const std::string& scriptName) {
    return std::make_shared<BoolScriptFlag>(scriptName);
}

template <>
FlagDataPtr FlagData::MakeFlagFromLogicBlock<int>(const std::string& logicBlock) {
    return std::make_shared<IntLogicFlag>(logicBlock);
}

template <>
FlagDataPtr FlagData::MakeFlagFromLogicBlock<float>(const std::string& logicBlock) {
    return std::make_shared<FloatLogicFlag>(logicBlock);
}

template <>
FlagDataPtr FlagData::MakeFlagFromLogicBlock<std::string>(const std::string& logicBlock) {
    return std::make_shared<StrLogicFlag>(logicBlock);
}

template <>
FlagDataPtr FlagData::MakeFlagFromLogicBlock<bool>(const std::string& logicBlock) {
    return std::make_shared<BoolLogicFlag>(logicBlock);
}

Flag& Flags::getFlag(const std::string& name) {
    // auto it = .find(name);
    // if (it != .end()) {
    //     return it->second;
    // }

    // [name] = Flag();
    // return [name];

    return Loader<Flag>::get(name, true);
}

FlagData& FlagData::operator=(int v)
{
    if(type != INT) {
        WARNING_MESSAGE("Trying to assign int to non-int Flag");
        return *this;
    }

    if(isScripted) {
        WARNING_MESSAGE("Trying to assign int to scripted Flag");
        return *this;
    }

    if(isLogicBlock) {
        WARNING_MESSAGE("Trying to assign int to logic block Flag");
        return *this;
    }

    static_cast<IntFlag*>(this)->value = v;
    return *this;
}

FlagData& FlagData::operator=(float v)
{
    if(type != FLOAT) {
        WARNING_MESSAGE("Trying to assign float to non-float Flag");
        return *this;
    }

    if(isScripted) {
        WARNING_MESSAGE("Trying to assign float to scripted Flag");
        return *this;
    }

    if(isLogicBlock) {
        WARNING_MESSAGE("Trying to assign float to logic block Flag");
        return *this;
    }

    static_cast<FloatFlag*>(this)->value = v;
    return *this;
}

FlagData& FlagData::operator=(const std::string& v)
{
    if(type != STRING) {
        WARNING_MESSAGE("Trying to assign string to non-string Flag");
        return *this;
    }

    if(isScripted) {
        WARNING_MESSAGE("Trying to assign string to scripted Flag");
        return *this;
    }

    if(isLogicBlock) {
        WARNING_MESSAGE("Trying to assign string to logic block Flag");
        return *this;
    }

    static_cast<StrFlag*>(this)->value = v;
    return *this;
}

FlagData& FlagData::operator=(bool v)
{
    if(type != BOOL) {
        WARNING_MESSAGE("Trying to assign bool to non-bool Flag");
        return *this;
    }

    if(isScripted) {
        WARNING_MESSAGE("Trying to assign bool to scripted Flag");
        return *this;
    }

    if(isLogicBlock) {
        WARNING_MESSAGE("Trying to assign bool to logic block Flag");
        return *this;
    }

    static_cast<BoolFlag*>(this)->value = v;
    return *this;
}

template <>
FlagData& FlagData::operator=<int>(const ScriptNameWrapper& v)
{
    if(type != INT) {
        WARNING_MESSAGE("Trying to assign scripted int to non-int Flag");
        return *this;
    }

    if(!isScripted) {
        WARNING_MESSAGE("Trying to assign scripted int to non-scripted Flag");
        return *this;
    }

    static_cast<IntScriptFlag*>(this)->luaScriptName = v.scriptName;
    return *this;
}

template <>
FlagData& FlagData::operator=<float>(const ScriptNameWrapper& v)
{
    if(type != FLOAT) {
        WARNING_MESSAGE("Trying to assign scripted float to non-float Flag");
        return *this;
    }

    if(!isScripted) {
        WARNING_MESSAGE("Trying to assign scripted float to non-scripted Flag");
        return *this;
    }

    static_cast<FloatScriptFlag*>(this)->luaScriptName = v.scriptName;
    return *this;
}

template <>
FlagData& FlagData::operator=<std::string>(const ScriptNameWrapper& v)
{
    if(type != STRING) {
        WARNING_MESSAGE("Trying to assign scripted string to non-string Flag");
        return *this;
    }

    if(!isScripted) {
        WARNING_MESSAGE("Trying to assign scripted string to non-scripted Flag");
        return *this;
    }

    static_cast<StrScriptFlag*>(this)->luaScriptName = v.scriptName;
    return *this;
}

template <>
FlagData& FlagData::operator=<bool>(const ScriptNameWrapper& v)
{
    if(type != BOOL) {
        WARNING_MESSAGE("Trying to assign scripted bool to non-bool Flag");
        return *this;
    }

    if(!isScripted) {
        WARNING_MESSAGE("Trying to assign scripted bool to non-scripted Flag");
        return *this;
    }

    static_cast<BoolScriptFlag*>(this)->luaScriptName = v.scriptName;
    return *this;
}

std::string FlagData::typeToString() {
    switch (type) {
        case INT: return "int";
        case FLOAT: return "float";
        case STRING: return "string";
        case BOOL: return "bool";
        case NONE: return "none";
        default: return "unknown";
    }
}

// DATA_WRITE_FUNC_INIT(Flags)
template <>
VulpineTextOutputRef DataLoader<Flags>::write(const Flags &data, VulpineTextOutputRef out) 
{
    out->write("~ ", 2);
    out->Entry();
    out->write("Flags",sizeof("Flags") -1);
    out->Tabulate();

        std::vector<std::pair<std::string, Flag>> valueFlags = data.getAllByValueFlags();
        out->Entry();
        out->write("ValueFlags", sizeof("ValueFlags") - 1);
        out->Tabulate();
            for (const auto& pair : valueFlags) {
                out->Entry();
                std::string s;
                if (pair.second->isLogicBlock) 
                {
                    s = "\"" + pair.second->typeToString() + "\" \"" + pair.first + "\" \"$(" + ((LogicFlag*)(pair.second.flag.get()))->logicBlock + ")\"";
                }
                else {
                    s = "\"" + pair.second->typeToString() + "\" \"" + pair.first + "\" \"" + pair.second->as_string() + "\"";
                }
                out->write(CONST_STRING_SIZED(s));
            }    
        out->Break();
        
        if(!data.writeAsSaveFileMode)
        {
            std::vector<std::pair<std::string, Flag>> scriptFlags = data.getAllFlags();
            out->Entry();
            out->write("ScriptFlags", sizeof("ScriptFlags") - 1);
            out->Tabulate();
                for (const auto& pair : scriptFlags) {
                    if (pair.second->isScripted) {
                        out->Entry();
                        std::string s = "\"" + pair.second->typeToString() + "\" \"" + pair.first + "\" \"" + ((ScriptFlagBase*)(pair.second.flag.get()))->luaScriptName + "\"";
                        out->write(CONST_STRING_SIZED(s));
                    }
                }
            out->Break();
        }

    out->Break();
    return out;
}

template <> 
Flags DataLoader<Flags>::read(VulpineTextBuffRef buff) 
{
    Flags data;

    buff->read();

    const char* name = buff->read();
    if (strcmp(name, "Flags") != 0) {
        ERROR_MESSAGE("Expected to read 'Flags' but read '" + std::string(name) + "'");
        return data;
    }

    while (NEW_VALUE) 
    {                                                          
        const char* member = buff->read();
        // std::cout << "Reading Flags member: " << member << std::endl;
        if (strcmp(member, "ValueFlags") == 0) {
            while (NEW_VALUE) {
                const char* typeStr = buff->read();
                str2upper((char*)typeStr);
                FlagData::Type type = FlagData::NONE;
                MAP_SAFE_READ(FlagData::TypeMap, buff, type, typeStr);
                if (type == FlagData::NONE) {
                    WARNING_MESSAGE("Unknown flag type: " + std::string(typeStr));
                    continue;
                }
                // buff->read();
                const char* name = buff->read();
                // buff->read();
                const char* valueStr = buff->read();
                bool isLogicBlock = false;
                std::string logicBlockStr;
                if (valueStr[0] == '$' && valueStr[1] == '(')
                {
                    int paren_count = 1;
                    isLogicBlock = true;
                    logicBlockStr = std::string(valueStr);

                    // read until )
                    
                    // while (paren_count > 0)
                    // {
                    //     valueStr = buff->read();
                    //     for (int i = 0; i < strlen(valueStr); i++)
                    //     {
                    //         if (valueStr[i] == '(')
                    //             paren_count++;
                    //         else if (valueStr[i] == ')')
                    //             paren_count--;
                    //     }
                    //     logicBlockStr += " ";
                    //     logicBlockStr += valueStr;
                    // }

                    // remove $() from logicBlockStr
                    logicBlockStr = logicBlockStr.substr(2, logicBlockStr.length() - 3);

                    // std::cout << "Logic Block for flag " << name << ": " << logicBlockStr << std::endl;
                }
                

                // std::cout << "Flag: " << typeStr << " " << name << " = " << valueStr << std::endl;

                switch (type) {
                    case FlagData::INT:
                    {
                        if (isLogicBlock)
                        {
                            data.setFlagFromLogicBlock<int>(name, logicBlockStr);
                            break;
                        }

                        int intValue;
                        if (!isInteger(std::string(valueStr), intValue))
                        {
                            WARNING_MESSAGE("Invalid integer value for flag '" + std::string(name) + "': " + std::string(valueStr));
                            intValue = 0;
                        }
                        // std::cout << "Parsed int flag value: " << intValue << std::endl;
                        data.setFlag<int>(name, intValue);
                        break;
                    }
                    case FlagData::FLOAT:
                    {
                        if (isLogicBlock)
                        {
                            data.setFlagFromLogicBlock<float>(name, logicBlockStr);
                            break;
                        }

                        float floatValue;
                        if (!isFloat(std::string(valueStr), floatValue))
                        {
                            WARNING_MESSAGE("Invalid float value for flag '" + std::string(name) + "': " + std::string(valueStr));
                            floatValue = 0.0f;
                        }
                        // std::cout << "Parsed float flag value: " << floatValue << std::endl;
                        data.setFlag<float>(name, floatValue);
                        break;
                    }
                    case FlagData::STRING:
                        if (isLogicBlock)
                        {
                            data.setFlagFromLogicBlock<std::string>(name, logicBlockStr);
                            break;
                        }

                        data.setFlag<std::string>(name, std::string(valueStr));
                        break;
                    case FlagData::BOOL:
                        if (isLogicBlock)
                        {
                            data.setFlagFromLogicBlock<bool>(name, logicBlockStr);
                            break;
                        }
                        data.setFlag<bool>(name, FastTextParser::read<bool>(valueStr));
                        break;
                    default:
                        WARNING_MESSAGE("Unsupported flag type: " + std::string(typeStr));
                        break;
                }
            }
        } else if (strcmp(member, "ScriptFlags") == 0) {
            while (NEW_VALUE) {
                const char* typeStr = buff->read();
                str2upper((char*)typeStr);
                FlagData::Type type = FlagData::NONE;
                MAP_SAFE_READ(FlagData::TypeMap, buff, type, typeStr);
                if (type == FlagData::NONE) {
                    WARNING_MESSAGE("Unknown flag type: " + std::string(typeStr));
                    continue;
                }
                const char* name = buff->read();
                const char* scriptName = buff->read();

                switch (type) {
                    case FlagData::INT:
                        data.setFlagFromScript<int>(name, std::string(scriptName));
                        break;
                    case FlagData::FLOAT:
                        data.setFlagFromScript<float>(name, std::string(scriptName));
                        break;
                    case FlagData::STRING:
                        data.setFlagFromScript<std::string>(name, std::string(scriptName));
                        break;
                    case FlagData::BOOL:
                        data.setFlagFromScript<bool>(name, std::string(scriptName));
                        break;
                    default:
                        WARNING_MESSAGE("Unsupported flag type: " + std::string(typeStr));
                        break;
                }
            }
        }
    }
    
    return data;
}

bool FlagData::equals(const FlagDataPtr a, const FlagDataPtr b) {
    switch(a->type) {
        case FlagData::INT:
            return a->as_int() == b->as_int();
        case FlagData::FLOAT:
            return a->as_float() == b->as_float();
        case FlagData::STRING:
            return a->as_string() == b->as_string();
        case FlagData::BOOL:
            return a->as_bool() == b->as_bool();
        default:
            WARNING_MESSAGE("Unsupported flag type for equality operation");
            return false;
    }
}

bool FlagData::notEquals(const FlagDataPtr a, const FlagDataPtr b) {
    return !equals(a, b);
}

bool FlagData::lessThan(const FlagDataPtr a, const FlagDataPtr b) {
    switch(a->type) {
        case FlagData::INT:
            return a->as_int() < b->as_int();
        case FlagData::FLOAT:
            return a->as_float() < b->as_float();
        default:
            WARNING_MESSAGE("Unsupported flag type for less than operation");
            return false;
    }
}

bool FlagData::greaterThan(const FlagDataPtr a, const FlagDataPtr b) {
    switch(a->type) {
        case FlagData::INT:
            return a->as_int() > b->as_int();
        case FlagData::FLOAT:
            return a->as_float() > b->as_float();
        default:
            WARNING_MESSAGE("Unsupported flag type for greater than operation");
            return false;
    }
}

bool FlagData::lessThanOrEqual(const FlagDataPtr a, const FlagDataPtr b) {
    return !greaterThan(a, b);
}

bool FlagData::greaterThanOrEqual(const FlagDataPtr a, const FlagDataPtr b) {
    return !lessThan(a, b);
}

bool FlagData::logicalAnd(const FlagDataPtr a, const FlagDataPtr b) {
    return a->as_bool() && b->as_bool();
}

bool FlagData::logicalOr(const FlagDataPtr a, const FlagDataPtr b) {
    return a->as_bool() || b->as_bool();
}

bool FlagData::logicalNot(const FlagDataPtr a) {
    return !a->as_bool();
}

FlagDataPtr FlagData::add(const FlagDataPtr a, const FlagDataPtr b)
{
    if (b->type == STRING)
    {
        return FlagData::MakeFlag(a->as_string() + b->as_string());
    }

    switch(a->type) {
        case FlagData::INT:
            return FlagData::MakeFlag(a->as_int() + b->as_int());
        case FlagData::FLOAT:
            return FlagData::MakeFlag(a->as_float() + b->as_float());
        case FlagData::STRING:
            return FlagData::MakeFlag(a->as_string() + b->as_string());
        default:
            WARNING_MESSAGE("Unsupported flag type for addition operation, returning 0");
            return FlagData::MakeFlag(0);
    }
}

FlagDataPtr FlagData::subtract(const FlagDataPtr a, const FlagDataPtr b)
{
    switch(a->type) {
        case FlagData::INT:
            return FlagData::MakeFlag(a->as_int() - b->as_int());
        case FlagData::FLOAT:
            return FlagData::MakeFlag(a->as_float() - b->as_float());
        default:
            WARNING_MESSAGE("Unsupported flag type for subtraction operation, returning 0");
            return FlagData::MakeFlag(0);
    }
}

FlagDataPtr FlagData::multiply(const FlagDataPtr a, const FlagDataPtr b)
{
    switch(a->type) {
        case FlagData::INT:
            return FlagData::MakeFlag(a->as_int() * b->as_int());
        case FlagData::FLOAT:
            return FlagData::MakeFlag(a->as_float() * b->as_float());
        default:
            WARNING_MESSAGE("Unsupported flag type for multiplication operation, returning 0");
            return FlagData::MakeFlag(0);
    }
}

FlagDataPtr FlagData::divide(const FlagDataPtr a, const FlagDataPtr b)
{
    switch(a->type) {
        case FlagData::INT:
            if (b->as_int() == 0) {
                WARNING_MESSAGE("Division by zero in int division, returning 0");
                return FlagData::MakeFlag(0);
            }
            return FlagData::MakeFlag(a->as_int() / b->as_int());
        case FlagData::FLOAT:
            if (b->as_float() == 0.0f) {
                WARNING_MESSAGE("Division by zero in float division, returning 0.0f");
                return FlagData::MakeFlag(0.0f);
            }
            return FlagData::MakeFlag(a->as_float() / b->as_float());
        default:
            WARNING_MESSAGE("Unsupported flag type for division operation, returning 0");
            return FlagData::MakeFlag(0);
    }
}

FlagDataPtr FlagData::clone()
{
    if (isScripted)
    {
        switch (type) {
            case INT:
                return FlagData::MakeFlagFromScript<int>(((ScriptFlagBase*)this)->luaScriptName);
            case FLOAT:
                return FlagData::MakeFlagFromScript<float>(((ScriptFlagBase*)this)->luaScriptName);
            case STRING:
                return FlagData::MakeFlagFromScript<std::string>(((ScriptFlagBase*)this)->luaScriptName);
            case BOOL:
                return FlagData::MakeFlagFromScript<bool>(((ScriptFlagBase*)this)->luaScriptName);
            default:
                WARNING_MESSAGE("Unsupported flag type for cloning");
                return nullptr;
        }
    }
    else if (isLogicBlock)
    {
        switch (type) {
            case INT:
                return FlagData::MakeFlagFromLogicBlock<int>(((LogicFlag*)this)->logicBlock);
            case FLOAT:
                return FlagData::MakeFlagFromLogicBlock<float>(((LogicFlag*)this)->logicBlock);
            case STRING:
                return FlagData::MakeFlagFromLogicBlock<std::string>(((LogicFlag*)this)->logicBlock);
            case BOOL:
                return FlagData::MakeFlagFromLogicBlock<bool>(((LogicFlag*)this)->logicBlock);
            default:
                WARNING_MESSAGE("Unsupported flag type for cloning");
                return nullptr;
        }
    }
    else {
        switch (type) {
            case INT:
                return FlagData::MakeFlag(as_int());
            case FLOAT:
                return FlagData::MakeFlag(as_float());
            case STRING:
                return FlagData::MakeFlag(as_string());
            case BOOL:
                return FlagData::MakeFlag(as_bool());
            default:
                WARNING_MESSAGE("Unsupported flag type for cloning");
                return nullptr;
        }
    }
}

FlagData::operator Flag()
{
    return Flag(shared_from_this());
}

FlagDataPtr LogicBlock::FunctionNode::evaluate() 
{
    static std::vector<FlagDataPtr> argValues(16);
    argValues.resize(0);
    
    Function func = functions[functionName];

    if (func.getName() == "") {
        // WARNING_MESSAGE("Function '" + functionName + "' not found");
        error.success = false;
        error.errorType = ErrorInfos::ErrorType::UNKNOWN_FUNCTION;
        error.messageExtra = &functionName;
        error.printError();
        return FlagData::MakeFlag(0);
    }

    int errorIndexStart = error.column_start;
    for (int i = 0; i < argumentStrings.size(); i++) {
        std::string argString = argumentStrings[i];
        FlagDataPtr argValue = parse_substring(argString, 0, argString.length());
        if (!argValue) {
            // WARNING_MESSAGE("Failed to evaluate argument: " + argString);
            error.success = false;
            error.errorType = ErrorInfos::ErrorType::INVALID_ARGUMENTS;
            error.messageExtra = &argString;
            error.column_start = errorIndexStart + argumentStringOffsets[i].first;
            error.column_end = errorIndexStart + argumentStringOffsets[i].second;
            error.printError();
            switch (func.getArgType(i))
            {
                case FlagData::INT:
                    argValue = FlagData::MakeFlag(0);
                    break;
                case FlagData::FLOAT:
                    argValue = FlagData::MakeFlag(0.0f);
                    break;
                case FlagData::STRING:
                    argValue = FlagData::MakeFlag("");
                    break;
                case FlagData::BOOL:
                    argValue = FlagData::MakeFlag(false);
                    break;
                default:
                    WARNING_MESSAGE("Unsupported argument type for function: " + func.getName());
                    argValue = FlagData::MakeFlag(0);
                    break;
            }
        }
        argValues.push_back(argValue);
    }

    std::optional<FlagDataPtr> result = func.call(argValues, argumentStringOffsets);
    if (error.success == false) 
    {
        return FlagData::MakeFlag(0);
    }

    if (!result.has_value()) { // shouldn't be reached if error.success is properly set in func.call
        // WARNING_MESSAGE("Function '" + functionName + "' call failed");
        error.success = false;
        error.errorType = ErrorInfos::ErrorType::FUNCTION_CALL_FAILED;
        error.printError();
        return FlagData::MakeFlag(0);
    }
    
    return result.value();
}

FlagDataPtr LogicBlock::NotOperatorNode::evaluate() 
{
    if (children.size() != 1) {
        // WARNING_MESSAGE("NotOperatorNode must have exactly one child");
        error.success = false;
        error.errorType = ErrorInfos::ErrorType::SYNTAX_ERROR;
        error.printError();
        return FlagData::MakeFlag(false);
    }

    FlagDataPtr operandValue = children[0]->evaluate();
    bool result = FlagData::logicalNot(operandValue);
    return FlagData::MakeFlag(result);
}

FlagDataPtr LogicBlock::AdditionOperatorNode::evaluate() 
{
    if (children.size() != 2) {
        // WARNING_MESSAGE("AdditionOperatorNode must have exactly two children");
        error.success = false;
        error.errorType = ErrorInfos::ErrorType::SYNTAX_ERROR;
        error.printError();
        return FlagData::MakeFlag(0);
    }

    FlagDataPtr leftValue = children[0]->evaluate();
    FlagDataPtr rightValue = children[1]->evaluate();

    switch (additionType) {
        case ADD:
            return FlagData::add(leftValue, rightValue);
        case SUBTRACT:
            return FlagData::subtract(leftValue, rightValue);
        default:
            WARNING_MESSAGE("Unsupported addition operator type");
            return FlagData::MakeFlag(0);
    }
}

FlagDataPtr LogicBlock::MultiplicationOperatorNode::evaluate() 
{
    if (children.size() != 2) {
        // WARNING_MESSAGE("MultiplicationOperatorNode must have exactly two children");
        error.success = false;
        error.errorType = ErrorInfos::ErrorType::SYNTAX_ERROR;
        error.printError();
        return FlagData::MakeFlag(0);
    }

    FlagDataPtr leftValue = children[0]->evaluate();
    FlagDataPtr rightValue = children[1]->evaluate();

    switch (multiplicationType) {
        case MULTIPLY:
            return FlagData::multiply(leftValue, rightValue);
        case DIVIDE:
            return FlagData::divide(leftValue, rightValue);
        default:
            WARNING_MESSAGE("Unsupported multiplication operator type");
            return FlagData::MakeFlag(0);
    }
}


FlagDataPtr LogicBlock::ComparisonOperatorNode::evaluate() 
{
    if (children.size() != 2) {
        // WARNING_MESSAGE("ComparisonOperatorNode must have exactly two children");
        error.success = false;
        error.errorType = ErrorInfos::ErrorType::SYNTAX_ERROR;
        error.printError();
        return FlagData::MakeFlag(false);
    }

    FlagDataPtr leftValue = children[0]->evaluate();
    FlagDataPtr rightValue = children[1]->evaluate();

    bool result = false;
    switch (comparisonType) {
        case ComparisonType::LESS_THAN:
            result = FlagData::lessThan(leftValue, rightValue);
            break;
        case ComparisonType::GREATER_THAN:
            // std::cout << "Comparing " << leftValue->as_string() << " > " << rightValue->as_string() << std::endl;
            result = FlagData::greaterThan(leftValue, rightValue);
            break;
        case ComparisonType::LESS_THAN_OR_EQUAL:
            result = FlagData::lessThanOrEqual(leftValue, rightValue);
            break;
        case ComparisonType::GREATER_THAN_OR_EQUAL:
            result = FlagData::greaterThanOrEqual(leftValue, rightValue);
            break;
        default:
            WARNING_MESSAGE("Unsupported comparison operator type");
            break;
    }
    return FlagData::MakeFlag(result);
}

FlagDataPtr LogicBlock::EqualityOperatorNode::evaluate() 
{
    if (children.size() != 2) {
        // WARNING_MESSAGE("EqualityOperatorNode must have exactly two children");
        error.success = false;
        error.errorType = ErrorInfos::ErrorType::SYNTAX_ERROR;
        error.printError();
        return FlagData::MakeFlag(false);
    }

    FlagDataPtr leftValue = children[0]->evaluate();
    FlagDataPtr rightValue = children[1]->evaluate();

    bool result = false;
    switch (equalityType) {
        case EqualityType::EQUALS:
            result = FlagData::equals(leftValue, rightValue);
            break;
        case EqualityType::NOT_EQUALS:
            result = FlagData::notEquals(leftValue, rightValue);
            break;
        default:
            WARNING_MESSAGE("Unsupported equality operator type");
            break;
    }
    return FlagData::MakeFlag(result);
}

FlagDataPtr LogicBlock::LogicalAndOperatorNode::evaluate() 
{
    if (children.size() != 2) {
        // WARNING_MESSAGE("LogicalAndOperatorNode must have exactly two children");
        error.success = false;
        error.errorType = ErrorInfos::ErrorType::SYNTAX_ERROR;
        error.printError();
        return FlagData::MakeFlag(false);
    }

    FlagDataPtr leftValue = children[0]->evaluate();
    FlagDataPtr rightValue = children[1]->evaluate();

    bool result = FlagData::logicalAnd(leftValue, rightValue);
    return FlagData::MakeFlag(result);
}

FlagDataPtr LogicBlock::LogicalOrOperatorNode::evaluate() 
{
    if (children.size() != 2) {
        // WARNING_MESSAGE("LogicalOrOperatorNode must have exactly two children");
        error.success = false;
        error.errorType = ErrorInfos::ErrorType::SYNTAX_ERROR;
        error.printError();
        return FlagData::MakeFlag(false);
    }

    FlagDataPtr leftValue = children[0]->evaluate();
    FlagDataPtr rightValue = children[1]->evaluate();

    bool result = FlagData::logicalOr(leftValue, rightValue);
    return FlagData::MakeFlag(result);
}

bool LogicBlock::tokenize(const std::string& str, std::vector<Token> &tokens, std::vector<std::pair<int, int>> &tokenPositions)
{
    size_t i = 0;
    while (i < str.length()) {
        if (isspace(str[i])) {
            i++;
            continue;
        }
        else if (str[i] == '(') {
            tokens.emplace_back(Token::TokenType::PARENS_OPEN, "(");
            tokenPositions.push_back(std::make_pair(i, i));
            i++;
            continue;
        }
        else if (str[i] == ')') {
            tokens.emplace_back(Token::TokenType::PARENS_CLOSE, ")");
            tokenPositions.push_back(std::make_pair(i, i));
            i++;
            continue;
        }
        else if (str[i] == '&' && str[i+1] == '&') {
            tokens.emplace_back(Token::TokenType::LOGICAL_AND, "&&");
            tokenPositions.push_back(std::make_pair(i, i+1));
            i += 2;
            continue;
        }
        else if (str[i] == '|' && str[i+1] == '|') {
            tokens.emplace_back(Token::TokenType::LOGICAL_OR, "||");
            tokenPositions.push_back(std::make_pair(i, i+1));
            i += 2;
            continue;
        }
        else if (str[i] == '!') {
            tokens.emplace_back(Token::TokenType::NOT, "!");
            tokenPositions.push_back(std::make_pair(i, i));
            i += 1;
            continue;
        }
        else if (str[i] == '=' && str[i+1] == '=') {
            tokens.emplace_back(Token::TokenType::EQUALS, "==");
            tokenPositions.push_back(std::make_pair(i, i+1));
            i += 2;
            continue;
        }
        else if (str[i] == '!' && str[i+1] == '=') {
            tokens.emplace_back(Token::TokenType::NOT_EQUALS, "!=");
            tokenPositions.push_back(std::make_pair(i, i+1));
            i += 2;
            continue;
        }
        else if (str[i] == '<') {
            if (i + 1 < str.length() && str[i + 1] == '=') {
                tokens.emplace_back(Token::TokenType::LESS_THAN_OR_EQUAL, "<=");
                tokenPositions.push_back(std::make_pair(i, i+1));
                i += 2;
            } else {
                tokens.emplace_back(Token::TokenType::LESS_THAN, "<");
                tokenPositions.push_back(std::make_pair(i, i));
                i += 1;
            }
            continue;
        }
        else if (str[i] == '>') {
            if (i + 1 < str.length() && str[i + 1] == '=') {
                tokens.emplace_back(Token::TokenType::GREATER_THAN_OR_EQUAL, ">=");
                tokenPositions.push_back(std::make_pair(i, i+1));
                i += 2;
            } else {
                tokens.emplace_back(Token::TokenType::GREATER_THAN, ">");
                tokenPositions.push_back(std::make_pair(i, i));
                i += 1;
            }
            continue;
        }
        else if (str[i] == '+') {
            tokens.emplace_back(Token::TokenType::ADD, "+");
            tokenPositions.push_back(std::make_pair(i, i));
            i += 1;
            continue;
        }
        else if (str[i] == '-') {
            tokens.emplace_back(Token::TokenType::SUBTRACT, "-");
            tokenPositions.push_back(std::make_pair(i, i));
            i += 1;
            continue;
        }
        else if (str[i] == '*') {
            tokens.emplace_back(Token::TokenType::MULTIPLY, "*");
            tokenPositions.push_back(std::make_pair(i, i));
            i += 1;
            continue;
        }
        else if (str[i] == '/') {
            tokens.emplace_back(Token::TokenType::DIVIDE, "/");
            tokenPositions.push_back(std::make_pair(i, i));
            i += 1;
            continue;
        }
        else {
            // it's either a flag, a function or an immediate value
            size_t start = i;
            // tokenPositions.push_back(i);
            if ((str[i] == '\"') || (str[i] == '\'')) {
                char quoteChar = str[i];
                i++;
                while (i < str.length() && str[i] != quoteChar) {
                    i++;
                }
                i++; // consume closing quote
                std::string tokenStr = str.substr(start, i - start);
                tokens.emplace_back(Token::TokenType::FLAG, tokenStr);
                tokenPositions.push_back(std::make_pair(start, i));
            } else {
                // check if it's a function call (basically any word followed by any whitespace and then an opening parenthesis)
                size_t funcCheckPos = i;
                while (
                        funcCheckPos < str.length() && 
                        !isspace(str[funcCheckPos]) && 
                        str[funcCheckPos] != '(' && 
                        str[funcCheckPos] != ')' && 
                        !(str[i] == '&' && str[i+1] == '&')&&
                        !(str[i] == '|' && str[i+1] == '|')&&
                        !(str[i] == '=' && str[i+1] == '=')&&
                        !(str[i] == '!' && str[i+1] == '=')&&
                        str[funcCheckPos] != '<' && 
                        str[funcCheckPos] != '>'
                    ) {
                    funcCheckPos++;
                }
                size_t tempPos = funcCheckPos;
                while (tempPos < str.length() && isspace(str[tempPos])) {
                    tempPos++;
                }
                if (tempPos < str.length() && str[tempPos] == '(') {
                    // it's a function call, go to matching closing parenthesis
                    i = tempPos + 1; // consume opening parenthesis
                    int parenCount = 1;
                    while (i < str.length() && parenCount > 0) {
                        if (str[i] == '(') {
                            parenCount++;
                        } else if (str[i] == ')') {
                            parenCount--; 
                        }
                        i++;
                    }

                    if (parenCount != 0) {
                        // WARNING_MESSAGE("Unmatched parentheses in function call: " + str.substr(start, i - start));
                        error.success = false;
                        error.errorType = ErrorInfos::ErrorType::UNMATCHED_PARENS;
                        error.column_start = tempPos;
                        error.column_end = tempPos;
                        error.printError();
                        return false;
                    }

                    std::string tokenStr = str.substr(start, i - start);
                    tokens.emplace_back(Token::TokenType::FUNCTION, tokenStr);
                    tokenPositions.push_back(std::make_pair(start, i));
                }
                else {                
                    // else just parse as a flag/immediate value
                    while (
                            i < str.length() && 
                            !isspace(str[i]) && 
                            str[i] != '(' && 
                            str[i] != ')' && 
                            !(str[i] == '&' && str[i+1] == '&')&&
                            !(str[i] == '|' && str[i+1] == '|')&&
                            !(str[i] == '=' && str[i+1] == '=')&&
                            !(str[i] == '!' && str[i+1] == '=')&&
                            str[i] != '<' && 
                            str[i] != '>'
                        ) {
                        i++;
                    }
                    std::string tokenStr = str.substr(start, i - start);
                    tokens.emplace_back(Token::TokenType::FLAG, tokenStr);
                    tokenPositions.push_back(std::make_pair(start, i));
                }
            }
        }
    }
    return true;
}

LogicBlock::OperationNodePtr LogicBlock::Token::toOperationNode() const
{
    switch (type) {
        case FLAG: {
            // Determine if it's a flag, an immediate value, or a function call
            // First check for immediate values
            if (value == "true" || value == "false") {
                return std::make_shared<FlagValueNode>(FlagData::MakeFlag(value == "true"));
            }
            int intValue;
            if (isInteger(value, intValue)) {
                return std::make_shared<FlagValueNode>(FlagData::MakeFlag(intValue));
            }
            float floatValue;
            if (isFloat(value, floatValue)) {
                return std::make_shared<FlagValueNode>(FlagData::MakeFlag(floatValue));
            }
            if (
                ((value.front() == '\"' && value.back() == '\"') || (value.front() == '\'' && value.back() == '\'')) && value.length() >= 2
            ) {
                std::string strValue = value.substr(1, value.length() - 2);
                return std::make_shared<FlagValueNode>(FlagData::MakeFlag(strValue));
            }
            // Next check for function calls (not implemented yet)
            // Finally, if it's formated like a flag `${flag_name}`, extract the flag name
            if (value.length() >= 4 && (value[0] == '$' && value[1] == '{') && value.back() == '}') {
                std::string flagName = value.substr(2, value.length() - 3);
                Flag flag = Flags::getFlag(flagName);
                Flag flagClone;
                if (flag.flag) 
                {
                    flagClone = flag->clone();
                }
                else 
                {
                    // WARNING_MESSAGE("Flag '" + flagName + "' not found, defaulting to false");
                    error.success = false;
                    error.errorType = ErrorInfos::ErrorType::UNKNOWN_FLAG;
                    error.messageExtra = &flagName;
                    error.printError();
                    flagClone = FlagData::MakeFlag(false);
                }
                
                return std::make_shared<FlagValueNode>(flagClone.flag);
            }
            // WARNING_MESSAGE("Unrecognized flag or value: " + value + ", defaulting to false");
            error.success = false;
            error.errorType = ErrorInfos::ErrorType::SYNTAX_ERROR;
            error.printError();
            return std::make_shared<FlagValueNode>(FlagData::MakeFlag(false));
        }
        case FUNCTION:
        {
            // Parse function name and arguments
            size_t parenOpenPos = value.find('(');
            size_t parenClosePos = value.rfind(')');
            if (parenOpenPos == std::string::npos || parenClosePos == std::string::npos || parenClosePos <= parenOpenPos) {
                // WARNING_MESSAGE("Invalid function token: " + value);
                error.success = false;
                error.errorType = ErrorInfos::ErrorType::SYNTAX_ERROR; // shouldn't be happening due to prior checks (hopefully)
                error.printError();
                return nullptr;
            }
            std::string functionName = value.substr(0, parenOpenPos);
            std::string argsStr = value.substr(parenOpenPos + 1, parenClosePos - parenOpenPos - 1);

            // Split arguments by commas, taking care of nested parentheses
            std::vector<std::string> argumentStrings;
            std::vector<std::pair<int, int>> argumentStringOffsets;
            argumentStrings.reserve(4);
            size_t lastPos = 0;
            int parenCount = 0;
            for (size_t i = 0; i < argsStr.length(); i++) {
                if (argsStr[i] == '(') {
                    parenCount++;
                } else if (argsStr[i] == ')') {
                    parenCount--;
                } else if (argsStr[i] == ',' && parenCount == 0) {
                    argumentStrings.push_back(argsStr.substr(lastPos, i - lastPos));
                    argumentStringOffsets.push_back(std::make_pair(parenOpenPos + lastPos, i - 1));
                    lastPos = i + 1;
                }
            }
            if (lastPos < argsStr.length()) {
                argumentStrings.push_back(argsStr.substr(lastPos));
                // + 1 cause there's probably a space after the comma but this is a bad solution
                argumentStringOffsets.push_back(std::make_pair(parenOpenPos + lastPos + 1, parenClosePos - 1));
            }

            return std::make_shared<FunctionNode>(functionName, argumentStrings, argumentStringOffsets);
        }
        case LOGICAL_AND: 
            return std::make_shared<LogicalAndOperatorNode>();
        case LOGICAL_OR: 
            return std::make_shared<LogicalOrOperatorNode>();
        case NOT: 
            return std::make_shared<NotOperatorNode>();
        case EQUALS: 
            return std::make_shared<EqualityOperatorNode>(EqualityOperatorNode::EqualityType::EQUALS);
        case NOT_EQUALS: 
            return std::make_shared<EqualityOperatorNode>(EqualityOperatorNode::EqualityType::NOT_EQUALS);
        case LESS_THAN:
            return std::make_shared<ComparisonOperatorNode>(ComparisonOperatorNode::ComparisonType::LESS_THAN);
        case GREATER_THAN:
            return std::make_shared<ComparisonOperatorNode>(ComparisonOperatorNode::ComparisonType::GREATER_THAN);
        case LESS_THAN_OR_EQUAL:
            return std::make_shared<ComparisonOperatorNode>(ComparisonOperatorNode::ComparisonType::LESS_THAN_OR_EQUAL);
        case GREATER_THAN_OR_EQUAL:
            return std::make_shared<ComparisonOperatorNode>(ComparisonOperatorNode::ComparisonType::GREATER_THAN_OR_EQUAL);
        case ADD:
            return std::make_shared<AdditionOperatorNode>(AdditionOperatorNode::AdditionType::ADD);
        case SUBTRACT:
            return std::make_shared<AdditionOperatorNode>(AdditionOperatorNode::AdditionType::SUBTRACT);
        case MULTIPLY:
            return std::make_shared<MultiplicationOperatorNode>(MultiplicationOperatorNode::MultiplicationType::MULTIPLY);
        case DIVIDE:
            return std::make_shared<MultiplicationOperatorNode>(MultiplicationOperatorNode::MultiplicationType::DIVIDE);
        default:
            WARNING_MESSAGE("Unsupported token type for operation node conversion");
            return nullptr;
    }
}

LogicBlock::OperationNodePtr LogicBlock::buildOperationTree(std::vector<Token> tokens, const std::vector<std::pair<int, int>> &tokenPositions)
{
    if (tokens.empty()) { // should never happen
        WARNING_MESSAGE("No tokens to build operation tree");
        return nullptr;
    }

    OperationNodePtr currentNode = nullptr;

    int errorColumnOffsetStart = error.column_start;
    for (int i = 0; i < tokens.size(); i++) {
        error.column_start = errorColumnOffsetStart + tokenPositions[i].first;
        error.column_end = errorColumnOffsetStart + tokenPositions[i].second;
        const Token& token = tokens[i];
        if (token.type == Token::TokenType::PARENS_OPEN)
        {
            // Find the matching closing parenthesis index
            int parenCount = 1;
            size_t j = i + 1;
            for (; j < tokens.size(); j++) {
                if (tokens[j].type == Token::TokenType::PARENS_OPEN) {
                    parenCount++;
                } else if (tokens[j].type == Token::TokenType::PARENS_CLOSE) {
                    parenCount--;
                    if (parenCount == 0) {
                        break;
                    }
                }
            }
            if (parenCount != 0) {
                // WARNING_MESSAGE("Mismatched parentheses in logic block");
                error.success = false;
                error.errorType = ErrorInfos::ErrorType::UNMATCHED_PARENS;
                error.printError();
                return nullptr;
            }
            std::vector<Token> paren_tokens = std::vector<Token>(tokens.begin() + i + 1, tokens.begin() + j);
            std::vector<std::pair<int, int>> paren_token_positions = std::vector<std::pair<int, int>>(tokenPositions.begin() + i + 1, tokenPositions.begin() + j);
            if (!paren_tokens.empty()) 
            {

                OperationNodePtr subTree = buildOperationTree(
                    paren_tokens,
                    paren_token_positions
                );

                if (!subTree) {
                    // WARNING_MESSAGE("Failed to build subtree for parentheses");
                    return nullptr;
                }
                if (currentNode) {
                    currentNode->addChild(subTree);
                } else {
                    currentNode = subTree;
                }
            }
            i = j; // Move index to closing parenthesis
            continue;
        }
        else if (token.type == Token::TokenType::PARENS_CLOSE)
        {
            // WARNING_MESSAGE("Mismatched parentheses in logic block");
            error.success = false;
            error.errorType = ErrorInfos::ErrorType::UNMATCHED_PARENS;
            error.column_start = i;
            error.column_end = i;
            error.printError();
            return nullptr;
        }
        OperationNodePtr newNode = token.toOperationNode();
        if (!newNode) {
            // WARNING_MESSAGE("Failed to create operation node from token");
            return nullptr;
        }
        
        switch (newNode->nodeType) {
            case OperationNode::type::VALUE:
                if (currentNode) {
                    currentNode->addChild(newNode);
                } else {
                    currentNode = newNode;
                }
                break;
            case OperationNode::type::LOGICAL_NOT:
                if (currentNode) {
                    currentNode->addChild(newNode);
                    currentNode = newNode;
                }
                else {
                    currentNode = newNode;
                }
                break;
            case OperationNode::type::COMPARISON:
            case OperationNode::type::EQUALITY:
            case OperationNode::type::LOGICAL_AND:
            case OperationNode::type::LOGICAL_OR:
            case OperationNode::type::ADDITION:
            case OperationNode::type::MULTIPLICATION:
                if (currentNode) {
                    newNode->addChild(currentNode);
                    currentNode = newNode;
                } else {
                    // WARNING_MESSAGE("Operator node without left operand");
                    error.success = false;
                    error.errorType = ErrorInfos::ErrorType::SYNTAX_ERROR;
                    error.printError();
                    return nullptr;
                }
                break;
            default:
                WARNING_MESSAGE("Unsupported operation node type in tree building");
                return nullptr;
        }
    }

    while (currentNode && currentNode->parent) {
        currentNode = currentNode->parent;
    }
    return currentNode;
}

FlagDataPtr LogicBlock::parse_substring(const std::string& str, size_t idx_start, size_t idx_end)
{
    std::string substring = str.substr(idx_start, idx_end - idx_start);

    // std::cout << "Parsing logic block substring: " << substring << std::endl;

    // search for ifs and parse them and tokenize the contents of the test, the then and the else if present
    for (int i = 0; i < substring.length(); i++) {
        if (substring.substr(i, 2) == "if") {
            FlagDataPtr ifResult = nullptr;
            // find parens and if there is anything but whitespace before them, it's not an if statement
            size_t j = i + 2;
            while (j < substring.length() && isspace(substring[j])) {
                j++;
            }
            if (j >= substring.length() || substring[j] != '(') {
                // WARNING_MESSAGE("Malformed if statement in logic block");
                error.success = false;
                error.errorType = ErrorInfos::ErrorType::MALFORMED_IF_STATEMENT;
                error.column_start = i;
                error.column_end = i;
                static std::string info = " missing opening parenthesis for condition";
                error.messageExtra = &info;
                error.printError();
                return FlagData::MakeFlag(false);
            }
            // find matching closing paren
            int parenCount = 1;
            size_t k = j + 1;
            for (; k < substring.length(); k++) {
                if (substring[k] == '(') {
                    parenCount++;
                } else if (substring[k] == ')') {
                    parenCount--;
                    if (parenCount == 0) {
                        break;
                    }
                }
            }
            if (parenCount != 0) {
                // WARNING_MESSAGE("Mismatched parentheses in if statement");
                error.success = false;
                error.errorType = ErrorInfos::ErrorType::UNMATCHED_PARENS;
                error.column_start = j;
                error.column_end = j;
                error.printError();
                return FlagData::MakeFlag(false);
            }
            // parse the condition
            size_t col_start_old = error.column_start;
            size_t col_end_old = error.column_end;
            error.column_start = idx_start + j + 1;
            error.column_end = idx_start + k;
            FlagDataPtr conditionFlag = parse_substring(substring, j + 1, k);
            bool condition = conditionFlag->as_bool();
            // find then
            size_t thenIdx = k + 1;
            while (thenIdx < substring.length() && isspace(substring[thenIdx])) {
                thenIdx++;
            }
            if (substring.substr(thenIdx, 4) != "then") {
                // WARNING_MESSAGE("Malformed if statement, missing 'then'");
                error.success = false;
                error.errorType = ErrorInfos::ErrorType::MALFORMED_IF_STATEMENT;
                error.column_start = thenIdx;
                error.column_end = thenIdx;
                static std::string info = " missing 'then' keyword";
                error.messageExtra = &info;
                error.printError();
                return FlagData::MakeFlag(false);
            }
            thenIdx += 4;
            // get the content of the then block (delimited by parentheses)
            j = thenIdx;
            while (j < substring.length() && isspace(substring[j])) {
                j++;
            }
            if (j >= substring.length() || substring[j] != '(') {
                // WARNING_MESSAGE("Malformed then block in if statement");
                error.success = false;
                error.errorType = ErrorInfos::ErrorType::MALFORMED_IF_STATEMENT;
                error.column_start = j;
                error.column_end = j;
                static std::string info = " missing opening parenthesis for then block";
                error.messageExtra = &info;
                error.printError();
                return FlagData::MakeFlag(false);
            }
            parenCount = 1;
            k = j + 1;
            for (; k < substring.length(); k++) {
                if (substring[k] == '(') {
                    parenCount++;
                } else if (substring[k] == ')') {
                    parenCount--;
                    if (parenCount == 0) {
                        break;
                    }
                }
            }
            if (parenCount != 0) {
                // WARNING_MESSAGE("Mismatched parentheses in then block");
                error.success = false;
                error.errorType = ErrorInfos::ErrorType::UNMATCHED_PARENS;
                error.column_start = j;
                error.column_end = j;
                error.printError();
                return FlagData::MakeFlag(false);
            }
            size_t thenContentStart = j + 1;
            size_t thenContentEnd = k;
            std::string thenContent = substring.substr(j + 1, k - j - 1);

            // check for else
            size_t elseIdx = k + 1;
            while (elseIdx < substring.length() && isspace(substring[elseIdx])) 
                elseIdx++;
            
            std::string elseContent = "";
            size_t elseContentStart = 0;
            size_t elseContentEnd = 0;
            if (substring.substr(elseIdx, 4) == "else") {
                elseIdx += 4;
                // get the content of the else block (delimited by parentheses)
                j = elseIdx;
                while (j < substring.length() && isspace(substring[j])) {
                    j++;
                }
                if (j >= substring.length() || substring[j] != '(') {
                    // WARNING_MESSAGE("Malformed else block in if statement");
                    error.success = false;
                    error.errorType = ErrorInfos::ErrorType::MALFORMED_IF_STATEMENT;
                    error.column_start = j;
                    error.column_end = j;
                    static std::string info = " missing opening parenthesis for else block";
                    error.messageExtra = &info;
                    error.printError();
                    return FlagData::MakeFlag(false);
                }
                parenCount = 1;
                k = j + 1;
                for (; k < substring.length(); k++) {
                    if (substring[k] == '(') {
                        parenCount++;
                    } else if (substring[k] == ')') {
                        parenCount--;
                        if (parenCount == 0) {
                            break;
                        }
                    }
                }
                if (parenCount != 0) {
                    // WARNING_MESSAGE("Mismatched parentheses in else block");
                    error.success = false;
                    error.errorType = ErrorInfos::ErrorType::UNMATCHED_PARENS;
                    error.column_start = j;
                    error.column_end = j;
                    error.printError();
                    return FlagData::MakeFlag(false);
                }
                elseContentStart = j + 1;
                elseContentEnd = k;
                elseContent = substring.substr(j + 1, k - j - 1);
            }
            if (condition) {
                error.column_start = idx_start + thenContentStart;
                error.column_end = idx_start + thenContentEnd;
                ifResult = parse_substring(thenContent, 0, thenContent.length());
            } else if (!elseContent.empty()) {
                error.column_start = idx_start + elseContentStart;
                error.column_end = idx_start + elseContentEnd;
                // std::cout << "Parsing else content: " << elseContent << std::endl;
                ifResult = parse_substring(elseContent, 0, elseContent.length());  
            } 

            error.column_start = col_start_old;
            error.column_end = col_end_old;

            if (ifResult)
            {
                // replace the entire if statement with the result
                std::string ifResultStr;

                switch (ifResult->type) {
                    case FlagData::INT:
                    case FlagData::FLOAT:
                    case FlagData::BOOL:
                        ifResultStr = ifResult->as_string();
                        break;
                    case FlagData::STRING:
                        ifResultStr = "\"" + ifResult->as_string() + "\"";
                        break;
                    default:
                        WARNING_MESSAGE("Unsupported flag type in if result conversion to string");
                        ifResultStr = "false";
                        break;
                }

                substring = substring.substr(0, i) + ifResultStr + substring.substr(k + 1);
                // advance error.column_start by the length of the old if statement minus the length of the new result
                // std::cout << "i: " << i << ", k: " << k << ", ifResultStr.length: " << ifResultStr.length() << std::endl;
                // std::cout << "Old error.column_start: " << error.column_start << std::endl;
                error.column_start += (k + 1 - i) - ifResultStr.length();
                // std::cout << "Updated error.column_start to: " << error.column_start << std::endl;
                error.column_end = error.column_start;
                i += ifResultStr.length() - 1;
            }
            else 
            {
                // replace the entire if statement with nothing
                substring = substring.substr(0, i) + substring.substr(k + 1);
                i--;
            }
        }
    }

    if (substring.empty())
    {
        return FlagData::MakeFlag("");
    }

    std::vector<Token> tokens;
    std::vector<std::pair<int, int>> tokenPositions;
    bool success = tokenize(substring, tokens, tokenPositions);
    if (success && !tokens.empty()) 
    {   
        OperationNodePtr root = buildOperationTree(tokens, tokenPositions);

        if (root)
            return root->evaluate();
        else {
            // WARNING_MESSAGE("Failed to build operation tree for logic block");
            return FlagData::MakeFlag("");
        }
    }
    else {
        return FlagData::MakeFlag("");
    }
}

void LogicBlock::parse_string(std::string& str, std::string* filename)
{
    error.clear();
    error.fileName = filename;
    error.lineNumber = 0;
    std::string lineContentStr = str; // copy the content cause it's modified in place
    error.LineContent = lineContentStr.c_str();
    error.column_start = 0;
    error.column_end = 0;


    std::string logicBlockStart = "$(";
    std::string logicBlockEnd = ")";
    
    size_t pos = 0;
    size_t oldPos = 0;
    size_t startGlobalIdx = 1;
    while ((pos = str.find(logicBlockStart, pos)) != std::string::npos) {
        // poorly optimized but we need the count of newlines before pos
        size_t newlineCount = 0;
        size_t lineStart = 0;
        for (size_t i = 0; i < pos; i++) {
            if (str[i] == '\n')
            {
                newlineCount++;
                lineStart = i + 1;
            }
        }
        error.lineNumber = newlineCount;
        size_t startIdx = pos + logicBlockStart.length();
        size_t endIdx = startIdx;
        int parenCount = 1;
        while (endIdx < str.length() && parenCount > 0) {
            if (str[endIdx] == '(') {
                parenCount++;
            } else if (str[endIdx] == ')') {
                parenCount--;  
            }
            endIdx++;
        }
        if (parenCount != 0) {
            // WARNING_MESSAGE("Mismatched parentheses in logic block");
            error.success = false;
            error.errorType = ErrorInfos::ErrorType::UNMATCHED_PARENS;
            error.column_start = startGlobalIdx + (pos - oldPos) + 1;
            error.column_end = lineContentStr.length() + 1;
            
            error.printError();
            error.clear();
        }
        startGlobalIdx += pos - oldPos + logicBlockStart.length();
        error.column_start = startGlobalIdx - lineStart;
        error.column_end = startGlobalIdx - lineStart;
        FlagDataPtr resultFlag = parse_substring(str, startIdx, endIdx - 1);


        std::string resultStr = resultFlag->as_string();
        str.replace(pos, endIdx - pos, resultStr);
        pos += resultStr.length();

        startGlobalIdx += endIdx - startIdx;

        oldPos = pos;
    }
}

void writeInBuffer(
    char **buffer,
    char **writeHead,
    const char *readHead,
    size_t size,
    size_t & allocated
)
{
    size_t wanted = ((*writeHead)-(*buffer)+size);
    if(wanted > allocated)
    {
        while(wanted > allocated) allocated *= 2;

        char *newBuffer = new char[allocated];
        
        size_t writeHeadPos = (*writeHead)-(*buffer);
        memcpy(newBuffer, *buffer, writeHeadPos);

        delete [] *buffer;
        *writeHead = newBuffer+writeHeadPos;
        *buffer = newBuffer;
    }

    memcpy(*writeHead, readHead, size);
    *writeHead += size;
};

void LogicBlock::parse_string_cstr(char ** input, size_t &len, size_t allocated, std::string* filename) 
{
    error.clear();
    error.fileName = filename;
    error.lineNumber = 0;
    std::string lineContentStr(*input, len); // copy the content cause it's modified in place
    error.LineContent = lineContentStr.c_str();

    static const char logicBlockStart[] = "$(";
    // static const char logicBlockEnd[] = ")";

    static thread_local size_t blen = 4096;
    static thread_local char * buffer = new char[blen];

    char *readHead = *input;
    char *writeHead = buffer;

    char * pos = 0;
    size_t oldPos = 0;
    size_t startGlobalIdx = 1;
    while((pos = strstr(readHead, logicBlockStart)))
    {
        size_t globalPos = pos - *input;
        // poorly optimized (cause we recomputeit from scratch for each logic block) but we need the count of newlines before pos
        size_t newlineCount = 0;
        size_t lineStart = 0;
        for(size_t i = 0; i < (size_t)(pos - *input); i++)
        {
            if((*input)[i] == '\n')
            {
                newlineCount++;
                lineStart = i+1;
            }
        }
        error.lineNumber = newlineCount;

        size_t startIdx = (pos - readHead) + sizeof(logicBlockStart)-1;
        size_t endIdx = startIdx;
        int parenCount = 1;

        while (readHead[endIdx] && parenCount > 0)
        {
            if (readHead[endIdx] == '(')
                parenCount++;
            else if (readHead[endIdx] == ')')
                parenCount--;  

            endIdx++;
        }

        if (parenCount != 0)
        {
            error.success = false;
            error.errorType = ErrorInfos::ErrorType::UNMATCHED_PARENS;
            static std::string info = " unmatched parentheses in logic block";
            error.messageExtra = &info;
            
            error.column_start = startGlobalIdx + (globalPos - oldPos) + 1;
            error.column_end = lineContentStr.length() + 1;

            error.printError();
            error.clear();
        }

        startGlobalIdx += globalPos - oldPos + sizeof(logicBlockStart) - 1;

        // FlagDataPtr resultFlag = parse_substring_cstr(*input, len, startIdx, endIdx-1);
        error.column_start = startGlobalIdx - lineStart;
        error.column_end = startGlobalIdx - lineStart;

        FlagDataPtr resultFlag = parse_substring(readHead, startIdx, endIdx - 1);

        std::string resultStr = resultFlag->as_string();

        writeInBuffer(&buffer, &writeHead, readHead, pos-readHead, blen);
        writeInBuffer(&buffer, &writeHead, resultStr.c_str(), resultStr.size(), blen);

        readHead += endIdx;

        startGlobalIdx += endIdx - startIdx;
        oldPos = readHead - *input;
    }

    size_t remaining = ((*input)+len) - readHead + 1;
    if(remaining)
        writeInBuffer(&buffer, &writeHead, readHead, remaining, blen);

    size_t finalSize = writeHead - buffer;

    readHead = *input;
    writeInBuffer(input, &readHead, buffer, finalSize, allocated);

    len = finalSize-1;
}

std::string getSubStringErrorHint(const char *input, const size_t idx_start, const size_t idx_end, int idx_Error)
{
    std::string substr;
    substr.resize(idx_end-idx_start);
    memcpy(substr.data(), input, idx_end-idx_start);
    substr += '\0';

    return "\n\t\t" + substr;
}

/*
    TODO : fix
    this doesn"t work for nested if
*/
FlagDataPtr LogicBlock::LogicBlock::parse_substring_cstr(const char* input, const size_t len, const size_t idx_start, const size_t idx_end)
{
    // std::string input = str.substr(idx_start, idx_end - idx_start);


    // std::cout << "Parsing logic block input: " << input << std::endl;

    // search for ifs and parse them and tokenize the contents of the test, the then and the else if present

    std::string ifProcessedTmp = "";

    for (int i = idx_start; i < idx_end; i++)
    {
        // WARNING_MESSAGE(input[i])

        // if(false)
        if (input[i] == 'i' && input[i+1] == 'f')
        {
            
            FlagDataPtr ifResult = nullptr;
            // find parens and if there is anything but whitespace before them, it's not an if statement
            size_t j = i + 2;
            while (j < idx_end && isspace(input[j])) {
                j++;
            }
            if (j >= idx_end || input[j] != '(') {
                error.success = false;
                error.errorType = ErrorInfos::ErrorType::MALFORMED_IF_STATEMENT;
                static std::string info = " missing opening parenthesis for condition";
                error.messageExtra = &info;
                error.column_start = j;
                error.column_end = j;
                error.printError();
                return FlagData::MakeFlag(false);
            }
            // find matching closing paren
            int parenCount = 1;
            size_t k = j + 1;
            for (; k < idx_end; k++)
            {
                if (input[k] == '(')
                {
                    parenCount++;
                } 
                else 
                if (input[k] == ')')
                {
                    parenCount--;
                    if (parenCount == 0) {
                        break;
                    }
                }
            }
            if (parenCount != 0)
            {
                error.success = false;
                error.errorType = ErrorInfos::ErrorType::UNMATCHED_PARENS;
                error.column_start = j;
                error.column_end = j;
                error.printError();
                return FlagData::MakeFlag(false);
            }

            
            // parse the condition
            // FlagDataPtr conditionFlag = parse_substring(input, j + 1, k);
            FlagDataPtr conditionFlag = parse_substring_cstr(input, len, j+1, k);
            if(!error.success) return FlagData::MakeFlag(false);

            bool condition = conditionFlag->as_bool();
            // find then
            size_t thenIdx = k + 1;
            while (thenIdx < idx_end && isspace(input[thenIdx]))
            {
                thenIdx++;
            }
            if(
                input[thenIdx+0] != 't' ||
                input[thenIdx+1] != 'h' ||
                input[thenIdx+2] != 'e' ||
                input[thenIdx+3] != 'n'
            )
            {
                error.success = false;
                error.errorType = ErrorInfos::ErrorType::MALFORMED_IF_STATEMENT;
                static std::string info = " missing 'then' keyword";
                error.messageExtra = &info;
                error.column_start = k + 1;
                error.column_end = k + 1;
                error.printError();
                return FlagData::MakeFlag(false);
            }
            thenIdx += 4;
            // get the content of the then block (delimited by parentheses)
            j = thenIdx;
            while (j < idx_end && isspace(input[j])) j++;

            if (j >= idx_end || input[j] != '(')
            {
                error.success = false;
                error.errorType = ErrorInfos::ErrorType::MALFORMED_IF_STATEMENT;
                static std::string info = " missing opening parenthesis for then block";
                error.messageExtra = &info;
                error.column_start = j;
                error.column_end = j;
                error.printError();
                return FlagData::MakeFlag(false);
            }

            parenCount = 1;
            k = j + 1;

            for (; k < idx_end; k++)
            {
                if (input[k] == '(')
                    parenCount++;
                else if (input[k] == ')')
                {
                    parenCount--;
                    if (parenCount == 0)
                        break;
                }
            }

            if (parenCount != 0)
            {
                error.success = false;
                error.errorType = ErrorInfos::ErrorType::UNMATCHED_PARENS;
                error.column_start = j;
                error.column_end = j;
                error.printError();
                return FlagData::MakeFlag(false);
            }

            // std::string thenContent = input.substr(j + 1, k - j - 1);
            size_t thenContent_idx_start = j+1;
            size_t thenContent_idx_end = k;

            // check for else
            size_t elseIdx = k + 1;
            while (elseIdx < idx_end && isspace(input[elseIdx])) 
                elseIdx++;

            size_t elseContent_idx_start = 0;
            size_t elseContent_idx_end = 0;

            if (
                input[elseIdx+0] == 'e' &&
                input[elseIdx+1] == 'l' &&
                input[elseIdx+2] == 's' &&
                input[elseIdx+3] == 'e'
            )
            {
                elseIdx += 4;
                // get the content of the else block (delimited by parentheses)
                j = elseIdx;
                while (j < idx_end && isspace(input[j])) j++;

                if (j >= idx_end || input[j] != '(')
                {
                    error.success = false;
                    error.errorType = ErrorInfos::ErrorType::MALFORMED_IF_STATEMENT;
                    static std::string info = " missing opening parenthesis for else block";
                    error.messageExtra = &info;
                    error.column_start = j;
                    error.column_end = j;
                    error.printError();
                    return FlagData::MakeFlag(false);
                }

                parenCount = 1;
                k = j + 1;
                for (; k < idx_end; k++) 
                {
                    if (input[k] == '(')
                        parenCount++;
                    else
                    if (input[k] == ')')
                    {
                        parenCount--;
                        if (parenCount == 0)
                            break;
                    }
                }
                if (parenCount != 0)
                {
                    error.success = false;
                    error.errorType = ErrorInfos::ErrorType::UNMATCHED_PARENS;
                    error.column_start = j;
                    error.column_end = j;
                    error.printError();
                    return FlagData::MakeFlag(false);
                }

                // lseContent = substring.substr(j + 1, k - j - 1);
                elseContent_idx_start = j+1;
                elseContent_idx_end = k;
            }
            if (condition)
            {
                // ifResult = parse_substring(thenContent, 0, thenContent.length());
                ifResult = parse_substring_cstr(input, len, thenContent_idx_start, thenContent_idx_end);
            } 
            else if (elseContent_idx_start && elseContent_idx_end)
            {
                // std::cout << "Parsing else content: " << elseContent << std::endl;
                // ifResult = parse_substring(elseContent, 0, elseContent.length());  
                ifResult = parse_substring_cstr(input, len, elseContent_idx_start, elseContent_idx_end);
            } 

            if(!error.success)
                return FlagData::MakeFlag(false);

            if (ifResult)
            {
                // replace the entire if statement with the result
                std::string ifResultStr;

                switch (ifResult->type)
                {
                    case FlagData::INT:
                    case FlagData::FLOAT:
                    case FlagData::BOOL:
                        ifResultStr = ifResult->as_string();
                        break;
                    case FlagData::STRING:
                        ifResultStr = "\"" + ifResult->as_string() + "\"";
                        break;
                    default:
                        WARNING_MESSAGE("Unsupported flag type in if result conversion to string"); // shouldn't ever be called since it can only happen if the flag type is NONE
                        ifResultStr = "false";
                        break;
                }
                
                // input = input.substr(0, i) + ifResultStr + input.substr(k + 1);

                ifProcessedTmp.resize(ifResultStr.size() + (idx_end-idx_start) - (k+1-i));

                memcpy(ifProcessedTmp.data(), input+idx_start, i-idx_start);
                memcpy(ifProcessedTmp.data()+i-idx_start, ifResultStr.data(), ifResultStr.size());
                memcpy(ifProcessedTmp.data()+i-idx_start + ifResultStr.size(), input+k+1, idx_end-k-1);

                i += ifResultStr.length() - 1;
            }
            else 
            {
                // replace the entire if statement with nothing
                // input = input.substr(0, i) + input.substr(k + 1);

                ifProcessedTmp.resize((idx_end-idx_start) - (k+1-i));
                memcpy(ifProcessedTmp.data(), input+idx_start, i-idx_start);
                memcpy(ifProcessedTmp.data()+i-idx_start, input+k+1, idx_end-k-1);

                i--;
            }
        }
    }

    // ifProcessedTmp = "1 + 2 * 3";
    // NOTIF_MESSAGE(ifProcessedTmp)

    if (ifProcessedTmp.empty()) // TODO : adapt to the error checking that luna did
    {
        ifProcessedTmp.resize(idx_end-idx_start);
        memcpy(ifProcessedTmp.data(), input+idx_start, idx_end-idx_start);
    }


    std::vector<Token> tokens;
    std::vector<std::pair<int, int>> tokenPositions;
    bool success = tokenize(ifProcessedTmp, tokens, tokenPositions);
    if (success && !tokens.empty()) 
    {
        OperationNodePtr root = buildOperationTree(tokens, tokenPositions);

        if (root)
            return root->evaluate();
        else
        {
            // WARNING_MESSAGE("Failed to build operation tree for logic block");
            return FlagData::MakeFlag("");
        }
    }
    else 
    {
        return FlagData::MakeFlag("");
    }
}

void LogicBlock::registerAllFunctions()
{
    LogicBlock::registerFunction(
        LogicBlock::Function(
            "pow",
            FlagData::Type::FLOAT,
            {
                FlagData::Type::FLOAT,
                FlagData::Type::FLOAT
            },
            [](const std::vector<FlagDataPtr>& args) -> FlagDataPtr
            {
                double base = args[0]->as_float();
                double exponent = args[1]->as_float();
                return FlagData::MakeFlag((float)std::pow(base, exponent));
            }
        )
    );

    LogicBlock::registerFunction(
        LogicBlock::Function(
            "print",
            FlagData::Type::STRING,
            {
                FlagData::Type::STRING
            },
            [](const std::vector<FlagDataPtr>& args) -> FlagDataPtr
            {
                std::cout << args[0]->as_string() << std::endl;
                return FlagData::MakeFlag("");
            }
        )
    );
}