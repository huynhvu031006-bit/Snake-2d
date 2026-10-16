#pragma once
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace snake {
class Scores {
    std::filesystem::path path_;
    std::vector<int> values_;
public:
    explicit Scores(std::filesystem::path path) : path_(std::move(path)) {
        std::ifstream in(path_);
        std::string line;
        while (std::getline(in, line)) {
            try {
                std::size_t used;
                int value = std::stoi(line, &used);
                if (used == line.size() && value >= 0 && value % 10 == 0) values_.push_back(value);
            } catch (const std::exception&) {}
        }
        trim();
    }
    void trim() {
        if (values_.size() > 20) values_.resize(20);
    }
    bool add(int score) {
        if (score < 0 || score % 10 != 0) return false;
        values_.insert(values_.begin(), score); trim();
        auto temporary = path_; temporary += ".tmp";
        std::ofstream out(temporary, std::ios::trunc);
        for (int value : values_) out << value << '\n';
        out.close();
        if (!out) return false;
        // Windows rename does not replace an existing file.
        return replace(temporary);
    }
    bool clear() {
        auto temporary = path_; temporary += ".tmp";
        std::ofstream out(temporary, std::ios::trunc);
        out.close();
        if (!out || !replace(temporary)) return false;
        values_.clear();
        return true;
    }
    bool replace(const std::filesystem::path& temporary);
    const std::vector<int>& values() const { return values_; }
};
}



