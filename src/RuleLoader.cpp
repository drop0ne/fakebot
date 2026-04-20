#include "RuleLoader.h"
#include "TextUtils.h"

#include <cctype>
#include <fstream>
#include <string>

namespace fakebot {

static bool is_ignorable_line(std::string_view line)
{
    while (!line.empty() && std::isspace(static_cast<unsigned char>(line.front())) != 0)
    {
        line.remove_prefix(1);
    }

    return line.empty() || line.starts_with('#');
}

LoadResult load_rules(const std::filesystem::path& path)
{
    LoadResult result;

    std::ifstream file(path);
    if (!file.is_open())
    {
        result.file_opened = false;
        return result;
    }

    result.file_opened = true;

    std::vector<std::string> trigger_tokens;
    std::vector<std::string> responses;
    int line_number = 0;

    auto flush_rule = [&]()
    {
        if (trigger_tokens.empty())
        {
            return;
        }

        if (responses.empty())
        {
            result.errors.push_back(
                "Rule with trigger has no responses near line " + std::to_string(line_number));
        }
        else
        {
            result.rules.push_back(Rule{std::move(trigger_tokens), std::move(responses)});
        }

        trigger_tokens.clear();
        responses.clear();
    };

    std::string line;
    while (std::getline(file, line))
    {
        ++line_number;

        if (is_ignorable_line(line))
        {
            continue;
        }

        std::string trimmed = trim(line);

        if (trimmed.starts_with("trigger:"))
        {
            flush_rule();

            std::string phrase = trim(std::string(std::string_view(trimmed).substr(8)));
            if (phrase.empty())
            {
                result.errors.push_back("Empty trigger phrase at line " + std::to_string(line_number));
                continue;
            }

            trigger_tokens = tokenize(normalize(phrase));
            continue;
        }

        if (trimmed.starts_with("- "))
        {
            std::string response = trim(std::string(std::string_view(trimmed).substr(2)));
            if (response.empty())
            {
                result.errors.push_back("Empty response at line " + std::to_string(line_number));
                continue;
            }

            if (trigger_tokens.empty())
            {
                result.errors.push_back("Response without a trigger at line " + std::to_string(line_number));
                continue;
            }

            responses.push_back(std::move(response));
            continue;
        }

        result.errors.push_back(
            "Unrecognized line format at line " + std::to_string(line_number) + ": " + trimmed);
    }

    flush_rule();
    return result;
}

} // namespace fakebot
