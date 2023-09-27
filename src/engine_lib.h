#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
//#####################################################################################################################################
//                                                  Defines
//#####################################################################################################################################

#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak()
#define EXPORT_FN __declspec(dllexport)
#elif __linux__
#define DEBUGBREAK() __builtin_debugtrap()
#define EXPORT_FN
#elif __APPLE__
#define DEBUGBREAK() __builtin_trap()
#define EXPORT_FN 
#endif

#define b8 char
#define BIT(x) 1<<(x)
#define KB(x) ((unsigned long long)1024 * x)
#define MB(x) ((unsigned long long)1024 * KB(x))
#define GB(x) ((unsigned long long)1024 * MB(x))
//#####################################################################################################################################
//                                                  Logging
//#####################################################################################################################################

enum TextColor{
    TEXT_COLOR_BLACK, TEXT_COLOR_RED, TEXT_COLOR_GREEN, TEXT_COLOR_YELLOW, TEXT_COLOR_BLUE, TEXT_COLOR_MAGENTA, TEXT_COLOR_CYAN, 
    TEXT_COLOR_WHITE, TEXT_COLOR_BRIGHT_BLACK, TEXT_COLOR_BRIGHT_RED, TEXT_COLOR_BRIGHT_GREEN, TEXT_COLOR_BRIGHT_YELLOW, 
    TEXT_COLOR_BRIGHT_BLUE, TEXT_COLOR_BRIGHT_MAGENTA, TEXT_COLOR_BRIGHT_CYAN, TEXT_COLOR_BRIGHT_WHITE, TEXT_COLOR_COUNT
};

template <typename ...Args>

void _log(char* prefix, char* msg, TextColor textColor, Args ...args){
    static char* TextColorTable[TextColor::TEXT_COLOR_COUNT] = {
        "\x1b[30m", "\x1b[31m", "\x1b[32m", "\x1b[33m", "\x1b[34m", "\x1b[35m", "\x1b[36m", "\x1b[37m",
        "\x1b[90m", "\x1b[91m", "\x1b[92m", "\x1b[93m", "\x1b[94m", "\x1b[95m", "\x1b[96m", "\x1b[97m",
    };
    char formatBuffer[8192] = {};
    sprintf(formatBuffer, "%s %s %s \033[0m", TextColorTable[textColor], prefix, msg);
    char textBuffer[8192] = {};
    sprintf(textBuffer, formatBuffer, args...);
    puts(textBuffer);
}

#define SM_INFO(msg, ...) _log("INFO: ", msg, TEXT_COLOR_BRIGHT_BLACK, ##__VA_ARGS__);
#define SM_TRACE(msg, ...) _log("TRACE: ", msg, TEXT_COLOR_WHITE, ##__VA_ARGS__);
#define SM_OK(msg, ...) _log("OK: ", msg, TEXT_COLOR_GREEN, ##__VA_ARGS__);
#define SM_WARN(msg, ...) _log("WARNING: ", msg, TEXT_COLOR_YELLOW, ##__VA_ARGS__);
#define SM_ERROR(msg, ...) _log("ERROR: ", msg, TEXT_COLOR_RED, ##__VA_ARGS__);


