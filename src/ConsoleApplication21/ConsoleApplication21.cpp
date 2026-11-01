#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <ctime>
#include <tlhelp32.h>
#include <psapi.h>
#include <iphlpapi.h>
#include <winioctl.h>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std;

string padRight(const string& str, size_t width) {
    if (str.length() >= width) return str;
    return str + string(width - str.length(), ' ');
}

string formatNumber(double num, int precision = 2) {
    string result = to_string(num);
    size_t pos = result.find('.');
    if (pos != string::npos && pos + precision + 1 < result.length()) {
        result = result.substr(0, pos + precision + 1);
    }
    return result;
}

void clearScreen() {
    system("cls");
}

void printHeader(const string& title) {
    cout << "\n" << title << "\n";
    cout << string(80, '=') << "\n";
}


double getCPUUsage() {
    static FILETIME prevIdleTime = { 0, 0 };
    static FILETIME prevKernelTime = { 0, 0 };
    static FILETIME prevUserTime = { 0, 0 };
    static bool firstRun = true;

    FILETIME idleTime, kernelTime, userTime;
    GetSystemTimes(&idleTime, &kernelTime, &userTime);

    if (firstRun) {
        prevIdleTime = idleTime;
        prevKernelTime = kernelTime;
        prevUserTime = userTime;
        firstRun = false;
        return 0.0;
    }

    ULARGE_INTEGER idle, kernel, user;
    ULARGE_INTEGER prevIdle, prevKernel, prevUser;

    idle.LowPart = idleTime.dwLowDateTime;
    idle.HighPart = idleTime.dwHighDateTime;
    kernel.LowPart = kernelTime.dwLowDateTime;
    kernel.HighPart = kernelTime.dwHighDateTime;
    user.LowPart = userTime.dwLowDateTime;
    user.HighPart = userTime.dwHighDateTime;

    prevIdle.LowPart = prevIdleTime.dwLowDateTime;
    prevIdle.HighPart = prevIdleTime.dwHighDateTime;
    prevKernel.LowPart = prevKernelTime.dwLowDateTime;
    prevKernel.HighPart = prevKernelTime.dwHighDateTime;
    prevUser.LowPart = prevUserTime.dwLowDateTime;
    prevUser.HighPart = prevUserTime.dwHighDateTime;

    ULONGLONG idleDiff = idle.QuadPart - prevIdle.QuadPart;
    ULONGLONG kernelDiff = kernel.QuadPart - prevKernel.QuadPart;
    ULONGLONG userDiff = user.QuadPart - prevUser.QuadPart;
    ULONGLONG totalDiff = kernelDiff + userDiff;

    prevIdleTime = idleTime;
    prevKernelTime = kernelTime;
    prevUserTime = userTime;

    if (totalDiff == 0) return 0.0;
    return 100.0 * (totalDiff - idleDiff) / totalDiff;
}

int getCoreCount() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
}

