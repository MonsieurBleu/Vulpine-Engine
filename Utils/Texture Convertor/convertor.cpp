#include <iostream>
#include <random>

// #define STBIR_DEFAULT_FILTER_UPSAMPLE   STBIR_FILTER_
// #define STBIR_DEFAULT_FILTER_DOWNSAMPLE STBIR_FILTER_CATMULLROM

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>
#include <stb/stb_image_write.h>

#include <glm/glm.hpp>

const std::string TERMINAL_ERROR    = "\e[1;31m"; //"\033[91m";
const std::string TERMINAL_INFO     = "\033[94m";
const std::string TERMINAL_OK       = "\033[92m";
const std::string TERMINAL_RESET    = "\033[0m";
const std::string TERMINAL_TIMER    = "\033[93m";
const std::string TERMINAL_FILENAME = "\033[95m";
const std::string TERMINAL_WARNING  = "\e[38;5;208m";
const std::string TERMINAL_NOTIF    = "\e[1;36m";

using namespace glm;

void badFilenameExit(const char *filename)
{
    std::cout 
    << TERMINAL_ERROR << "\n Error reading file "
    << TERMINAL_FILENAME << filename
    << TERMINAL_ERROR << ". This file doesn't exist or isn't supported.\n"
    << TERMINAL_RESET;
    exit(1);
}

/*
    Customisable parameters 
*/
int outRes = 1024; 
float nDitherFactor = 1.0;
bool ditherNormal = true;

struct pixelRGB
{
    uint8 r, g, b;
};

struct pixelRGBA_t
{
    uint8 r, g, b, a;
};

union pixelRGBA
{
    uint8 c[4];
};

struct OutImageInfo
{
    pixelRGBA* data;
    int w, h, n;
};

#define NORMAL_IN_COMP   3
#define NORMAL_OUT_COMP  4

#define COLOR_IN_COMP   3
#define COLOR_OUT_COMP   4

OutImageInfo ProcessColorMap(const char *filename)
{
    if(stbi_is_hdr(filename))
    {
        int w, h, n;
        float *initialData = stbi_loadf(filename, &w, &h, &n, COLOR_IN_COMP);

        if(!initialData)
            badFilenameExit(filename);

        int ow = w * outRes/h;
        int oh = h * outRes/h;
        int pixelCount = ow*oh;

        vec3 *resizedData = new vec3[pixelCount];
        stbir_resize_float(initialData, w, h, 0, (float*)resizedData, ow, oh, 0, COLOR_IN_COMP);
        delete initialData;

        pixelRGBA *outData = new pixelRGBA[pixelCount];

        //vec3(1.0) - exp(-_fragColor.rgb * exposure)
        //pow(mapped, vec3(1.0 / gamma));

        float exposure = 0.5;
        float gamma = 1.0;
        float igamma = 1.0/gamma;

        for(int i = 0; i < pixelCount; i++)
        {
            uint8 x = 255*max(0.0, min(1.0, pow(1.-exp(-resizedData[i].x)*exposure, igamma)));
            uint8 y = 255*max(0.0, min(1.0, pow(1.-exp(-resizedData[i].y)*exposure, igamma)));
            uint8 z = 255*max(0.0, min(1.0, pow(1.-exp(-resizedData[i].z)*exposure, igamma)));
            outData[i].c[0] = x;
            outData[i].c[1] = y;
            outData[i].c[2] = z;
            outData[i].c[3] = 255;
        }

        // for(int i = 0; i < pixelCount; i++)
        // {
        //     uint8 x = pow(resizedData[i].x, 0.5)*255;
        //     uint8 y = pow(resizedData[i].y, 0.5)*255;
        //     uint8 z = pow(resizedData[i].z, 0.5)*255;
        //     outData[i].r = x;
        //     outData[i].g = y;
        //     outData[i].b = z;
        //     outData[i].a = 255;
        // }

        delete resizedData;

        return (OutImageInfo){outData, ow, oh, COLOR_OUT_COMP};
    }
    else
    {
        int w, h, n;
        unsigned char *initialData = stbi_load(filename, &w, &h, &n, COLOR_OUT_COMP);

        if(!initialData)
            badFilenameExit(filename);

        int ow = w * outRes/h;
        int oh = h * outRes/h;
        int pixelCount = ow*oh;

        pixelRGBA *resizedData = new pixelRGBA[pixelCount];
        stbir_resize_uint8(initialData, w, h, 0, (uint8*)resizedData, ow, oh, 0, COLOR_OUT_COMP);
        delete initialData;

        return (OutImageInfo){resizedData, ow, oh, COLOR_OUT_COMP};
    }

    return (OutImageInfo){NULL, 0, 0, 0};
}


