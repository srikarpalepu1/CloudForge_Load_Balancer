#ifndef SWITCH_H
#define SWITCH_H

#include "IPBlocker.h"
#include "LoadBalancer.h"
#include "Logger.h"
#include "Request.h"

#include <cstddef>

/**
 * @brief Stores routing and firewall statistics for the Switch.
 */
struct SwitchStats {
    std::size_t totalAccepted = 0;
    std::size_t processingRouted = 0;
    std::size_t streamingRouted = 0;
    std::size_t blockedRequests = 0;
    std::size_t invalidRequests = 0;
};

/**
 * @brief Routes requests to separate processing and streaming load balancers.
 *
 * The Switch is the higher-level bonus component. It checks incoming source
 * addresses against the firewall and routes accepted requests by job type.
 */
class Switch {
public:
    /**
     * @brief Constructs processing and streaming load balancers.
     */
    Switch(std::size_t totalInitialServers,
           std::size_t minimumServersPerBalancer,
           std::size_t lowerQueuePerServer,
           std::size_t upperQueuePerServer,
           int scaleCooldown);

    /**
     * @brief Applies firewall rules and routes one request.
     * @return True if accepted and routed, or false if rejected.
     */
    bool route(const Request& request,
               const IPBlocker& blocker,
               int currentCycle,
               Logger& logger);

    /**
     * @brief Captures both starting queue sizes.
     */
    void captureStartingQueueSizes();

    /**
     * @brief Advances both load balancers concurrently by one cycle.
     */
    void tick(int currentCycle, Logger& logger);

    /**
     * @brief Records both ending queue sizes.
     */
    void finalize();

    const LoadBalancer& processingBalancer() const;
    const LoadBalancer& streamingBalancer() const;
    const SwitchStats& stats() const;

    std::size_t totalQueueSize() const;
    std::size_t totalServerCount() const;
    std::size_t totalBusyServerCount() const;
    std::size_t totalIdleServerCount() const;

private:
    LoadBalancer processingBalancer_;
    LoadBalancer streamingBalancer_;
    SwitchStats stats_;
};

#endif