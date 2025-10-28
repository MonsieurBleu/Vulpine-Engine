#include "Flags.hpp"
#include "AssetManagerUtils.hpp"
#include "Utils.hpp"


FlagPtr Flag::MakeFlag(int value) {
    return std::make_shared<IntFlag>(value);
}

FlagPtr Flag::MakeFlag(float value) {
    return std::make_shared<FloatFlag>(value);
}

FlagPtr Flag::MakeFlag(const std::string& value) {
    return std::make_shared<StrFlag>(value);
}

FlagPtr Flag::MakeFlag(const char* value) {
    return std::make_shared<StrFlag>(std::string(value));
}

FlagPtr Flag::MakeFlag(bool value) {
    return std::make_shared<BoolFlag>(value);
}

template <>
FlagPtr Flag::MakeFlagFromScript<int>(const std::string& scriptName) {
    return std::make_shared<IntScriptFlag>(scriptName);
}

template <>
FlagPtr Flag::MakeFlagFromScript<float>(const std::string& scriptName) {
    return std::make_shared<FloatScriptFlag>(scriptName);
}

template <>
FlagPtr Flag::MakeFlagFromScript<std::string>(const std::string& scriptName) {
    return std::make_shared<StrScriptFlag>(scriptName);
}

template <>
FlagPtr Flag::MakeFlagFromScript<bool>(const std::string& scriptName) {
    return std::make_shared<BoolScriptFlag>(scriptName);
}

FlagWrapper& Flags::getFlag(const std::string& name) {
    auto it = flags.find(name);
    if (it != flags.end()) {
        return it->second;
    }

    flags[name] = FlagWrapper();
    return flags[name];
}

Flag& Flag::operator=(int v)
{
    if(type != INT) {
        WARNING_MESSAGE("Trying to assign int to non-int Flag");
        return *this;
    }

    if(isScripted) {
        WARNING_MESSAGE("Trying to assign int to scripted Flag");
        return *this;
    }

    static_cast<IntFlag*>(this)->value = v;
    return *this;
}

Flag& Flag::operator=(float v)
{
    if(type != FLOAT) {
        WARNING_MESSAGE("Trying to assign float to non-float Flag");
        return *this;
    }

    if(isScripted) {
        WARNING_MESSAGE("Trying to assign float to scripted Flag");
        return *this;
    }

    static_cast<FloatFlag*>(this)->value = v;
    return *this;
}

Flag& Flag::operator=(const std::string& v)
{
    if(type != STRING) {
        WARNING_MESSAGE("Trying to assign string to non-string Flag");
        return *this;
    }

    if(isScripted) {
        WARNING_MESSAGE("Trying to assign string to scripted Flag");
        return *this;
    }

    static_cast<StrFlag*>(this)->value = v;
    return *this;
}

Flag& Flag::operator=(bool v)
{
    if(type != BOOL) {
        WARNING_MESSAGE("Trying to assign bool to non-bool Flag");
        return *this;
    }

    if(isScripted) {
        WARNING_MESSAGE("Trying to assign bool to scripted Flag");
        return *this;
    }

    static_cast<BoolFlag*>(this)->value = v;
    return *this;
}

template <>
Flag& Flag::operator=<int>(const ScriptNameWrapper& v)
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
Flag& Flag::operator=<float>(const ScriptNameWrapper& v)
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
Flag& Flag::operator=<std::string>(const ScriptNameWrapper& v)
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
Flag& Flag::operator=<bool>(const ScriptNameWrapper& v)
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

std::string Flag::typeToString() {
    switch (type) {
        case INT: return "int";
        case FLOAT: return "float";
        case STRING: return "string";
        case BOOL: return "bool";
        case NONE: return "none";
        default: return "unknown";
    }
}

