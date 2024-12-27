#include <iostream>
#include <map>
#include <functional>
#include <filesystem>

namespace fs = std::filesystem;

std::vector<std::string> split_path(const std::string& path) {
    std::vector<std::string> directories;
    std::stringstream ss(path);
    std::string dir;

    while (std::getline(ss, dir, ';')) { //  Windows uses ';' as a separator, while Linux uses ':'.
        directories.push_back(dir);
    }

    return directories;
}

std::string find_executable(const std::string& cmd) {
    const char* path = std::getenv("PATH");
    if (!path) return "";

    for (const auto& dir : split_path(path)) {
        fs::path full_path = fs::path(dir) / cmd;
        if (fs::exists(full_path)) {
            return full_path.string();
        }
    }

    return "";
}

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // commands
    std::map<std::string, std::function<void(const std::string &)>> commands = {
            {"exit", [](const std::string &input) {
                size_t pos = input.find(' ');
                if (pos != std::string::npos) {
                    std::string exitCode = input.substr(pos + 1);
                    exit(std::stoi(exitCode));
                }
            }},
            {"echo", [](const std::string &input) {
                size_t pos = input.find(' ');
                if (pos != std::string::npos) {
                    std::string text = input.substr(pos + 1);
                    std::cout << text << std::endl;
                }
            }}
    };
    commands["type"] = [&commands](const std::string& input) {
        size_t pos = input.find(' ');
        if (pos != std::string::npos) {
            std::string cmd = input.substr(pos + 1);

            // builtin
            if (commands.count(cmd)) {
                std::cout << cmd << " is a shell builtin" << std::endl;
            }
            else {
                // PATH
                std::string path = find_executable(cmd);
                if (!path.empty()) {
                    std::cout << cmd << " is " << path << std::endl;
                }
                // not exists
                else {
                    std::cout << cmd << ": not found" << std::endl;
                }
            }
        }
    };

    // REPL (Read-Eval-Print Loop)
    while (true) {
        std::cout << "$ ";

        std::string input;
        std::getline(std::cin, input);

        std::string cmd = input.substr(0, input.find(' '));
        if (commands.count(cmd)) {
            commands[cmd](input);
        }
        else {
            std::cout << input << ": command not found" << std::endl;
        }
    }
}
