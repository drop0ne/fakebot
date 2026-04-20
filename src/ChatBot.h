#pragma once

#include "Rule.h"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace fakebot {

class ChatBot
{
public:
    ChatBot();

    std::size_t load_rules_from_file(const std::filesystem::path& path);
    std::optional<std::string> reply_to(const std::string& input);
    std::size_t rule_count() const;

private:
    std::vector<Rule> rules_;
    std::vector<std::string> fallback_;
    std::optional<std::string> user_name_;
    std::string last_response_;

    void add_rule(std::string_view trigger_phrase,
                  std::vector<std::string> responses);

    void sort_rules();

    std::optional<std::string> try_patterns(
        const std::vector<std::string>& input_tokens);
    std::optional<std::string> try_rules(
        const std::vector<std::string>& input_tokens);

    std::string make_fallback() const;
    std::string personalize(const std::string& response) const;
    std::string pick_response(const std::vector<std::string>& options);
};

} // namespace fakebot
