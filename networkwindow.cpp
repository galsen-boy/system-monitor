#include "header.h"

std::string runCommand(const char *cmd) {
  char buffer[128];
  std::string result = "";
  FILE *pipe = popen(cmd, "r");
  if (!pipe) {
    return "Command execution failed.";
  }
  while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
    result += buffer;
  }
  pclose(pipe);
  return result;
}
std::set<std::string> uniqueInterfaces;

std::vector<
    std::tuple<std::string, unsigned long long, unsigned long long,
               unsigned long long, unsigned long long, unsigned long long,
               unsigned long long, unsigned long long, unsigned long long,
               unsigned long long, unsigned long long, unsigned long long,
               unsigned long long, unsigned long long, unsigned long long,
               unsigned long long, unsigned long long>>
    interfaces;

std::string formatBytes(unsigned long long bytes) {
  const char *suffix[] = {"B", "KB", "MB", "GB"};
  char length = sizeof(suffix) / sizeof(suffix[0]);

  int i = 0;
  double dblBytes = static_cast<double>(bytes);

  if (bytes > 1000) {
    for (i = 0; (bytes / 1000) > 0 && i < length - 1; i++, bytes /= 1000) {
      dblBytes = bytes / 1000.0;
    }
  }

  char formattedBytes[32];
  snprintf(formattedBytes, sizeof(formattedBytes), "%.2f %s", dblBytes,
           suffix[i]);
  return std::string(formattedBytes);
}

void displayNetworkTable(
    const char *title,
    const std::vector<
        std::tuple<std::string, unsigned long long, unsigned long long,
                   unsigned long long, unsigned long long, unsigned long long,
                   unsigned long long, unsigned long long, unsigned long long,
                   unsigned long long, unsigned long long, unsigned long long,
                   unsigned long long, unsigned long long, unsigned long long,
                   unsigned long long, unsigned long long>> &interfaces) {

  if (ImGui::TreeNode(title)) {
    if (title == std::string("Receive (Rx)")) {
      ImGui::Columns(9, "NetworkData", true);
      ImGui::Separator();
      ImGui::Text("Interface");
      ImGui::NextColumn();
      ImGui::Text("Packets");
      ImGui::NextColumn();
      ImGui::Text("Bytes");
      ImGui::NextColumn();
      ImGui::Text("Errs");
      ImGui::NextColumn();
      ImGui::Text("Drop");
      ImGui::NextColumn();
      ImGui::Text("Fifo");
      ImGui::NextColumn();
      ImGui::Text("frame");
      ImGui::NextColumn();
      ImGui::Text("Compressed");
      ImGui::NextColumn();
      ImGui::Text("multicast");
      ImGui::NextColumn();
      ImGui::Separator();
    } else {
      ImGui::Columns(9, "NetworkData", true);
      ImGui::Separator();
      ImGui::Text("Interface");
      ImGui::NextColumn();
      ImGui::Text("Packets");
      ImGui::NextColumn();
      ImGui::Text("Bytes");
      ImGui::NextColumn();
      ImGui::Text("Errs");
      ImGui::NextColumn();
      ImGui::Text("Drop");
      ImGui::NextColumn();
      ImGui::Text("Fifo");
      ImGui::NextColumn();
      ImGui::Text("colls");
      ImGui::NextColumn();
      ImGui::Text("carrier");
      ImGui::NextColumn();
      ImGui::Text("compressed");
      ImGui::NextColumn();
      ImGui::Separator();
    }

    for (const auto &interface : interfaces) {
      if (title == std::string("Receive (Rx)")) {
        const std::string &interfaceName = std::get<0>(interface);
        const unsigned long long &rxpackets = std::get<1>(interface);
        const unsigned long long &rxbytes = std::get<2>(interface);
        const unsigned long long &rxerr = std::get<3>(interface);
        const unsigned long long &rxdrop = std::get<4>(interface);
        const unsigned long long &rxframe = std::get<6>(interface);
        const unsigned long long &rxfifo = std::get<7>(interface);
        const unsigned long long &rxcompressed = std::get<8>(interface);
        const unsigned long long &rxmulticast = std::get<5>(interface);
        ImGui::Text("%s", interfaceName.c_str());
        ImGui::NextColumn();
        ImGui::Text("%llu", rxpackets);
        ImGui::NextColumn();
        ImGui::Text("%llu", rxbytes);
        ImGui::NextColumn();
        ImGui::Text("%llu", rxerr);
        ImGui::NextColumn();
        ImGui::Text("%llu", rxdrop);
        ImGui::NextColumn();
        ImGui::Text("%llu", rxfifo);
        ImGui::NextColumn();
        ImGui::Text("%llu", rxframe);
        ImGui::NextColumn();
        ImGui::Text("%llu", rxcompressed);
        ImGui::NextColumn();
        ImGui::Text("%llu", rxmulticast);
        ImGui::NextColumn();
        ImGui::Separator();
      } else {
        const std::string &interfaceName = std::get<0>(interface);
        const unsigned long long &txpackets = std::get<9>(interface);
        const unsigned long long &txbytes = std::get<10>(interface);
        const unsigned long long &txerr = std::get<11>(interface);
        const unsigned long long &txdrop = std::get<12>(interface);
        const unsigned long long &txcolls = std::get<13>(interface);
        const unsigned long long &txCarrier = std::get<14>(interface);
        const unsigned long long &txcompressed = std::get<15>(interface);
        const unsigned long long &txfifo = std::get<16>(interface);
        ImGui::Text("%s", interfaceName.c_str());
        ImGui::NextColumn();
        ImGui::Text("%llu", txpackets);
        ImGui::NextColumn();
        ImGui::Text("%llu", txbytes);
        ImGui::NextColumn();
        ImGui::Text("%llu", txerr);
        ImGui::NextColumn();
        ImGui::Text("%llu", txdrop);
        ImGui::NextColumn();
        ImGui::Text("%llu", txcolls);
        ImGui::NextColumn();
        ImGui::Text("%llu", txCarrier);
        ImGui::NextColumn();
        ImGui::Text("%llu", txcompressed);
        ImGui::NextColumn();
        ImGui::Text("%llu", txfifo);
        ImGui::NextColumn();
        ImGui::Separator();
      }
    }
    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::TreePop();
  }
}

