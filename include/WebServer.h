#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "Request.h"

#include <cstddef>
#include <optional>

/**
 * @brief Processes one request at a time for a load balancer.
 *
 * A web server accepts a request when idle, processes it for the request's
 * required number of clock cycles, and reports when processing is complete.
 */
class WebServer {
public:
    /**
     * @brief Constructs an idle web server.
     * @param id Unique server identifier.
     */
    explicit WebServer(std::size_t id);

    /**
     * @brief Assigns a request to this server.
     * @param request Request to process.
     * @throws std::logic_error if the server is already busy.
     */
    void assignRequest(const Request& request);

    /**
     * @brief Advances this server by one clock cycle.
     * @return The completed request when processing finishes, or no value.
     */
    std::optional<Request> tick();

    /**
     * @brief Checks whether this server is processing a request.
     * @return True when busy.
     */
    bool isBusy() const;

    /**
     * @brief Returns this server's identifier.
     * @return Unique server ID.
     */
    std::size_t id() const;

    /**
     * @brief Returns the remaining processing time.
     * @return Remaining clock cycles, or zero when idle.
     */
    int remainingTime() const;

    /**
     * @brief Returns the number of requests completed by this server.
     * @return Completed-request count.
     */
    std::size_t completedCount() const;

private:
    std::size_t id_;
    std::optional<Request> currentRequest_;
    int remainingTime_;
    std::size_t completedCount_;
};

#endif