#include "header.h"

static std::unordered_map<int, bool> previousSelection;
static std::vector<ProcessSelection> processSelectionList;

void fetchProcessData(std::vector<ProcessInfo> &processList,
                      const std::string &filterText) {

  std::string command = "ps -e -o pid,comm,state,%cpu,%mem --sort=-%cpu";

  FILE *fp = popen(command.c_str(), "r");
  if (!fp) {
    std::cerr << "Error executing ps command." << std::endl;
    return;
  }

  char line2[256];
  char line3[256];
  char line[256];

  // Store previous selection states
  for (const auto &process : processList) {
    previousSelection[process.pid] = process.selected;
  }

  // Clear the existing list
  processList.clear();
  // processSelectionList.clear();

  while (fgets(line, sizeof(line), fp)) {
    int pid;
    std::string name, state;
    float cpuUsage, memoryUsage;

    if (sscanf(line, "%d %s %s %f %f", &pid, line2, line3, &cpuUsage,
               &memoryUsage) != 5) {
      continue;
    }

    name = line2;
    state = line3;
    if (!filterText.empty() && name.find(filterText) == std::string::npos) {
      continue;
    }

    bool previousSelected =
        previousSelection.find(pid) != previousSelection.end()
            ? previousSelection[pid]
            : false;

    // Store selection state in the new list
    processSelectionList.emplace_back(pid, previousSelected);

    processList.emplace_back(pid, name, state, cpuUsage, memoryUsage, false);
  }

  pclose(fp);
}

