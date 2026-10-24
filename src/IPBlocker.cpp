#include "IPBlocker.h"

#include <algorithm>
#include <stdexcept>

void IPBlocker::addBlockedPrefix(const std::string& prefix) {
    if (prefix.empty()) {
        throw std::invalid_argument("Blocked IP prefix cannot be empty.");
    }

    const auto existingPrefix =
        std::find(blockedPrefixes_.begin(), blockedPrefixes_.end(), prefix);

    if (existingPrefix == blockedPrefixes_.end()) {
        blockedPrefixes_.push_back(prefix);
    }
}

bool IPBlocker::isBlocked(const std::string& ipAddress) const {
    for (const std::string& prefix : blockedPrefixes_) {
        if (ipAddress.rfind(prefix, 0) == 0) {
            return true;
        }
    }

    return false;
}

std::size_t IPBlocker::blockedPrefixCount() const {
    return blockedPrefixes_.size();
}