#define SM_ASSERT(x, msg, ...)      \
    if(!(x)) {                        \
        SM_ERROR(msg, ##__VA_ARGS__); \
        DEBUG_BREAK();                \
        SM_ERROR("Assertion HIT!");    \
    }                                 


#define SM_ASSERT_GUARD(x, ret, msg, ...)  \
    if (!(x)){                           \
        SM_ASSERT(false, msg, ##__VA_ARGS__);        \
        return ret;                   \
    }                                 
//#####################################################################################################################################
//                                                  Array
//#####################################################################################################################################
template<typename T, int N>
struct Array{
    static constexpr int maxElements = N;
    int count = 0;
    T elements[N];

    T& operator[](int idx){
        SM_ASSERT(idx >= 0, "index is negative!");
        SM_ASSERT(idx < count, "index is out of bounds");
        return elements[idx];
    }
    int add(T element){
        SM_ASSERT(count < maxElements, "Array is full");
        elements[count] = element;
        return count++;
    }
    void remove_idx_and_swap(int idx){
        SM_ASSERT(idx >=0, "index is negative");
        SM_ASSERT(idx < count, "index is out of bounds");
        element[idx] = element[--count];
    }
    void clear(){ count = 0; }
    bool is_full(){ return count ==N; }
};
//#####################################################################################################################################
//                                                  Bump Allocator
//#####################################################################################################################################
struct BumpAllocator{
    size_t capacity;
    size_t used;
    char* memory;
};

BumpAllocator make_bump_allocator(size_t size){
    BumpAllocator ba = {};
    ba.memory = (char*)malloc(size);
    SM_ASSERT(ba.memory, "Failed to allocate Memory!");
    ba.capacity = size;
    memset(ba.memory, 0, size);
    return ba;
}

char* bump_alloc(BumpAllocator* bumpAllocator, size_t size){
    char* result = nullptr;

    size_t allignedSize = (size + 7) & ~ 7;
    SM_ASSERT(bumpAllocator->used + allignedSize <= bumpAllocator->capacity, "Bump Allocator is full");
    result = bumpAllocator->memory + bumpAllocator->used;
    bumpAllocator->used += allignedSize;
    return result;
}

//#####################################################################################################################################
//                                                  File I/O
//#####################################################################################################################################

long long get_timestamp(char* file){
    struct stat file_stat = {};
    stat(file, &file_stat);
    return file_stat.st_mtime;
}

bool file_exists(char* filepath){
    SM_ASSERT(filepath, "No file Path supplied!");
    auto file = fopen(filepath, "rb");
    if(!file)return false;
    fclose(file);
    return true;
}

long get_file_size(char* filepath){
    SM_ASSERT(filepath, "No file Path supplied!");
    long fileSize = 0;
    auto file = fopen(filepath, "rb");
    if(!file){
        SM_ERROR("Failed opening File: %s", filepath);
        return 0;
    }
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    fclose(file);
    return fileSize;
}

char* read_file(char* filepath, int* fileSize, char* buffer){
    SM_ASSERT(filepath, "No file Path supplied!");
    SM_ASSERT(fileSize, "No file Size supplied!");
    SM_ASSERT(buffer, "No buffer supplied!");
    *fileSize = 0;
    auto file = fopen(filepath, "rb");
    if(!file){
        SM_ERROR("Failed opening File: %s", filepath);
        return nullptr;
    }
    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    memset(buffer, 0, *fileSize+1);
    fread(buffer, sizeof(char), *fileSize, file);
    fclose(file);
    return buffer;
}

char* read_file(char* filepath, int* fileSize, BumpAllocator* bumpAllocator){
    char* file = nullptr;
    long _fileSize = get_file_size(filepath);
    if (!_fileSize) return file;
    char* buffer = bump_alloc(bumpAllocator, _fileSize + 1);
    file = read_file(filepath, fileSize, buffer);
    return file;
}

void write_file(char* filepath, char* buffer, int size){
    SM_ASSERT(filepath, "No file Path supplied!");
    SM_ASSERT(buffer, "No buffer supplied!");
    auto file = fopen(filepath, "wb");
    if(!file){
        SM_ERROR("Failed opening File: %s", filepath);
        return;
    }
    fwrite(buffer, sizeof(char), size, file);
    fclose(file);
}

bool copy_file(char* filename, char* outputName, char* buffer){
    int fileSize = 0;
    char* data = read_file(filename, &fileSize, buffer);
    auto outputFile = fopen(outputName, "wb");
    if(!outputFile){
        SM_ERROR("Failed opening File: %s", outputName);
        return false;
    }

    int result = fwrite(data, sizeof(char), fileSize, outputFile);
    if(!result){
        SM_ERROR("Failed opening File: %s", outputName);
        return false;
    }
    fclose(outputFile);
    return true;
}

bool copy_file(char* filename, char* outputName, BumpAllocator* bumpAllocator){
    char* file = 0;
    long _fileSize = get_file_size(filename);
    if(!_fileSize) return false;
    char* buffer = bump_alloc(bumpAllocator, _fileSize+1);
    return copy_file(filename, outputName, buffer);
}


//#####################################################################################################################################
//                                                  Math stuff
//#####################################################################################################################################

struct Vec2{ 
    float x, y; 
    Vec2 operator/(float scalar){ return {x / scalar, y / scalar};}
    Vec2 operator-(Vec2 other){return {x - other.x, y - other.y};}
};

struct IVec2{ 
    int x, y;
    IVec2 operator-(IVec2 other){return {x - other.x, y - other.y};}
};
Vec2 vec_2(IVec2 v) {return Vec2{(float)v.x, (float)v.y};}

struct Vec3{
    union{
        float values[3];
        struct{ float x, y, z; };
        struct{ float r, g, b; };
    };
    float& operator[] (int idx){ return values[idx];}
};

struct Vec4{
    union{
        float values[4];
        struct{ float x, y, z, w; };
        struct{ float r, g, b, a; };
    };
    float& operator[] (int idx){ return values[idx];}
};

struct Mat4{
    union{
        Vec4 values[4];
        struct{
            float ax, bx, cx, dx;
            float ay, by, cy, dy;
            float az, bz, cz, dz;
            float aw, bw, cw, dw;
        };
    };
    Vec4& operator[] (int idx){ return values[idx];}
};

Mat4 orthographic_projection(float left, float right, float top, float bottom){
    Mat4 result = {};
    result.aw = -(right + left) / (right - left);
    result.bw = (top + bottom) / (top - bottom);
    result.cw = 0.0f;
    result[0][0] = 2.0f / (right - left);
    result[1][1] = 2.0f / (top - bottom);
    result[2][2] = 1.0f / (1.0f - 0.0f);
    result[3][3] = 1.0f;
    return result;
}