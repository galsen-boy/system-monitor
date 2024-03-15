#include "header.h"
#include <sys/sysinfo.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/time.h>
#include <thread>
#include <chrono>

//#include <unistd.h>

double get_total_ram_memory() {
    struct sysinfo memInfo;
    if (sysinfo(&memInfo) != 0) {
        // Error handling
        return -1.0;
    }

    double totalRam = memInfo.totalram * memInfo.mem_unit / (1024.0 * 1024.0 * 1024.0);
    return totalRam;
}

double getPhysicalMemoryUsedInGB() {
     std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        while (std::getline(meminfo, line)) {
            if (line.find("MemAvailable:") != std::string::npos) {
                unsigned long long availableMemoryKB;
                if (sscanf(line.c_str(), "MemAvailable: %llu", &availableMemoryKB) == 1) {
                    double availableMemoryGB = static_cast<double>(availableMemoryKB) / (1024 * 1024);
                    return availableMemoryGB;
                }
            }
        }
        meminfo.close();
    }

    // If we can't read the value, return a default or error value
    return -1.0;
}


double getSwapSpaceInGB() {
#ifdef _WIN32
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);

    if (GlobalMemoryStatusEx(&memoryStatus)) {
        return static_cast<double>(memoryStatus.ullTotalPageFile) / (1024 * 1024 * 1024);
    } else {
        return -1.0; // Error
    }
#else
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return static_cast<double>(info.totalswap) * info.mem_unit / (1024 * 1024 * 1024);
    } else {
        return -1.0; // Error
    }
#endif
}

double getUsedSwapSpaceInGB() {
  std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        unsigned long long totalSwapKB = 0;
        unsigned long long freeSwapKB = 0;

        while (std::getline(meminfo, line)) {
            if (line.find("SwapTotal:") != std::string::npos) {
                sscanf(line.c_str(), "SwapTotal: %llu", &totalSwapKB);
            } else if (line.find("SwapFree:") != std::string::npos) {
                sscanf(line.c_str(), "SwapFree: %llu", &freeSwapKB);
            }
        }
        meminfo.close();

        if (totalSwapKB > 0) {
            double usedSwapGB = static_cast<double>(totalSwapKB - freeSwapKB) / (1024 * 1024);
            return usedSwapGB;
        }
    }

    // If we can't read the value, return a default or error value
    return -1.0;
}

double getDiskSizeInGB(const std::string& path) {
#ifdef _WIN32
    ULARGE_INTEGER freeBytesAvailable;
    ULARGE_INTEGER totalNumberOfBytes;
    ULARGE_INTEGER totalNumberOfFreeBytes;

    if (GetDiskFreeSpaceExA(path.c_str(), &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        return static_cast<double>(totalNumberOfBytes.QuadPart) / (1024 * 1024 * 1024);
    } else {
        return -1.0; // Error
    }
#else
    struct statvfs vfs;

    if (statvfs(path.c_str(), &vfs) == 0) {
        return static_cast<double>(vfs.f_frsize * vfs.f_blocks) / (1024 * 1024 * 1024);
    } else {
        return -1.0; // Error
    }
#endif
}

double getUsedDiskSpaceInGB(const std::string& path) {
#ifdef _WIN32
    ULARGE_INTEGER freeBytesAvailable;
    ULARGE_INTEGER totalNumberOfBytes;
    ULARGE_INTEGER totalNumberOfFreeBytes;

    if (GetDiskFreeSpaceExA(path.c_str(), &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        ULONGLONG usedSpace = totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart;
        return static_cast<double>(usedSpace) / (1024 * 1024 * 1024);
    } else {
        return -1.0; // Error
    }
#else
    struct statvfs vfs;

    if (statvfs(path.c_str(), &vfs) == 0) {
        unsigned long long totalSpace = vfs.f_frsize * vfs.f_blocks;
        unsigned long long freeSpace = vfs.f_frsize * vfs.f_bfree;
        unsigned long long usedSpace = totalSpace - freeSpace;

        return static_cast<double>(usedSpace) / (1024 * 1024 * 1024);
    } else {
        return -1.0; // Error
    }
#endif
}

/*double getProcessCPUUsage(int processID) {
     std::string stat_filepath = "/proc/" + std::to_string(processID) + "/stat";
    std::ifstream stat_file(stat_filepath.c_str());

    if (!stat_file) {
        std::cerr << "Error: Unable to open " << stat_filepath << std::endl;
        return -1.0; // Error occurred
    }

    std::string line;
    std::getline(stat_file, line);

    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.size() >= 15) {
        long utime = std::stol(tokens[13]);
        long stime = std::stol(tokens[14]);
        long cutime = std::stol(tokens[15]);
        long cstime = std::stol(tokens[16]);

        long total_time = utime + stime + cutime + cstime;
        long hertz = sysconf(_SC_CLK_TCK);

        double seconds = static_cast<double>(total_time) / hertz;
        double cpu_usage = (seconds > 0.0) ? (total_time / hertz) / seconds  : 0.0;

        return cpu_usage;
    }

    return -1.0; // Error occurred
}*/


float CalculateCPUPercentage(const std::string& statFilePath, float uSys) {
    std::ifstream CPU_file(statFilePath);
    std::string result;

    if (!CPU_file.is_open()) {
        // Gérer l'erreur ou retourner une valeur par défaut si nécessaire.
        return 0.0;
    }

    std::getline(CPU_file, result);
    CPU_file.close();

    int space_counter = 0;
    std::string user;
    std::string system;

    for (size_t i = 0; i < result.length(); i++) {
        if (result[i] == ' ' && space_counter != 13) {
            space_counter++;
        } else if (space_counter == 13) {
            for (size_t j = i; j < result.length(); j++) {
                if (result[j] == ' ') {
                    i = j;
                    break;
                }
                user += result[j];
            }
            space_counter++;
        } else if (space_counter == 14) {
            for (size_t j = i; j < result.length(); j++) {
                if (result[j] == ' ') {
                    i = j;
                    break;
                }
                system += result[j];
            }
            break;
        }
    }

    if (uSys <= 0.0f) {
        return 0.0f;
    } else {
        return 100.0f * (std::stof(system) + std::stof(user)) / uSys;
    }
}


double getProcessMemoryUsage(int pid) {
    // Paths to the process's status and system's memory info
    std::string statusFile = "/proc/" + std::to_string(pid) + "/status";
    std::string meminfoFile = "/proc/meminfo";

    std::ifstream processStatus(statusFile);
    std::ifstream meminfo(meminfoFile);
    std::string line;

    long long processMemory = 0;
    long long totalMemory = 0;

    // Read the process's memory from its status file
    while (std::getline(processStatus, line)) {
        if (line.find("VmRSS:") != std::string::npos) {
            std::istringstream iss(line);
            std::string key;
            long long value;
            iss >> key >> value;
            processMemory = value * 1024; // Convert from kB to bytes
            break;
        }
    }

    // Read the total system memory from /proc/meminfo
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") != std::string::npos) {
            std::istringstream iss(line);
            std::string key;
            long long value;
            iss >> key >> value;
            totalMemory = value * 1024; // Convert from kB to bytes
            break;
        }
    }

    if (totalMemory > 0) {
        // Calculate memory usage percentage
        double memoryUsagePercentage = (static_cast<double>(processMemory) / totalMemory) * 100.0;
        return memoryUsagePercentage;
    }

    // Return -1 to indicate an error
    return -1.0;
}

