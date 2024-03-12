#include "header.h"

// Function to execute the top command and parse its output
void fetchProcessData(std::vector<ProcessInfo> &processList,
                      const std::string &filterText) {
    static std::time_t lastFetchTime = std::time(nullptr);
    std::time_t currentTime = std::time(nullptr);
    std::time_t elapsedTime = currentTime - lastFetchTime;

    FILE *topFile = popen("top -bn1", "r");
    if (!topFile) {
        std::cerr << "Error opening top command" << std::endl;
        return;
    }

    // Ignore the first six lines of the top output
    for (int i = 0; i < 6; ++i)
        while (getc(topFile) != '\n');

    while (!feof(topFile)) {
        ProcessInfo process;
        char line[512];
        if (fgets(line, sizeof(line), topFile) == nullptr) {
            break;
        }

        sscanf(line, "%d %s %s %*d %*d %*d %*d %*d %*d %f %f %*s",
               &process.pid, &process.name[0], &process.state[0], &process.cpuUsage, &process.memoryUsage);

        // Calculate rates per second
        process.cpuUsage /= elapsedTime;
        process.memoryUsage /= elapsedTime;

        processList.push_back(process);
    }

    lastFetchTime = currentTime;

    pclose(topFile);
}

int main() {
    std::vector<ProcessInfo> processes;

    // Clean up resources

    return 0;
}
