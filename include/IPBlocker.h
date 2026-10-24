#ifndef IP_BLOCKER_H
#define IP_BLOCKER_H

#include <cstddef>
#include <string>
#include <vector>

/**
 * @brief Blocks incoming requests from configured IP prefixes.
 *
 * This class provides the firewall and denial-of-service prevention
 * component of the CloudForge load balancer.
 */
class IPBlocker {
public:
    /**
     * @brief Adds an IP prefix to the blocked list.
     * @param prefix Prefix such as "203.0.113.".
     */
    void addBlockedPrefix(const std::string& prefix);

    /**
     * @brief Checks whether an IP address matches a blocked prefix.
     * @param ipAddress Incoming source IP address.
     * @return True if the address should be rejected.
     */
    bool isBlocked(const std::string& ipAddress) const;

    /**
     * @brief Returns the number of configured blocked prefixes.
     * @return Number of blocked prefixes.
     */
    std::size_t blockedPrefixCount() const;

private:
    std::vector<std::string> blockedPrefixes_;
};

#endif