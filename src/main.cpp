#include <iostream>
#include <map>
#include <functional>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

namespace fs = std::filesystem;

std::vector<std::string> split_path(const std::string& path) {
    std::vector<std::string> directories;
    std::stringstream ss(path);
    std::string dir;

    while (std::getline(ss, dir, ':')) { //  Windows uses ';' as a separator, while Linux uses ':'.
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

std::vector<std::string> parse_command(const std::string& input) {
    std::vector<std::string> args;
    std::string current_arg;
    bool in_quotes = false;
    char quote_char = 0;

    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i];

        if ((c == '\'' || c == '"') && (!in_quotes || c == quote_char)) {
            if (in_quotes) {
                in_quotes = false;
                quote_char = 0;
            } else {
                in_quotes = true;
                quote_char = c;
            }
            continue;
        }

        if (std::isspace(c) && !in_quotes) {
            if (!current_arg.empty()) {
                args.push_back(current_arg);
                current_arg.clear();
            }
        } else {
            current_arg += c;
        }
    }

    if (!current_arg.empty()) {
        args.push_back(current_arg);
    }

    return args;
}

void execute_external_command(const std::string& input) {
    std::vector<std::string> args = parse_command(input);
    if (args.empty()) return;

    std::string program_path = find_executable(args[0]);
    if (program_path.empty()) {
        std::cout << args[0] << ": command not found" << std::endl;
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {  // Child process
        std::vector<char*> c_args;
        for (const auto& arg : args) {
            c_args.push_back(strdup(arg.c_str()));
        }
        c_args.push_back(nullptr);

        execv(program_path.c_str(), c_args.data());
        exit(1);  // execv only returns on error
    }
    else if (pid > 0) {  // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
}

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // commands
    std::map<std::string, std::function<void(const std::string &)>> commands = {
            {"exit", [](const std::string &input) {
                auto args = parse_command(input);
                if (args.size() > 1) {
                    exit(std::stoi(args[1]));
                }
                exit(0);
            }},
            {"echo", [](const std::string &input) {
                size_t pos = input.find(' ');
                if (pos != std::string::npos) {
                    std::cout << input.substr(pos + 1) << std::endl;
                }
            }},
            {"type", [&commands](const std::string& input) {
                auto args = parse_command(input);
                if (args.size() > 1) {
                    if (commands.count(args[1])) {
                        std::cout << args[1] << " is a shell builtin" << std::endl;
                    } else {
                        std::string path = find_executable(args[1]);
                        if (!path.empty()) {
                            std::cout << args[1] << " is " << path << std::endl;
                        } else {
                            std::cout << args[1] << ": not found" << std::endl;
                        }
                    }
                }
            }},

        // Navigation

            // print working directory
            {"pwd", [](const std::string &input) {
                std::cout << fs::current_path().string() << std::endl;
            }},

            // change directory
            {"cd", [](const std::string &input) {
                const char* home = std::getenv("HOME");
                auto args = parse_command(input);
                std::string path;

                if (args.size() > 1) {
                    path = args[1];
                    // start with '~'
                    if (path[0] == '~' && home != nullptr) {
                        if (path == "~") {
                            path = home;
                        } else {
                            path = std::string(home) + path.substr(1);
                        }
                    }
                }
                // without argument
                else if (home != nullptr) {
                    path = home;
                }

                try {
                    fs::current_path(path);
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "cd: " << path << ": No such file or directory" << std::endl;
                }
            }}
    };

    // REPL (Read-Eval-Print Loop)
    while (true) {
        std::cout << "$ ";

        std::string input;
        std::getline(std::cin, input);

        auto args = parse_command(input);
        if (args.empty()) {
            continue;
        }

        std::string cmd = args[0];
        if (commands.count(cmd)) {
            commands[cmd](input);
        }
        else {
            execute_external_command(input);
        }
    }
}
