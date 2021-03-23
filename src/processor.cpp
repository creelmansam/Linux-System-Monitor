#include <vector>
#include <string>

#include "linux_parser.h"
#include "processor.h"

using std::vector;
using std::string;

float Processor::Utilization() { 
  vector<long> cpu_utilization = LinuxParser::CpuUtilization();

  return static_cast<float>(cpu_utilization[0] - cpu_utilization[2]) / cpu_utilization[0]; 
}