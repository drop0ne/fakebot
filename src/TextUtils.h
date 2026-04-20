#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace fakebot {

std::string to_lower(std::string text);
std::string strip_punctuation(std::string text);
std::string trim(std::string text);
std::string normalize(std::string text);
std::vector<std::string> tokenize(std::string_view text);

bool contains_phrase(std::string_view text, std::string_view phrase);
std::optional<std::string> extract_tail(std::string_view text, std::string_view prefix);
std::string random_choice(const std::vector<std::string>& options);

} // namespace fakebot
