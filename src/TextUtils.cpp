#include "TextUtils.h"

#include <algorithm>
#include <cctype>
#include <random>
#include <stdexcept>
#include <sstream>

namespace fakebot {

std::string to_lower(std::string text)
{
    std::ranges::transform(text, text.begin(),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return text;
}

std::string strip_punctuation(std::string text)
{
    std::erase_if(text, [](unsigned char ch) { return std::ispunct(ch) != 0; });
    return text;
}

std::string trim(std::string text)
{
    auto not_space = [](unsigned char ch) { return std::isspace(ch) == 0; };

    auto first = std::ranges::find_if(text, not_space);
    auto last  = std::find_if(text.rbegin(), text.rend(), not_space).base();

    if (first >= last)
    {
        return "";
    }

    return std::string(first, last);
}

std::string normalize(std::string text)
{
    return strip_punctuation(to_lower(std::move(text)));
}

bool contains_phrase(std::string_view text, std::string_view phrase)
{
    return text.find(phrase) != std::string_view::npos;
}

std::optional<std::string> extract_tail(std::string_view text,
                                        std::string_view prefix)
{
    if (!text.starts_with(prefix))
    {
        return std::nullopt;
    }

    std::string tail = trim(std::string(text.substr(prefix.size())));

    if (tail.empty())
    {
        return std::nullopt;
    }

    return tail;
}

std::string random_choice(const std::vector<std::string>& options)
{
    if (options.empty())
    {
        throw std::invalid_argument("random_choice called with empty vector");
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<std::size_t> dist(0, options.size() - 1);
    return options[dist(gen)];
}

std::vector<std::string> tokenize(std::string_view text)
{
    std::vector<std::string> result;

    std::string str_copy(text);
    std::istringstream iss(str_copy);

    std::string word;
    while (iss >> word)
    {
        result.push_back(word);
    }

    return result;
}

} // namespace fakebot
