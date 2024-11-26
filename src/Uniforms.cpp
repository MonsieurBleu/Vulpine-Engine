#include <Uniforms.hpp>
#include <iostream>
#include <algorithm>
#include <Utils.hpp>

int getUniformTypeSize( UniformTypes type)
{
    switch (type)
    {
        case _2fv  :
        case _2iv  :
        case _2uiv :
        return 2;

        case _3fv  :
        case _3iv  :
        case _3uiv :
        return 3;

        case _4fv  :
        case _4iv  :
        case _4uiv :
        case Matrix2fv :
        return 4;
        
        case Matrix3fv : return 9;
        case Matrix4fv : return 16;

        case Matrix2x3fv :
        case Matrix3x2fv :
        return 6;

        case Matrix2x4fv :
        case Matrix4x2fv :
        return 8;

        case Matrix3x4fv :
        case Matrix4x3fv :
        return 12;

        default: break;
    }

    return 1;
}

void ShaderUniform::activate() const
{
    if(location == UNIFORM_NO_LOCATION) return;

    int count = _count;
    if(dataState != reference)
        count = 1;

    const void* udata = dataState == copiedInAdditionalData ? additionalData : data;

    switch (type)
    {
        case _1f  : glUniform1f(location,  *(float*)udata);     break;
        case _2fv : glUniform2fv(location, count, (float *) udata); break;
        case _3fv : glUniform3fv(location, count, (float *) udata); break;
        case _4fv : glUniform4fv(location, count, (float *) udata); break;
        
        case _1i  : glUniform1i(location, *(int*)udata); break;
        case _2iv : glUniform2iv(location, count, (int *) udata); break;
        case _3iv : glUniform3iv(location, count, (int *) udata); break;
        case _4iv : glUniform4iv(location, count, (int *) udata); break;

        case _1ui  : glUniform1i(location, *((uint32*)&udata)); break;
        case _2uiv : glUniform2uiv(location, count, (uint32 *) udata); break;
        case _3uiv : glUniform3uiv(location, count, (uint32 *) udata); break;
        case _4uiv : glUniform4uiv(location, count, (uint32 *) udata); break;

        case Matrix2fv : glUniformMatrix2fv(location, count, false, (float*)udata); break;
        case Matrix3fv : glUniformMatrix3fv(location, count, false, (float*)udata); break;
        case Matrix4fv : glUniformMatrix4fv(location, count, false, (float*)udata); break;

        default: break;
    }
}

ShaderUniform& ShaderUniform::setCount(int count)
{
    _count = count;
    return *this;
}

bool ShaderUniform::operator==(const ShaderUniform &a)
{
    if(
        location != a.location
        || dataState != a.dataState
        || type != a.type
        || _count != a._count
        )
        return false;

    if(dataState == copiedInAdditionalData)
    {
        if(
               additionalData[0] != a.additionalData[0]
            || additionalData[1] != a.additionalData[1]
            || additionalData[2] != a.additionalData[2]
            || additionalData[3] != a.additionalData[3]
            || additionalData[4] != a.additionalData[4]
            || additionalData[5] != a.additionalData[5]
            || additionalData[6] != a.additionalData[6]
            || additionalData[7] != a.additionalData[7]
        )
            return false;
    }
    // else
    // { 
    //     if(data != a.data)
    //         return false;
    // }

    return true;
}

ShaderUniformGroup::ShaderUniformGroup(std::vector<ShaderUniform> uniforms, bool autoCheckLocations) 
                                      : autoCheckLocations(autoCheckLocations),
                                      uniforms(uniforms)
{
    sort();
    if(autoCheckLocations)
        safe = checkLocations();
}

void ShaderUniformGroup::sort()
{
    std::sort(uniforms.begin(), 
              uniforms.end(), 
              [](ShaderUniform i, ShaderUniform j){return i.getLocation()< j.getLocation();});
}

bool ShaderUniformGroup::checkLocations()
{
    for(uint64 i = 0; i < uniforms.size(); i++)
    {
        if(uniforms[i].getLocation() == UNIFORM_NO_LOCATION)
        {
            std::cerr << TERMINAL_ERROR << "ShaderUniformGroup at " << this << "\n ERROR : ";
            std::cerr << "Uniform at position" << i << " has no location.\n";
            std::cerr <<  TERMINAL_RESET;
            std::cerr << "\t" << uniforms[i] << "\n";
            std::cerr <<  TERMINAL_ERROR;
            std::cerr << "=> This ShaderUniformGroup won't be able to update itself" << "\n";
            return false;
        }

        if(i > 0 && uniforms[i].getLocation() == uniforms[i-1].getLocation())
        {
            std::cerr << TERMINAL_ERROR << "ShaderUniformGroup at " << this << "\n ERROR : ";
            std::cerr << "Uniforms at position " << i << " and " << i-1 << " have the same locations.\n";
            std::cerr <<  TERMINAL_RESET;
            std::cerr << "\t" << uniforms[i] << "\n";
            std::cerr << "\t" << uniforms[i-1] << "\n";
            std::cerr <<  TERMINAL_ERROR;
            std::cerr << TERMINAL_INFO << "====> This ShaderUniformGroup won't be able to update itself" << "\n";
            return false;
        }
    }

    return true;
}

void ShaderUniformGroup::forEach(void (*func)(int, ShaderUniform&))
{
    for(uint64 i = 0; i < uniforms.size(); i++)
    {
        func(i, uniforms[i]);
    }
}

void ShaderUniformGroup::forEach(void (*func)(int, const ShaderUniform&)) const
{
    for(uint64 i = 0; i < uniforms.size(); i++)
    {
        func(i, uniforms[i]);
        // uniforms.data();
    }
}

void ShaderUniformGroup::update() const
{
    forEach([](int i, const ShaderUniform& uniform)
    {
        uniform.activate();
    });
}

void ShaderUniformGroup::add(ShaderUniform newUniform)
{
    uniforms.push_back(newUniform);
}

std::ostream& operator<<(std::ostream& os, ShaderUniform u)
{
    os << "ShaderUniform { ";
    
    os << "location : " << (u.location == UNIFORM_NO_LOCATION ? "N" : std::to_string(u.location));

    os << ", type : " << UniformTypesNames[u.type];

    os << ", dataState : " << UniformDataStateNames[u.dataState];

    os << ", data : " << u.data;

    if(u.dataState == reference)
        os << ", *data : " << std::hex << *(uint64 *)u.data;
    if(u.dataState == copiedInAdditionalData)
        os << ", *data : " << std::hex << u.additionalData[0];

    os << " }";

    return os;
}

std::ostream& operator<<(std::ostream& os, ShaderUniformGroup g)
{
    std::cout << TERMINAL_INFO;
    std::cout << "ShaderUniformGroup\n" << "{\n"; 

    os << "\tstate : " << (g.safe ? "safe" : TERMINAL_ERROR + "unsafe" + TERMINAL_INFO) << "\n";

    for(uint64 i = 0; i < g.uniforms.size(); i++)
    {
        os << "\t" << g.uniforms[i] << "\n"; 
    }

    std::cout << "}\n";
    std::cout << TERMINAL_RESET;

    return os;
}

std::vector<ShaderUniform>& operator+(std::vector<ShaderUniform>& a, std::vector<ShaderUniform>& b)
{
    for(auto i = b.begin(); i != b.end(); i++)
    {
        a.push_back(*i);
    }
    
    return a;
}

std::vector<ShaderUniform>& operator+(std::vector<ShaderUniform>& a, ShaderUniform& b)
{
    a.push_back(b);
    
    return a;
}