float CalculateTotalCPUTime() {
    std::ifstream statFile("/proc/stat");
    std::string line;

    if (!statFile.is_open()) {
        // Gérer l'erreur ou retourner une valeur par défaut si nécessaire.
        return 0.0;
    }

    std::getline(statFile, line);
    statFile.close();

    std::istringstream iss(line);
    std::string cpuLabel;
    long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    
    iss >> cpuLabel >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

    // Calcul du temps CPU total (uSys) en additionnant le temps CPU utilisateur et le temps CPU système.
    long totalCPU = user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;

    return static_cast<float>(totalCPU);
}

void listProcesses(const char* searchFilter) {
    
    DIR* dir;
    struct dirent* entry;
    std::string statusFile, line;

    dir = opendir("/proc");
    if (!dir) {
        std::cerr << "Error opening /proc directory." << std::endl;
        return;
    }

    ImGui::Columns(5, "ProcessListColumns", true);

    ImGui::Text("PID");
    ImGui::NextColumn();
    ImGui::Text("Name");
    ImGui::NextColumn();
    ImGui::Text("State");
    ImGui::NextColumn();
    ImGui::Text("CPU Usage");
    ImGui::NextColumn();
    ImGui::Text("Memory Usage");
    ImGui::NextColumn();
    ImGui::Separator();
    
    float uSys = CalculateTotalCPUTime();

    while ((entry = readdir(dir)) != NULL) {
        if (isdigit(entry->d_name[0])) {
            statusFile = "/proc/" + std::string(entry->d_name) + "/status";
            std::ifstream file(statusFile);
            if (file.is_open()) {
                std::string processName;
                int processID = std::stoi(entry->d_name);
                std::string statFilePath = "/proc/" + std::to_string(processID) + "/stat";
                double cpuUsage = CalculateCPUPercentage(statFilePath, uSys); // Vous devez implémenter cette fonction
                double memoryUsageKB = getProcessMemoryUsage(processID); // Vous devez implémenter cette fonction
                std::string state;

                while (std::getline(file, line)) {
                    if (line.find("Name:") == 0) {
                        processName = line.substr(6); // Extraire le nom du processus
                    } else if (line.find("State:") == 0) {
                        state = line.substr(7);
                        state = state.substr(1, state.size() - 1); // Supprimer l'espace initial
                    }
                }

                if (strstr(processName.c_str(), searchFilter) != nullptr) {
                    ImGui::Text("%d", processID);
                    ImGui::NextColumn();
                    ImGui::Text("%s", processName.c_str());
                    ImGui::NextColumn();
                    ImGui::Text("%s", state.c_str());
                    ImGui::NextColumn();
                    ImGui::Text("%.2f%%", cpuUsage); // Afficher l'utilisation CPU
                    ImGui::NextColumn();
                    ImGui::Text("%.2f%%", memoryUsageKB); // Afficher l'utilisation mémoire en KB
                    ImGui::NextColumn();
                }

                file.close();
            }
        }
    }

    ImGui::Columns(1);
    closedir(dir);
}