double getCPUFrequency() {
    HKEY hKey;
    DWORD freq = 0;
    DWORD size = sizeof(freq);
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, L"~MHz", NULL, NULL, (LPBYTE)&freq, &size);
        RegCloseKey(hKey);
    }
    return (double)freq;
}
/// <summary>
/// ЦПУ сколько использвует
/// </summary>
void task1_CPU() {
    clearScreen();
    printHeader("CPU USAGE");

    int cores = getCoreCount();
    double freq = getCPUFrequency();
    cout << "Collecting data...\n";
    Sleep(1000);

    while (true) {
        double usage = getCPUUsage();

        string output = "CPU Usage: " + formatNumber(usage, 2) + "% | ";
        output += "Cores: " + to_string(cores) + " | ";
        if (freq > 0) {
            output += "Frequency: " + formatNumber(freq, 0) + " MHz";
        }
        else {
            output += "Frequency: N/A";
        }

        cout << "\r" << padRight(output, 80) << flush;
        Sleep(1000);
    }
}
/// <summary>
/// мониторинг за РАМом
/// </summary>
void task2_RAM() {
    clearScreen();
    printHeader("RAM MONITORING");

    while (true) {
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);

        if (GlobalMemoryStatusEx(&memStatus)) {
            unsigned long long totalPhys = memStatus.ullTotalPhys;
            unsigned long long availPhys = memStatus.ullAvailPhys;
            unsigned long long usedPhys = totalPhys - availPhys;

            double totalGB = totalPhys / (1024.0 * 1024.0 * 1024.0);
            double usedGB = usedPhys / (1024.0 * 1024.0 * 1024.0);
            double freeGB = availPhys / (1024.0 * 1024.0 * 1024.0);
            double usagePercent = (double)usedPhys / totalPhys * 100.0;

            string output = "RAM: Total: " + formatNumber(totalGB, 2) + " GB | ";
            output += "Used: " + formatNumber(usedGB, 2) + " GB | ";
            output += "Free: " + formatNumber(freeGB, 2) + " GB | ";
            output += "Usage: " + formatNumber(usagePercent, 1) + "%";

            cout << "\r" << padRight(output, 80) << flush;
        }

        Sleep(1000);
    }
}

struct NetworkStats {
    long long rxBytes;
    long long txBytes;
};

NetworkStats getNetworkStats() {
    NetworkStats stats = { 0, 0 };
    MIB_IFTABLE* pIfTable = NULL;
    DWORD dwSize = 0;

    GetIfTable(NULL, &dwSize, FALSE);
    pIfTable = (MIB_IFTABLE*)malloc(dwSize);

    if (pIfTable && GetIfTable(pIfTable, &dwSize, FALSE) == NO_ERROR) {
        for (int i = 0; i < pIfTable->dwNumEntries; i++) {
            MIB_IFROW& row = pIfTable->table[i];
            if (row.dwType != MIB_IF_TYPE_LOOPBACK &&
                row.dwOperStatus == MIB_IF_OPER_STATUS_CONNECTED) {
                stats.rxBytes += row.dwInOctets;
                stats.txBytes += row.dwOutOctets;
            }
        }
    }

    if (pIfTable) free(pIfTable);
    return stats;
}

string formatSpeed(double speed) {
    if (speed >= 1024) {
        return formatNumber(speed / 1024.0, 2) + " MB/s";
    }
    return formatNumber(speed, 2) + " KB/s";
}
/// <summary>
///  статы о инете
/// </summary>
void task3_Network() {
    clearScreen();
    printHeader("NETWORK TRAFFIC");

    NetworkStats prevStats = getNetworkStats();
    Sleep(1000);

    while (true) {
        NetworkStats currStats = getNetworkStats();

        long long rxDiff = currStats.rxBytes - prevStats.rxBytes;
        long long txDiff = currStats.txBytes - prevStats.txBytes;

        double rxSpeed = rxDiff / 1024.0;
        double txSpeed = txDiff / 1024.0;

        string output = "RX: " + formatSpeed(rxSpeed) + " | ";
        output += "TX: " + formatSpeed(txSpeed);

        cout << "\r" << padRight(output, 60) << flush;

        prevStats = currStats;
        Sleep(1000);
    }
}

struct DiskStats {
    long long readBytes;
    long long writeBytes;
};

