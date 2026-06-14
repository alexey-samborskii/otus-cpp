#include "server/HttpResponse.hpp"

#include <gtest/gtest.h>

#include <string>

namespace server
{

namespace
{

void expectResponse(
    const HttpResponse &response,
    unsigned            expected_status,
    const std::string  &expected_content_type,
    const std::string  &expected_body,
    bool                expected_keep_alive,
    unsigned            expected_version)
{
    EXPECT_EQ(response.status, expected_status);
    EXPECT_EQ(response.content_type, expected_content_type);
    EXPECT_EQ(response.body, expected_body);
    EXPECT_EQ(response.keep_alive, expected_keep_alive);
    EXPECT_EQ(response.version, expected_version);
}

} // namespace

//------------------------------------------------------------------------------

TEST(HttpResponseTests, OkCreates200Response)
{
    const HttpResponse response = HttpResponse::ok(
        "Hello",
        "text/html; charset=utf-8",
        true,
        10);

    expectResponse(
        response,
        200,
        "text/html; charset=utf-8",
        "Hello",
        true,
        10);

    EXPECT_TRUE(response.headers.empty());
}

//------------------------------------------------------------------------------

TEST(HttpResponseTests, CreatedCreates201ResponseWithLocationHeader)
{
    const HttpResponse response = HttpResponse::created(
        R"({"id":42})",
        "/api/tasks/42",
        true,
        11);

    expectResponse(
        response,
        201,
        "application/json; charset=utf-8",
        R"({"id":42})",
        true,
        11);

    ASSERT_EQ(response.headers.size(), 1U);
    EXPECT_EQ(response.headers[0].first, "Location");
    EXPECT_EQ(response.headers[0].second, "/api/tasks/42");
}

//------------------------------------------------------------------------------

TEST(HttpResponseTests, NoContentCreates204Response)
{
    const HttpResponse response = HttpResponse::noContent(
        false,
        11);

    expectResponse(
        response,
        204,
        "",
        "",
        false,
        11);

    EXPECT_TRUE(response.headers.empty());
}

//------------------------------------------------------------------------------

TEST(HttpResponseTests, BadRequestCreates400Response)
{
    const HttpResponse response = HttpResponse::badRequest(
        R"({"error":"Bad Request"})",
        true,
        10);

    expectResponse(
        response,
        400,
        "application/json; charset=utf-8",
        R"({"error":"Bad Request"})",
        true,
        10);

    EXPECT_TRUE(response.headers.empty());
}

//------------------------------------------------------------------------------

TEST(HttpResponseTests, NotFoundCreates404Response)
{
    const HttpResponse response = HttpResponse::notFound(
        R"({"error":"Not Found"})",
        false,
        11);

    expectResponse(
        response,
        404,
        "application/json; charset=utf-8",
        R"({"error":"Not Found"})",
        false,
        11);

    EXPECT_TRUE(response.headers.empty());
}

//------------------------------------------------------------------------------

TEST(HttpResponseTests, MethodNotAllowedCreates405Response)
{
    const HttpResponse response = HttpResponse::methodNotAllowed(
        R"({"error":"Method Not Allowed"})",
        true,
        10);

    expectResponse(
        response,
        405,
        "application/json; charset=utf-8",
        R"({"error":"Method Not Allowed"})",
        true,
        10);

    EXPECT_TRUE(response.headers.empty());
}

//------------------------------------------------------------------------------

TEST(HttpResponseTests, ConflictCreates409Response)
{
    const HttpResponse response = HttpResponse::conflict(
        R"({"error":"Task already exists"})",
        false,
        11);

    expectResponse(
        response,
        409,
        "application/json; charset=utf-8",
        R"({"error":"Task already exists"})",
        false,
        11);

    EXPECT_TRUE(response.headers.empty());
}

//------------------------------------------------------------------------------

TEST(HttpResponseTests, InternalServerErrorCreates500Response)
{
    const HttpResponse response = HttpResponse::internalServerError(
        R"({"error":"Internal Server Error"})",
        false,
        11);

    expectResponse(
        response,
        500,
        "application/json; charset=utf-8",
        R"({"error":"Internal Server Error"})",
        false,
        11);

    EXPECT_TRUE(response.headers.empty());
}

} // namespace server