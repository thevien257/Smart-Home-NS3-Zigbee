/*
 * ZigBee Smart Home Network Simulation - INDOOR VERSION
 * Focus: Gaussian Noise, Rayleigh Fading, Distance, and Network Scale
 * 
 * OPTIMIZED FOR INDOOR SMART HOME Environment:
 *   - Distance range: 5m-20m (typical room-to-room distances)
 *   - Path loss exponent: 3.0-3.5 (indoor with obstacles)
 *   - Node count: 4-10 (typical home automation setup)
 *   - TX Power: 0 dBm (1mW - standard for ZigBee)
 * 
 * Purpose: Analyze the combined impact of:
 *   1. AWGN Gaussian Noise
 *   2. Rayleigh Fading (multipath in indoor)
 *   3. Inter-node Distance (room-to-room)
 *   4. Number of Nodes (smart home scale)
 */

#include "ns3/constant-position-mobility-model.h"
#include "ns3/core-module.h"
#include "ns3/log.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/mobility-module.h"
#include "ns3/packet.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/simulator.h"
#include "ns3/single-model-spectrum-channel.h"
#include "ns3/zigbee-module.h"
#include "ns3/netanim-module.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <random>
#include <vector>

using namespace ns3;
using namespace ns3::lrwpan;
using namespace ns3::zigbee;

NS_LOG_COMPONENT_DEFINE("ZigbeeIndoorSimulation");

// ============================================================
// GLOBAL VARIABLES
// ============================================================
ZigbeeStackContainer g_zigbeeStacks;
NodeContainer g_allNodes;
std::mt19937 g_rng(42);

// ============================================================
// CHANNEL MODEL CONFIGURATION - INDOOR OPTIMIZED
// ============================================================
struct ChannelConfig {
    // Control flags
    bool enableNoise = true;
    bool enableFading = true;
    
    // Topology parameters - REALISTIC DEFAULTS
    double nodeDistance = 10.0;            // 10m = typical room-to-room distance
    uint32_t numNodes = 6;                 // 6 nodes = typical smart home
    
    // Transmitter - ZigBee REALISTIC (CC2530, CC2652 modules)
    double txPowerDbm = 4.0;               // +4 dBm = typical ZigBee modules (up to +20 dBm available)
    
    // Path Loss (Log-distance model) - REALISTIC INDOOR
    // Based on IEEE 802.15.4 @ 2.4 GHz measurements
    double refDistance = 1.0;              // Reference distance (m)
    double refPathLossDb = 40.77;          // Free space loss at 1m for 2.4 GHz: 20*log10(4*pi*d/lambda)
    double pathLossExp = 2.0;              // 2.0 = free space, 2.5-3.0 = light indoor, 3.5-4.0 = heavy indoor
    
    // Gaussian Noise (AWGN) - REALISTIC
    double noiseFloorDbm = -174.0;         // Thermal noise density: -174 dBm/Hz @ room temp
    double noiseFigureDb = 3.0;            // Typical ZigBee receiver noise figure (2-6 dB)
    // Effective noise for 2 MHz BW: -174 + 10*log10(2e6) + 3 = -107.99 dBm
    
    // Receiver - ZigBee CC2530/CC2652 specs
    double sensitivityDbm = -97.0;         // CC2530: -97 dBm, CC2652: -100 dBm
    double snrThresholdDb = 3.0;           // O-QPSK with DSSS needs ~3-4 dB SNR
    
    // Calculated effective noise
    double effectiveNoiseDbm() const {
        return noiseFloorDbm + noiseFigureDb;
    }
};

ChannelConfig g_channel;

// ============================================================
// SIMULATION STATISTICS
// ============================================================
struct SimStats {
    // Packet counts
    uint32_t totalSent = 0;
    uint32_t totalReceived = 0;
    uint32_t totalDropped = 0;
    
    // Drop reasons
    uint32_t droppedByNoise = 0;
    uint32_t droppedByFading = 0;
    uint32_t droppedBySensitivity = 0;
    
    // Channel measurements
    std::vector<double> snrSamples;
    std::vector<double> rxPowerSamples;
    std::vector<double> fadingSamples;
    std::vector<double> distanceSamples;
    
    // Timing
    std::vector<double> delaysSamples;
    std::map<uint32_t, Time> sendTimes;
    Time firstSend = Seconds(0);
    Time lastRecv = Seconds(0);
    
