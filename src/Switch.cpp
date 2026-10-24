#include "Switch.h"

Switch::Switch(std::size_t totalInitialServers,
               std::size_t minimumServersPerBalancer,
               std::size_t lowerQueuePerServer,
               std::size_t upperQueuePerServer,
               int scaleCooldown)
    : processingBalancer_(
          "Processing Load Balancer",
          'P',
          (totalInitialServers + 1) / 2,
          minimumServersPerBalancer,
          lowerQueuePerServer,
          upperQueuePerServer,
          scaleCooldown),
      streamingBalancer_(
          "Streaming Load Balancer",
          'S',
          totalInitialServers / 2,
          minimumServersPerBalancer,
          lowerQueuePerServer,
          upperQueuePerServer,
          scaleCooldown) {
}

bool Switch::route(const Request& request,
                   const IPBlocker& blocker,
                   int currentCycle,
                   Logger& logger) {
    if (blocker.isBlocked(request.ipIn)) {
        ++stats_.blockedRequests;

        logger.event(
            currentCycle,
            "Firewall rejected request " +
                std::to_string(request.id) + " from " + request.ipIn,
            "\033[1;31m");

        return false;
    }

    if (request.jobType == 'P') {
        processingBalancer_.enqueue(request);
        ++stats_.processingRouted;
    } else if (request.jobType == 'S') {
        streamingBalancer_.enqueue(request);
        ++stats_.streamingRouted;
    } else {
        ++stats_.invalidRequests;

        logger.event(
            currentCycle,
            "Switch rejected request " +
                std::to_string(request.id) +
                " because its job type was invalid.",
            "\033[1;31m");

        return false;
    }

    ++stats_.totalAccepted;

    logger.event(
        currentCycle,
        "Switch routed request " + std::to_string(request.id) +
            " as job type " + request.jobType,
        "\033[1;36m");

    return true;
}

void Switch::captureStartingQueueSizes() {
    processingBalancer_.captureStartingQueueSize();
    streamingBalancer_.captureStartingQueueSize();
}

void Switch::tick(int currentCycle, Logger& logger) {
    processingBalancer_.tick(currentCycle, logger);
    streamingBalancer_.tick(currentCycle, logger);
}

void Switch::finalize() {
    processingBalancer_.finalize();
    streamingBalancer_.finalize();
}

const LoadBalancer& Switch::processingBalancer() const {
    return processingBalancer_;
}

const LoadBalancer& Switch::streamingBalancer() const {
    return streamingBalancer_;
}

const SwitchStats& Switch::stats() const {
    return stats_;
}

std::size_t Switch::totalQueueSize() const {
    return processingBalancer_.queueSize() +
           streamingBalancer_.queueSize();
}

std::size_t Switch::totalServerCount() const {
    return processingBalancer_.serverCount() +
           streamingBalancer_.serverCount();
}

std::size_t Switch::totalBusyServerCount() const {
    return processingBalancer_.busyServerCount() +
           streamingBalancer_.busyServerCount();
}

std::size_t Switch::totalIdleServerCount() const {
    return processingBalancer_.idleServerCount() +
           streamingBalancer_.idleServerCount();
}