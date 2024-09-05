#pragma once

// TODO : maybe move it in another utils header
#include <string_view>
template <typename T>
constexpr auto type_name() {
  std::string_view name, prefix, suffix;
#ifdef __clang__
  name = __PRETTY_FUNCTION__;
  prefix = "auto type_name() [T = ";
  suffix = "]";
#elif defined(__GNUC__)
  name = __PRETTY_FUNCTION__;
  prefix = "constexpr auto type_name() [with T = ";
  suffix = "]";
#elif defined(_MSC_VER)
  name = __FUNCSIG__;
  prefix = "auto __cdecl type_name<";
  suffix = ">(void)";
#endif
  name.remove_prefix(prefix.size());
  name.remove_suffix(suffix.size());
  return name;
}


/************ VULPINE TEXT BUFF ************/
    #define NEW_VALUE *buff->read() == ':'
    #define END_VALUE *buff->read() == ';'



/************ ASSET LOADER  ************/
    // #define ASSET_DEBUG_PRINT
    #ifdef ASSET_DEBUG_PRINT
        #define PRINT_LOADER_DEBUG_CREATE std::cout \
            << TERMINAL_INFO << "Loading " \
            << TERMINAL_NOTIF << name \
            << TERMINAL_INFO << " from "\
            << TERMINAL_UNDERLINE << type_name<decltype(r)>() \
            << TERMINAL_RESET << "\n" ;
    #else
        #define PRINT_LOADER_DEBUG_CREATE ;
    #endif

    #define EARLY_RETURN_IF_LOADED \
        auto check = loadedAssets.find(name); \
        if(check != loadedAssets.end()){\
            if(buff->data + buff->getReadHead() < end) buff->setHead(end); /*if the current buffer is ahead of the end of the info, this means that the dependant info is currently reading it as a reference*/ \
            return check->second;} \
        auto &r = loadedAssets[name]; \
        PRINT_LOADER_DEBUG_CREATE \
        buff->setHead(values); 

    #define EXIT_ROUTINE_AND_RETURN end = buff->data + buff->getReadHead(); return r;


    #define LOADER_DO_RUNTIME_ASSERT

    #ifdef LOADER_DO_RUNTIME_ASSERT
        #define LOADER_ASSERT(cond) assert(cond);
    #else
        #define LOADER_ASSERT(cond) if(cond){ EXIT_ROUTINE_AND_RETURN }
    #endif

    #define LOAD_VALUE(type) Loader<type>::addInfos(buff).loadFromInfos()

    #define LOAD_MODEL_STATE_3D(state) {\
                ModelState3D &stateRefTmp = state; \
                while(NEW_VALUE) \
                {\
                    char *stateMember = buff->read(); \
                    if(!strcmp(stateMember, "position")) \
                        stateRefTmp.setPosition(fromStr<vec3>(buff->read()));\
                    else \
                    if(!strcmp(stateMember, "rotation"))\
                        stateRefTmp.setRotation(radians(fromStr<vec3>(buff->read())));\
                    else \
                    if(!strcmp(stateMember, "scalev3"))\
                        stateRefTmp.setScale(fromStr<vec3>(buff->read()));\
                    else \
                    if(!strcmp(stateMember, "scalev1"))\
                        stateRefTmp.scaleScalar(fromStr<float>(buff->read()));\
                    else\
                    if(!strcmp(stateMember, "frustum"))\
                        stateRefTmp.frustumCulled = fromStr<int>(buff->read());\
                    else\
                        FILE_ERROR_MESSAGE(name, "ModelState3D member '" << stateMember << "' not recognized.");\
                }    }\