    // Reset all stats
    void reset() {
        totalSent = totalReceived = totalDropped = 0;
        droppedByNoise = droppedByFading = droppedBySensitivity = 0;
        snrSamples.clear();
        rxPowerSamples.clear();
        fadingSamples.clear();
        distanceSamples.clear();
        delaysSamples.clear();
        sendTimes.clear();
        firstSend = lastRecv = Seconds(0);
    }
};

SimStats g_stats;

// ============================================================
// CHANNEL MODEL FUNCTIONS
// ============================================================

/**
 * Generate Rayleigh fading coefficient
 * E[h^2] = 1 (normalized)
 */
double GenerateRayleighFading()
{
    if (!g_channel.enableFading) {
        return 1.0;
    }
    
    std::normal_distribution<double> gaussian(0.0, 1.0 / std::sqrt(2.0));
    double real = gaussian(g_rng);
    double imag = gaussian(g_rng);
    return std::sqrt(real * real + imag * imag);
}

/**
 * Generate Gaussian noise power (AWGN)
 */
double GenerateNoisePower()
{
    if (!g_channel.enableNoise) {
        return -200.0;  // Effectively no noise
    }
    
    std::normal_distribution<double> variation(0.0, 1.0);
    return g_channel.effectiveNoiseDbm() + variation(g_rng);
}

/**
 * Calculate path loss: PL(d) = PL(d0) + 10*n*log10(d/d0)
 * 
 * For indoor: n = 3.0 (walls, furniture, multipath)
 */
double CalculatePathLoss(double distance)
{
    double d = std::max(distance, g_channel.refDistance);
    
    double pathLossDb = g_channel.refPathLossDb + 
                        10.0 * g_channel.pathLossExp * 
                        std::log10(d / g_channel.refDistance);
    
    return pathLossDb;
}

/**
 * Simulate complete channel for one packet transmission
 * Returns: true if packet successfully received
 */
bool SimulateChannel(uint32_t srcId, uint32_t dstId, uint32_t pktSize)
{
    // Get actual distance between nodes
    Ptr<MobilityModel> srcMob = g_allNodes.Get(srcId)->GetObject<MobilityModel>();
    Ptr<MobilityModel> dstMob = g_allNodes.Get(dstId)->GetObject<MobilityModel>();
    double distance = srcMob->GetDistanceFrom(dstMob);
    
    g_stats.distanceSamples.push_back(distance);
    
    // === Step 1: Path Loss (distance-dependent, indoor) ===
    double pathLossDb = CalculatePathLoss(distance);
    
    // === Step 2: Rayleigh Fading (indoor multipath) ===
    double fadingCoef = GenerateRayleighFading();
    double fadingDb = 20.0 * std::log10(std::max(fadingCoef, 1e-10));
    
    // === Step 3: Calculate Received Power ===
    // Pr = Pt - PathLoss + Fading (all in dB)
    double rxPowerDbm = g_channel.txPowerDbm - pathLossDb + fadingDb;
    
    // === Step 4: Add Noise ===
    double noisePowerDbm = GenerateNoisePower();
    
    // === Step 5: Calculate SNR ===
    double snrDb = rxPowerDbm - noisePowerDbm;
    
    // === Step 6: Store measurements ===
    g_stats.snrSamples.push_back(snrDb);
    g_stats.rxPowerSamples.push_back(rxPowerDbm);
    g_stats.fadingSamples.push_back(fadingCoef);
    
    // === Step 7: Determine packet success ===
    
    // Check 1: Receiver sensitivity (absolute minimum power)
    if (rxPowerDbm < g_channel.sensitivityDbm) {
        g_stats.droppedBySensitivity++;
        NS_LOG_DEBUG("Dropped by sensitivity: " << rxPowerDbm << " < " 
                     << g_channel.sensitivityDbm << " dBm (distance=" << distance << "m)");
        return false;
    }
    
    // Check 2: SNR threshold (noise comparison)
    if (snrDb < g_channel.snrThresholdDb) {
        // Classify: Fading or Noise dominated?
        if (fadingCoef < 0.5 && g_channel.enableFading) {
            g_stats.droppedByFading++;
        } else {
            g_stats.droppedByNoise++;
        }
        NS_LOG_DEBUG("Dropped by low SNR: " << snrDb << " < " 
                     << g_channel.snrThresholdDb << " dB (distance=" << distance << "m)");
        return false;
    }
    
    return true;
}

