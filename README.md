# Smart Home ZigBee Network Simulation

A comprehensive implementation of a ZigBee-based smart home network using the ns-3 network simulator. This project demonstrates network formation, multi-hop routing, and IoT device communication in a realistic smart home environment.

## Overview

This simulation implements a complete ZigBee network stack including the Network Layer (NWK) and Application Support Sub-layer (APS) to model a smart home system with temperature sensors and intelligent lighting control. The implementation follows the ZigBee Pro Specification R22 1.0 and provides a foundation for research and development in wireless sensor networks and IoT systems.

**Key Capabilities:**
- Network formation with coordinator-based topology
- Association-based device joining mechanism
- Mesh routing and Many-to-One routing protocols
- Unicast and Groupcast data transmission
- Multi-endpoint device support
- Room-based group control for lighting systems

## System Requirements

- ns-3 version 3.46 or later
- Linux environment (Ubuntu 20.04+ recommended)
- GCC 11+ with C++23 support
- CMake 3.10 or higher

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/thevien257/Smart-Home-NS3-Zigbee.git
cd Smart-Home-NS3-Zigbee
```

### 2. Install to ns-3

Copy the simulation files to your ns-3 installation:

```bash
cp src/zigbee/examples/smart-home-zigbee-complete.cc ~/ns3-workspace/ns-3-dev/src/zigbee/examples/
cp src/zigbee/examples/CMakeLists.txt ~/ns3-workspace/ns-3-dev/src/zigbee/examples/
```

### 3. Build

```bash
cd ~/ns3-workspace/ns-3-dev/
./ns3 configure --enable-examples
./ns3 build
```

### 4. Run the Simulation

```bash
./ns3 run smart-home-zigbee-complete
```

## Network Architecture

The simulation models a 6-node smart home network with the following topology:

```
Coordinator [00:00]
    |
    +--- Router 1 --- Router 2 --- Router 3 (Temperature Sensor)
              |           |
         Router 4     Router 5
      (Living Room)  (Bedroom)
```

### Device Roles

| Node | Role | Function | Endpoints |
|------|------|----------|-----------|
| 0 | Coordinator | Network formation, data collection | 1 |
| 1 | Router | Packet relay | - |
| 2 | Router | Packet relay | - |
| 3 | Router | Temperature monitoring | 1 |
| 4 | Router | Living room lighting control | 2 |
| 5 | Router | Bedroom lighting control | 1 |

### Group Configuration

- **Group 0x0001 (Living Room)**: Node 4 Endpoints 1, 2
- **Group 0x0002 (Bedroom)**: Node 5 Endpoint 1
- **Group 0x0003 (All Lights)**: All lighting endpoints

## Usage

### Basic Execution

```bash
./ns3 run smart-home-zigbee-complete
```

### Command Line Options

```bash
./ns3 run "smart-home-zigbee-complete --simTime=<seconds> --verbose=<0|1> --manyToOne=<0|1>"
```

**Parameters:**

- `--simTime`: Simulation duration in seconds (default: 300)
- `--verbose`: Enable detailed logging (default: 0)
- `--manyToOne`: Use Many-to-One routing; 0 for Mesh routing (default: 1)

**Examples:**

```bash
# Run 10-minute simulation with verbose output
./ns3 run "smart-home-zigbee-complete --simTime=600 --verbose=1"

# Use Mesh routing instead of Many-to-One
./ns3 run "smart-home-zigbee-complete --manyToOne=0"
```

## Simulation Results

The simulation produces comprehensive output including:

### Network Formation
- Coordinator initialization and channel selection
- Device discovery and association process
- Network address assignment

### Routing Information
- Route discovery process (RREQ/RREP messages)
- Routing table contents for all nodes
- Hop-by-hop route traces

### Data Transmission
- Temperature sensor reports (periodic, every 20 seconds)
- Group-based lighting commands
- Packet delivery confirmations

### Performance Metrics
- Join success rate
- Packet delivery ratio
- End-to-end delay
- Route convergence time

**Expected Performance:**
- Join Success Rate: 100% (5/5 devices)
- Packet Delivery: 155% (multiple endpoints per groupcast)
- Average Latency: 3-161ms
- Route Discoveries: 1 (Many-to-One mode)

## Implementation Details

### Network Layer (NWK)

The NWK layer implements the following ZigBee primitives:

- `NLME-NETWORK-FORMATION`: Coordinator network initialization
- `NLME-NETWORK-DISCOVERY`: Active scanning for available networks
- `NLME-JOIN`: Association-based network joining
- `NLME-START-ROUTER`: Router capability activation
- `NLME-ROUTE-DISCOVERY`: Mesh and Many-to-One route establishment
- `NLDE-DATA`: Data transmission with automatic routing

### Application Support Sub-layer (APS)

The APS layer provides:

- `APSDE-DATA`: Unicast and Groupcast data transmission
- `APSME-ADD-GROUP`: Group membership management
- `APSME-REMOVE-GROUP`: Group member removal
- Multiple endpoint support per device
- ZigBee Cluster Library (ZCL) cluster addressing

### Routing Protocols

**Mesh Routing:**
- On-demand route discovery using RREQ/RREP mechanism
- Link Quality Indicator (LQI) based path cost calculation
- Route table management with discovery and neighbor tables

**Many-to-One Routing:**
- Single route discovery establishes paths from all nodes to concentrator
- Reduced network overhead for sensor-to-coordinator communication
- Optimized for data collection applications

## Project Structure

```
Smart-Home-NS3-Zigbee/
├── src/
│   └── zigbee/
│       └── examples/
│           ├── smart-home-zigbee-complete.cc
│           └── CMakeLists.txt
├── docs/
│   ├── Complete_Guide.md
│   ├── Quick_Start.md
│   └── Simulation_Results.md
├── README.md
├── LICENSE
├── CHANGELOG.md
└── .gitignore
```

## Extending the Simulation

The simulation framework supports various extensions:

- **Additional Device Types**: Motion sensors, door locks, thermostats
- **Network Resilience Testing**: Node failure and recovery scenarios
- **Mobility Models**: Dynamic topology with moving devices
- **Traffic Patterns**: Event-driven and periodic reporting mechanisms
- **Network Scaling**: Larger topologies with 50+ nodes

Refer to the [Complete Guide](docs/projectGuide.pdf) for implementation examples.

## Troubleshooting

### Build Issues

**Program not found after installation:**
```bash
./ns3 clean
./ns3 configure --enable-examples
./ns3 build
```

**Compilation errors:**
- Verify all required headers are included
- Ensure CMakeLists.txt is properly updated
- Check ns-3 version compatibility (3.46+)

### Runtime Issues

**Low join success rate:**
- Increase scan duration parameter
- Verify node positions are within communication range
- Check channel configuration (0x7800 for channels 11-14)

**Route discovery failures:**
- Ensure route discovery is scheduled after network formation
- Verify all routers have executed NLME-START-ROUTER
- Check network connectivity with position adjustments

## Performance Benchmarks

Simulation environment: ns-3.46 on Ubuntu 22.04, Intel Core i7, 16GB RAM

- **Execution Time**: 5-10 seconds (for 300s simulation)
- **Memory Usage**: ~50MB
- **Network Formation**: ~2 seconds
- **Complete Network Join**: ~11 seconds (5 devices)
- **Route Convergence**: <1 second (Many-to-One)
- **Scalability**: Tested up to 50 nodes

## Contributing

Contributions are welcome. Please follow these guidelines:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/new-feature`)
3. Implement changes with appropriate documentation
4. Test thoroughly with various configurations
5. Submit a pull request with detailed description