void getNetworkStatistics() {
  interfaces.clear(); // Clear existing data
  // Get all network interfaces
  struct ifaddrs *ifaddr, *ifa;
  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    return;
  }

  // Iterate over network interfaces
  for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == nullptr || ifa->ifa_name == nullptr) {
      continue;
    }
    // Check if we have already processed this interface
    if (uniqueInterfaces.find(ifa->ifa_name) != uniqueInterfaces.end()) {
      continue;
    }
    // Run 'ifconfig' for the current interface
    std::string ifconfigOutput =
        runCommand(("ifconfig " + std::string(ifa->ifa_name)).c_str());
    uniqueInterfaces.insert(ifa->ifa_name);

    // RX values section
    std::string ifconfigspecificRX = runCommand(
        ("ifconfig " + std::string(ifa->ifa_name) + " | grep 'RX'").c_str());

    std::regex rxPacketsPattern("RX packets (\\d+)");
    std::regex rxBytesPattern("bytes (\\d+)");
    std::regex rxErrorsPattern("errors (\\d+)");
    std::regex rxDroppedPattern("dropped (\\d+)");
    std::regex rxOverrunsPattern("overruns (\\d+)");
    std::regex rxframePattern("frame (\\d+)");
    std::regex rxCompressedPattern("compressed (\\d+)");
    std::regex rxFifoPattern("fifo (\\d+)");

    std::smatch matchrx;

    unsigned long long rxPackets = 0, rxBytes = 0;
    unsigned long long rxErrors = 0, rxdropped = 0;
    unsigned long long rxOverruns = 0, rxframe = 0;
    unsigned long long rxCompressed = 0, rxfifo = 0;

    if (std::regex_search(ifconfigspecificRX, matchrx, rxPacketsPattern)) {
      rxPackets = std::stoull(matchrx[1]);
    }
    if (std::regex_search(ifconfigspecificRX, matchrx, rxBytesPattern)) {
      rxBytes = std::stoull(matchrx[1]);
    }
    if (std::regex_search(ifconfigspecificRX, matchrx, rxErrorsPattern)) {
      rxErrors = std::stoull(matchrx[1]);
    }
    if (std::regex_search(ifconfigspecificRX, matchrx, rxDroppedPattern)) {
      rxdropped = std::stoull(matchrx[1]);
    }
    if (std::regex_search(ifconfigspecificRX, matchrx, rxOverrunsPattern)) {
      rxOverruns = std::stoull(matchrx[1]);
    }
    if (std::regex_search(ifconfigspecificRX, matchrx, rxframePattern)) {
      rxframe = std::stoull(matchrx[1]);
    }
    if (std::regex_search(ifconfigspecificRX, matchrx, rxCompressedPattern)) {
      rxCompressed = std::stoull(matchrx[1]);
    }
    if (std::regex_search(ifconfigspecificRX, matchrx, rxFifoPattern)) {
      rxfifo = std::stoull(matchrx[1]);
    }

    // TX values section
    std::string ifconfigspecificTX = runCommand(
        ("ifconfig " + std::string(ifa->ifa_name) + " | grep 'TX'").c_str());

    std::regex txPacketsPattern("TX packets (\\d+)");
    std::regex txBytesPattern("bytes (\\d+)");
    std::regex txErrorsPattern("errors (\\d+)");
    std::regex txDroppedPattern("dropped (\\d+)");
    std::regex txCollisionsPattern("collisions   (\\d+)");
    std::regex txCarrierPattern("carrier (\\d+)");
    std::regex txCompressedPattern("compressed (\\d+)");
    std::regex txFifoPattern("fifo (\\d+)");
    std::smatch match;

    unsigned long long txPackets = 0, txBytes = 0;
    unsigned long long txErrors = 0, txDropped = 0;
    unsigned long long txColls = 0, txCarrier = 0;
    unsigned long long txCompressed = 0, txfifo = 0;

    if (std::regex_search(ifconfigspecificTX, match, txPacketsPattern)) {
      txPackets = std::stoull(match[1]);
    }
    if (std::regex_search(ifconfigspecificTX, match, txBytesPattern)) {
      txBytes = std::stoull(match[1]);
    }
    if (std::regex_search(ifconfigspecificTX, match, txErrorsPattern)) {
      txErrors = std::stoull(match[1]);
    }
    if (std::regex_search(ifconfigspecificTX, match, rxDroppedPattern)) {
      txDropped = std::stoull(match[1]);
    }
    if (std::regex_search(ifconfigspecificTX, match, txCollisionsPattern)) {
      txColls = std::stoull(match[1]);
    }
    if (std::regex_search(ifconfigspecificTX, match, txCarrierPattern)) {
      txCarrier = std::stoull(match[1]);
    }
    if (std::regex_search(ifconfigspecificTX, match, txCompressedPattern)) {
      txCompressed = std::stoull(match[1]);
    }
    if (std::regex_search(ifconfigspecificTX, match, txFifoPattern)) {
      txfifo = std::stoull(match[1]);
    }
    // store all fetched data into the vector

    interfaces.push_back(std::make_tuple(
        std::string(ifa->ifa_name), rxPackets, rxBytes, rxErrors, rxdropped,
        rxOverruns, rxframe, rxCompressed, rxfifo, txPackets, txBytes, txErrors,
        txDropped, txColls, txCarrier, txCompressed, txfifo));
  }
  uniqueInterfaces.clear();
  freeifaddrs(ifaddr); // Free memory allocated by getifaddrs
}

