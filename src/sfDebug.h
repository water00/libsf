#pragma once

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <cerrno>
#include <memory>

enum class DebugLevel
{
    DBG_None,
    DBG_Error,
    DBG_Warning,
    DBG_Info,
    DBG_Verbose
};

// Foreground Colors
enum class SFColors 
{
    Color_Default = 39,
    Color_Black = 30,
    Color_Red = 31,
    Color_Green = 32,
    Color_Yellow = 33,
    Color_Blue = 34,
    Color_Magenta = 35,
    Color_Cyan = 36,
    Color_LightGray = 37,
    Color_DarkGray = 90,
    Color_LightRed = 91,
    Color_LightGreen = 92,
    Color_LightYellow = 93,
    Color_LightBlue = 94,
    Color_LightMagenta = 95,
    Color_LightCyan = 96,
    Color_White = 97
};

#define DBG_PRINT { if (dbgLevel <= maxDbg && *ofs) { *ofs << fgColor << n; } return *this; }
#define DBG_PRINT_FLUSH { if (dbgLevel <= maxDbg && *ofs) { *ofs << fgColor << n; } if (*n == '\n') (*ofs).flush(); return *this; }

class SFDebug
{
private:
    DebugLevel dbgLevel;
    std::ostream* ofs;
    std::unique_ptr<std::ofstream> ownedOfs;
    std::string fgColor;
    DebugLevel maxDbg;

public:
    SFDebug(DebugLevel dbgLvl, DebugLevel (max_dbg)(), [[maybe_unused]]SFColors clr = SFColors::Color_Default, std::string fName = "stdout")
    {
        dbgLevel = dbgLvl;
        maxDbg = max_dbg();
        if (fName == "stdout")
        {
            ofs = &std::cout;
        }
        else if (fName == "stderr")
        {
            ofs = &std::cerr;
        }
        else
        {
            ownedOfs = std::make_unique<std::ofstream>(fName);
            if (!ownedOfs->is_open())
            {
                std::cout << "Opening of Debug File failed" << std::endl;
                ofs = &std::cout;
                ownedOfs.reset();
            }
            else
            {
                ofs = ownedOfs.get();
            }
        }
        #ifndef _WIN32
            std::stringstream ss;
            ss << "\033[" << static_cast<int32_t>(clr) << "m";
            fgColor = ss.str();
        #endif
    }

    ~SFDebug() = default;

    // For internal debug printing
    static void SF_print(const std::string& str)
    {
        std::cout << str << std::endl;
    }

    SFDebug& operator<<(const void* n)        DBG_PRINT
    SFDebug& operator<<(std::string& n)       DBG_PRINT
    SFDebug& operator<<(const std::string& n) DBG_PRINT
    SFDebug& operator<<(const char* n)        DBG_PRINT_FLUSH
    SFDebug& operator<<(char* n)              DBG_PRINT
    SFDebug& operator<<(int8_t n)             DBG_PRINT
    SFDebug& operator<<(uint8_t n)            DBG_PRINT
    SFDebug& operator<<(int16_t n)            DBG_PRINT
    SFDebug& operator<<(uint16_t n)           DBG_PRINT
    SFDebug& operator<<(int32_t n)            DBG_PRINT
    SFDebug& operator<<(uint32_t n)           DBG_PRINT
    SFDebug& operator<<(int64_t n)            DBG_PRINT
    SFDebug& operator<<(uint64_t n)           DBG_PRINT
    SFDebug& operator<<(float n)              DBG_PRINT
    SFDebug& operator<<(double n)             DBG_PRINT
    SFDebug& operator<<(bool n)               DBG_PRINT
};

