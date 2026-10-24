#include "WebServer.h"

#include <stdexcept>

WebServer::WebServer(std::size_t id)
    : id_(id),
      currentRequest_(std::nullopt),
      remainingTime_(0),
      completedCount_(0) {
}

void WebServer::assignRequest(const Request& request) {
    if (isBusy()) {
        throw std::logic_error(
            "Cannot assign a request to a busy web server.");
    }

    if (request.processingTime <= 0) {
        throw std::invalid_argument(
            "Request processing time must be positive.");
    }

    currentRequest_ = request;
    remainingTime_ = request.processingTime;
}

std::optional<Request> WebServer::tick() {
    if (!isBusy()) {
        return std::nullopt;
    }

    --remainingTime_;

    if (remainingTime_ > 0) {
        return std::nullopt;
    }

    Request completedRequest = *currentRequest_;
    currentRequest_.reset();
    remainingTime_ = 0;
    ++completedCount_;

    return completedRequest;
}

bool WebServer::isBusy() const {
    return currentRequest_.has_value();
}

std::size_t WebServer::id() const {
    return id_;
}

int WebServer::remainingTime() const {
    return remainingTime_;
}

std::size_t WebServer::completedCount() const {
    return completedCount_;
}