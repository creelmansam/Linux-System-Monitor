#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() { 
  long mem_total = 0, mem_free = 0, line_long;
  string key, line;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> line_long) {
        if (key == "MemTotal") {
          mem_total = line_long;
        }
        if (key == "MemFree") {
          mem_free = line_long;
        }
      }
      if(mem_total != 0 && mem_free != 0)
      {
        break;
      }
  }
  return static_cast<float>(mem_total - mem_free) / mem_total; 
}

long LinuxParser::UpTime() { 
  long uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    std::replace(line.begin(), line.end(), '.', ' ');
    linestream >> uptime;
  }
  return uptime; 
}

long LinuxParser::Jiffies() { 
  long jiffies = 0;
  string line, key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      if (linestream >> key) {
        if (key == "cpu") {
          long data;
          while(linestream >> data){
            jiffies += data;
          }
          return jiffies;
        }
      }
  }
  return {}; 
}

long LinuxParser::ActiveJiffies(int pid) {
    int pid1, ppid4, pgrp5, session6, tty_nr7, tpgid8;
  string comm2, line;
  char state3;
  uint flags9;
  long minflt10, cminflt11, majflt12, cmajflt13, utime, stime, cutime, cstime;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> pid1 >> comm2 >> state3 >> ppid4 >> pgrp5 >> session6 >> tty_nr7 >> 
      tpgid8 >> flags9 >> minflt10 >> cminflt11 >> majflt12 >> cmajflt13 >> utime >> 
      stime >> cutime >> cstime;
    return utime + stime + cutime + cstime; 
  }
  
  return 0;
}

long LinuxParser::ActiveJiffies() { 
  vector<long> cpus;
  string line, key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      if (linestream >> key) {
        if (key == "cpu") {
          long data;
          while(linestream >> data){
            cpus.emplace_back(data);
          }
          long active_jiffies = 0;
          for (int i = 0; i < cpus.size(); i++)
          {
            if(i != 3 && i != 4){
              active_jiffies += cpus[i];
            }
          }
          return active_jiffies;
        }
      }
  }
  return {}; 
}

long LinuxParser::IdleJiffies() { 
  vector<long> cpus;
  string line, key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      if (linestream >> key) {
        if (key == "cpu") {
          long data;
          while(linestream >> data){
            cpus.emplace_back(data);
          }
          return cpus[3] + cpus[4];
        }
      }
  }
  return {}; 
}

vector<long> LinuxParser::CpuUtilization() { 
  vector<long> cpu_utilization;
  cpu_utilization.emplace_back(Jiffies());
  cpu_utilization.emplace_back(ActiveJiffies());
  cpu_utilization.emplace_back(IdleJiffies());

  return cpu_utilization;
}

int LinuxParser::TotalProcesses() { 
  string key;
  int total;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      if (linestream >> key >> total) {
        if (key == "processes") {
          return total;
        }
      }
  }
  return 0; 
}

int LinuxParser::RunningProcesses() { 
  string key;
  int total;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      if (linestream >> key >> total) {
        if (key == "procs_running") {
          return total;
        }
      }
  }
  return 0; 
}

string LinuxParser::Command(int pid) { 
  string command;
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);

  if(std::getline(stream, line)){
    return line;
  }

  return string(); 
}

string LinuxParser::Ram(int pid) { 
  string key, byte_size;
  int ram_kb;
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      if (linestream >> key >> ram_kb >> byte_size) {
        if (key == "VmSize") {
          long ram_mb = ram_kb / 1024.0;
          return to_string(ram_mb);
        }
      }
  }
  
  return string(); 
}

string LinuxParser::Uid(int pid) { 
  string key;
  int uid;
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      if (linestream >> key >> uid) {
        if (key == "Uid") {
          return to_string(uid);
        }
      }
  }

  return string(); 
}

string LinuxParser::User(int pid) { 
  string name, x, uid;
  string process_uid = Uid(pid);
  string line;
  std::ifstream stream(kPasswordPath);
  while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      if (linestream >> name >> x >> uid) {
        if (process_uid == uid) {
          return name;
        }
      }
  }
  return string(); 
}

long LinuxParser::UpTime(int pid) { 
  int pid1, ppid4, pgrp5, session6, tty_nr7, tpgid8;
  string comm2, line;
  char state3;
  uint flags9;
  long minflt10, cminflt11, majflt12, cmajflt13, utime14, stime15, cutime16, cstime17, priority18,
        nice19, num_threads20, itrealvalue21;
  long long starttime;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> pid1 >> comm2 >> state3 >> ppid4 >> pgrp5 >> session6 >> tty_nr7 >> 
      tpgid8 >> flags9 >> minflt10 >> cminflt11 >> majflt12 >> cmajflt13 >> utime14 >> 
      stime15 >> cutime16 >> cstime17 >> priority18 >> nice19 >> num_threads20 >>
      itrealvalue21 >> starttime;
  }
  
  return UpTime() - starttime/sysconf(_SC_CLK_TCK); 
}