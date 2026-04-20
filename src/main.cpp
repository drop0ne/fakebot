#include "ChatBot.h"

#include <iostream>
#include <string>

int main()
{
    fakebot::ChatBot bot;

    std::size_t loaded = bot.load_rules_from_file("rules.txt");

    std::cout << "FakeBot: Hello. I know "
              << bot.rule_count() << " rules. Type 'quit' to exit.\n";

    if (loaded > 0)
    {
        std::cout << "FakeBot: (Loaded " << loaded
                  << " rules from file.)\n";
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
