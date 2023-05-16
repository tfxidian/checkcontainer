#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <array>

// Execute a command and return the output as a string
std::string executeCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::ostringstream stream;
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe) {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            stream << buffer.data();
        }
        pclose(pipe);
    }
    return stream.str();
}

// Check current user
void checkCurrentUser() {
    std::cout << "Current User: ";
    std::cout << executeCommand("id");
}

// Check available users on the system
void checkAvailableUsers() {
    std::cout << "Available Users: ";
    std::ifstream file("/etc/passwd");
    std::string line;
    while (std::getline(file, line, ':')) {
        std::cout << line << " ";
    }
    std::cout << std::endl;
}

// Check container OS
void checkContainerOS() {
    std::cout << "Container OS: ";
    std::ifstream file("/etc/os-release");
    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << std::endl;
    }
}

// Check running processes
void checkRunningProcesses() {
    std::cout << "Running Processes:" << std::endl;
    std::cout << executeCommand("ps aux");
}

// Check host OS
void checkHostOS() {
    std::cout << "Host OS: ";
    std::cout << executeCommand("uname -a");
}

// Check process capabilities
void checkProcessCapabilities() {
    std::cout << "Process Capabilities: ";
    std::cout << executeCommand("grep CapEff /proc/self/status");
}

// Check if the container is running in privileged mode
bool checkPrivilegedMode() {
    std::string result = executeCommand("grep CapEff /proc/self/status");
    return result == "CapEff: 0000003fffffffff\n";
}

// Check mounted volumes
void checkMountedVolumes() {
    std::cout << "Mounted Volumes:" << std::endl;
    std::ifstream file("/proc/mounts");
    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << std::endl;
    }
}

// Check sensitive information in environment variables
void checkSensitiveEnvVariables() {
    std::cout << "Sensitive Environment Variables:" << std::endl;
    std::cout << executeCommand("env");
}

// Check if Docker Socket is mounted inside the container
bool checkDockerSocket() {
    std::ifstream file("/proc/mounts");
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("docker.sock") != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Check reachable hosts on the network
void checkReachableHosts() {
    std::string containerIP;
    std::ifstream file("/etc/hosts");
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("containerIP") != std::string::npos) {
            std::istringstream iss(line);
            std::string ip;
            iss >> ip;
            containerIP = ip;
            break;
        }
    }

    if (!containerIP.empty()) {

        std::string command = "nmap -sn " + containerIP + "/24";
        std::cout << "Reachable Hosts:" << std::endl;
        std::cout << executeCommand(command);
    } else {
        std::cout << "Container IP not found in /etc/hosts." << std::endl;
    }
}

int main() {
    checkCurrentUser();
    checkAvailableUsers();
    checkContainerOS();
    //checkRunningProcesses();
    checkHostOS();
    checkProcessCapabilities();
    if (checkPrivilegedMode()) {
        std::cout << "Container is running in privileged mode." << std::endl;
    }

    checkMountedVolumes();
    checkSensitiveEnvVariables();

    if (checkDockerSocket()) {
        std::cout << "Docker Socket is mounted inside the container." << std::endl;
    }

    checkReachableHosts();

    return 0;
}
