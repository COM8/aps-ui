#pragma once

#include <chrono>
#include <string>

namespace backend::teams {
struct Token {
    const std::string tenant;
    const std::string clientId;
    const std::string clientSecret;

    std::string type{};
    std::string token{};
    std::chrono::system_clock::time_point validUntil{std::chrono::system_clock::time_point::min()};

 private:
    Token(std::string&& tenant, std::string&& clientId, std::string&& clientSecret);
    void parse_response(const std::string& response);

 public:
    [[nodiscard]] bool is_expired() const;
    [[nodiscard]] bool is_valid() const;
    bool renew();

    static Token request_new_token(std::string&& tenant, std::string&& clientId, std::string&& clientSecret);
} __attribute__((aligned(128)));
}  // namespace backend::teams