// ============================================================
// HELPER FUNCTIONS
// ============================================================

void PrintMsg(Ptr<ZigbeeStack> stack, const std::string& msg)
{
    std::cout << std::fixed << std::setprecision(2)
              << "[" << Simulator::Now().GetSeconds() << "s] "
              << "Node " << stack->GetNode()->GetId() << ": "
              << msg << std::endl;
}

// ============================================================
// ZIGBEE CALLBACKS
// ============================================================

void OnDataReceived(Ptr<ZigbeeStack> stack, ApsdeDataIndicationParams params, Ptr<Packet> pkt)
{
    g_stats.totalReceived++;
    g_stats.lastRecv = Simulator::Now();
    
    uint32_t uid = pkt->GetUid();
    if (g_stats.sendTimes.count(uid)) {
        double delayMs = (Simulator::Now() - g_stats.sendTimes[uid]).GetMilliSeconds();
        g_stats.delaysSamples.push_back(delayMs);
        g_stats.sendTimes.erase(uid);
    }
    
    PrintMsg(stack, "RECEIVED packet (size=" + std::to_string(pkt->GetSize()) + " bytes)");
}

void OnDataConfirm(Ptr<ZigbeeStack> stack, ApsdeDataConfirmParams params)
{
    if (params.m_status != ApsStatus::SUCCESS) {
        g_stats.totalDropped++;
    }
}

void OnNetworkFormation(Ptr<ZigbeeStack> stack, NlmeNetworkFormationConfirmParams params)
{
    if (params.m_status == NwkStatus::SUCCESS) {
        PrintMsg(stack, "COORDINATOR: Network formed successfully");
    }
}

void OnNetworkDiscovery(Ptr<ZigbeeStack> stack, NlmeNetworkDiscoveryConfirmParams params)
{
    if (params.m_status == NwkStatus::SUCCESS && !params.m_netDescList.empty()) {
        PrintMsg(stack, "Found network, joining...");
        
        NlmeJoinRequestParams joinParams;
        CapabilityInformation capInfo;
        capInfo.SetDeviceType(ROUTER);
        capInfo.SetAllocateAddrOn(true);
        
        joinParams.m_rejoinNetwork = JoiningMethod::ASSOCIATION;
        joinParams.m_capabilityInfo = capInfo.GetCapability();
        joinParams.m_extendedPanId = params.m_netDescList[0].m_extPanId;
        
        Simulator::ScheduleNow(&ZigbeeNwk::NlmeJoinRequest, 
                               stack->GetNwk(), joinParams);
    }
}

void OnJoinConfirm(Ptr<ZigbeeStack> stack, NlmeJoinConfirmParams params)
{
    if (params.m_status == NwkStatus::SUCCESS) {
        PrintMsg(stack, "JOINED network successfully");
        
        NlmeStartRouterRequestParams routerParams;
        Simulator::ScheduleNow(&ZigbeeNwk::NlmeStartRouterRequest,
                               stack->GetNwk(), routerParams);
    }
}

void OnRouteDiscovery(Ptr<ZigbeeStack> stack, NlmeRouteDiscoveryConfirmParams params)
{
    PrintMsg(stack, params.m_status == NwkStatus::SUCCESS ? 
             "Route discovery SUCCESS" : "Route discovery FAILED");
}

// ============================================================
// DATA TRANSMISSION
// ============================================================

void SendSensorData(Ptr<ZigbeeStack> sensor, Ptr<ZigbeeStack> coordinator)
{
    uint32_t srcId = sensor->GetNode()->GetId();
    uint32_t dstId = coordinator->GetNode()->GetId();
    uint32_t pktSize = 10;
    
    g_stats.totalSent++;
    if (g_stats.firstSend == Seconds(0)) {
        g_stats.firstSend = Simulator::Now();
    }
    
    // Simulate channel effects
    bool success = SimulateChannel(srcId, dstId, pktSize);
    
    if (!success) {
        g_stats.totalDropped++;
        PrintMsg(sensor, "DROPPED by channel");
        return;
    }
    
    // Create and send packet
    Ptr<Packet> pkt = Create<Packet>(pktSize);
    g_stats.sendTimes[pkt->GetUid()] = Simulator::Now();
    
    ApsdeDataRequestParams params;
    ZigbeeApsTxOptions txOpt;
    
    params.m_useAlias = false;
    params.m_txOptions = txOpt.GetTxOptions();
    params.m_srcEndPoint = 1;
    params.m_dstEndPoint = 1;
    params.m_clusterId = 0x0402;
    params.m_profileId = 0x0104;
    params.m_dstAddrMode = ApsDstAddressMode::DST_ADDR16_DST_ENDPOINT_PRESENT;
    params.m_dstAddr16 = coordinator->GetNwk()->GetNetworkAddress();
    
    PrintMsg(sensor, "SENDING sensor data...");
    Simulator::ScheduleNow(&ZigbeeAps::ApsdeDataRequest,
                           sensor->GetAps(), params, pkt);
}