void networkWindow(const char *id, ImVec2 size, ImVec2 position) {
  getNetworkStatistics();

  ImGui::Begin(id);
  ImGui::SetWindowSize(id, size);
  ImGui::SetWindowPos(id, position);

  // Add margins
  const float marginSize = 10.0f;
  ImGui::Dummy(ImVec2(marginSize, marginSize));

  // Display system date and time
  time_t currentTime = time(nullptr);
  struct tm *localTime = localtime(&currentTime);
  char timeBuffer[80];
  strftime(timeBuffer, sizeof(timeBuffer), "%A %B %d %T %Y", localTime);
  ImGui::Text("System Date and Time: %s", timeBuffer);

  // Add margins
  ImGui::Dummy(ImVec2(marginSize, marginSize));
  ImGui::Separator();
  ImGui::Dummy(ImVec2(marginSize, marginSize));

  ImGui::Text("IPv4 Network:");

  // Iterate over network interfaces
  for (const auto &interface : interfaces) {
    const std::string &interfaceName = std::get<0>(interface);

    // Display IPv4 information
    std::string ifconfigOutput =
        runCommand(("ifconfig " + interfaceName).c_str());
    size_t ipv4Start = ifconfigOutput.find("inet ");
    if (ipv4Start != std::string::npos) {
      size_t ipv4End = ifconfigOutput.find(' ', ipv4Start + 5);
      if (ipv4End != std::string::npos) {
        std::string ipv4Address =
            ifconfigOutput.substr(ipv4Start + 5, ipv4End - ipv4Start - 5);
        ImGui::Text("%s: %s", interfaceName.c_str(), ipv4Address.c_str());
      }
    }
  }

  // Add margins
  ImGui::Dummy(ImVec2(marginSize, marginSize));
  ImGui::Separator();
  ImGui::Dummy(ImVec2(marginSize, marginSize));

  ImGui::Text("Network Table:");

  // Display network tables
  displayNetworkTable("Receive (Rx)", interfaces);

  // Tx section
  displayNetworkTable("Transfer (Tx)", interfaces);

  // Add margins
  ImGui::Dummy(ImVec2(marginSize, marginSize));
  ImGui::Separator();
  ImGui::Dummy(ImVec2(marginSize, marginSize));
  // ImGui::EndChild();

  // Tabs for Receive (Rx) and Transfer (Tx)
  ImGui::BeginTabBar("NetworkTabs");

  // Receive (Rx) tab
  if (ImGui::BeginTabItem("Receive (Rx)")) {
    // Iterate over network interfaces
    for (const auto &interface : interfaces) {
      const std::string &interfaceName = std::get<0>(interface);
      const unsigned long long &rxBytes = std::get<2>(interface);
      // Convert bytes to human-readable format (GB, MB, KB)
      std::string rxBytesFormatted = formatBytes(rxBytes);

      // Display progress bar
      ImGui::Text("%s", interfaceName.c_str());
      float progress = static_cast<float>(rxBytes) / 2e9; // Normalize to 0-2 GB
      // Construct the string for the progress bar label
      std::string progressBarLabel = std::string(rxBytesFormatted) + ":";

      // Use the constructed string with ImGui::ProgressBar
      ImGui::ProgressBar(progress, ImVec2(-1, 0), progressBarLabel.data());
    }
    ImGui::Dummy(ImVec2(marginSize, marginSize));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(marginSize, marginSize));
    ImGui::EndTabItem();
  }
  // Transfer (Tx) tab
  if (ImGui::BeginTabItem("Transfer (Tx)")) {
    // Iterate over network interfaces
    for (const auto &interface : interfaces) {
      const std::string &interfaceName = std::get<0>(interface);
      const unsigned long long &txBytes = std::get<10>(interface);

      // Convert bytes to human-readable format (GB, MB, KB)
      std::string txBytesFormatted = formatBytes(txBytes);

      // Display progress bar
      ImGui::Text("%s", interfaceName.c_str());
      float progress = static_cast<float>(txBytes) / 2e9; // Normalize to 0-2 GB
      // Construct the string for the progress bar label
      std::string progressBarLabel = std::string(txBytesFormatted) + ":";

      // Use the constructed string with ImGui::ProgressBar
      ImGui::ProgressBar(progress, ImVec2(-1, 0), progressBarLabel.c_str());
    }
    ImGui::Dummy(ImVec2(marginSize, marginSize));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(marginSize, marginSize));
    ImGui::EndTabItem();
  }

  ImGui::EndTabBar();
  ImGui::End();
}