DiskStats getDiskStats() {
    DiskStats stats = { 0, 0 };
    HANDLE hDevice = CreateFileA("\\\\.\\C:", GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
        OPEN_EXISTING, 0, NULL);

    if (hDevice != INVALID_HANDLE_VALUE) {
        DISK_PERFORMANCE perf;
        DWORD bytesReturned;
        if (DeviceIoControl(hDevice, IOCTL_DISK_PERFORMANCE,
            NULL, 0, &perf, sizeof(perf), &bytesReturned, NULL)) {
            stats.readBytes = perf.BytesRead.QuadPart;
            stats.writeBytes = perf.BytesWritten.QuadPart;
        }
        CloseHandle(hDevice);
    }
    return stats;
}
/// <summary>
/// Вывести статы диска
/// </summary>
void task4_DiskIO() {
    clearScreen();
    printHeader("DISK I/O");


    DiskStats prevStats = getDiskStats();
    Sleep(1000);

    while (true) {
        DiskStats currStats = getDiskStats();

        long long readDiff = currStats.readBytes - prevStats.readBytes;
        long long writeDiff = currStats.writeBytes - prevStats.writeBytes;

        double readSpeed = readDiff / (1024.0 * 1024.0);
        double writeSpeed = writeDiff / (1024.0 * 1024.0);

        string output = "Read: " + formatNumber(readSpeed, 2) + " MB/s | ";
        output += "Write: " + formatNumber(writeSpeed, 2) + " MB/s";

        cout << "\r" << padRight(output, 60) << flush;

        prevStats = currStats;
        Sleep(1000);
    }
}
/// <summary>
/// Статус батарейки, если она есть
/// </summary>

void task5_Battery() {
    clearScreen();
    printHeader("BATTERY STATUS");


    while (true) {
        SYSTEM_POWER_STATUS powerStatus;
        string output;

        if (GetSystemPowerStatus(&powerStatus)) {
            if (powerStatus.BatteryFlag == 128) {
                output = "Battery: Not detected | Running on AC power";
            }
            else {
                output = "Battery: " + to_string((int)powerStatus.BatteryLifePercent) + "% | ";

                if (powerStatus.ACLineStatus == 1) {
                    output += "Status: Charging";
                    if (powerStatus.BatteryLifePercent == 100) {
                        output += " (Fully charged)";
                    }
                }
                else if (powerStatus.ACLineStatus == 0) {
                    output += "Status: Discharging";
                }
                else {
                    output += "Status: Unknown";
                }

                if (powerStatus.BatteryLifeTime > 0 && powerStatus.BatteryLifeTime != (DWORD)-1) {
                    int hours = powerStatus.BatteryLifeTime / 3600;
                    int minutes = (powerStatus.BatteryLifeTime % 3600) / 60;
                    output += " | Time: " + to_string(hours) + "h " + to_string(minutes) + "m";
                }
            }
        }

        cout << "\r" << padRight(output, 70) << flush;
        Sleep(1000);
    }
}

/// <summary>
/// Фалйовая система с диска С
/// </summary>

