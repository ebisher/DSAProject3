#include <iostream>
#include <string>
#include "campusCompass.h"

int main() {
    CampusCompass cc;

    cc.loadEdges("data/edges.csv");
    cc.loadClasses("data/classes.csv");

    int numCommands;
    std::cin >> numCommands;
    std::cin.ignore();

    for (int i = 0; i < numCommands; ++i) {
        std::string line;
        std::getline(std::cin, line);

        if (line.empty()) {
            --i;
            continue;
        }

        cc.processLine(line);
    }

    return 0;
}