DATA_WRITE_FUNC_INIT(Flags)
{
    std::vector<std::pair<std::string, FlagWrapper>> valueFlags = data.getAllByValueFlags();
    out->Entry();
    out->write("ValueFlags", sizeof("ValueFlags") - 1);
    out->Tabulate();
    for (const auto& pair : valueFlags) {
        out->Entry();
        std::string s = "\"" + pair.second->typeToString() + "\" \"" + pair.first + "\" \"" + pair.second->as_string() + "\"";
        out->write(CONST_STRING_SIZED(s));
    }
    out->Break();
    std::vector<std::pair<std::string, FlagWrapper>> scriptFlags = data.getAllFlags();
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
DATA_WRITE_END_FUNC

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
                Flag::Type type = Flag::NONE;
                MAP_SAFE_READ(Flag::TypeMap, buff, type, typeStr);
                if (type == Flag::NONE) {
                    WARNING_MESSAGE("Unknown flag type: " + std::string(typeStr));
                    continue;
                }
                // buff->read();
                const char* name = buff->read();
                // buff->read();
                const char* valueStr = buff->read();

                std::cout << "Flag: " << typeStr << " " << name << " = " << valueStr << std::endl;

                switch (type) {
                    case Flag::INT:
                    {
                        int intValue;
                        if (!isInteger(std::string(valueStr), intValue))
                        {
                            WARNING_MESSAGE("Invalid integer value for flag '" + std::string(name) + "': " + std::string(valueStr));
                            intValue = 0;
                        }
                        std::cout << "Parsed int flag value: " << intValue << std::endl;
                        data.setFlag<int>(name, intValue);
                        break;
                    }
                    case Flag::FLOAT:
                    {
                        float floatValue;
                        if (!isFloat(std::string(valueStr), floatValue))
                        {
                            WARNING_MESSAGE("Invalid float value for flag '" + std::string(name) + "': " + std::string(valueStr));
                            floatValue = 0.0f;
                        }
                        std::cout << "Parsed float flag value: " << floatValue << std::endl;
                        data.setFlag<float>(name, floatValue);
                        break;
                    }
                    case Flag::STRING:
                        data.setFlag<std::string>(name, std::string(valueStr));
                        break;
                    case Flag::BOOL:
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
                Flag::Type type = Flag::NONE;
                MAP_SAFE_READ(Flag::TypeMap, buff, type, typeStr);
                if (type == Flag::NONE) {
                    WARNING_MESSAGE("Unknown flag type: " + std::string(typeStr));
                    continue;
                }
                const char* name = buff->read();
                const char* scriptName = buff->read();

                switch (type) {
                    case Flag::INT:
                        data.setFlagFromScript<int>(name, std::string(scriptName));
                        break;
                    case Flag::FLOAT:
                        data.setFlagFromScript<float>(name, std::string(scriptName));
                        break;
                    case Flag::STRING:
                        data.setFlagFromScript<std::string>(name, std::string(scriptName));
                        break;
                    case Flag::BOOL:
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

bool Flag::equals(const FlagPtr a, const FlagPtr b) {
    switch(a->type) {
        case Flag::INT:
            return a->as_int() == b->as_int();
        case Flag::FLOAT:
            return a->as_float() == b->as_float();
        case Flag::STRING:
            return a->as_string() == b->as_string();
        case Flag::BOOL:
            return a->as_bool() == b->as_bool();
        default:
            WARNING_MESSAGE("Unsupported flag type for equality operation");
            return false;
    }
}

bool Flag::notEquals(const FlagPtr a, const FlagPtr b) {
    return !equals(a, b);
}

bool Flag::lessThan(const FlagPtr a, const FlagPtr b) {
    switch(a->type) {
        case Flag::INT:
            return a->as_int() < b->as_int();
        case Flag::FLOAT:
            return a->as_float() < b->as_float();
        default:
            WARNING_MESSAGE("Unsupported flag type for less than operation");
            return false;
    }
}

bool Flag::greaterThan(const FlagPtr a, const FlagPtr b) {
    switch(a->type) {
        case Flag::INT:
            return a->as_int() > b->as_int();
        case Flag::FLOAT:
            return a->as_float() > b->as_float();
        default:
            WARNING_MESSAGE("Unsupported flag type for greater than operation");
            return false;
    }
}

bool Flag::lessThanOrEqual(const FlagPtr a, const FlagPtr b) {
    return !greaterThan(a, b);
}

bool Flag::greaterThanOrEqual(const FlagPtr a, const FlagPtr b) {
    return !lessThan(a, b);
}

bool Flag::logicalAnd(const FlagPtr a, const FlagPtr b) {
    return a->as_bool() && b->as_bool();
}

bool Flag::logicalOr(const FlagPtr a, const FlagPtr b) {
    return a->as_bool() || b->as_bool();
}

bool Flag::logicalNot(const FlagPtr a) {
    return !a->as_bool();
}

FlagPtr Flag::add(const FlagPtr a, const FlagPtr b)
{
    if (b->type == STRING)
    {
        return Flag::MakeFlag(a->as_string() + b->as_string());
    }

    switch(a->type) {
        case Flag::INT:
            return Flag::MakeFlag(a->as_int() + b->as_int());
        case Flag::FLOAT:
            return Flag::MakeFlag(a->as_float() + b->as_float());
        case Flag::STRING:
            return Flag::MakeFlag(a->as_string() + b->as_string());
        default:
            WARNING_MESSAGE("Unsupported flag type for addition operation, returning 0");
            return Flag::MakeFlag(0);
    }
}

FlagPtr Flag::subtract(const FlagPtr a, const FlagPtr b)
{
    switch(a->type) {
        case Flag::INT:
            return Flag::MakeFlag(a->as_int() - b->as_int());
        case Flag::FLOAT:
            return Flag::MakeFlag(a->as_float() - b->as_float());
        default:
            WARNING_MESSAGE("Unsupported flag type for subtraction operation, returning 0");
            return Flag::MakeFlag(0);
    }
}

FlagPtr Flag::multiply(const FlagPtr a, const FlagPtr b)
{
    switch(a->type) {
        case Flag::INT:
            return Flag::MakeFlag(a->as_int() * b->as_int());
        case Flag::FLOAT:
            return Flag::MakeFlag(a->as_float() * b->as_float());
        default:
            WARNING_MESSAGE("Unsupported flag type for multiplication operation, returning 0");
            return Flag::MakeFlag(0);
    }
}

FlagPtr Flag::divide(const FlagPtr a, const FlagPtr b)
{
    switch(a->type) {
        case Flag::INT:
            if (b->as_int() == 0) {
                WARNING_MESSAGE("Division by zero in int division, returning 0");
                return Flag::MakeFlag(0);
            }
            return Flag::MakeFlag(a->as_int() / b->as_int());
        case Flag::FLOAT:
            if (b->as_float() == 0.0f) {
                WARNING_MESSAGE("Division by zero in float division, returning 0.0f");
                return Flag::MakeFlag(0.0f);
            }
            return Flag::MakeFlag(a->as_float() / b->as_float());
        default:
            WARNING_MESSAGE("Unsupported flag type for division operation, returning 0");
            return Flag::MakeFlag(0);
    }
}

FlagPtr Flag::clone()
{
    if (isScripted)
    {
        switch (type) {
            case INT:
                return Flag::MakeFlagFromScript<int>(((ScriptFlagBase*)this)->luaScriptName);
            case FLOAT:
                return Flag::MakeFlagFromScript<float>(((ScriptFlagBase*)this)->luaScriptName);
            case STRING:
                return Flag::MakeFlagFromScript<std::string>(((ScriptFlagBase*)this)->luaScriptName);
            case BOOL:
                return Flag::MakeFlagFromScript<bool>(((ScriptFlagBase*)this)->luaScriptName);
            default:
                WARNING_MESSAGE("Unsupported flag type for cloning");
                return nullptr;
        }
    }
    else {
        switch (type) {
            case INT:
                return Flag::MakeFlag(as_int());
            case FLOAT:
                return Flag::MakeFlag(as_float());
            case STRING:
                return Flag::MakeFlag(as_string());
            case BOOL:
                return Flag::MakeFlag(as_bool());
            default:
                WARNING_MESSAGE("Unsupported flag type for cloning");
                return nullptr;
        }
    }
}

Flag::operator FlagWrapper()
{
    return FlagWrapper(shared_from_this());
}

FlagPtr LogicBlockParser::NotOperatorNode::evaluate(Flags& flags) 
{
    if (children.size() != 1) {
        WARNING_MESSAGE("NotOperatorNode must have exactly one child");
        return Flag::MakeFlag(false);
    }

    FlagPtr operandValue = children[0]->evaluate(flags);
    bool result = Flag::logicalNot(operandValue);
    return Flag::MakeFlag(result);
}

FlagPtr LogicBlockParser::AdditionOperatorNode::evaluate(Flags& flags) 
{
    if (children.size() != 2) {
        WARNING_MESSAGE("AdditionOperatorNode must have exactly two children");
        return Flag::MakeFlag(0);
    }

    FlagPtr leftValue = children[0]->evaluate(flags);
    FlagPtr rightValue = children[1]->evaluate(flags);

    switch (additionType) {
        case ADD:
            return Flag::add(leftValue, rightValue);
        case SUBTRACT:
            return Flag::subtract(leftValue, rightValue);
        default:
            WARNING_MESSAGE("Unsupported addition operator type");
            return Flag::MakeFlag(0);
    }
}

FlagPtr LogicBlockParser::MultiplicationOperatorNode::evaluate(Flags& flags) 
{
    if (children.size() != 2) {
        WARNING_MESSAGE("MultiplicationOperatorNode must have exactly two children");
        return Flag::MakeFlag(0);
    }

    FlagPtr leftValue = children[0]->evaluate(flags);
    FlagPtr rightValue = children[1]->evaluate(flags);

    switch (multiplicationType) {
        case MULTIPLY:
            return Flag::multiply(leftValue, rightValue);
        case DIVIDE:
            return Flag::divide(leftValue, rightValue);
        default:
            WARNING_MESSAGE("Unsupported multiplication operator type");
            return Flag::MakeFlag(0);
    }
}


FlagPtr LogicBlockParser::ComparisonOperatorNode::evaluate(Flags& flags) 
{
    if (children.size() != 2) {
        WARNING_MESSAGE("ComparisonOperatorNode must have exactly two children");
        return Flag::MakeFlag(false);
    }

    FlagPtr leftValue = children[0]->evaluate(flags);
    FlagPtr rightValue = children[1]->evaluate(flags);

    bool result = false;
    switch (comparisonType) {
        case ComparisonType::LESS_THAN:
            result = Flag::lessThan(leftValue, rightValue);
            break;
        case ComparisonType::GREATER_THAN:
            // std::cout << "Comparing " << leftValue->as_string() << " > " << rightValue->as_string() << std::endl;
            result = Flag::greaterThan(leftValue, rightValue);
            break;
        case ComparisonType::LESS_THAN_OR_EQUAL:
            result = Flag::lessThanOrEqual(leftValue, rightValue);
            break;
        case ComparisonType::GREATER_THAN_OR_EQUAL:
            result = Flag::greaterThanOrEqual(leftValue, rightValue);
            break;
        default:
            WARNING_MESSAGE("Unsupported comparison operator type");
            break;
    }
    return Flag::MakeFlag(result);
}

FlagPtr LogicBlockParser::EqualityOperatorNode::evaluate(Flags& flags) 
{
    if (children.size() != 2) {
        WARNING_MESSAGE("EqualityOperatorNode must have exactly two children");
        return Flag::MakeFlag(false);
    }

    FlagPtr leftValue = children[0]->evaluate(flags);
    FlagPtr rightValue = children[1]->evaluate(flags);

    bool result = false;
    switch (equalityType) {
        case EqualityType::EQUALS:
            result = Flag::equals(leftValue, rightValue);
            break;
        case EqualityType::NOT_EQUALS:
            result = Flag::notEquals(leftValue, rightValue);
            break;
        default:
            WARNING_MESSAGE("Unsupported equality operator type");
            break;
    }
    return Flag::MakeFlag(result);
}

FlagPtr LogicBlockParser::LogicalAndOperatorNode::evaluate(Flags& flags) 
{
    if (children.size() != 2) {
        WARNING_MESSAGE("LogicalAndOperatorNode must have exactly two children");
        return Flag::MakeFlag(false);
    }

    FlagPtr leftValue = children[0]->evaluate(flags);
    FlagPtr rightValue = children[1]->evaluate(flags);

    bool result = Flag::logicalAnd(leftValue, rightValue);
    return Flag::MakeFlag(result);
}

FlagPtr LogicBlockParser::LogicalOrOperatorNode::evaluate(Flags& flags) 
{
    if (children.size() != 2) {
        WARNING_MESSAGE("LogicalOrOperatorNode must have exactly two children");
        return Flag::MakeFlag(false);
    }

    FlagPtr leftValue = children[0]->evaluate(flags);
    FlagPtr rightValue = children[1]->evaluate(flags);

    bool result = Flag::logicalOr(leftValue, rightValue);
    return Flag::MakeFlag(result);
}

std::vector<LogicBlockParser::Token> LogicBlockParser::tokenize(const std::string& str)
{
    std::vector<Token> tokens;
    size_t i = 0;
    while (i < str.length()) {
        if (isspace(str[i])) {
            i++;
            continue;
        }
        else if (str[i] == '(') {
            tokens.emplace_back(Token::TokenType::PARENS_OPEN, "(");
            i++;
            continue;
        }
        else if (str[i] == ')') {
            tokens.emplace_back(Token::TokenType::PARENS_CLOSE, ")");
            i++;
            continue;
        }
        else if (str.substr(i, 2) == "&&") {
            tokens.emplace_back(Token::TokenType::LOGICAL_AND, "&&");
            i += 2;
            continue;
        }
        else if (str.substr(i, 2) == "||") {
            tokens.emplace_back(Token::TokenType::LOGICAL_OR, "||");
            i += 2;
            continue;
        }
        else if (str[i] == '!') {
            tokens.emplace_back(Token::TokenType::NOT, "!");
            i += 1;
            continue;
        }
        else if (str.substr(i, 2) == "==") {
            tokens.emplace_back(Token::TokenType::EQUALS, "==");
            i += 2;
            continue;
        }
        else if (str.substr(i, 2) == "!=") {
            tokens.emplace_back(Token::TokenType::NOT_EQUALS, "!=");
            i += 2;
            continue;
        }
        else if (str[i] == '<') {
            if (i + 1 < str.length() && str[i + 1] == '=') {
                tokens.emplace_back(Token::TokenType::LESS_THAN_OR_EQUAL, "<=");
                i += 2;
            } else {
                tokens.emplace_back(Token::TokenType::LESS_THAN, "<");
                i += 1;
            }
            continue;
        }
        else if (str[i] == '>') {
            if (i + 1 < str.length() && str[i + 1] == '=') {
                tokens.emplace_back(Token::TokenType::GREATER_THAN_OR_EQUAL, ">=");
                i += 2;
            } else {
                tokens.emplace_back(Token::TokenType::GREATER_THAN, ">");
                i += 1;
            }
            continue;
        }
        else if (str[i] == '+') {
            tokens.emplace_back(Token::TokenType::ADD, "+");
            i += 1;
            continue;
        }
        else if (str[i] == '-') {
            tokens.emplace_back(Token::TokenType::SUBTRACT, "-");
            i += 1;
            continue;
        }
        else if (str[i] == '*') {
            tokens.emplace_back(Token::TokenType::MULTIPLY, "*");
            i += 1;
            continue;
        }
        else if (str[i] == '/') {
            tokens.emplace_back(Token::TokenType::DIVIDE, "/");
            i += 1;
            continue;
        }
        else {
            // it's either a flag, a function or an immediate value
            size_t start = i;
            if ((str[i] == '\"') || (str[i] == '\'')) {
                char quoteChar = str[i];
                i++;
                while (i < str.length() && str[i] != quoteChar) {
                    i++;
                }
                i++; // consume closing quote
            } else {
                while (
                        i < str.length() && 
                        !isspace(str[i]) && 
                        str[i] != '(' && 
                        str[i] != ')' && 
                        str.substr(i, 2) != "&&" && 
                        str.substr(i, 2) != "||" && 
                        str.substr(i, 2) != "==" && 
                        str.substr(i, 2) != "!=" && 
                        str[i] != '<' && 
                        str[i] != '>'
                    ) {
                    i++;
                }
            }
            std::string tokenStr = str.substr(start, i - start);
            tokens.emplace_back(Token::TokenType::FLAG, tokenStr);
        }
    }
    return tokens;
}

LogicBlockParser::OperationNodePtr LogicBlockParser::Token::toOperationNode(Flags& flags) const
{
    switch (type) {
        case FLAG: {
            // Determine if it's a flag, an immediate value, or a function call
            // First check for immediate values
            if (value == "true" || value == "false") {
                return std::make_shared<FlagValueNode>(Flag::MakeFlag(value == "true"));
            }
            int intValue;
            if (isInteger(value, intValue)) {
                return std::make_shared<FlagValueNode>(Flag::MakeFlag(intValue));
            }
            float floatValue;
            if (isFloat(value, floatValue)) {
                return std::make_shared<FlagValueNode>(Flag::MakeFlag(floatValue));
            }
            if (
                ((value.front() == '\"' && value.back() == '\"') || (value.front() == '\'' && value.back() == '\'')) && value.length() >= 2
            ) {
                std::string strValue = value.substr(1, value.length() - 2);
                return std::make_shared<FlagValueNode>(Flag::MakeFlag(strValue));
            }
            // Next check for function calls (not implemented yet)
            // Finally, if it's formated like a flag `${flag_name}`, extract the flag name
            if (value.length() >= 4 && value.substr(0, 2) == "${" && value.back() == '}') {
                std::string flagName = value.substr(2, value.length() - 3);
                FlagWrapper flag = flags.getFlag(flagName);
                FlagWrapper flagClone;
                if (flag.flag) 
                {
                    flagClone = flag->clone();
                }
                else 
                {
                    WARNING_MESSAGE("Flag '" + flagName + "' not found, defaulting to false");
                    flagClone = Flag::MakeFlag(false);
                }
                
                return std::make_shared<FlagValueNode>(flagClone.flag);
            }
            WARNING_MESSAGE("Unrecognized flag or value: " + value + ", defaulting to false");
            return std::make_shared<FlagValueNode>(Flag::MakeFlag(false));
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

LogicBlockParser::OperationNodePtr LogicBlockParser::buildOperationTree(std::vector<Token> tokens, Flags& flags)
{
    if (tokens.empty()) {
        WARNING_MESSAGE("No tokens to build operation tree");
        return nullptr;
    }

    OperationNodePtr currentNode = nullptr;

    for (int i = 0; i < tokens.size(); i++) {
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
                WARNING_MESSAGE("Mismatched parentheses in logic block");
                return nullptr;
            }
            OperationNodePtr subTree = buildOperationTree(
                std::vector<Token>(tokens.begin() + i + 1, tokens.begin() + j), 
                flags);
            if (!subTree) {
                WARNING_MESSAGE("Failed to build subtree for parentheses");
                return nullptr;
            }
            if (currentNode) {
                currentNode->addChild(subTree);
            } else {
                currentNode = subTree;
            }
            i = j; // Move index to closing parenthesis
            continue;
        }
        else if (token.type == Token::TokenType::PARENS_CLOSE)
        {
            WARNING_MESSAGE("Mismatched parentheses in logic block");
            return nullptr;
        }
        OperationNodePtr newNode = token.toOperationNode(flags);
        if (!newNode) {
            WARNING_MESSAGE("Failed to create operation node from token");
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
                    WARNING_MESSAGE("Operator node without left operand");
                    return nullptr;
                }
                break;
            default:
                WARNING_MESSAGE("Unsupported operation node type in tree building");
                return nullptr;
        }
    }

    while (currentNode->parent) {
        currentNode = currentNode->parent;
    }
    return currentNode;
}

FlagPtr LogicBlockParser::parse_substring(const std::string& str, size_t idx_start, size_t idx_end, Flags& flags)
{
    std::string substring = str.substr(idx_start, idx_end - idx_start);

    // std::cout << "Parsing logic block substring: " << substring << std::endl;

    // search for ifs and parse them and tokenize the contents of the test, the then and the else if present
    for (int i = 0; i < substring.length(); i++) {
        if (substring.substr(i, 2) == "if") {
            FlagPtr ifResult = nullptr;
            // find parens and if there is anything but whitespace before them, it's not an if statement
            size_t j = i + 2;
            while (j < substring.length() && isspace(substring[j])) {
                j++;
            }
            if (j >= substring.length() || substring[j] != '(') {
                WARNING_MESSAGE("Malformed if statement in logic block");
                return Flag::MakeFlag(false);
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
                WARNING_MESSAGE("Mismatched parentheses in if statement");
                return Flag::MakeFlag(false);
            }
            // parse the condition
            FlagPtr conditionFlag = parse_substring(substring, j + 1, k, flags);
            bool condition = conditionFlag->as_bool();
            // find then
            size_t thenIdx = k + 1;
            while (thenIdx < substring.length() && isspace(substring[thenIdx])) {
                thenIdx++;
            }
            if (substring.substr(thenIdx, 4) != "then") {
                WARNING_MESSAGE("Malformed if statement, missing 'then'");
                return Flag::MakeFlag(false);
            }
            thenIdx += 4;
            // get the content of the then block (delimited by parentheses)
            j = thenIdx;
            while (j < substring.length() && isspace(substring[j])) {
                j++;
            }
            if (j >= substring.length() || substring[j] != '(') {
                WARNING_MESSAGE("Malformed then block in if statement");
                return Flag::MakeFlag(false);
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
                WARNING_MESSAGE("Mismatched parentheses in then block");
                return Flag::MakeFlag(false);
            }
            std::string thenContent = substring.substr(j + 1, k - j - 1);

            // check for else
            size_t elseIdx = k + 1;
            while (elseIdx < substring.length() && isspace(substring[elseIdx])) 
                elseIdx++;
            
            std::string elseContent = "";
            if (substring.substr(elseIdx, 4) == "else") {
                elseIdx += 4;
                // get the content of the else block (delimited by parentheses)
                j = elseIdx;
                while (j < substring.length() && isspace(substring[j])) {
                    j++;
                }
                if (j >= substring.length() || substring[j] != '(') {
                    WARNING_MESSAGE("Malformed else block in if statement");
                    return Flag::MakeFlag(false);
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
                    WARNING_MESSAGE("Mismatched parentheses in else block");
                    return Flag::MakeFlag(false);
                }
                elseContent = substring.substr(j + 1, k - j - 1);
            }
            if (condition) {
                ifResult = parse_substring(thenContent, 0, thenContent.length(), flags);
            } else if (!elseContent.empty()) {
                // std::cout << "Parsing else content: " << elseContent << std::endl;
                ifResult = parse_substring(elseContent, 0, elseContent.length(), flags);  
            } 

            if (ifResult)
            {
                // replace the entire if statement with the result
                std::string ifResultStr;

                switch (ifResult->type) {
                    case Flag::INT:
                    case Flag::FLOAT:
                    case Flag::BOOL:
                        ifResultStr = ifResult->as_string();
                        break;
                    case Flag::STRING:
                        ifResultStr = "\"" + ifResult->as_string() + "\"";
                        break;
                    default:
                        WARNING_MESSAGE("Unsupported flag type in if result conversion to string");
                        ifResultStr = "false";
                        break;
                }

                substring = substring.substr(0, i) + ifResultStr + substring.substr(k + 1);
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
        return Flag::MakeFlag("");
    }

    std::vector<Token> tokens = tokenize(substring);
    
    OperationNodePtr root = buildOperationTree(tokens, flags);

    if (root)
        return root->evaluate(flags);
    else {
        WARNING_MESSAGE("Failed to build operation tree for logic block");
        return Flag::MakeFlag("");
    }
}

void LogicBlockParser::parse_string(std::string& str, Flags& flags)
{
    std::string logicBlockStart = "$(";
    std::string logicBlockEnd = ")";
    
    size_t pos = 0;
    while ((pos = str.find(logicBlockStart, pos)) != std::string::npos) {
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
            WARNING_MESSAGE("Mismatched parentheses in logic block");
            break;
        }
        FlagPtr resultFlag = parse_substring(str, startIdx, endIdx - 1, flags);
        std::string resultStr = resultFlag->as_string();
        str.replace(pos, endIdx - pos, resultStr);
        pos += resultStr.length();
    }
}

