#include "ChatBot.h"
#include "RuleLoader.h"
#include "TextUtils.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <stdexcept>

namespace fakebot {

namespace {

std::string join_tail(const std::vector<std::string>& tokens,
                      std::size_t start_index)
{
    std::string result;

    for (std::size_t i = start_index; i < tokens.size(); ++i)
    {
        if (!result.empty())
        {
            result += ' ';
        }

        result += tokens[i];
    }

    return result;
}

bool starts_with_tokens(const std::vector<std::string>& input_tokens,
                        std::initializer_list<std::string_view> prefix)
{
    if (input_tokens.size() < prefix.size())
    {
        return false;
    }

    std::size_t index = 0;
    for (std::string_view expected : prefix)
    {
        if (input_tokens[index] != expected)
        {
            return false;
        }
        ++index;
    }

    return true;
}

} // namespace

ChatBot::ChatBot()
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
    if (fallback_.empty())
    {
        throw std::logic_error("ChatBot requires at least one fallback response");
    }

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

std::size_t ChatBot::load_rules_from_file(const std::filesystem::path& path)
{
    LoadResult result = load_rules(path);

    if (!result.file_opened)
    {
        std::cerr << "FakeBot: Could not open rules file: "
                  << path << "\n";
        return 0;
    }

    for (const std::string& error : result.errors)
    {
        std::cerr << "FakeBot: [parse warning] " << error << "\n";
    }

    const std::size_t loaded_count = result.rules.size();

    for (Rule& rule : result.rules)
    {
        rules_.push_back(std::move(rule));
    }

    sort_rules();
    return loaded_count;
}

void ChatBot::add_rule(std::string_view trigger_phrase,
                       std::vector<std::string> responses)
{
    rules_.push_back(
        Rule{ tokenize(normalize(std::string(trigger_phrase))),
              std::move(responses) });
}

void ChatBot::sort_rules()
{
    std::ranges::stable_sort(
        rules_,
        std::greater<>{},
        [](const Rule& rule)
        {
            return rule.trigger_tokens.size();
        });
}

std::optional<std::string> ChatBot::reply_to(const std::string& input)
{
    const std::string clean = normalize(input);

    if (clean == "quit" || clean == "exit" || clean == "bye")
    {
        return std::nullopt;
    }

    const std::vector<std::string> tokens = tokenize(clean);

    if (auto response = try_patterns(tokens); response.has_value())
    {
        last_response_ = *response;
        return response;
    }

    if (auto response = try_rules(tokens); response.has_value())
    {
        last_response_ = *response;
        return response;
    }

    std::string response = make_fallback();
    last_response_ = response;
    return response;
}

std::optional<std::string> ChatBot::try_patterns(
    const std::vector<std::string>& input_tokens)
{
    if (starts_with_tokens(input_tokens, { "my", "name", "is" }) &&
        input_tokens.size() > 3)
    {
        std::string name = join_tail(input_tokens, 3);
        user_name_ = name;
        return "Nice to meet you, " + name + ".";
    }

    if (starts_with_tokens(input_tokens, { "i", "am" }) &&
        input_tokens.size() > 2)
    {
        return "How long have you been " + join_tail(input_tokens, 2) + "?";
    }

    if (starts_with_tokens(input_tokens, { "i", "like" }) &&
        input_tokens.size() > 2)
    {
        return "What do you like about " + join_tail(input_tokens, 2) + "?";
    }

    if (starts_with_tokens(input_tokens, { "i", "hate" }) &&
        input_tokens.size() > 2)
    {
        return "Why do you hate " + join_tail(input_tokens, 2) + "?";
    }

    return std::nullopt;
}

std::optional<std::string> ChatBot::try_rules(
    const std::vector<std::string>& input_tokens)
{
    for (const Rule& rule : rules_)
    {
        if (rule.trigger_tokens.empty() || rule.responses.empty())
        {
            continue;
        }

        auto match = std::ranges::search(input_tokens, rule.trigger_tokens);
        if (match.begin() != input_tokens.end())
        {
            if (rule.trigger_tokens.size() == 1 &&
                (rule.trigger_tokens[0] == "hello" ||
                 rule.trigger_tokens[0] == "hi"))
            {
                if (user_name_.has_value())
                {
                    return "Hello, " + *user_name_ + ".";
                }
            }

            return pick_response(rule.responses);
        }
    }

    return std::nullopt;
}

std::string ChatBot::make_fallback() const
{
    if (fallback_.empty())
    {
        return personalize("I am not sure what to say.");
    }

    return personalize(random_choice(fallback_));
}

std::string ChatBot::personalize(const std::string& response) const
{
    if (user_name_.has_value())
    {
        return *user_name_ + ", " + response;
    }

    return response;
}

std::string ChatBot::pick_response(const std::vector<std::string>& options)
{
    for (int attempt = 0; attempt < 3; ++attempt)
    {
        std::string choice = random_choice(options);

        if (choice != last_response_ || options.size() == 1)
        {
            return choice;
        }
    }

    return random_choice(options);
}

std::size_t ChatBot::rule_count() const
{
    return rules_.size();
}

} // namespace fakebot
