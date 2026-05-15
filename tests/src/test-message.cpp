#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "nlohmann/json.hpp"
#include "type.hpp"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static nlohmann::json makeUser()
{
    return {
        {"id", 123456789ULL},
        {"is_bot", false},
        {"first_name", "Test"},
        {"username", "testuser"}};
}

static nlohmann::json makeChat(const std::string &type = "private")
{
    nlohmann::json c = {
        {"id", 987654321ULL},
        {"type", type},
        {"first_name", "Test"},
        {"username", "testuser"}};
    if (type != "private")
    {
        c.erase("first_name");
        c.erase("username");
        c["title"] = "Test Group";
    }
    return c;
}

// ---------------------------------------------------------------------------
// Message::parse — date and message_id
// ---------------------------------------------------------------------------

TEST_CASE("Message::parse populates id and dtime from integer fields")
{
    nlohmann::json j = {
        {"message_id", 42ULL},
        {"date", 1700000000ULL},
        {"from", makeUser()},
        {"chat", makeChat()},
        {"text", "hello"}};

    Message m;
    CHECK(m.parse(j));
    CHECK(m.id == 42LL);
    CHECK(static_cast<long long>(m.dtime) == 1700000000LL);
}

// ---------------------------------------------------------------------------
// Message::parse — message_thread_id (integer in Telegram API)
// ---------------------------------------------------------------------------

TEST_CASE("Message::parse sets threadId from integer message_thread_id")
{
    nlohmann::json j = {
        {"message_id", 7ULL},
        {"date", 1700000001ULL},
        {"message_thread_id", 99ULL},
        {"from", makeUser()},
        {"chat", makeChat()},
        {"text", "forum reply"}};

    Message m;
    CHECK(m.parse(j));
    CHECK(m.threadId == 99LL);
}

TEST_CASE("Message::parse sets threadId to 0 when message_thread_id is absent")
{
    nlohmann::json j = {
        {"message_id", 8ULL},
        {"date", 1700000002ULL},
        {"from", makeUser()},
        {"chat", makeChat()},
        {"text", "regular message"}};

    Message m;
    CHECK(m.parse(j));
    CHECK(m.threadId == 0LL);
}

// ---------------------------------------------------------------------------
// Message::parse — text and caption
// ---------------------------------------------------------------------------

TEST_CASE("Message::parse captures text field")
{
    nlohmann::json j = {
        {"message_id", 10ULL},
        {"date", 1700000003ULL},
        {"from", makeUser()},
        {"chat", makeChat()},
        {"text", "hello world"}};

    Message m;
    CHECK(m.parse(j));
    CHECK(m.text == "hello world");
    CHECK(m.caption.empty());
}

TEST_CASE("Message::parse captures caption field when text is absent")
{
    nlohmann::json j = {
        {"message_id", 11ULL},
        {"date", 1700000004ULL},
        {"from", makeUser()},
        {"chat", makeChat()},
        {"caption", "photo caption"}};

    Message m;
    CHECK(m.parse(j));
    CHECK(m.caption == "photo caption");
}

// ---------------------------------------------------------------------------
// Message::parse — empty() contract
// ---------------------------------------------------------------------------

TEST_CASE("Message is empty before parse and non-empty after successful parse")
{
    Message m;
    CHECK(m.empty());

    nlohmann::json j = {
        {"message_id", 1ULL},
        {"date", 1700000005ULL},
        {"from", makeUser()},
        {"chat", makeChat()},
        {"text", "x"}};

    CHECK(m.parse(j));
    CHECK_FALSE(m.empty());
}

TEST_CASE("Message::parse returns false and resets on missing mandatory field")
{
    nlohmann::json j = {
        {"date", 1700000006ULL},
        {"from", makeUser()},
        {"chat", makeChat()}
        // "message_id" intentionally absent
    };

    Message m;
    CHECK_FALSE(m.parse(j));
    CHECK(m.empty());
}
