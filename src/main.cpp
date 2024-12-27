#include <iostream>
#include <map>
#include <functional>

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
