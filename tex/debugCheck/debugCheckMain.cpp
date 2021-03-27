#include "sfDebug.h"
#include <sstream>
#include <iomanip>

DebugLevel max_debug() { return DBG_Info; }

int main()
{
	int32_t count = 100;
    SFDebug errdbg(DBG_Error, max_debug, Color_Red, "ErrOut");    // Prints to file
    SFDebug wngdbg(DBG_Warning, max_debug, Color_Blue, "stderr"); // Prints to stderr
    SFDebug ifodbg(DBG_Info, max_debug, Color_Cyan);              // Prints to stdout
    SFDebug vbsdbg(DBG_Verbose, max_debug, Color_White);

    errdbg << "Hello World!!\n";
    errdbg << count++ << "\n";

    wngdbg << "Hello World!!\n";
    wngdbg << count++ << "\n";

    ifodbg << "Hello World!!\n";
    ifodbg << count++ << "\n";

    // The following will not be printed because debug level is only info
    vbsdbg << "Hello World!!\n";
    vbsdbg << count++ << "\n";

    errdbg << "Hello World!!\n";
    errdbg << count++ << "\n";

    // Use stringstream for manipulations and then debug print
    std::stringstream ss;
    ss << std::hex << count++;
    wngdbg << "In Hex: " << ss.str() << "\n";
}