/************ DATA LOADER WRITE  ************/
    #define DATA_WRITE_FUNC(T) template<> \
        void DataLoader<T>::write(const T & data, VulpineTextOutputRef out)

    #define DATA_WRITE_INIT(T) out->Entry(); WRITE_NAME(T, out); out->Tabulate();

    #define DATA_WRITE_INIT_AN(T) out->Tabulate();

    #define DATA_WRITE_END out->Break();

    #define DATA_WRITE_FUNC_INIT(T) DATA_WRITE_FUNC(T) { DATA_WRITE_INIT(T)
    #define DATA_WRITE_END_FUNC  DATA_WRITE_END; }

    #define FTXTP_WRITE_ELEMENT(data, elem) \
        {out->Entry(); \
        WRITE_NAME(elem, out); \
        FastTextParser::write<decltype(data.elem)>(data.elem, out->getReadHead());}
    
    #define FTXTP_WRITE_ELEMENT_COND(data, elem, cond) if(cond) {FTXTP_WRITE_ELEMENT(data, elem)}

    #define FTXTP_WRITE_ELEMENT_COND_E(data, elem, cond) if(elem cond) {FTXTP_WRITE_ELEMENT(data, elem)}

    #define CONST_CSTRING_SIZED(str) str, sizeof(str)-1
    #define CONST_STRING_SIZED(str) str.c_str(), str.size()

    #define WRITE_NAME(T, out) out->write(CONST_CSTRING_SIZED(#T " "));

    #define WRITE_FUNC_RESULT(name, f) \
            out->Entry();\
            WRITE_NAME(name, out);\
            FastTextParser::write<>(f, out->getReadHead());

    #define WRITE_FUNC_RESULT_COND(name, f, cond) \
            {\
                auto tmpWFRC = f;\
                if(tmpWFRC cond) {WRITE_FUNC_RESULT(name, tmpWFRC)} \
            }\


