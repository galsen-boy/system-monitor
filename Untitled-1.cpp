
// int read_fields(FILE *fp, unsigned long long int *fields) {
//   int retval;
//   char buffer[BUF_MAX];

//   if (!fgets(buffer, BUF_MAX, fp)) {
//     perror("Error");
//   }
//   retval = sscanf(buffer, "cpu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",
//                   &fields[0], &fields[1], &fields[2], &fields[3], &fields[4],
//                   &fields[5], &fields[6], &fields[7], &fields[8],
//                   &fields[9]);
//   if (retval < 4) /* Atleast 4 fields is to be read */
//   {
//     fprintf(stderr, "Error reading /proc/stat cpu field\n");
//     return 0;
//   }
//   return 1;
// }

// int cpudata(void) {
//   FILE *fp;
//   unsigned long long int fields[10], total_tick, total_tick_old, idle,
//   idle_old,
//       del_total_tick, del_idle;
//   int update_cycle = 0, i, flag = 1;
//   double percent_usage;

//   fp = fopen("/proc/stat", "r");
//   if (fp == NULL) {
//     perror("Error");
//   }

//   if (!read_fields(fp, fields)) {
//     return 0;
//   }

//   for (i = 0, total_tick = 0; i < 10; i++) {
//     total_tick += fields[i];
//   }
//   idle = fields[3]; /* idle ticks index */

//   while (flag) {
//     sleep(1);
//     total_tick_old = total_tick;
//     idle_old = idle;

//     fseek(fp, 0, SEEK_SET);
//     fflush(fp);
//     if (!read_fields(fp, fields)) {
//       return 0;
//     }

//     for (i = 0, total_tick = 0; i < 10; i++) {
//       total_tick += fields[i];
//     }
//     idle = fields[3];

//     del_total_tick = total_tick - total_tick_old;
//     del_idle = idle - idle_old;

//     percent_usage = ((del_total_tick - del_idle) / (double)del_total_tick) *
//                     100; /* 3 is index of idle time */
//     return (percent_usage);
//     update_cycle++;
//   }

//   fclose(fp); /* Ctrl + C quit, therefore this will not be reached. We rely
//   on
//                  the kernel to close this file */

//   return 0;
// }

// if (ImPlot::BeginPlot("CPU Usage", ImVec2(600, cpuYScale))) {
//   // Plot the CPU usage data
//   if (!animationPaused) {
//     ImPlot::PlotLine("Usage", cpuUsageHistory, MaxDataPoints, 100.0f,
//                      0.0f, 0, dataOffset);
//   }
//   // End the plot
//   ImPlot::EndPlot();
// }

// char line[256];
// while (fgets(line, sizeof(line), statFile)) {
//   if (strncmp(line, "cpu", 3) == 0) {
//     int core;
//     unsigned long long user, nice, system, idle, iowait, irq, softirq,
//         steal, guest, guest_nice;
//     if (sscanf(line,
//                "cpu%d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
//                &core, &user, &nice, &system, &idle, &iowait, &irq, &softirq,
//                &steal, &guest, &guest_nice) == 11) {
//       unsigned long long totalCpuTime =
//           user + nice + system + idle + iowait + irq + softirq + steal;
//       unsigned long long idleTime = idle + iowait;

//       usagePetcentage =
//           (totalCpuTime - idleTime) / (double)totalCpuTime * 100;
//     }
//   }
// }
