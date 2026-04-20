#pragma once

#include "Rule.h"

#include <filesystem>
#include <string>
#include <vector>

namespace fakebot {

struct LoadResult
{
    std::vector<Rule> rules;
    std::vector<std::string> errors;
    bool file_opened = false;
};

LoadResult load_rules(const std::filesystem::path& path);

} // namespace fakebot