float nDitherAmplitude = 0.0025;
/*
    TODO : the dither seems stronger on non hdr images, investigate it
*/
OutImageInfo ProcessNormalMap(const char *filename)
{
    if(stbi_is_hdr(filename))
    {
        int w, h, n;
        float *initialData = stbi_loadf(filename, &w, &h, &n, 0);

        if(!initialData)
            badFilenameExit(filename);

        int ow = w * outRes/h;
        int oh = h * outRes/h;
        int pixelCount = ow*oh;

        vec3 *resizedData = new vec3[pixelCount];
        stbir_resize_float(initialData, w, h, 0, (float*)resizedData, ow, oh, 0, NORMAL_IN_COMP);
        delete initialData;

        if(ditherNormal)
        {
            for(int i = 0; i < pixelCount; i++)
            {
                int r1 = std::rand();
                int r2 = std::rand();
                int r3 = std::rand();
                float x = 1.0 + nDitherAmplitude*((r1%256)-1.0);
                float y = 1.0 + nDitherAmplitude*((r2%256)-1.0);
                float z = 1.0 + nDitherAmplitude*((r3%256)-1.0);

                vec3 p = resizedData[i] * vec3(2.0) - vec3(1.0);

                resizedData[i] = vec3(0.5) + vec3(0.5)*normalize(p*vec3(x, y, z));
            }
        }

        pixelRGBA *outData = new pixelRGBA[pixelCount];

        for(int i = 0; i < pixelCount; i++)
        {
            uint8 x = pow(resizedData[i].x, 0.5)*255;
            uint8 y = pow(resizedData[i].y, 0.5)*255;
            // uint8 z = pow(resizedData[i].z, 0.5)*255;
            outData[i].c[0] = x;
            outData[i].c[1] = y;
            outData[i].c[2] = 0.0;
            outData[i].c[3]= 255;
        }

        delete resizedData;

        return (OutImageInfo){outData, ow, oh, NORMAL_OUT_COMP};
    }
    else
    {
        int w, h, n;
        unsigned char *initialData = stbi_load(filename, &w, &h, &n, NORMAL_OUT_COMP);

        if(!initialData)
            badFilenameExit(filename);

        int ow = w * outRes/h;
        int oh = h * outRes/h;
        int pixelCount = ow*oh;

        pixelRGBA *resizedData = new pixelRGBA[pixelCount];
        stbir_resize_uint8(initialData, w, h, 0, (uint8*)resizedData, ow, oh, 0, NORMAL_OUT_COMP);
        delete initialData;

        if(ditherNormal)
        {
            for(int i = 0; i < pixelCount; i++)
            {
                int r1 = std::rand();
                int r2 = std::rand();
                int r3 = std::rand();
                float x = 1.0 + nDitherAmplitude*((r1%256)-1.0);
                float y = 1.0 + nDitherAmplitude*((r2%256)-1.0);
                float z = 1.0 + nDitherAmplitude*((r3%256)-1.0);
                vec3 p = vec3(resizedData[i].c[0], resizedData[i].c[1], resizedData[i].c[2]) * vec3(1.0/128.0) - vec3(1.0);

                p = vec3(0.5) + vec3(0.5)*normalize(p*vec3(x, y, z));

                resizedData[i].c[0] = round(p.x*255.0);
                resizedData[i].c[1] = round(p.y*255.0);
                resizedData[i].c[2] = 0;
                resizedData[i].c[3] = 255;
            }
        }
        else
        {
            for(int i = 0; i < pixelCount; i++)
            {
                resizedData[i].c[0] = resizedData[i].c[0];
                resizedData[i].c[1] = resizedData[i].c[1];
                resizedData[i].c[2] = 255;
                resizedData[i].c[3] = 255;
            }
        }

        return (OutImageInfo){resizedData, ow, oh, NORMAL_OUT_COMP};
    }

    return (OutImageInfo){NULL, 0, 0, 0};
}

