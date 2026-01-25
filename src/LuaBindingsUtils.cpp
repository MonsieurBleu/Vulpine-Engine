#include <Scripting/LuaBindingUtils.hpp>
#include <Utils.hpp>

std::string_view getLuaTypeName(const std::string_view &name)
{
    // #define REMOVE_PRETTY(type) if(name == type_name<type>()) return #type;

    if(name.empty()) return "";

    std::string name2(name);
    replace(name2, "const ", "");
    replace(name2, " &", "");
    replace(name2, " *", "");

    auto alias = luaBindingTypesAlias.find(name2);
    if(alias != luaBindingTypesAlias.end())
        return alias->second;

    WARNING_MESSAGE(
        "When generating LUA documentation for type '" ,  name ,  "'. No declared alias exist for this type."
    )

    return name;
}


std::vector<std::vector<std::string>> processArgsTraits(const std::vector<std::string_view> &args)
{
    const int size = args.size();
    std::vector<std::vector<std::string>> newargs(size);

    for(int i = 0; i < size; i++)
    {
        std::string_view tmp = args[i];

        if(tmp.substr(0, 10) == "std::tuple")
        {
            tmp.remove_prefix(sizeof("std::tuple<")-1);
            tmp.remove_suffix(sizeof(">")-1);
        }

        newargs[i].push_back("");

        int templateCount = 0;
        for(auto &c : tmp)
        {
            bool pullback = false;
            switch (c) {
                case ' ' : if(!newargs[i].back().empty()) pullback = true; break;
                case ',' : if(templateCount) pullback = true; else newargs[i].push_back(""); break;
                case '<' : pullback = true; templateCount++; break;
                case '>' : pullback = true; templateCount--; break;
                default  : pullback = true; break;
            }

            if(pullback) newargs[i].back() += c;
        }

        for(auto &s : newargs[i]) s = getLuaTypeName(s);

        // for(auto &s : newargs[i]) NOTIF_MESSAGE(s)
    }

    return newargs;
}

std::string getAutoParamName(int i, std::string type)
{
    return type[0] + std::to_string(i+1);
    // return "_" + type.substr(0, 3) + "_" + std::to_string(i+1);
}

void writeConstructorDocumentation(
    const std::string_view &type, 
    const std::vector<std::vector<std::string>> &argsNames,
    const std::vector<std::vector<std::string>> &argsType
)
{
    luaHeaderAppendBuffer = std::stringstream();;

    const int overloadSize = argsType.size();

    luaHeaderAppendBuffer << "--->>> Constructor" << (overloadSize ? "s" : "") << "\n";
    
    for(int i = 0; i < overloadSize; i++)
    {
        luaHeaderAppendBuffer << "---@overload fun(";

        int argsSize = argsType[i].size();
        for(int j = 0; j < argsSize; j++)
        {
            if(j) luaHeaderAppendBuffer << ", ";

            if(i >= argsNames.size() || j >= argsNames[i].size())
                luaHeaderAppendBuffer << getAutoParamName(j, argsType[i][j]) << " : " << argsType[i][j];
            else
                luaHeaderAppendBuffer << argsNames[i][j] << " : " << argsType[i][j];
        }

        luaHeaderAppendBuffer << ") : " << type << "\n";
    }
    luaHeaderAppendBuffer << "---@return " << type << "\n";
    luaHeaderAppendBuffer << "function " << type << "() end\n";
}

void writeFunctionDocumentation(
    const std::string_view &name, 
    const std::vector<std::vector<std::string>> &argsNames,
    const std::vector<std::vector<std::string>> &argsType
)
{
    const int overloadSize = argsType.size();

    // luaHeader << "--->>> Constructor" << (overloadSize ? "s" : "") << "\n";

    luaHeader << "---@return nil\n";
    
    for(int i = 0; i < overloadSize; i+=2)
    {
        luaHeader << "---@overload fun(";

        int argsSize = argsType[i].size();
        for(int j = 0; j < argsSize; j++)
        {
            if(j) luaHeader << ", ";

            if(i/2 >= argsNames.size() || j >= argsNames[i].size())
                luaHeader << getAutoParamName(j, argsType[i][j]) << " : " << argsType[i][j];
            else
                luaHeader << argsNames[i/2][j] << " : " << argsType[i][j];
        }

        luaHeader << ") : " << argsType[i+1][0] << "\n";
    }

    luaHeader << "function " << name << "() end\n\n";

    // luaHeader << "---@return " << type << "\n";
    // luaHeader << "function " << type << "() end\n";
}

void writeFunctionDocumentation(
    bool singleLign,
    const std::string_view &name,
    const std::string_view &ret,
    const std::vector<std::string> &argsNames,
    const std::vector<std::string> &argsType
)
{   
    int size = argsType.size();
    if(size and argsType[0].empty()) size = 0;

    if(singleLign)
    {   
        luaHeader << "---@field " << name << " fun(self";

        for(int i = 0; i < size; i++)
        {
            luaHeader << ", ";

            if(i >= argsNames.size())
                luaHeader << getAutoParamName(i, argsType[i]) << " : " << argsType[i];
            else 
                luaHeader << argsNames[i] << " : " << argsType[i];
        }

        luaHeader << ") : " << ret << "\n";
        return;
    }



    for(int i = 0; i < size; i++)
    {
        if(i >= argsNames.size())
            luaHeader << "---@param " << getAutoParamName(i, argsType[i]) << " " << argsType[i] << "\n";
        else 
            luaHeader << "---@param " << argsNames[i] << " " << argsType[i] << "\n";
    }

    luaHeader << "---@return " << ret << "\n";
    luaHeader << "function " << name << "(";
    for(int i = 0; i < size; i++)
    {
        if(i) luaHeader << ", ";

        if(i >= argsNames.size())
            luaHeader << getAutoParamName(i, argsType[i]);
        else
            luaHeader << argsNames[i];
    }
    luaHeader << ") end\n\n";
}
