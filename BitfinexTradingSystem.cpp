#include <iostream>
#include <string>
#include <ctime>
#include <curl/curl.h>
#include <cstdlib>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <limits>
#include "json.hpp"

using json = nlohmann::json;

class BitfinexAPI {
private:
    std::string api_key;
    std::string api_secret;
    std::string base_url = "https://api-pub.bitfinex.com/";

    static int WriteCallback(char* contents, size_t size, size_t nmemb, std::string* output) {
        size_t total_size = size * nmemb;
        const size_t MAX_INT = static_cast<size_t>((std::numeric_limits<int>::max)());

        if (total_size > MAX_INT) {
            // Handle overflow
            std::cerr << "Warning: WriteCallback size overflow" << std::endl;
            return 0; // Indicate failure to CURL
        }
        output->append(contents, total_size);
        return static_cast<int>(total_size);
    }

    std::string hmac_sha384(const std::string& key, const std::string& data) {
        unsigned char digest[SHA384_DIGEST_LENGTH];
        unsigned int digest_len = SHA384_DIGEST_LENGTH;

        HMAC(EVP_sha384(), key.c_str(), key.length(),
            reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), digest, &digest_len);

        std::string result;
        for (size_t i = 0; i < digest_len; i++) {
            char hex[3];
            snprintf(hex, sizeof(hex), "%02x", digest[i]);
            result.append(hex);
        }
        return result;
    }


    json sendRequest(const std::string& endpoint, const std::string& method, const std::string& body = "") {
        CURL* curl = curl_easy_init();
        std::string response;

        if (curl) {
            std::string url = base_url + endpoint;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");

            if (method == "POST") {
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            }

            // Authentication
            std::string nonce = std::to_string(std::time(nullptr) * 1000);
            std::string signature_payload = "/v2/" + endpoint + nonce + body;
            std::string signature = hmac_sha384(api_secret, signature_payload);

            headers = curl_slist_append(headers, ("bfx-nonce: " + nonce).c_str());
            headers = curl_slist_append(headers, ("bfx-apikey: " + api_key).c_str());
            headers = curl_slist_append(headers, ("bfx-signature: " + signature).c_str());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            // Disable SSL verification for debugging
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

            CURLcode res = curl_easy_perform(curl);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK) {
                throw std::runtime_error("CURL request failed: " + std::string(curl_easy_strerror(res)));
            }
        }

        return json::parse(response);
    }


public:
    BitfinexAPI(const std::string& key, const std::string& secret)
        : api_key(key), api_secret(secret) {}

    json placeOrder(const std::string& symbol, double amount, double price) {
        json body = {
            {"type", "EXCHANGE LIMIT"},
            {"symbol", symbol},
            {"amount", std::to_string(amount)},
            {"price", std::to_string(price)}
        };
        return sendRequest("auth/w/order/submit", "POST", body.dump());
    }

    json modifyOrder(const std::string& orderId, double price) {
        json body = {
            {"id", orderId},
            {"price", std::to_string(price)}
        };
        return sendRequest("auth/w/order/update", "POST", body.dump());
    }

    json cancelOrder(const std::string& orderId) {
        json body = { {"id", orderId} };
        return sendRequest("auth/w/order/cancel", "POST", body.dump());
    }

    json getOrderbook(const std::string& symbol) {
        return sendRequest("book/" + symbol + "/P0", "GET");
    }

    json getPositions() {
        return sendRequest("auth/r/positions", "POST");
    }
};

int main() {
    std::string api_key = std::getenv("BITFINEX_API_KEY");
    std::string api_secret = std::getenv("BITFINEX_API_SECRET");

    if (api_key.empty() || api_secret.empty()) {
        throw std::runtime_error("API credentials not found in environment variables");
    }

    BitfinexAPI api(api_key, api_secret);

    try {
        // Example: Get orderbook
        json orderbook = api.getOrderbook("tBTCUSD");
        std::cout << "Orderbook: " << orderbook.dump(4) << std::endl;

        // Example: Place a new order
        json new_order = api.placeOrder("tBTCUSD", 0.1, 50000);
        std::cout << "New Order: " << new_order.dump(4) << std::endl;

        if (new_order.contains("error")) {
            std::cerr << "Error placing order: " << new_order.dump(4) << std::endl;
            return 1;
        }

        // Extract the order ID from the response
        std::string orderId = new_order[4][0].get<std::string>();

        // Example: Modify the order
        json modified_order = api.modifyOrder(orderId, 51000);
        std::cout << "Modified Order: " << modified_order.dump(4) << std::endl;

        // Example: Cancel the order
        json cancelled_order = api.cancelOrder(orderId);
        std::cout << "Cancelled Order: " << cancelled_order.dump(4) << std::endl;

        // Example: Get positions
        json positions = api.getPositions();
        std::cout << "Positions: " << positions.dump(4) << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}