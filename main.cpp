#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sys/stat.h>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cmath>

static const int BUFFER_SIZE = 1024;
double eps = 1e-5;

long GetFileSize(const std::string& filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

std::unordered_set<std::string> GetFiles(const std::string& path) {
    std::unordered_set<std::string> files;
    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        files.insert(entry.path());
        std::cout << entry.path() << '\n';
    }
    return files;
}


int Compare(std::string file_1, std::string file_2, double percent) {
    auto firstSize = GetFileSize(file_1);
    auto secondSize = GetFileSize(file_2);
    if (firstSize > secondSize) {
        std::swap(file_1, file_2);
        std::swap(firstSize, secondSize);
    }

    std::system("> tmp.txt");

    const char* formatting_string_1 = "cmp -l -i %d:0 %s %s | wc -l 2> /dev/null 1>> tmp.txt";
    char command[BUFFER_SIZE];
    for (int i = 0; i < firstSize; ++i) {
        snprintf(command, BUFFER_SIZE, formatting_string_1, i, file_1.c_str(), file_2.c_str());
        std::system(command);
    }
    const char* formatting_string_2 = "cmp -l -i 0:%d %s %s | wc -l 2> /dev/null 1>> tmp.txt";
    for (int j = 0; j < secondSize; ++j) {
        snprintf(command, BUFFER_SIZE, formatting_string_2, j, file_1.c_str(), file_2.c_str());
        std::system(command);
    }

    auto file_tmp = std::ifstream("tmp.txt");
    std::string currentDiffs;
    double best = 0;
    while (getline(file_tmp, currentDiffs)) {
        if (std::stoi(currentDiffs) == 0) {
            return 100;
        }
        best = std::max(best, static_cast<double>(secondSize - std::stoi(currentDiffs)) / static_cast<double>(secondSize));
    }
    if ( best >= percent - eps) {
        return round(best * 100);
    } else {
        return 0;
    }
}

std::vector<std::pair<std::string, std::string>> identity;
std::vector<std::tuple<std::string, std::string, int>> similar;

int main(int argc, char** argv) {

    std::string dir_1 = argv[1];
    std::string dir_2 = argv[2];
    double percent = std::stod(argv[3]);

    std::unordered_set<std::string> filesInFirstDir = GetFiles(dir_1);
    std::unordered_set<std::string> filesInSecondDir = GetFiles(dir_2);

    std::unordered_set<std::string> usedInFirstDir;
    std::unordered_set<std::string> usedInSecondDir;
    std::system("> tmp.txt");
    for (const auto& first_file: filesInFirstDir) {
        for (const auto& second_file: filesInSecondDir) {
             int result = Compare(first_file, second_file, percent);
             if (result != 0) {
                 if (result == 100) {
                     identity.emplace_back(first_file, second_file);
                 } else {
                     similar.emplace_back(first_file, second_file, result);
                 }
                 usedInFirstDir.insert(first_file);
                 usedInSecondDir.insert(second_file);
             }
        }
    }
    remove("tmp.txt");

    std::cout << "Идентичны: " << '\n';
    for (const auto& [first, second]: identity) {
        std::cout << first << " - " << second << '\n';
    }
    std::cout << "Похожи(процент сходства не меньше, чем " << static_cast<int>(round(percent * 100)) <<"):\n";
    for (const auto& [first, second, per]: similar) {
        std::cout << first << " - " << second << '\n' << per;
    }

    std::cout << "Есть в директории 1, но нет в директории 2:\n";
    for (const auto& file: filesInFirstDir) {
        if (usedInFirstDir.find(file) == usedInFirstDir.end()) {
            std::cout << file << "\n";
        }
    }

    std::cout << "Есть в директории 2 но нет в директории 1:\n";
    for (const auto& file: filesInSecondDir) {
        if (usedInSecondDir.find(file) == usedInSecondDir.end()) {
            std::cout << file << "\n";
        }
    }
}

