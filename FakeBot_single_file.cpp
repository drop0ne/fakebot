#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace fakebot {

struct Rule
{
    std::vector<std::string> trigger_tokens;
    std::vector<std::string> responses;
};

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

struct LoadResult
{
    std::vector<Rule> rules;
    std::vector<std::string> errors;
    bool file_opened = false;
};

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
            result.errors.push_back("Rule with trigger has no responses near line " + std::to_string(line_number));
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

        std::string cleaned = trim(line);
        if (cleaned.starts_with("trigger:"))
        {
            flush_rule();
            std::string phrase = trim(std::string(std::string_view(cleaned).substr(8)));
            if (!phrase.empty())
            {
                trigger_tokens = tokenize(normalize(phrase));
            }
            continue;
        }

        if (cleaned.starts_with("- "))
        {
            std::string response = trim(std::string(std::string_view(cleaned).substr(2)));
            if (!response.empty() && !trigger_tokens.empty())
            {
                responses.push_back(std::move(response));
            }
            continue;
        }
    }

    flush_rule();
    return result;
}

class ChatBot
{
public:
    ChatBot()
        : fallback_{
            "Tell me more.",
            "Why do you say that?",
            "That is interesting.",
            "How does that make you feel?",
            "Can you elaborate?",
            "I see. Go on.",
            "What makes you say that?"
          }
    {
        add_rule("how are you", {
            "I am functioning within normal parameters.",
            "Pretty good for a pile of if-statements.",
            "I feel chatty."
        });
        add_rule("whats your name", {
            "I am FakeBot. I am not very smart, but I try.",
            "Call me FakeBot.",
            "FakeBot. Version 0.4, approximately."
        });
        add_rule("hello", { "Hello.", "Hi there.", "Hey." });
        add_rule("hi",    { "Hello.", "Hi there.", "Hi." });
        sort_rules();
    }

    std::size_t load_rules_from_file(const std::filesystem::path& path)
    {
        LoadResult result = load_rules(path);
        if (!result.file_opened)
        {
            return 0;
        }

        for (Rule& rule : result.rules)
        {
            rules_.push_back(std::move(rule));
        }

        sort_rules();
        return result.rules.size();
    }

    std::optional<std::string> reply_to(const std::string& input)
    {
        const std::string clean = normalize(input);
        if (clean == "quit" || clean == "exit" || clean == "bye")
        {
            return std::nullopt;
        }

        const auto tokens = tokenize(clean);

        if (starts_with_tokens(tokens, {"my", "name", "is"}) && tokens.size() > 3)
        {
            std::string name = join_tail(tokens, 3);
            user_name_ = name;
            last_response_ = "Nice to meet you, " + name + ".";
            return last_response_;
        }

        if (starts_with_tokens(tokens, {"i", "am"}) && tokens.size() > 2)
        {
            last_response_ = "How long have you been " + join_tail(tokens, 2) + "?";
            return last_response_;
        }

        for (const Rule& rule : rules_)
        {
            auto match = std::ranges::search(tokens, rule.trigger_tokens);
            if (match.begin() != tokens.end())
            {
                if (rule.trigger_tokens.size() == 1 &&
                    (rule.trigger_tokens[0] == "hello" || rule.trigger_tokens[0] == "hi") &&
                    user_name_.has_value())
                {
                    last_response_ = "Hello, " + *user_name_ + ".";
                    return last_response_;
                }

                last_response_ = random_choice(rule.responses);
                return last_response_;
            }
        }

        last_response_ = user_name_.has_value()
            ? *user_name_ + ", " + random_choice(fallback_)
            : random_choice(fallback_);
        return last_response_;
    }

    std::size_t rule_count() const
    {
        return rules_.size();
    }

private:
    std::vector<Rule> rules_;
    std::vector<std::string> fallback_;
    std::optional<std::string> user_name_;
    std::string last_response_;

    void add_rule(std::string_view trigger_phrase, std::vector<std::string> responses)
    {
        rules_.push_back(Rule{tokenize(normalize(std::string(trigger_phrase))), std::move(responses)});
    }

    void sort_rules()
    {
        std::ranges::stable_sort(rules_, std::greater<>{}, [](const Rule& r) {
            return r.trigger_tokens.size();
        });
    }

    static std::string join_tail(const std::vector<std::string>& tokens, std::size_t start)
    {
        std::string result;
        for (std::size_t i = start; i < tokens.size(); ++i)
        {
            if (!result.empty())
            {
                result += ' ';
            }
            result += tokens[i];
        }
        return result;
    }

    static bool starts_with_tokens(const std::vector<std::string>& tokens,
                                   std::initializer_list<std::string_view> prefix)
    {
        if (tokens.size() < prefix.size())
        {
            return false;
        }

        std::size_t i = 0;
        for (auto item : prefix)
        {
            if (tokens[i] != item)
            {
                return false;
            }
            ++i;
        }
        return true;
    }
};

} // namespace fakebot

int main()
{
    fakebot::ChatBot bot;
    std::size_t loaded = bot.load_rules_from_file("rules.txt");

    std::cout << "FakeBot: Hello. I know "
              << bot.rule_count() << " rules. Type 'quit' to exit.\n";

    if (loaded > 0)
    {
        std::cout << "FakeBot: (Loaded " << loaded << " rules from file.)\n";
    }

    for (;;)
    {
        std::cout << "You: ";
        std::string input;
        if (!std::getline(std::cin, input))
        {
            break;
        }

        auto response = bot.reply_to(input);
        if (!response.has_value())
        {
            std::cout << "FakeBot: Goodbye.\n";
            break;
        }

        std::cout << "FakeBot: " << *response << "\n";
    }

    return 0;
}
