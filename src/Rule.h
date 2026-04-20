#pragma once

#include <string>
#include <vector>

namespace fakebot {

struct Rule
{
    std::vector<std::string> trigger_tokens;
    std::vector<std::string> responses;
};

} // namespace fakebot
