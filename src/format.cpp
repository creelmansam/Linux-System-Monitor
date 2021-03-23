#include <string>
#include <cmath>

#include "format.h"

using std::string;
using std::to_string;

string Format::ElapsedTime(long seconds) { 

    long hours = std::floor(seconds/3600.0);
    long minutes = (seconds / 60) % 60;
    long mod_seconds = seconds % 60;
    
    return (hours < 10 ? "0" : "") + to_string(hours) + ":" + (minutes < 10 ? "0" : "") + 
                to_string(minutes) + ":" + (mod_seconds < 10 ? "0" : "") + 
                to_string(mod_seconds); 
}