void task6_Filesystem() {
    clearScreen();
    printHeader("FILESYSTEM AND SWAP");


    while (true) {
        string output = "";


        ULARGE_INTEGER freeBytes, totalBytes;
        if (GetDiskFreeSpaceExA("C:\\", &freeBytes, &totalBytes, NULL)) {
            double totalGB = totalBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
            double usedGB = (totalBytes.QuadPart - freeBytes.QuadPart) / (1024.0 * 1024.0 * 1024.0);
            double freeGB = freeBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
            double usagePercent = (double)(totalBytes.QuadPart - freeBytes.QuadPart) / totalBytes.QuadPart * 100.0;

            output = "C: Total: " + formatNumber(totalGB, 2) + " GB | ";
            output += "Used: " + formatNumber(usedGB, 2) + " GB | ";
            output += "Free: " + formatNumber(freeGB, 2) + " GB | ";
            output += "Usage: " + formatNumber(usagePercent, 1) + "%";
        }

        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);

        if (GlobalMemoryStatusEx(&memStatus)) {
            unsigned long long totalVirtual = memStatus.ullTotalPageFile;
            unsigned long long usedVirtual = memStatus.ullTotalPageFile - memStatus.ullAvailPageFile;

            double totalMB = totalVirtual / (1024.0 * 1024.0);
            double usedMB = usedVirtual / (1024.0 * 1024.0);

            if (!output.empty()) output += " | ";
            output += "Swap: " + formatNumber(usedMB, 1) + " / " +
                formatNumber(totalMB, 1) + " MB";
        }

        cout << "\r" << padRight(output, 80) << flush;
        Sleep(1000);
    }
}
/// <summary>
///  Процесы которые работают
/// </summary>
void task7_Processes() {
    clearScreen();
    printHeader("PROCESS DYNAMICS");
    


    while (true) {
        vector<pair<DWORD, string>> processes;
        int total = 0;
        int running = 0;

        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 pe32;
            pe32.dwSize = sizeof(PROCESSENTRY32);

            if (Process32First(hSnapshot, &pe32)) {
                do {
                    total++;
                    char processName[256];
                    WideCharToMultiByte(CP_ACP, 0, pe32.szExeFile, -1, processName, 256, NULL, NULL);
                    processes.push_back(pair<DWORD, string>(pe32.th32ProcessID, string(processName)));

                    if (pe32.th32ProcessID == 0 || pe32.th32ProcessID == 4) {
                        running++;
                    }
                } while (Process32Next(hSnapshot, &pe32));
            }
            CloseHandle(hSnapshot);
        }

        
        string topProcess = "N/A";
        SIZE_T maxMemory = 0;
        DWORD topPID = 0;

        for (const auto& p : processes) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, p.first);
            if (hProcess) {
                PROCESS_MEMORY_COUNTERS pmc;
                if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                    if (pmc.WorkingSetSize > maxMemory) {
                        maxMemory = pmc.WorkingSetSize;
                        topProcess = p.second;
                        topPID = p.first;
                    }
                }
                CloseHandle(hProcess);
            }
        }

        string output = "Total: " + to_string(total) + " | ";
        output += "Running: " + to_string(running) + " | ";
        output += "Sleeping: " + to_string(total - running) + " | ";
        output += "Top Memory: PID " + to_string(topPID);
        if (!topProcess.empty() && topProcess != "N/A") {
            output += " (" + topProcess + ")";
        }
        if (maxMemory > 0) {
            output += " - " + formatNumber(maxMemory / (1024.0 * 1024.0), 2) + " MB";
        }

        cout << "\r" << padRight(output, 80) << flush;
        Sleep(1000);
    }
}
/// <summary>
///Время загрузки и время работы
/// </summary>

void task8_LoadUptime() {
    clearScreen();
    printHeader("SYSTEM LOAD AND UPTIME");;
    cout << "Collecting data...\n";
    Sleep(1000);

    while (true) {

        DWORD uptime = GetTickCount() / 1000;
        int hours = uptime / 3600;
        int minutes = (uptime % 3600) / 60;
        int seconds = uptime % 60;

        string output = "Uptime: ";
        if (hours > 0) output += to_string(hours) + "h ";
        if (minutes > 0 || hours > 0) output += to_string(minutes) + "m ";
        output += to_string(seconds) + "s | ";

        double cpuLoad = getCPUUsage();
        output += "CPU Load: " + formatNumber(cpuLoad, 1) + "%";

        char timeStr[9];
        _snprintf_s(timeStr, sizeof(timeStr), _TRUNCATE, "%02d:%02d:%02d", hours, minutes, seconds);
        output += " | " + string(timeStr);

        cout << "\r" << padRight(output, 70) << flush;
        Sleep(1000);
    }
}

