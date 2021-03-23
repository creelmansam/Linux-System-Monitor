#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

int Process::Pid() { return pid_; }

float Process::CpuUtilization() const {

  long process_uptime = LinuxParser::UpTime(pid_);

  long process_jiffies = LinuxParser::ActiveJiffies(pid_);

  return (process_jiffies / sysconf(_SC_CLK_TCK)) / static_cast<float>(process_uptime);
}

string Process::Command() { return LinuxParser::Command(Pid()); }

string Process::Ram() { return LinuxParser::Ram(Pid()); }

string Process::User() { return LinuxParser::User(Pid()); }

long int Process::UpTime() { return LinuxParser::UpTime(Pid()); }

bool Process::operator<(Process const& a) const { 
  return a.CpuUtilization() < this->CpuUtilization();
}