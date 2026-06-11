#include "server/HttpResponse.hpp"

#include <gtest/gtest.h>

namespace server
{

TEST(HttpResponseTests, OkCreates200Response)
{
    const HttpResponse response = HttpResponse::ok(
        "Hello",
        "text/html",
        true,
        11);

    EXPECT_EQ(response.status, 200);
    EXPECT_EQ(response.content_type, "text/html");
    EXPECT_EQ(response.body, "Hello");
    EXPECT_TRUE(response.keep_alive);
    EXPECT_EQ(response.version, 11);
}

//------------------------------------------------------------------------------

TEST(HttpResponseTests, BadRequestCreates400Response)
{
    const HttpResponse response = HttpResponse::badRequest();

    EXPECT_EQ(response.status, 400);
    EXPECT_EQ(response.content_type, "text/plain");
    EXPECT_EQ(response.body, "Bad Request\n");
    EXPECT_FALSE(response.keep_alive);
    EXPECT_EQ(response.version, 11);
}

//------------------------------------------------------------------------------

TEST(HttpResponseTests, NotFoundCreates404Response)
{
    const HttpResponse response = HttpResponse::notFound();

    EXPECT_EQ(response.status, 404);
    EXPECT_EQ(response.content_type, "text/plain");
    EXPECT_EQ(response.body, "Not Found\n");
    EXPECT_FALSE(response.keep_alive);
    EXPECT_EQ(response.version, 11);
}

//------------------------------------------------------------------------------

TEST(HttpResponseTests, MethodNotAllowedCreates405Response)
{
    const HttpResponse response = HttpResponse::methodNotAllowed();

    EXPECT_EQ(response.status, 405);
    EXPECT_EQ(response.content_type, "text/plain");
    EXPECT_EQ(response.body, "Method Not Allowed\n");
    EXPECT_FALSE(response.keep_alive);
    EXPECT_EQ(response.version, 11);
}

//------------------------------------------------------------------------------

TEST(HttpResponseTests, InternalServerErrorCreates500Response)
{
    const HttpResponse response = HttpResponse::internalServerError();

    EXPECT_EQ(response.status, 500);
    EXPECT_EQ(response.content_type, "text/plain");
    EXPECT_EQ(response.body, "Internal Server Error\n");
    EXPECT_FALSE(response.keep_alive);
    EXPECT_EQ(response.version, 11);
}

} // namespace server