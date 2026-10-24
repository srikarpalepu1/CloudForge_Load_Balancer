#include "LoadBalancer.h"

#include <algorithm>
#include <stdexcept>

LoadBalancer::LoadBalancer(const std::string& name,
                           char acceptedJobType,
                           std::size_t initialServers,
                           std::size_t minimumServers,
                           std::size_t lowerQueuePerServer,
                           std::size_t upperQueuePerServer,
                           int scaleCooldown)
    : name_(name),
      acceptedJobType_(acceptedJobType),
      minimumServers_(minimumServers),
      lowerQueuePerServer_(lowerQueuePerServer),
      upperQueuePerServer_(upperQueuePerServer),
      scaleCooldown_(scaleCooldown),
      lastScaleCycle_(-scaleCooldown),
      nextServerId_(1) {
    if (acceptedJobType_ != 'P' && acceptedJobType_ != 'S') {
        throw std::invalid_argument("Job type must be P or S.");
    }

    if (initialServers == 0 || minimumServers == 0 ||
        initialServers < minimumServers) {
        throw std::invalid_argument("Invalid initial server count.");
    }

    if (lowerQueuePerServer_ == 0 ||
        upperQueuePerServer_ <= lowerQueuePerServer_ ||
        scaleCooldown_ < 0) {
        throw std::invalid_argument("Invalid scaling configuration.");
    }

    for (std::size_t index = 0; index < initialServers; ++index) {
        servers_.emplace_back(nextServerId_++);
    }

    stats_.peakServerCount = initialServers;
    stats_.minimumServerCount = initialServers;
}

void LoadBalancer::enqueue(const Request& request) {
    if (request.jobType != acceptedJobType_) {
        throw std::invalid_argument(
            "Request sent to the wrong load balancer.");
    }

    requestQueue_.push(request);
    ++stats_.receivedRequests;

    stats_.peakQueueSize =
        std::max(stats_.peakQueueSize, requestQueue_.size());
}

void LoadBalancer::captureStartingQueueSize() {
    stats_.startingQueueSize = requestQueue_.size();
}

void LoadBalancer::tick(int currentCycle, Logger& logger) {
    processServers(currentCycle, logger);
    assignRequests(currentCycle, logger);
    evaluateScaling(currentCycle, logger);
}

void LoadBalancer::processServers(int currentCycle, Logger& logger) {
    for (WebServer& server : servers_) {
        const std::optional<Request> completedRequest = server.tick();

        if (completedRequest.has_value()) {
            ++stats_.completedRequests;

            logger.event(
                currentCycle,
                name_ + " server " + std::to_string(server.id()) +
                    " completed request " +
                    std::to_string(completedRequest->id),
                "\033[1;32m");
        }
    }
}

void LoadBalancer::assignRequests(int currentCycle, Logger& logger) {
    for (WebServer& server : servers_) {
        if (requestQueue_.empty()) {
            break;
        }

        if (server.isBusy()) {
            continue;
        }

        const Request request = requestQueue_.front();
        requestQueue_.pop();
        server.assignRequest(request);

        ++stats_.assignedRequests;

        const int waitTime =
            std::max(0, currentCycle - request.arrivalCycle);
        stats_.totalWaitCycles +=
            static_cast<unsigned long long>(waitTime);

        logger.event(
            currentCycle,
            name_ + " assigned request " +
                std::to_string(request.id) + " to server " +
                std::to_string(server.id()),
            "\033[1;33m");
    }
}

void LoadBalancer::evaluateScaling(int currentCycle, Logger& logger) {
    if (currentCycle - lastScaleCycle_ < scaleCooldown_) {
        return;
    }

    const std::size_t currentServers = servers_.size();
    const std::size_t upperLimit =
        upperQueuePerServer_ * currentServers;
    const std::size_t lowerLimit =
        lowerQueuePerServer_ * currentServers;

    if (requestQueue_.size() > upperLimit) {
        addServer(currentCycle, logger);
    } else if (requestQueue_.size() < lowerLimit &&
               currentServers > minimumServers_) {
        removeIdleServer(currentCycle, logger);
    }
}

void LoadBalancer::addServer(int currentCycle, Logger& logger) {
    const std::size_t newServerId = nextServerId_++;
    servers_.emplace_back(newServerId);

    ++stats_.serversAdded;
    stats_.peakServerCount =
        std::max(stats_.peakServerCount, servers_.size());
    lastScaleCycle_ = currentCycle;

    logger.event(
        currentCycle,
        name_ + " scaled up by adding server " +
            std::to_string(newServerId) + ". Active servers: " +
            std::to_string(servers_.size()),
        "\033[1;34m");
}

bool LoadBalancer::removeIdleServer(int currentCycle, Logger& logger) {
    const auto idleServer = std::find_if(
        servers_.begin(), servers_.end(),
        [](const WebServer& server) {
            return !server.isBusy();
        });

    if (idleServer == servers_.end()) {
        return false;
    }

    const std::size_t removedServerId = idleServer->id();
    servers_.erase(idleServer);

    ++stats_.serversRemoved;
    stats_.minimumServerCount =
        std::min(stats_.minimumServerCount, servers_.size());
    lastScaleCycle_ = currentCycle;

    logger.event(
        currentCycle,
        name_ + " scaled down by removing idle server " +
            std::to_string(removedServerId) + ". Active servers: " +
            std::to_string(servers_.size()),
        "\033[1;35m");

    return true;
}

void LoadBalancer::finalize() {
    stats_.endingQueueSize = requestQueue_.size();
}

const std::string& LoadBalancer::name() const {
    return name_;
}

char LoadBalancer::acceptedJobType() const {
    return acceptedJobType_;
}

std::size_t LoadBalancer::queueSize() const {
    return requestQueue_.size();
}

std::size_t LoadBalancer::serverCount() const {
    return servers_.size();
}

std::size_t LoadBalancer::busyServerCount() const {
    return static_cast<std::size_t>(
        std::count_if(
            servers_.begin(), servers_.end(),
            [](const WebServer& server) {
                return server.isBusy();
            }));
}

std::size_t LoadBalancer::idleServerCount() const {
    return serverCount() - busyServerCount();
}

const LoadBalancerStats& LoadBalancer::stats() const {
    return stats_;
}

double LoadBalancer::averageWaitTime() const {
    if (stats_.assignedRequests == 0) {
        return 0.0;
    }

    return static_cast<double>(stats_.totalWaitCycles) /
           static_cast<double>(stats_.assignedRequests);
}