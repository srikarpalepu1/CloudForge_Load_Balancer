# CloudForge Load Balancer

**Route. Scale. Protect.**

CloudForge Load Balancer is a configurable C++ simulation of a large-company request-routing environment. It uses real queue-based load balancing, dynamically allocated web servers, randomized requests, IP-range blocking, operational logging, and separate processing and streaming services.

## Features

- Request queue implemented with `std::queue`
- Random incoming and outgoing IP addresses
- Random task durations
- Processing (`P`) and streaming (`S`) job types
- Dedicated web-server objects
- Dynamic server allocation and deallocation
- Configurable scaling thresholds and cooldown
- IP-prefix firewall and rejected-request tracking
- Quiet and verbose logging modes
- Colored terminal output
- External configuration file
- Doxygen HTML documentation
- Bonus higher-level Switch
- Separate processing and streaming load balancers
- Separate operational statistics for both services

## Architecture

Incoming requests first pass through the `IPBlocker`. Accepted requests enter the bonus `Switch`, which routes `P` jobs to the Processing Load Balancer and `S` jobs to the Streaming Load Balancer.

Each load balancer owns:

- A queue of `Request` objects
- A dynamically sized collection of `WebServer` objects
- Scaling thresholds
- A cooldown timer
- Independent statistics

## Main Components

- `Request`: Stores request ID, IP addresses, processing time, job type, and arrival cycle.
- `RequestGenerator`: Creates randomized and reproducible traffic.
- `IPBlocker`: Rejects configured IP address ranges.
- `WebServer`: Processes one request at a time.
- `LoadBalancer`: Manages queues, servers, assignments, and scaling.
- `Switch`: Routes processing and streaming requests.
- `Config`: Loads settings from an external file.
- `Logger`: Produces quiet summaries and verbose event logs.

## Requirements

- C++17 compiler
- GNU Make
- Doxygen
- Graphviz

## Build

```bash
make
```

## Run

```bash
./cloudforge_lb.exe
```

The program asks for:

1. Total starting servers
2. Total clock cycles
3. Quiet (`Q`) or verbose (`V`) mode

## Required Quiet Run

Use:

```text
Starting servers: 20
Clock cycles: 100000
Mode: Q
```

The summary is written to:

```text
logs/cloudforge_quiet.log
```

## Verbose Demonstration

Verbose mode records useful operational events, including:

- Request routing
- Server assignment
- Request completion
- Firewall rejection
- Server scale-up
- Server scale-down

The verbose log is written to:

```text
logs/cloudforge_verbose.log
```

## Configuration

Simulation settings are stored in:

```text
config/cloudforge.conf
```

The file controls task-time ranges, request-generation probability, blocked-traffic rate, scaling thresholds, cooldown time, random seed, and logging defaults.

## Dynamic Scaling

The system attempts to maintain between 50 and 80 queued requests per server.

- Above `80 × active servers`: add one server.
- Below `50 × active servers`: remove one idle server.
- Wait for the configured cooldown before checking again.
- Maintain at least one server per load balancer.

## Firewall

The default demonstration range is:

```text
203.0.113.0/24
```

Requests originating from that configured prefix are rejected and counted as blocked traffic.

## Documentation

Generate Doxygen HTML documentation with:

```bash
make docs
```

Open:

```text
docs/html/index.html
```

## Clean Compiled Files

```bash
make clean
```

## Project Structure

```text
CloudForge_Load_Balancer/
├── config/
├── docs/
├── include/
├── logs/
├── src/
├── tests/
├── Doxyfile
├── Makefile
└── README.md
```

## Branding

**CloudForge Load Balancer**  
**Route. Scale. Protect.**