#include <iostream>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // REPL (Read-Eval-Print Loop)
  while(true) {
      // Uncomment this block to pass the first stage
      std::cout << "$ ";

      std::string input;
      std::getline(std::cin, input);

      // exit
      if (input.substr(0, 4) == "exit") {
          size_t spacePos = input.find(' ');
          if (spacePos != std::string::npos) {
              std::string exitCode = input.substr(spacePos + 1);
              exit(std::stoi(exitCode));
          }
      }

      std::cout << input << ": command not found" << std::endl;
  }
}
