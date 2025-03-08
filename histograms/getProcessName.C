#include <string>
#include <TColor.h>

std::string getProcessName(int color) {
    switch (color) {
        case kYellow: return "drell_yan";
        case kMagenta: return "single_top";
        case kOrange: return "w_jets";
        case kCyan: return "ttbar";
        default: return "unknown";
    }
}

