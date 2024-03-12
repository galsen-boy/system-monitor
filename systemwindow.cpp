#include "header.h"
#include "imgui/lib/imgui.h"
#include <cstddef>
#include <iomanip>

float cpuYScale = 100.0f; // Default Y Scale for CPU tab
float cpuFPS = 30.0f;     // Default FPS for CPU tab

float fanYScale = 100.0f; // Default Y Scale for FAN tab
float fanFPS = 30.0f;     // Default FPS for FAN tab

float tempYScale = 100.0f; // Default Y Scale for TEMP tab
float tempFPS = 30.0f;     // Default FPS for TEMP tab
float yScaleMin = 0.0f;

const int MaxDataPoints = 100;
int dataOffset = 0;
float cpuUsageHistory[MaxDataPoints];
std::vector<float> temperatureData;
float temperatureDataHistory[MaxDataPoints];

std::vector<float> fanSpeedData;

const int MaxFanSpeedDataPoints = 100;
// Define fan speed circular buffer and offset
std::vector<float> fanSpeedHistory(MaxFanSpeedDataPoints, 0.0f);
int fanSpeedDataOffset = 0;
float totalFanSpeedHistory[MaxDataPoints];
// std::vector<float> totalFanSpeedHistory; // To store history for plotting

float GetCpuUsage() {
  std::ifstream proc_stat("/proc/stat");
  if (!proc_stat.is_open()) {
    // Handle error: Unable to open /proc/stat
    return 0.0f;
  }

  proc_stat.ignore(5, ' '); // Skip the 'cpu' prefix.
  std::vector<size_t> times;
  for (size_t time; proc_stat >> time; times.push_back(time))
    ;

  proc_stat.close();

  if (times.size() < 4) {
    // Handle error: Insufficient data in /proc/stat
    return 0.0f;
  }

  size_t idle_time = times[3];
  size_t total_time = std::accumulate(times.begin(), times.end(), 0);

  static size_t previous_idle_time = 0;
  static size_t previous_total_time = 0;

  const float idle_time_delta = idle_time - previous_idle_time;
  const float total_time_delta = total_time - previous_total_time;

  const float utilization = 100.0 * (1.0 - idle_time_delta / total_time_delta);

  // Update previous values for the next iteration
  previous_idle_time = idle_time;
  previous_total_time = total_time;

  return utilization;
}