// ============================================================
// STATISTICS REPORTING
// ============================================================

void PrintResults(const std::string& scenario)
{
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║            INDOOR SIMULATION RESULTS                         ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
    std::cout << "║ Scenario: " << std::left << std::setw(51) << scenario << "║\n";
    std::cout << "║ Distance: " << std::setw(8) << g_channel.nodeDistance << " m"
              << "  Nodes: " << std::setw(35) << g_channel.numNodes << "║\n";
    std::cout << "║ Noise: " << std::setw(6) << (g_channel.enableNoise ? "ON" : "OFF")
              << "  Fading: " << std::setw(37) << (g_channel.enableFading ? "ON" : "OFF") << "║\n";
    std::cout << "║ Path Loss Exp: " << std::setw(45) << g_channel.pathLossExp << "║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
    
    // Packet statistics
    double pdr = g_stats.totalSent > 0 ? 
                 100.0 * g_stats.totalReceived / g_stats.totalSent : 0.0;
    
    std::cout << "║ PACKET STATISTICS                                            ║\n";
    std::cout << "║   Total Sent:     " << std::setw(43) << g_stats.totalSent << "║\n";
    std::cout << "║   Total Received: " << std::setw(43) << g_stats.totalReceived << "║\n";
    std::cout << "║   Total Dropped:  " << std::setw(43) << g_stats.totalDropped << "║\n";
    std::cout << "║   PDR:            " << std::fixed << std::setprecision(2) 
              << std::setw(40) << pdr << " % ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
    
    // Drop breakdown
    std::cout << "║ DROP ANALYSIS                                                ║\n";
    std::cout << "║   By Noise:       " << std::setw(43) << g_stats.droppedByNoise << "║\n";
    std::cout << "║   By Fading:      " << std::setw(43) << g_stats.droppedByFading << "║\n";
    std::cout << "║   By Sensitivity: " << std::setw(43) << g_stats.droppedBySensitivity << "║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
    
    // Channel quality
    if (!g_stats.snrSamples.empty()) {
        double sumSnr = 0, minSnr = g_stats.snrSamples[0], maxSnr = g_stats.snrSamples[0];
        for (double s : g_stats.snrSamples) {
            sumSnr += s;
            minSnr = std::min(minSnr, s);
            maxSnr = std::max(maxSnr, s);
        }
        double avgSnr = sumSnr / g_stats.snrSamples.size();
        
        std::cout << "║ CHANNEL QUALITY                                              ║\n";
        std::cout << "║   Avg SNR:  " << std::setw(10) << avgSnr << " dB"
                  << std::setw(36) << " " << "║\n";
        std::cout << "║   Min SNR:  " << std::setw(10) << minSnr << " dB"
                  << std::setw(36) << " " << "║\n";
        std::cout << "║   Max SNR:  " << std::setw(10) << maxSnr << " dB"
                  << std::setw(36) << " " << "║\n";
    }
    
    // Distance statistics
    if (!g_stats.distanceSamples.empty()) {
        double sumDist = 0, minDist = g_stats.distanceSamples[0], maxDist = g_stats.distanceSamples[0];
        for (double d : g_stats.distanceSamples) {
            sumDist += d;
            minDist = std::min(minDist, d);
            maxDist = std::max(maxDist, d);
        }
        double avgDist = sumDist / g_stats.distanceSamples.size();
        
        std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ DISTANCE STATISTICS (Indoor)                                 ║\n";
        std::cout << "║   Average: " << std::setw(10) << std::setprecision(2) << avgDist << " m"
                  << std::setw(37) << " " << "║\n";
        std::cout << "║   Min:     " << std::setw(10) << minDist << " m"
                  << std::setw(37) << " " << "║\n";
        std::cout << "║   Max:     " << std::setw(10) << maxDist << " m"
                  << std::setw(37) << " " << "║\n";
    }
    
    // Delay statistics
    if (!g_stats.delaysSamples.empty()) {
        double sumD = 0, minD = g_stats.delaysSamples[0], maxD = g_stats.delaysSamples[0];
        for (double d : g_stats.delaysSamples) {
            sumD += d;
            minD = std::min(minD, d);
            maxD = std::max(maxD, d);
        }
        double avgD = sumD / g_stats.delaysSamples.size();
        
        std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ DELAY (ms)                                                   ║\n";
        std::cout << "║   Average: " << std::setw(10) << std::setprecision(2) << avgD
                  << std::setw(39) << " " << "║\n";
        std::cout << "║   Min:     " << std::setw(10) << minD
                  << std::setw(39) << " " << "║\n";
        std::cout << "║   Max:     " << std::setw(10) << maxD
                  << std::setw(39) << " " << "║\n";
    }
    
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
}

