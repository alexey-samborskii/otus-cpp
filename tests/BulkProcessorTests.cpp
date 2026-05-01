#include "Formatter.hpp"
#include "CommandBlockProcessor.hpp"
#include "ICommandBlockHandler.hpp"

#include <gtest/gtest.h>

#include <ctime>
#include <string>
#include <vector>

namespace
{

class CollectingBulkHandler : public bulk::ICommandBlockHandler
{
public:
    void handle(const bulk::CommandBlock& bulk) override
    {
        bulks.push_back(bulk);
    }

    std::vector<bulk::CommandBlock> bulks;
};

//------------------------------------------------------------------------------

std::vector<std::string> formatBulks(const std::vector<bulk::CommandBlock>& bulks)
{
    std::vector<std::string> result;

    for (const auto& bulk : bulks)
    {
        result.push_back(bulk::Formatter::format(bulk));
    }

    return result;
}

//------------------------------------------------------------------------------

TEST(CommandBlockProcessor, StaticBlocksFromTaskExample)
{
    CollectingBulkHandler handler;

    std::time_t now = 100;

    bulk::CommandBlockProcessor processor(
        3,
        handler,
        [&now] {
            return now;
        });

    processor.processLine("cmd1");
    processor.processLine("cmd2");
    processor.processLine("cmd3");

    now = 200;

    processor.processLine("cmd4");
    processor.processLine("cmd5");

    processor.finish();

    const std::vector<std::string> expected_output = {
        "bulk: cmd1, cmd2, cmd3",
        "bulk: cmd4, cmd5",
    };

    EXPECT_EQ(expected_output, formatBulks(handler.bulks));

    ASSERT_EQ(2u, handler.bulks.size());

    EXPECT_EQ(100, handler.bulks[0].timestamp);
    EXPECT_EQ(200, handler.bulks[1].timestamp);
}

//------------------------------------------------------------------------------

TEST(CommandBlockProcessor, DynamicBlocksFromTaskExample)
{
    CollectingBulkHandler handler;

    std::time_t now = 100;

    bulk::CommandBlockProcessor processor(
        3,
        handler,
        [&now] {
            return now;
        });

    processor.processLine("cmd1");
    processor.processLine("cmd2");

    now = 200;

    processor.processLine("{");
    processor.processLine("cmd3");
    processor.processLine("cmd4");
    processor.processLine("}");

    now = 300;

    processor.processLine("{");
    processor.processLine("cmd5");
    processor.processLine("cmd6");

    processor.processLine("{");
    processor.processLine("cmd7");
    processor.processLine("cmd8");
    processor.processLine("}");

    processor.processLine("cmd9");
    processor.processLine("}");

    now = 400;

    processor.processLine("{");
    processor.processLine("cmd10");
    processor.processLine("cmd11");

    processor.finish();

    const std::vector<std::string> expected_output = {
        "bulk: cmd1, cmd2",
        "bulk: cmd3, cmd4",
        "bulk: cmd5, cmd6, cmd7, cmd8, cmd9",
    };

    EXPECT_EQ(expected_output, formatBulks(handler.bulks));

    ASSERT_EQ(3u, handler.bulks.size());

    EXPECT_EQ(100, handler.bulks[0].timestamp);
    EXPECT_EQ(200, handler.bulks[1].timestamp);
    EXPECT_EQ(300, handler.bulks[2].timestamp);
}

//------------------------------------------------------------------------------

TEST(CommandBlockProcessor, DynamicBlockIgnoresStaticLimit)
{
    CollectingBulkHandler handler;

    std::time_t now = 100;

    bulk::CommandBlockProcessor processor(
        3,
        handler,
        [&now] {
            return now;
        });

    processor.processLine("{");
    processor.processLine("cmd1");
    processor.processLine("cmd2");
    processor.processLine("cmd3");
    processor.processLine("cmd4");

    EXPECT_TRUE(handler.bulks.empty());

    processor.processLine("}");

    const std::vector<std::string> expected_output = {
        "bulk: cmd1, cmd2, cmd3, cmd4",
    };

    EXPECT_EQ(expected_output, formatBulks(handler.bulks));
}

//------------------------------------------------------------------------------

TEST(CommandBlockProcessor, UnclosedDynamicBlockIsIgnoredOnFinish)
{
    CollectingBulkHandler handler;

    std::time_t now = 100;

    bulk::CommandBlockProcessor processor(
        3,
        handler,
        [&now] {
            return now;
        });

    processor.processLine("{");
    processor.processLine("cmd1");
    processor.processLine("cmd2");

    processor.finish();

    EXPECT_TRUE(handler.bulks.empty());
}

//------------------------------------------------------------------------------

TEST(Formatter, FormatsBulk)
{
    bulk::CommandBlock test_bulk;
    test_bulk.timestamp = 100;
    test_bulk.commands  = {"cmd1", "cmd2", "cmd3"};

    EXPECT_EQ("bulk: cmd1, cmd2, cmd3",
              bulk::Formatter::format(test_bulk));
}

} // namespace