int systemWindow(const char *id, ImVec2 size, ImVec2 position) {

  ImGui::Begin(id);
  ImGui::SetWindowSize(id, size);
  ImGui::SetWindowPos(id, position);
  // float usagePetcentage;
  ImGui::Text("Operating System: %s", getOsName());
  char *username = getlogin();
  ImGui::Text("Logged in user: %s", username);
  char hostname[HOST_NAME_MAX];
  if (gethostname(hostname, sizeof(hostname)) == 0) {
    ImGui::Text("host Name: %s", hostname);
  }
  // Display CPU Info
  ImGui::Text("CPU Info: %s", CPUinfo().c_str());
  if (sensors_init(nullptr) < 0) {
    ImGui::Text("Error initializing libsensors");
  } else {
    // Display Fan Speeds and Thermals
    temperatureData.clear();
    fanSpeedData.clear();

    int chip_nr = 0;
    const sensors_chip_name *chip;

    while ((chip = sensors_get_detected_chips(nullptr, &chip_nr)) != nullptr) {
      const sensors_feature *feature;
      int feature_nr = 0;

      while ((feature = sensors_get_features(chip, &feature_nr)) != nullptr) {
        if (feature->type == SENSORS_FEATURE_FAN) {
          double speed;
          if (sensors_get_value(chip, feature->number, &speed) == 0) {
            fanSpeedData.push_back(
                static_cast<float>(speed)); // Store fan speed data
          }
        }

        if (feature->type == SENSORS_FEATURE_TEMP) {
          double temp;
          if (sensors_get_value(chip, feature->number, &temp) == 0) {
            temperatureData.push_back(
                static_cast<float>(temp)); // Store temperature data
          }
        }
      }
    }
    sensors_cleanup();
    // sleep(60);
  }

  const char *command = "ps aux --no-header | wc -l";
  FILE *fp = popen(command, "r");
  if (fp != nullptr) {
    char output[16];
    if (fgets(output, sizeof(output), fp) != nullptr) {
      int processCount = std::atoi(output);
      ImGui::Text("Total Running Processes: %d", processCount);
    }
    pclose(fp);
  }

  FILE *statFile = fopen("/proc/stat", "r");
  if (statFile != nullptr) {

    fclose(statFile);

    if (ImGui::BeginTabBar("TabBar")) {
      if (ImGui::BeginTabItem("CPU")) {
        const int bufferSize = 100;
        // float cpuUsageBuffer[bufferSize] = {};
        int plotOffset = 0;
        float cpuUsage = GetCpuUsage();

        // Update the CPU usage buffer
        // cpuUsageBuffer[plotOffset] = cpuUsage;
        plotOffset = (plotOffset + 1) % bufferSize;

        ImGui::Text("CPU Usage: %3.2lf%%", cpuUsage);

        // Update the circular buffer with the new CPU usage value
        cpuUsageHistory[dataOffset] = cpuUsage;
        static bool animationPausedCPU = false;

        ImGui::PlotLines("CPU usage", cpuUsageHistory, MaxDataPoints,
                         dataOffset, "Usage", 0.0f, cpuYScale, ImVec2(0, 200));

        // Stop/Resume animation button
        if (ImGui::Button(animationPausedCPU ? "Resume" : "Pause")) {
          animationPausedCPU = !animationPausedCPU;
        }

        auto startTime = std::chrono::high_resolution_clock::now();
        if (!animationPausedCPU) {
          dataOffset = (dataOffset + 1) % MaxDataPoints;

          // Calculate frame time and sleep if necessary
          auto endTime = std::chrono::high_resolution_clock::now();
          float frameTime =
              std::chrono::duration<float>(endTime - startTime).count();
          float targetFrameTime = 1.0f / tempFPS;
          if (frameTime < targetFrameTime) {
            float sleepTime = targetFrameTime - frameTime;
            std::this_thread::sleep_for(
                std::chrono::duration<float>(sleepTime));
          }
        }

        // FPS slider
        ImGui::SliderFloat("FPS", &cpuFPS, 1.0f, 60.0f);

        // Y Scale slider
        ImGui::SliderFloat("Y Scale", &cpuYScale, 10.0f, 200.0f);

        ImGui::EndTabItem();
      }

      // FAN SECTION
      if (ImGui::BeginTabItem("FAN")) {
        ImGui::Text("Fan Speeds:");
        static bool animationPausedFAN = false;
        float totalFanSpeed = 0.0f;

        // Display individual fan speeds
        for (size_t i = 0; i < fanSpeedData.size(); ++i) {
          ImGui::Text("Fan %zu speed: %.2f RPM", i + 1, fanSpeedData[i]);
          totalFanSpeed += fanSpeedData[i];
        }

        totalFanSpeedHistory[dataOffset] = totalFanSpeed;

        ImGui::Text("Combined Fan Speed: %.2f RPM", totalFanSpeed);

        // Plot the FAN speed data
        ImGui::PlotLines("FAN speed", totalFanSpeedHistory,
                         MaxFanSpeedDataPoints, dataOffset, "Speed", 0.0f,
                         fanYScale, ImVec2(0, 200));

        if (ImGui::Button(animationPausedFAN ? "Resume" : "Pause")) {
          animationPausedFAN = !animationPausedFAN;
        }

        if (!animationPausedFAN) {
          dataOffset = (dataOffset + 1) % MaxDataPoints;
        }
        ImGui::SliderFloat("FPS", &fanFPS, 1.0f, 60.0f);
        // Y Scale slider
        ImGui::SliderFloat("Y Scale", &fanYScale, 10.0f, 200.0f);

        if (!animationPausedFAN) {
          auto startTime = std::chrono::high_resolution_clock::now();

          auto endTime = std::chrono::high_resolution_clock::now();
          float frameTime =
              std::chrono::duration<float>(endTime - startTime).count();
          float targetFrameTime = 1.0f / fanFPS;

          float sleepTime = std::max(targetFrameTime - frameTime, 0.0f);
          std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
        }

        ImGui::EndTabItem();
      }

      // TEMP SECTION
      if (ImGui::BeginTabItem("TEMP")) {
        static bool animationPausedTEMP = false;

        if (!temperatureData.empty()) {
          temperatureDataHistory[dataOffset] = (float)temperatureData[0];

          ImGui::Text("CPU Temperature: %.2f°C", temperatureData[0]);
          ImGui::PlotLines("CPU Temperature", &temperatureDataHistory[0],
                           MaxDataPoints, dataOffset, "Temp", 0.0f, tempYScale,
                           ImVec2(0, 200));
        }

        if (ImGui::Button(animationPausedTEMP ? "Resume" : "Pause")) {
          animationPausedTEMP = !animationPausedTEMP;
        }

        if (!animationPausedTEMP) {
          dataOffset = (dataOffset + 1) % MaxDataPoints;
        }

        ImGui::SliderFloat("FPS", &tempFPS, 1.0f, 60.0f);
        ImGui::SliderFloat("Y Scale", &tempYScale, 10.0f, 200.0f);

        if (!animationPausedTEMP) {
          auto startTime = std::chrono::high_resolution_clock::now();
          auto endTime = std::chrono::high_resolution_clock::now();
          float frameTime =
              std::chrono::duration<float>(endTime - startTime).count();
          float targetFrameTime = 1.0f / tempFPS;
          float sleepTime = std::max(targetFrameTime - frameTime, 0.0f);
          std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
        }
        ImGui::EndTabItem();
      }

      ImGui::EndTabBar();
    }
  } else {
    ImGui::Text("Error: Cannot read CPU usage data");
  }

  ImGui::End();
  return 0;
}

void UpdateFanSpeedData(float fanSpeed) {
  // Update circular buffer
  fanSpeedHistory[fanSpeedDataOffset] = fanSpeed;

  // Increment circular buffer offset
  fanSpeedDataOffset = (fanSpeedDataOffset + 1) % MaxFanSpeedDataPoints;
}
