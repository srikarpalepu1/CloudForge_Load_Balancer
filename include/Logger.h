#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>

/**
 * @brief Records simulation events and required summary information.
 *
 * Quiet mode records major starting and ending summaries. Verbose mode
 * additionally records request assignments, completions, scaling, and
 * firewall events without logging every clock cycle.
 */
class Logger {
public:
    /**
     * @brief Opens the specified log file.
     * @param fileName Path of the output log.
     * @param verbose Enables detailed event logging.
     */
    Logger(const std::string& fileName, bool verbose);

    /**
     * @brief Records a detailed event when verbose mode is enabled.
     * @param cycle Current simulation clock cycle.
     * @param message Event description.
     * @param color ANSI terminal color.
     */
    void event(int cycle,
               const std::string& message,
               const std::string& color = "\033[0m");

    /**
     * @brief Records information in both quiet and verbose modes.
     * @param message Summary text.
     */
    void summary(const std::string& message);

    /**
     * @brief Creates a labeled section in the output.
     * @param title Section title.
     */
    void section(const std::string& title);

    /**
     * @brief Indicates whether detailed logging is enabled.
     * @return True when running in verbose mode.
     */
    bool isVerbose() const;

private:
    std::ofstream outputFile_;
    bool verbose_;
};

#endif