void ExportCSV(const std::string& filename, const std::string& scenario)
{
    std::ofstream file;
    bool exists = std::ifstream(filename).good();
    file.open(filename, std::ios::app);
    
    if (!exists) {
        file << "Scenario,Distance,NumNodes,Noise,Fading,"
             << "Sent,Received,Dropped,"
             << "DroppedNoise,DroppedFading,DroppedSensitivity,"
             << "PDR,AvgSNR,MinSNR,MaxSNR,AvgDelay\n";
    }
    
    double pdr = g_stats.totalSent > 0 ? 
                 100.0 * g_stats.totalReceived / g_stats.totalSent : 0.0;
    
    // Calculate averages
    double avgSnr = 0, minSnr = 0, maxSnr = 0;
    if (!g_stats.snrSamples.empty()) {
        minSnr = maxSnr = g_stats.snrSamples[0];
        for (double s : g_stats.snrSamples) {
            avgSnr += s;
            minSnr = std::min(minSnr, s);
            maxSnr = std::max(maxSnr, s);
        }
        avgSnr /= g_stats.snrSamples.size();
    }
    
    double avgDelay = 0;
    if (!g_stats.delaysSamples.empty()) {
        for (double d : g_stats.delaysSamples) avgDelay += d;
        avgDelay /= g_stats.delaysSamples.size();
    }
    
    file << scenario << ","
         << g_channel.nodeDistance << ","
         << g_channel.numNodes << ","
         << (g_channel.enableNoise ? 1 : 0) << ","
         << (g_channel.enableFading ? 1 : 0) << ","
         << g_stats.totalSent << ","
         << g_stats.totalReceived << ","
         << g_stats.totalDropped << ","
         << g_stats.droppedByNoise << ","
         << g_stats.droppedByFading << ","
         << g_stats.droppedBySensitivity << ","
         << pdr << ","
         << avgSnr << "," << minSnr << "," << maxSnr << ","
         << avgDelay << "\n";
    
    file.close();
    std::cout << "Results exported to: " << filename << std::endl;
}

// ============================================================
// MAIN
// ============================================================

