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

#define PRINT_LOADER_DEBUG_CREATE std::cout \
    << TERMINAL_INFO << "Creating object " \
    << TERMINAL_NOTIF << name \
    << TERMINAL_INFO << " from "\
    << TERMINAL_UNDERLINE << type_name<decltype(r)>() \
    << TERMINAL_RESET << "\n" ;

#define EARLY_RETURN_IF_LOADED \
    auto check = loadedAssets.find(name); \
    if(check != loadedAssets.end()){\
        if(buff->data + buff->getReadHead() < end) buff->setHead(end); /*if the current buffer is ahead of the end of the info, this means that the dependant info is currently reading it as a reference*/ \
        return check->second;} \
    auto &r = loadedAssets[name]; \
    PRINT_LOADER_DEBUG_CREATE \
    buff->setHead(values); 

#define EXIT_ROUTINE_AND_RETURN end = buff->data + buff->getReadHead(); return r;

#define NEW_VALUE *buff->read() == ':'
#define END_VALUE *buff->read() == ';'

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

