#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

#include "Logger.h"
#include "Request.h"
#include "WebServer.h"

#include <cstddef>
#include <queue>
#include <string>
#include <vector>

/**
 * @brief Stores operational statistics for one load balancer.
 */
struct LoadBalancerStats {
    std::size_t startingQueueSize = 0;
    std::size_t endingQueueSize = 0;
    std::size_t receivedRequests = 0;
    std::size_t assignedRequests = 0;
    std::size_t completedRequests = 0;
    std::size_t serversAdded = 0;
    std::size_t serversRemoved = 0;
    std::size_t peakQueueSize = 0;
    std::size_t peakServerCount = 0;
    std::size_t minimumServerCount = 0;
    unsigned long long totalWaitCycles = 0;
};

/**
 * @brief Manages a request queue and a dynamically sized server pool.
 *
 * Each instance handles one job type. The bonus Switch class will route
 * processing and streaming requests to separate LoadBalancer objects.
 */
class LoadBalancer {
public:
    /**
     * @brief Constructs a load balancer and its initial servers.
     */
    LoadBalancer(const std::string& name,
                 char acceptedJobType,
                 std::size_t initialServers,
                 std::size_t minimumServers,
                 std::size_t lowerQueuePerServer,
                 std::size_t upperQueuePerServer,
                 int scaleCooldown);

    /**
     * @brief Adds a request to this load balancer's queue.
     */
    void enqueue(const Request& request);

    /**
     * @brief Records the queue size immediately before simulation begins.
     */
    void captureStartingQueueSize();

    /**
     * @brief Advances servers, assignments, and scaling by one clock cycle.
     */
    void tick(int currentCycle, Logger& logger);

    /**
     * @brief Records final queue information after the simulation.
     */
    void finalize();

    const std::string& name() const;
    char acceptedJobType() const;
    std::size_t queueSize() const;
    std::size_t serverCount() const;
    std::size_t busyServerCount() const;
    std::size_t idleServerCount() const;
    const LoadBalancerStats& stats() const;

    /**
     * @brief Calculates average queue waiting time for assigned requests.
     */
    double averageWaitTime() const;

private:
    void processServers(int currentCycle, Logger& logger);
    void assignRequests(int currentCycle, Logger& logger);
    void evaluateScaling(int currentCycle, Logger& logger);
    void addServer(int currentCycle, Logger& logger);
    bool removeIdleServer(int currentCycle, Logger& logger);

    std::string name_;
    char acceptedJobType_;
    std::queue<Request> requestQueue_;
    std::vector<WebServer> servers_;

    std::size_t minimumServers_;
    std::size_t lowerQueuePerServer_;
    std::size_t upperQueuePerServer_;
    int scaleCooldown_;
    int lastScaleCycle_;
    std::size_t nextServerId_;

    LoadBalancerStats stats_;
};

#endif