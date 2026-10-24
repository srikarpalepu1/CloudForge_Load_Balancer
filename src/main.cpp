#include "Config.h"
#include "IPBlocker.h"
#include "Logger.h"
#include "RequestGenerator.h"
#include "Switch.h"
#include <stdexcept>
#include <exception>
#include <iostream>
#include <string>
#include <cstddef>
#include <cstdint>

int main() {
    try {
        Config config =
            Config::loadFromFile("config/cloudforge.conf");

        std::cout << "\n"
                  << "========================================\n"
                  << "       CloudForge Load Balancer\n"
                  << "          Route. Scale. Protect.\n"
                  << "========================================\n\n";
        std::cout << "Enter total starting servers: ";

        if (!(std::cin >> config.initialServers)) {
            throw std::runtime_error(
                "Starting server count must be a whole number.");
        }

        std::cout << "Enter total clock cycles: ";

        if (!(std::cin >> config.totalCycles)) {
            throw std::runtime_error(
                "Clock-cycle count must be a whole number.");
        }

        char modeChoice;

        std::cout << "Select mode, Q for quiet or V for verbose: ";
        std::cin >> modeChoice;

        if (modeChoice == 'Q' || modeChoice == 'q') {
            config.verbose = false;
            config.logFile = "logs/cloudforge_quiet.log";
        } else if (modeChoice == 'V' || modeChoice == 'v') {
            config.verbose = true;
            config.logFile = "logs/cloudforge_verbose.log";
        } else {
            throw std::runtime_error(
                "Mode must be Q for quiet or V for verbose.");
        }

        config.validate();
        Logger logger(config.logFile, config.verbose);

        IPBlocker blocker;
        blocker.addBlockedPrefix(config.blockedPrefix);

        RequestGenerator generator(
            config.minProcessingTime,
            config.maxProcessingTime,
            config.blockedTrafficRate,
            config.randomSeed);

        Switch requestSwitch(
            static_cast<std::size_t>(config.initialServers),
            static_cast<std::size_t>(
                config.minimumServersPerBalancer),
            static_cast<std::size_t>(
                config.lowerQueuePerServer),
            static_cast<std::size_t>(
                config.upperQueuePerServer),
            config.scaleCooldown);

        std::uint64_t nextRequestId = 1;

        const std::size_t targetStartingQueue =
            static_cast<std::size_t>(config.initialServers) *
            static_cast<std::size_t>(
                config.initialQueuePerServer);

        while (requestSwitch.totalQueueSize() <
               targetStartingQueue) {
            const Request request =
                generator.generate(nextRequestId, 0);

            ++nextRequestId;

            requestSwitch.route(
                request, blocker, 0, logger);
        }

        requestSwitch.captureStartingQueueSizes();

        logger.section("CLOUDFORGE STARTING STATUS");

        logger.summary(
            "Mode: " +
            std::string(config.verbose ? "Verbose" : "Quiet"));

        logger.summary(
            "Starting servers: " +
            std::to_string(requestSwitch.totalServerCount()));

        logger.summary(
            "Starting queue size: " +
            std::to_string(requestSwitch.totalQueueSize()));

        logger.summary(
            "Clock cycles: " +
            std::to_string(config.totalCycles));

        logger.summary(
            "Task-time range: " +
            std::to_string(config.minProcessingTime) +
            " to " +
            std::to_string(config.maxProcessingTime) +
            " clock cycles");

        logger.summary(
            "Blocked IP prefix: " +
            config.blockedPrefix);

        logger.summary(
            "Processing queue: " +
            std::to_string(
                requestSwitch.processingBalancer().queueSize()));

        logger.summary(
            "Streaming queue: " +
            std::to_string(
                requestSwitch.streamingBalancer().queueSize()));
        
        for (int currentCycle = 1;
             currentCycle <= config.totalCycles;
             ++currentCycle) {
            if (generator.shouldGenerate(
                    config.newRequestProbability)) {
                const Request request =
                    generator.generate(
                        nextRequestId, currentCycle);

                ++nextRequestId;

                requestSwitch.route(
                    request,
                    blocker,
                    currentCycle,
                    logger);
            }

            requestSwitch.tick(currentCycle, logger);
        }

        requestSwitch.finalize();

        const LoadBalancerStats& processingStats =
            requestSwitch.processingBalancer().stats();

        const LoadBalancerStats& streamingStats =
            requestSwitch.streamingBalancer().stats();

        const SwitchStats& routingStats =
            requestSwitch.stats();

        const std::size_t startingQueueSize =
            processingStats.startingQueueSize +
            streamingStats.startingQueueSize;

        const std::size_t endingQueueSize =
            processingStats.endingQueueSize +
            streamingStats.endingQueueSize;

        const std::size_t completedRequests =
            processingStats.completedRequests +
            streamingStats.completedRequests;

        const std::size_t assignedRequests =
            processingStats.assignedRequests +
            streamingStats.assignedRequests;

        const unsigned long long totalWaitCycles =
            processingStats.totalWaitCycles +
            streamingStats.totalWaitCycles;

        const double averageWaitTime =
            assignedRequests == 0
                ? 0.0
                : static_cast<double>(totalWaitCycles) /
                      static_cast<double>(assignedRequests);

        const std::size_t serversAdded =
            processingStats.serversAdded +
            streamingStats.serversAdded;

        const std::size_t serversRemoved =
            processingStats.serversRemoved +
            streamingStats.serversRemoved;

        const std::size_t peakQueueSize =
            processingStats.peakQueueSize +
            streamingStats.peakQueueSize;

        const std::size_t peakServerCount =
            processingStats.peakServerCount +
            streamingStats.peakServerCount;

        logger.section("CLOUDFORGE ENDING STATUS");

        logger.summary("Simulation status: COMPLETE");

        logger.summary(
            "Starting queue size: " +
            std::to_string(startingQueueSize));

        logger.summary(
            "Ending queue size: " +
            std::to_string(endingQueueSize));

        logger.summary(
            "Task-time range: " +
            std::to_string(config.minProcessingTime) +
            " to " +
            std::to_string(config.maxProcessingTime) +
            " clock cycles");

        logger.summary(
            "Total requests generated: " +
            std::to_string(nextRequestId - 1));

        logger.summary(
            "Accepted requests: " +
            std::to_string(routingStats.totalAccepted));

        logger.summary(
            "Processing requests routed: " +
            std::to_string(routingStats.processingRouted));

        logger.summary(
            "Streaming requests routed: " +
            std::to_string(routingStats.streamingRouted));

        logger.summary(
            "Completed requests: " +
            std::to_string(completedRequests));

        logger.summary(
            "Blocked or discarded requests: " +
            std::to_string(
                routingStats.blockedRequests +
                routingStats.invalidRequests));

        logger.summary(
            "Active servers: " +
            std::to_string(
                requestSwitch.totalServerCount()));

        logger.summary(
            "Busy servers: " +
            std::to_string(
                requestSwitch.totalBusyServerCount()));

        logger.summary(
            "Idle servers: " +
            std::to_string(
                requestSwitch.totalIdleServerCount()));

        logger.summary(
            "Inactive or removed servers: " +
            std::to_string(serversRemoved));

        logger.summary(
            "Servers added through scaling: " +
            std::to_string(serversAdded));

        logger.summary(
            "Servers removed through scaling: " +
            std::to_string(serversRemoved));

        logger.summary(
            "Peak combined queue size: " +
            std::to_string(peakQueueSize));

        logger.summary(
            "Peak combined server count: " +
            std::to_string(peakServerCount));

        logger.summary(
            "Average queue wait: " +
            std::to_string(averageWaitTime) +
            " clock cycles");

        logger.summary(
            "Processing ending queue: " +
            std::to_string(
                processingStats.endingQueueSize));

        logger.summary(
            "Streaming ending queue: " +
            std::to_string(
                streamingStats.endingQueueSize));

        logger.summary(
            "Log file: " + config.logFile);

    } catch (const std::exception& error) {
        std::cerr << "CloudForge error: "
                  << error.what() << '\n';
        return 1;
    }

    return 0;
}