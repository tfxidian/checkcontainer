#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <array> // 添加这一行

// Check if /.dockerenv file exists
bool checkDockerEnvFile() {
    std::ifstream file("/.dockerenv");
    return file.good();
}

// Check if /proc/1/cgroup contains "/docker/"
bool checkCgroup() {
    std::ifstream file("/proc/1/cgroup");
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("/docker/") != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Check if the number of processes is less than 5
bool checkProcessCount() {
    std::string command = "ps aux | wc -l";
    std::string result;
    std::array<char, 128> buffer;
    std::ostringstream stream;
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe) {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            stream << buffer.data();
        }
        pclose(pipe);
    }
    result = stream.str();

    int count;
    try {
        count = std::stoi(result);
    } catch (const std::exception& e) {
        return false; // Error occurred while converting to integer, assume condition is not satisfied
    }

    return count < 5;
}

// Check if the process with process id 1 is a common initial process
bool checkInitialProcess() {
    std::ifstream file("/proc/1/comm");
    std::string line;
    std::getline(file, line);
    return (line != "init" && line != "systemd");
}

// Check if common libraries and binaries are present
bool checkCommonBinaries() {
    std::vector<std::string> binaries = {"sudo", "ls", "grep", "ps"};
    for (const auto& binary : binaries) {
        std::string command = "which " + binary + " > /dev/null 2>&1";
        if (system(command.c_str()) != 0) {
            return false;
        }
    }
    return true;
}

int main() {
    std::vector<std::string> conditions;

    if (checkDockerEnvFile()) {
        conditions.push_back("Condition: /.dockerenv exists");
    }
    if (checkCgroup()) {
        conditions.push_back("Condition: /proc/1/cgroup contains \"/docker/\"");
    }
    if (checkProcessCount()) {
        conditions.push_back("Condition: Fewer than 5 processes");
    }
    if (checkInitialProcess()) {
        conditions.push_back("Condition: Process with ID 1 is not init or systemd");
    }
    if (!checkCommonBinaries()) {
        conditions.push_back("Condition: Common libraries and binaries are not present");
    }

    if (conditions.empty()) {
        std::cout << "No conditions are satisfied. Not running inside a container." << std::endl;
    } else {
        std::cout << "The following conditions are satisfied:" << std::endl;
        for (const auto& condition : conditions) {
            std::cout << "- " << condition << std::endl;
        }
        std::cout << "Most likely running inside a container." << std::endl;
    }

    return 0;
}