void memoryProcessesWindow(const char *id, ImVec2 size, ImVec2 position,
                           std::vector<ProcessInfo> &processList,
                           std::string &filterText) {

  FILE *meminfo = fopen("/proc/meminfo", "r");
  if (meminfo == NULL) {
    return;
  }

  char line[256];
  unsigned long long total_memory = 0;
  unsigned long long available_memory = 0;

  while (fgets(line, sizeof(line), meminfo)) {
    if (strstr(line, "MemTotal:") == line) {
      sscanf(line, "MemTotal: %llu", &total_memory);
    } else if (strstr(line, "MemAvailable:") == line) {
      sscanf(line, "MemAvailable: %llu", &available_memory);
    }
  }

  fclose(meminfo);

  float ramUsage =
      100.0f * (1.0f - (float)available_memory / (float)total_memory);
  std::string command_used_space = "df -P / | tail -1 | awk '{print $5}'";

  // Command to get total disk space
  std::string command_total_space = "df -P / | tail -1 | awk '{print $2}'";

  FILE *df_output_used = popen(command_used_space.c_str(), "r");
  FILE *df_output_total = popen(command_total_space.c_str(), "r");

  float diskUsage = 0.0f;
  float diskSize = 0.0f;

  // Read the output of the df command
  char buffer[128];
  if (fgets(buffer, sizeof(buffer), df_output_used) != NULL) {
    diskUsage = strtof(buffer, nullptr);
  }

  if (fgets(buffer, sizeof(buffer), df_output_total) != NULL) {
    diskSize = strtof(buffer, nullptr);
  }
  diskSize = diskSize / (1024 * 1024);
  diskSize = static_cast<float>(std::ceil(diskSize));
  // Close the file pointers
  pclose(df_output_used);
  pclose(df_output_total);

  ImGui::Begin(id);
  ImGui::SetWindowSize(id, size);
  ImGui::SetWindowPos(id, position);

  // ram area
  unsigned long long used_memory = total_memory - available_memory;
  used_memory = used_memory / 1024;
  used_memory = used_memory / 1024;

  ImGui::Text("Physical Memory (RAM) total size: %lliGB, Used: %lliGB",
              total_memory / 1000000, used_memory);
  float progressBarWidth = -1; // Full width
  // Progress bar
  ImGui::ProgressBar(ramUsage / 100.0f, ImVec2(progressBarWidth, 0));
  // Calculate the position for the 0% and 100% labels
  ImVec2 progressBarStart = ImGui::GetItemRectMin();
  ImVec2 progressBarEnd = ImGui::GetItemRectMax();
  ImVec2 textPosition0 = ImVec2(progressBarStart.x, progressBarEnd.y + 5);
  ImVec2 textPosition100 = ImVec2(progressBarEnd.x - 30, progressBarEnd.y + 5);
  // Draw the text for 0% at the start of the progress bar
  ImGui::GetWindowDrawList()->AddText(textPosition0, IM_COL32_WHITE, "0%");
  // Draw the text for 100% at the end of the progress bar
  ImGui::GetWindowDrawList()->AddText(textPosition100, IM_COL32_WHITE, "100%");
  // Move the cursor to the next line
  ImGui::NewLine();
  ImGui::Separator();

  // swap area
  long long totalSwap = getSwapTotal();
  float swapUsage = getSwapUsage();
  ImGui::Text("Virtual Memory (SWAP) total size %lldGB:", totalSwap);
  ImGui::ProgressBar(swapUsage / 100.0f, ImVec2(-1, 0));
  ImVec2 progressBarStart2 = ImGui::GetItemRectMin();
  ImVec2 progressBarEnd2 = ImGui::GetItemRectMax();
  ImVec2 textPosition02 = ImVec2(progressBarStart2.x, progressBarEnd2.y + 5);
  ImVec2 textPosition1002 =
      ImVec2(progressBarEnd2.x - 30, progressBarEnd2.y + 5);
  // Draw the text for 0% at the start of the SWAP progress bar
  ImGui::GetWindowDrawList()->AddText(textPosition02, IM_COL32_WHITE, "0%");
  // Draw the text for 100% at the end of the SWAP progress bar
  ImGui::GetWindowDrawList()->AddText(textPosition1002, IM_COL32_WHITE, "100%");
  // Move the cursor to the next line
  ImGui::NewLine();
  ImGui::Separator();

  // disk area
  ImGui::Text("Disk total size %0.2f GB:", diskSize);
  // Construct the string for disk usage
  std::string diskUsageText = "Disk Usage: ";
  diskUsageText += std::to_string(diskUsage);
  diskUsageText += "%%";

  // Use the constructed string with ImGui::ProgressBar
  ImGui::ProgressBar(diskUsage / 100.0f, ImVec2(-1, 0), diskUsageText.c_str());

  ImVec2 progressBarStart3 = ImGui::GetItemRectMin();
  ImVec2 progressBarEnd3 = ImGui::GetItemRectMax();
  ImVec2 textPosition03 = ImVec2(progressBarStart3.x, progressBarEnd3.y + 5);
  ImVec2 textPosition1003 =
      ImVec2(progressBarEnd3.x - 30, progressBarEnd3.y + 5);
  // Draw the text for 0% at the start of the Disk progress bar
  ImGui::GetWindowDrawList()->AddText(textPosition03, IM_COL32_WHITE, "0%");
  // Draw the text for 100% at the end of the Disk progress bar
  ImGui::GetWindowDrawList()->AddText(textPosition1003, IM_COL32_WHITE, "100%");
  // Move the cursor to the next line
  ImGui::NewLine();
  ImGui::Separator();

  // processes area
  if (ImGui::BeginTabBar("ProcessTabs")) {
    if (ImGui::BeginTabItem("Processes")) {
      char filterBuffer[256];
      strcpy(filterBuffer, filterText.c_str());
      std::transform(filterBuffer, filterBuffer + strlen(filterBuffer),
                     filterBuffer, ::tolower);
      ImGui::InputText("Filter", filterBuffer, sizeof(filterBuffer));

      filterText = filterBuffer;
      ImGui::BeginTable("ProcessTable", 5,
                        ImGuiTableFlags_ContextMenuInBody |
                            ImGuiTableFlags_RowBg);
      ImGui::TableSetupColumn("PID");
      ImGui::TableSetupColumn("Name");
      ImGui::TableSetupColumn("State");
      ImGui::TableSetupColumn("CPU Usage");
      ImGui::TableSetupColumn("Memory Usage");
      ImGui::TableHeadersRow();
      ImGui::Separator();

      for (auto i = 0u; i < processList.size(); ++i) {

        std::transform(processList[i].name.begin(), processList[i].name.end(),
                       processList[i].name.begin(), ::tolower);
        if (!filterText.empty() &&
            processList[i].name.find(filterText) == std::string::npos)
          continue;

        ImGui::TableNextRow();

        bool isRowHovered = false;

        for (int col = 0; col < 5; col++) {
          ImGui::TableSetColumnIndex(col);

          // Your existing code for displaying text in each column
          if (col == 0)
            ImGui::Text("%d", processList[i].pid);
          else if (col == 1)
            ImGui::Text("%s", processList[i].name.c_str());
          else if (col == 2)
            ImGui::Text("%s", processList[i].state.c_str());
          else if (col == 3)
            ImGui::Text("%.2f%%", processList[i].cpuUsage);
          else if (col == 4)
            ImGui::Text("%.2f%%", processList[i].memoryUsage);

          // Check if the entire row is hovered
          if (ImGui::IsItemHovered()) {
            isRowHovered = true;

            // Set row background color for hover
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                                   IM_COL32(255, 255, 0, 100));
          }
        }

        if (isRowHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
          processSelectionList[i].selected = !processSelectionList[i].selected;
          std::cout << "Row " << i
                    << " Selected: " << processSelectionList[i].selected
                    << std::endl;
        }

        if (processSelectionList[i].selected) {
          ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                                 IM_COL32(0, 0, 255, 100));
        }
      }

      ImGui::EndTable();
      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }
  ImGui::End();
}

float getSwapUsage() {
  FILE *fp = fopen("/proc/meminfo", "r");
  if (fp == NULL) {
    return -1.0f;
  }

  char line[256];
  float totalSwap = 0.0f;
  float freeSwap = 0.0f;

  while (fgets(line, sizeof(line), fp)) {
    if (strncmp(line, "SwapTotal:", 10) == 0) {
      sscanf(line, "SwapTotal: %f kB", &totalSwap);
    } else if (strncmp(line, "SwapFree:", 9) == 0) {
      sscanf(line, "SwapFree: %f kB", &freeSwap);
    }
  }

  fclose(fp);

  if (totalSwap == 0.0f) {
    return -1.0f;
  }

  float swapUsage = 100.0f * ((totalSwap - freeSwap) / totalSwap);
  return swapUsage;
}

long long getSwapTotal() {
  std::ifstream meminfo("/proc/meminfo");
  std::string line;

  while (std::getline(meminfo, line)) {
    if (line.find("SwapTotal") != std::string::npos) {
      long long swapTotal;
      if (sscanf(line.c_str(), "SwapTotal: %lld kB", &swapTotal) == 1) {
        // Convert from kilobytes to bytes
        double swapTotalGB =
            swapTotal / (1024.0 * 1024.0); // Convert from KB to GB
        return static_cast<float>(std::ceil(swapTotalGB));
      }
    }
  }

  // Return 0 if SwapTotal is not found (or in case of any error)
  return 0;
}