string getTCPState(DWORD state) {
    switch (state) {
    case 1: return "ESTABLISHED";
    case 2: return "SYN_SENT";
    case 3: return "SYN_RECV";
    case 4: return "FIN_WAIT1";
    case 5: return "FIN_WAIT2";
    case 6: return "TIME_WAIT";
    case 7: return "CLOSE";
    case 8: return "CLOSE_WAIT";
    case 9: return "LAST_ACK";
    case 10: return "LISTEN";
    case 11: return "CLOSING";
    default: return "UNKNOWN";
    }
}
/// <summary>
/// Соединения
/// </summary>
void task9_Connections() {
    clearScreen();
    printHeader("NETWORK CONNECTIONS");

    while (true) {
        map<string, int> tcpStates;
        MIB_TCPTABLE* pTcpTable = NULL;
        DWORD dwSize = 0;

        GetTcpTable(NULL, &dwSize, FALSE);
        pTcpTable = (MIB_TCPTABLE*)malloc(dwSize);

        if (pTcpTable && GetTcpTable(pTcpTable, &dwSize, FALSE) == NO_ERROR) {
            for (int i = 0; i < pTcpTable->dwNumEntries; i++) {
                DWORD state = pTcpTable->table[i].dwState;
                string stateName = getTCPState(state);
                tcpStates[stateName]++;
            }
        }

        if (pTcpTable) free(pTcpTable);

        string output = "";
        int total = 0;
        for (const auto& pair : tcpStates) {
            if (!output.empty()) output += " | ";
            output += pair.first + ": " + to_string(pair.second);
            total += pair.second;
        }
        output = "Total: " + to_string(total) + " | " + output;

        cout << "\r" << padRight(output, 80) << flush;
        Sleep(1000);
    }
}
/// <summary>
///  Подключение и питание (от провода или батарейки)
/// </summary>

void task10_Temperature() {
    clearScreen();
    printHeader("THERMAL STATUS");

    while (true) {
        string output = "";

        SYSTEM_POWER_STATUS powerStatus;
        if (GetSystemPowerStatus(&powerStatus)) {
            if (powerStatus.ACLineStatus == 1) {
                output = "Power: AC | ";
            }
            else if (powerStatus.ACLineStatus == 0) {
                output = "Power: Battery | ";
            }
            else {
                output = "Power: Unknown | ";
            }

            output += "Battery: " + to_string((int)powerStatus.BatteryLifePercent) + "%";
        }

        HKEY hKey;
        char processorName[256] = { 0 };
        DWORD size = sizeof(processorName);
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            RegQueryValueEx(hKey, L"ProcessorNameString", NULL, NULL, (LPBYTE)processorName, &size);
            RegCloseKey(hKey);
        }

        if (strlen(processorName) > 0) {
            output += " | CPU: " + string(processorName);
        }

        cout << "\r" << padRight(output, 80) << flush;
        Sleep(1000);
    }
}
/// <summary>
/// Меню
/// </summary>

void showMenu() {
    clearScreen();
    cout << "\n";
    cout << "SYSTEM MONITOR - ALL TASKS\n";
    cout << "1. CPU Usage\n";
    cout << "2. RAM Monitoring\n";
    cout << "3. Network Traffic\n";
    cout << "4. Disk I/O\n";
    cout << "5. Battery Status\n";
    cout << "6. Filesystem and Swap\n";
    cout << "7. Process Dynamics\n";
    cout << "8. System Load and Uptime\n";
    cout << "9. Network Connections\n";
    cout << "10. Thermal Status\n";
    cout << "0. Exit\n";
    cout << "\nSelect task (0-10):";
    cout << "Сделанно Анисимовым Дмитрием И43";
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    int choice;

    while (true) {
        showMenu();
        cin >> choice;

        if (choice == 0) {
            cout << "\nExiting program...\n";
            break;
        }

        switch (choice) {
        case 1: task1_CPU(); break;
        case 2: task2_RAM(); break;
        case 3: task3_Network(); break;
        case 4: task4_DiskIO(); break;
        case 5: task5_Battery(); break;
        case 6: task6_Filesystem(); break;
        case 7: task7_Processes(); break;
        case 8: task8_LoadUptime(); break;
        case 9: task9_Connections(); break;
        case 10: task10_Temperature(); break;
        default:
            cout << "\nInvalid choice! Press Enter to continue...";
            cin.ignore();
            cin.get();
            continue;
        }

        break;
    }

    return 0;
}
