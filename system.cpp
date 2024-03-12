#include "header.h"

using namespace std;

#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <cpuid.h>
#endif
// get cpu id and information, you can use `proc/cpuinfo`
string CPUinfo()
{
    char CPUBrandString[0x40];
    unsigned int CPUInfo[4] = {0, 0, 0, 0};

    __cpuid(0x80000000, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
    unsigned int nExIds = CPUInfo[0];

    memset(CPUBrandString, 0, sizeof(CPUBrandString));

    for (unsigned int i = 0x80000000; i <= nExIds; ++i)
    {
        __cpuid_count(i, 0, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);

        if (i == 0x80000002)
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000003)
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000004)
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
    }
    string str(CPUBrandString);
    return str;
}

// Function to get OS name
const char *getOsName()
{
#ifdef _WIN32
    return "Windows 32-bit";
#elif _WIN64
    return "Windows 64-bit";
#elif __APPLE__ || __MACH__
    return "Mac OSX";
#elif __linux__
    return "Linux";
#elif __FreeBSD__
    return "FreeBSD";
#elif __unix || __unix__
    return "Unix";
#else
    return "Other";
#endif
}


int getNumberOfCores() {
    long numCores = sysconf(_SC_NPROCESSORS_ONLN);
    return static_cast<int>(numCores);
}

long long getCPUtimeForCore(int core) {
    std::ifstream statFile("/proc/stat");
    std::string line;

    // Search for the line related to the requested core
    while (std::getline(statFile, line)) {
        if (line.find("cpu" + std::to_string(core)) == 0) {
            std::istringstream iss(line);
            std::string cpuLabel;
            iss >> cpuLabel;

            if (cpuLabel == "cpu" + std::to_string(core)) {
                long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
                iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
                return user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;
            }
        }
    }

    return 0;
}

// Function to get CPU usage percentage for a specific core
float getCPUUsageForCore(int core) {
    long long startTime = getCPUtimeForCore(core);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep for a short interval
    long long endTime = getCPUtimeForCore(core);

    long long deltaTime = endTime - startTime;
    long long totalCPUTime = deltaTime * getNumberOfCores(); // Total CPU time for all cores
    float usagePercentage = static_cast<float>(deltaTime) / totalCPUTime * 100.0f;

    return usagePercentage;
}