/************ DATA LOADER READ  ************/
    #define DATA_READ_FUNC(T) template<> \
        T DataLoader<T>::read(VulpineTextBuffRef buff) 

    #define DATA_READ_INIT(T) T data;

    #define DATA_READ_END return data;

    #define DATA_READ_FUNC_INIT(T) DATA_READ_FUNC(T) { DATA_READ_INIT(T) WHILE_NEW_VALUE
    #define DATA_READ_FUNC_INITI(T, initcode) DATA_READ_FUNC(T) { DATA_READ_INIT(T) initcode; WHILE_NEW_VALUE
    #define DATA_READ_END_FUNC  WHILE_NEW_VALUE_END DATA_READ_END}

    #define IF_MEMBER_READ_VALUE(x) if(!strcmp(member, #x) && (value = buff->read()) != nullptr)
    #define IF_MEMBER(x) if(!strcmp(member, #x))

    #define IF_MEMBER_FTXTP_LOAD(data, elem) \
        IF_MEMBER_READ_VALUE(elem)\
            data.elem = FastTextParser::read<decltype(data.elem)>(value);

    #define MAP_SAFE_READ(map, buff, data, value) \
        {                                  \
            const char *mapSafeReadStrTmp = value; \
            auto mapSafeReadIterTmp = map.find(mapSafeReadStrTmp); \
            if(mapSafeReadIterTmp == map.end())\
                WARNING_MESSAGE(#map << ": Can't find map element'" << mapSafeReadStrTmp << "'. nothing will be loaded for this elemnt")\
            else \
                data = mapSafeReadIterTmp->second; \
        } 

    #define MAP_SAFE_READ_F(map, buff, data, value) \
        {                                  \
            const char *mapSafeReadStrTmp = value; \
            auto mapSafeReadIterTmp = map.find(mapSafeReadStrTmp); \
            if(mapSafeReadIterTmp == map.end())\
                WARNING_MESSAGE(#map << ": Can't find map element'" << mapSafeReadStrTmp << "'. nothing will be loaded for this elemnt")\
            else \
                data(mapSafeReadIterTmp->second); \
        } 

    #define MAP_SAFE_READ_FC(map, buff, data, castType, value) \
        {                                  \
            const char *mapSafeReadStrTmp = value; \
            auto mapSafeReadIterTmp = map.find(mapSafeReadStrTmp); \
            if(mapSafeReadIterTmp == map.end())\
                WARNING_MESSAGE(#map << ": Can't find map element'" << mapSafeReadStrTmp << "'. nothing will be loaded for this element")\
            else \
                data((castType)mapSafeReadIterTmp->second); \
        } 

    #define MEMBER_NOTRECOGNIZED_ERROR {\
        FILE_ERROR_MESSAGE(buff->getSource(), type_name<decltype(data)>() << " member '" << member << "' not recognized. This element will be skipped.");\
        buff->skipUntilNewElement();\
    }\

    #define WHILE_NEW_VALUE \
            while (NEW_VALUE){ \
        const char *member = buff->read(); \
        const char *value = nullptr; \

    #define WHILE_NEW_VALUE_END \
            else MEMBER_NOTRECOGNIZED_ERROR\
            if(value > buff->data + buff->getReadHead())\
                buff->setHead(value);\
            }


/************ DATA LOADER AUTOGEN  ************/

    #include <MappedEnum.hpp>

    #define FTXTP_WRITE_ELEMENT_AUTOGEN(x) FTXTP_WRITE_ELEMENT(data, x)

    #define IF_MEMBER_FTXTP_LOAD_AUTOGEN(x) else IF_MEMBER_FTXTP_LOAD(data, x)

    #define AUTOGEN_DATA_WRITE_FUNC(T, ...) \
        DATA_WRITE_FUNC(T) \
            { \
                DATA_WRITE_INIT(T); \
                MAPGEN_FOR_EACH(FTXTP_WRITE_ELEMENT_AUTOGEN, __VA_ARGS__) \
                DATA_WRITE_END; \
            }

    #define AUTOGEN_DATA_WRITE_FUNC_AN(T, ...) \
        DATA_WRITE_FUNC(T) \
            { \
                DATA_WRITE_INIT_AN(T); \
                MAPGEN_FOR_EACH(FTXTP_WRITE_ELEMENT_AUTOGEN, __VA_ARGS__) \
                DATA_WRITE_END; \
            }

    #define AUTOGEN_DATA_READ_FUNC(T, ...) \
        DATA_READ_FUNC(T) \
            { \
                DATA_READ_INIT(T); \
                while (NEW_VALUE) \
                { \
                    const char *member = buff->read(); \
                    const char *value = nullptr; \
                    if(false); \
                    MAPGEN_FOR_EACH(IF_MEMBER_FTXTP_LOAD_AUTOGEN, __VA_ARGS__) \
                    else \
                        MEMBER_NOTRECOGNIZED_ERROR; \
                    if(value > buff->data + buff->getReadHead())\
                        buff->setHead(value);\
                } \
                DATA_READ_END; \
            }

    /*
        Generate Automaticlly the read and write functions for DataLoader<T>.
        All given members should be pure data and writtable and readable by a 
        FastTextParser template.
    */
    #define AUTOGEN_DATA_RW_FUNC(T, ...) \
        AUTOGEN_DATA_WRITE_FUNC(T, __VA_ARGS__) \
        AUTOGEN_DATA_READ_FUNC(T, __VA_ARGS__) 

    /*
        Generate Automaticlly the read and write functions for DataLoader<T>.
        All given members should be pure data and writtable and readable by a 
        FastTextParser template.

        _AN means anonymous, the type name will not be written
     */
    #define AUTOGEN_DATA_RW_FUNC_AN(T, ...) \
        AUTOGEN_DATA_WRITE_FUNC_AN(T, __VA_ARGS__) \
        AUTOGEN_DATA_READ_FUNC(T, __VA_ARGS__) 


    #define AUTOGEN_COMPONENT_RWFUNC(type) \
        COMPONENT_ADD_RW(type) \
        COMPONENT_DEFINE_READ(type) \
        {entity->set<type>(DataLoader<type>::read(buff));} \
        COMPONENT_DEFINE_WRITE(type) \
        {DataLoader<type>::write(entity->comp<type>(), out);}

