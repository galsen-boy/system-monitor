
#include "imgui/lib/backend/imgui_impl_opengl3.h"
#include "imgui/lib/backend/imgui_impl_sdl.h"
#include "imgui/lib/gl3w/GL/gl3w.h"
#include "imgui/lib/imgui.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <algorithm>
#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>
#include <numeric>
#include <sensors/sensors.h>
#include </usr/include/sensors/sensors.h>
#include <chrono>
#include <cmath>
#include <cpuid.h>
#include <cstdio>
#include <ctime>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits.h>
#include <regex>
#include <sstream>
#include <stdio.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>
#define BUF_MAX 1024
#define MAX_CPU 124
#include <arpa/inet.h>
#include <cstring>
#include <ifaddrs.h>
#include <iterator> // for std::begin and std::end
#include <map>
#include <netinet/in.h>
#include <regex>
#include <set>
#include <string>
#include <tuple>


using namespace std;

struct CPUStats {
  long long int user;
  long long int nice;
  long long int system;
  long long int idle;
  long long int iowait;
  long long int irq;
  long long int softirq;
  long long int steal;
  long long int guest;
  long long int guestNice;
};

// processes `stat`
struct Proc {
  int pid;
  string name;
  char state;
  long long int vsize;
  long long int rss;
  long long int utime;
  long long int stime;
};

struct IP4 {
  char *name;
  char addressBuffer[INET_ADDRSTRLEN];
};

struct Networks {
  vector<IP4> ip4s;
};

struct TX {
  int bytes;
  int packets;
  int errs;
  int drop;
  int fifo;
  int frame;
  int compressed;
  int multicast;
};

struct ProcessInfo {
  int pid;
  std::string name;
  std::string state;
  float cpuUsage;
  float memoryUsage;
  bool selected;

  ProcessInfo(int _pid, const std::string &_name, const std::string &_state,
              float _cpuUsage, float _memoryUsage, bool _selected)
      : pid(_pid), name(_name), state(_state), cpuUsage(_cpuUsage),
        memoryUsage(_memoryUsage), selected(_selected) {}
  ProcessInfo() : pid(0), cpuUsage(0.0f), memoryUsage(0.0f), selected(false) {}

};

struct ProcessSelection {
    int pid;
    bool selected;

    ProcessSelection(int _pid, bool _selected) : pid(_pid), selected(_selected) {}
};

struct RX {
  int bytes;
  int packets;
  int errs;
  int drop;
  int fifo;
  int colls;
  int carrier;
  int compressed;
};

// student TODO : system stats
string CPUinfo();
const char *getOsName();
int getNumberOfCores();
long long getCPUtimeForCore(int core);
float getCPUUsageForCore(int core);
int systemWindow(const char *id, ImVec2 size, ImVec2 position);
void fetchProcessData(std::vector<ProcessInfo> &processList,
                      const std::string &filterText);
void memoryProcessesWindow(const char *id, ImVec2 size, ImVec2 position,
                           std::vector<ProcessInfo> &processList,
                           std::string &filterText);
void networkWindow(const char *id, ImVec2 size, ImVec2 position);
int read_fields(FILE *fp, unsigned long long int *fields);
int cpudata(void);
std::vector<double> readCoreUsage();
float getSwapUsage();
float GetCpuUsage();
void UpdateCpuUsage();
void RenderCpuUsage();
long long getSwapTotal();
void printProcessList(const std::vector<ProcessInfo> &processList);
void UpdateFanSpeedData(float fanSpeed);
std::string formatBytes(double bytes);
std::string formatBytes(unsigned long long bytes);