int main(int argc, char* argv[])
{
    // Default parameters - INDOOR OPTIMIZED
    uint32_t numNodes = 6;                  // Typical smart home
    uint32_t simTime = 120;
    uint32_t numPackets = 50;
    double packetInterval = 2.0;
    bool enableNoise = true;
    bool enableFading = true;
    double noiseFloor = -100.0;
    double nodeDistance = 10.0;             // 10m = typical indoor distance
    double pathLossExp = 3.0;               // Indoor with obstacles
    std::string scenario = "Default";
    std::string csvFile = "zigbee_extended_results.csv";
    
    // Command line parsing
    CommandLine cmd;
    cmd.AddValue("nodes", "Number of nodes (4-10 for smart home)", numNodes);
    cmd.AddValue("distance", "Distance between nodes in meters (5-20m indoor)", nodeDistance);
    cmd.AddValue("time", "Simulation time (s)", simTime);
    cmd.AddValue("packets", "Number of packets to send", numPackets);
    cmd.AddValue("interval", "Packet interval (s)", packetInterval);
    cmd.AddValue("noise", "Enable Gaussian noise", enableNoise);
    cmd.AddValue("fading", "Enable Rayleigh fading", enableFading);
    cmd.AddValue("noiseFloor", "Noise floor (dBm)", noiseFloor);
    cmd.AddValue("pathLossExp", "Path loss exponent (3.0-3.5 indoor)", pathLossExp);
    cmd.AddValue("scenario", "Scenario name", scenario);
    cmd.AddValue("csv", "Output CSV file", csvFile);
    cmd.Parse(argc, argv);
    
    // Apply configuration
    g_channel.enableNoise = enableNoise;
    g_channel.enableFading = enableFading;
    g_channel.noiseFloorDbm = noiseFloor;
    g_channel.nodeDistance = nodeDistance;
    g_channel.numNodes = numNodes;
    g_channel.pathLossExp = pathLossExp;
    
    // Auto-generate scenario name if default
    if (scenario == "Default") {
        scenario = "D" + std::to_string((int)nodeDistance) + 
                  "_N" + std::to_string(numNodes);
        if (enableNoise) scenario += "_Noise";
        if (enableFading) scenario += "_Fading";
    }
    
    // Print configuration
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║     ZIGBEE INDOOR SMART HOME SIMULATION                      ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
    std::cout << "║ Scenario:    " << std::left << std::setw(48) << scenario << "║\n";
    std::cout << "║ Nodes:       " << std::setw(48) << numNodes << "║\n";
    std::cout << "║ Distance:    " << std::setw(45) << nodeDistance << " m ║\n";
    std::cout << "║ Packets:     " << std::setw(48) << numPackets << "║\n";
    std::cout << "║ Noise:       " << std::setw(48) << (enableNoise ? "ENABLED" : "DISABLED") << "║\n";
    std::cout << "║ Fading:      " << std::setw(48) << (enableFading ? "ENABLED" : "DISABLED") << "║\n";
    std::cout << "║ Path Loss n: " << std::setw(45) << pathLossExp << "   ║\n";
    std::cout << "║ TX Power:    " << std::setw(45) << g_channel.txPowerDbm << " dBm║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    // Setup logging
    LogComponentEnableAll(LogLevel(LOG_PREFIX_TIME | LOG_PREFIX_NODE));
    RngSeedManager::SetSeed(42);
    RngSeedManager::SetRun(1);
    
    // Create nodes
    g_allNodes.Create(numNodes);
    
    // LR-WPAN setup
    LrWpanHelper lrWpanHelper;
    NetDeviceContainer devices = lrWpanHelper.Install(g_allNodes);
    lrWpanHelper.SetExtendedAddresses(devices);
    
    // Channel setup - INDOOR MODEL
    Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel>();
    Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel>();
    lossModel->SetPathLossExponent(pathLossExp);
    lossModel->SetReference(1.0, 40.0);  // 40 dB at 1m (indoor)
    
    Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel>();
    channel->AddPropagationLossModel(lossModel);
    channel->SetPropagationDelayModel(delayModel);
    
    for (uint32_t i = 0; i < devices.GetN(); i++) {
        devices.Get(i)->GetObject<LrWpanNetDevice>()->SetChannel(channel);
    }
    
    // Mobility - Grid layout with INDOOR spacing
    MobilityHelper mobility;
    uint32_t gridWidth = (uint32_t)std::ceil(std::sqrt((double)numNodes));
    
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(nodeDistance),
                                  "DeltaY", DoubleValue(nodeDistance),
                                  "GridWidth", UintegerValue(gridWidth),
                                  "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(g_allNodes);
    
    // Print node positions for verification
    std::cout << "Node Positions (Indoor Layout):\n";
    for (uint32_t i = 0; i < numNodes; i++) {
        Ptr<MobilityModel> mob = g_allNodes.Get(i)->GetObject<MobilityModel>();
        Vector pos = mob->GetPosition();
        std::cout << "  Node " << i << ": (" << pos.x << ", " << pos.y << ") m\n";
    }
    std::cout << "\n";
    
    // ZigBee stack
    ZigbeeHelper zigbeeHelper;
    g_zigbeeStacks = zigbeeHelper.Install(devices);
    
    // Configure callbacks
    for (uint32_t i = 0; i < g_zigbeeStacks.GetN(); i++) {
        Ptr<ZigbeeStack> stack = g_zigbeeStacks.Get(i)->GetObject<ZigbeeStack>();
        stack->GetNwk()->AssignStreams(i * 10);
        
        stack->GetAps()->SetApsdeDataIndicationCallback(
            MakeBoundCallback(&OnDataReceived, stack));
        stack->GetAps()->SetApsdeDataConfirmCallback(
            MakeBoundCallback(&OnDataConfirm, stack));
        stack->GetNwk()->SetNlmeNetworkFormationConfirmCallback(
            MakeBoundCallback(&OnNetworkFormation, stack));
        stack->GetNwk()->SetNlmeNetworkDiscoveryConfirmCallback(
            MakeBoundCallback(&OnNetworkDiscovery, stack));
        stack->GetNwk()->SetNlmeJoinConfirmCallback(
            MakeBoundCallback(&OnJoinConfirm, stack));
        stack->GetNwk()->SetNlmeRouteDiscoveryConfirmCallback(
            MakeBoundCallback(&OnRouteDiscovery, stack));
    }
    
    Ptr<ZigbeeStack> coordinator = g_zigbeeStacks.Get(0)->GetObject<ZigbeeStack>();
    Ptr<ZigbeeStack> sensor = g_zigbeeStacks.Get(numNodes - 1)->GetObject<ZigbeeStack>();
    
    // ===== NETWORK FORMATION =====
    NlmeNetworkFormationRequestParams formParams;
    formParams.m_scanChannelList.channelPageCount = 1;
    formParams.m_scanChannelList.channelsField[0] = ALL_CHANNELS;
    formParams.m_scanDuration = 0;
    formParams.m_superFrameOrder = 15;
    formParams.m_beaconOrder = 15;
    
    Simulator::ScheduleWithContext(coordinator->GetNode()->GetId(),
                                   Seconds(1.0),
                                   &ZigbeeNwk::NlmeNetworkFormationRequest,
                                   coordinator->GetNwk(), formParams);
    
    // ===== DEVICE JOINING =====
    NlmeNetworkDiscoveryRequestParams discParams;
    discParams.m_scanChannelList.channelPageCount = 1;
    discParams.m_scanChannelList.channelsField[0] = 0x00007800;
    discParams.m_scanDuration = 2;
    
    double joinTime = 3.0;
    for (uint32_t i = 1; i < numNodes; i++) {
        Ptr<ZigbeeStack> stack = g_zigbeeStacks.Get(i)->GetObject<ZigbeeStack>();
        Simulator::ScheduleWithContext(stack->GetNode()->GetId(),
                                       Seconds(joinTime),
                                       &ZigbeeNwk::NlmeNetworkDiscoveryRequest,
                                       stack->GetNwk(), discParams);
        joinTime += 2.0;
    }
    
    // ===== ROUTE DISCOVERY =====
    double routeTime = joinTime + 3.0;
    NlmeRouteDiscoveryRequestParams routeParams;
    routeParams.m_dstAddrMode = NO_ADDRESS;
    Simulator::Schedule(Seconds(routeTime),
                       &ZigbeeNwk::NlmeRouteDiscoveryRequest,
                       coordinator->GetNwk(), routeParams);
    
    // ===== DATA TRANSMISSION =====
    double dataStartTime = routeTime + 5.0;
    for (uint32_t i = 0; i < numPackets; i++) {
        Simulator::Schedule(Seconds(dataStartTime + i * packetInterval),
                           &SendSensorData, sensor, coordinator);
    }
    
    // ===== NETANIM VISUALIZATION =====
    AnimationInterface anim("zigbee-indoor.xml");
    anim.UpdateNodeDescription(coordinator->GetNode(), "Coordinator");
    anim.UpdateNodeColor(coordinator->GetNode(), 255, 0, 0);  // Red
    
    for (uint32_t i = 1; i < numNodes - 1; i++) {
        anim.UpdateNodeDescription(g_allNodes.Get(i), "Router-" + std::to_string(i));
        anim.UpdateNodeColor(g_allNodes.Get(i), 0, 0, 255);  // Blue
    }
    
    anim.UpdateNodeDescription(sensor->GetNode(), "Sensor");
    anim.UpdateNodeColor(sensor->GetNode(), 0, 255, 0);  // Green
    
    // ===== SCHEDULE RESULTS OUTPUT =====
    Simulator::Schedule(Seconds(simTime - 1.0), &PrintResults, scenario);
    Simulator::Schedule(Seconds(simTime - 0.5), &ExportCSV, csvFile, scenario);
    
    // ===== RUN =====
    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    Simulator::Destroy();
    
    return 0;
}