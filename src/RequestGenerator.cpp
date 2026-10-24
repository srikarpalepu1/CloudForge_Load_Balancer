#include "RequestGenerator.h"

#include <sstream>
#include <stdexcept>

RequestGenerator::RequestGenerator(int minProcessingTime,
                                   int maxProcessingTime,
                                   double blockedTrafficRate,
                                   std::uint32_t seed)
    : minProcessingTime_(minProcessingTime),
      maxProcessingTime_(maxProcessingTime),
      blockedTrafficRate_(blockedTrafficRate),
      randomEngine_(seed) {
    if (minProcessingTime_ <= 0 ||
        maxProcessingTime_ < minProcessingTime_) {
        throw std::invalid_argument("Invalid processing-time range.");
    }

    if (blockedTrafficRate_ < 0.0 || blockedTrafficRate_ > 1.0) {
        throw std::invalid_argument(
            "Blocked traffic rate must be between 0.0 and 1.0.");
    }
}

std::string RequestGenerator::generateIPAddress() {
    std::uniform_int_distribution<int> firstOctet(1, 223);
    std::uniform_int_distribution<int> octet(0, 255);

    int first = firstOctet(randomEngine_);

    // Reserve 203.0.113.0/24 exclusively for firewall-test traffic.
    if (first == 203) {
        first = 202;
    }

    std::ostringstream address;
    address << first << '.'
            << octet(randomEngine_) << '.'
            << octet(randomEngine_) << '.'
            << octet(randomEngine_);

    return address.str();
}

std::string RequestGenerator::generateBlockedIPAddress() {
    std::uniform_int_distribution<int> host(1, 254);

    std::ostringstream address;
    address << "203.0.113." << host(randomEngine_);
    return address.str();
}

Request RequestGenerator::generate(std::uint64_t id, int arrivalCycle) {
    std::uniform_int_distribution<int> processingTime(
        minProcessingTime_, maxProcessingTime_);
    std::bernoulli_distribution blockedTraffic(blockedTrafficRate_);
    std::bernoulli_distribution jobType(0.5);

    Request request;
    request.id = id;
    request.ipIn = blockedTraffic(randomEngine_)
                       ? generateBlockedIPAddress()
                       : generateIPAddress();
    request.ipOut = generateIPAddress();
    request.processingTime = processingTime(randomEngine_);
    request.jobType = jobType(randomEngine_) ? 'P' : 'S';
    request.arrivalCycle = arrivalCycle;

    return request;
}

bool RequestGenerator::shouldGenerate(double probability) {
    if (probability < 0.0 || probability > 1.0) {
        throw std::invalid_argument(
            "Generation probability must be between 0.0 and 1.0.");
    }

    std::bernoulli_distribution decision(probability);
    return decision(randomEngine_);
}