/*
    TODO : add src channel support to HDR 
*/
OutImageInfo ProcessSingleChannelMap(OutImageInfo NRM, const char *filename, int channel = 0, int srcChannel = 0)
{
    if(stbi_is_hdr(filename))
    {
        int w, h, n;
        float *initialData = stbi_loadf(filename, &w, &h, &n, 1);

        if(!initialData)
            badFilenameExit(filename);

        int pixelCount = NRM.w*NRM.h;
        float *resizedData = new float[pixelCount];
        stbir_resize_float(initialData, w, h, 0, (float*)resizedData, NRM.w, NRM.h, 0, 1);
        delete initialData;

        switch (channel)
        {
        case 0 :
        case 1 :
        case 2 :
            for(int i = 0; i < pixelCount; i++)
                NRM.data[i].c[channel] = pow(resizedData[i], 0.5)*255;
            break;

        case 3 :
            for(int i = 0; i < pixelCount; i++)
                NRM.data[i].c[channel]  = 255 - pow(resizedData[i], 0.5)*255;
            break;

        default:
            break;
        }

        delete resizedData;
    }
    else
    {
        int w, h, n;
        unsigned char *initialData = stbi_load(filename, &w, &h, &n, 4);

        if(!initialData)
            badFilenameExit(filename);

        int pixelCount = NRM.w*NRM.h;
        pixelRGBA *resizedData = new pixelRGBA[pixelCount];
        stbir_resize_uint8(initialData, w, h, 0, (uint8*)resizedData, NRM.w, NRM.h, 0, 4);
        delete initialData;

        switch (channel)
        {
        case 0 :
        case 1 :
        case 2 :
            for(int i = 0; i < pixelCount; i++)
                NRM.data[i].c[channel] = resizedData[i].c[srcChannel];
            break;

        case 3 :
            for(int i = 0; i < pixelCount; i++)
                NRM.data[i].c[channel]  = 255 - resizedData[i].c[srcChannel];
            break;

        default:
            break;
        }

        delete resizedData;
    }

    return NRM;
}



int main(int argc, char *argv[])
{
    char *color = NULL;
    char *emmissive = NULL;
    char *normal = NULL;
    char *roughness = NULL;;
    char *metallic = NULL;
    char *orm = NULL;

    for(int i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "-res"))
        {
            i++;
            outRes = atoi(argv[i]);
            if(outRes <= 0)
            {
                std::cout << TERMINAL_ERROR << "Error : bad out resolution given\n" << TERMINAL_RESET;
                exit(1);
            }
        }

        if(!strcmp(argv[i], "-Ndither"))
        {
            i++;
            nDitherFactor = atof(argv[i]);
            if(nDitherFactor <= 0)
            {
                std::cout << TERMINAL_ERROR << "Error : bad dither factor given\n" << TERMINAL_RESET;
                exit(1);
            }
        }

        if(!strcmp(argv[i], "-noNdither")) ditherNormal = false;
        if(!strcmp(argv[i], "-c")) color = argv[++i];
        if(!strcmp(argv[i], "-e")) emmissive = argv[++i];
        if(!strcmp(argv[i], "-n")) normal = argv[++i];
        if(!strcmp(argv[i], "-r")) roughness = argv[++i];
        if(!strcmp(argv[i], "-m")) metallic = argv[++i];
        if(!strcmp(argv[i], "-orm")) orm = argv[++i];
    }


    nDitherAmplitude *= nDitherFactor;
    OutImageInfo NRM;

    if(normal)
        NRM = ProcessNormalMap(normal);
    else
    {
        NRM.data = new pixelRGBA[outRes*outRes];
        NRM.n = 4;
        NRM.w = outRes;
        NRM.h = outRes;

        // for(int i = 0; i < outRes*outRes)
    }

    if(roughness)
        ProcessSingleChannelMap(NRM, roughness, 2);

    if(metallic)
        ProcessSingleChannelMap(NRM, metallic, 3);

    if(orm)
    {
        ProcessSingleChannelMap(NRM, orm, 2, 1);
        ProcessSingleChannelMap(NRM, orm, 3, 2);
    }

    OutImageInfo CE;

    if(color)
        CE = ProcessColorMap(color);
    else
    {
        CE.data = new pixelRGBA[outRes*outRes];
        CE.n = 4;
        CE.w = outRes;
        CE.h = outRes;
    }

    if(emmissive)
        ProcessSingleChannelMap(CE, emmissive, 3);

    stbi_write_png("out/NRM.png", NRM.w, NRM.h, NRM.n, NRM.data, 0);
    stbi_write_png("out/CE.png", CE.w, CE.h, CE.n, CE.data, 0);
    delete NRM.data;
}