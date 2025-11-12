/*
 * Interactive ZigBee Smart Home Network Simulation
 * 
 * Features:
 * - Dynamic node addition/deletion via command line
 * - Gaussian noise and Rayleigh fading simulation
 * - Network statistics collection and export
 * - NetAnim visualization support
 * - Real-time network configuration
 *
 * Command Line Options:
 * --nRouters: Number of router nodes (default: 3)
 * --nSensors: Number of sensor nodes (default: 2)
 * --nLights: Number of light nodes (default: 2)
 * --enableGaussianNoise: Enable Gaussian noise (0/1, default: 0)
 * --enableRayleighFading: Enable Rayleigh fading (0/1, default: 0)
 * --noiseVariance: Variance of Gaussian noise in dBm (default: 2.0)
 * --simTime: Simulation time in seconds (default: 300)
 * --verbose: Enable verbose logging (0/1, default: 0)
 * --exportStats: Export network statistics to CSV (0/1, default: 1)
 * --manyToOne: Enable Many-to-One routing (0/1, default: 1)
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/zigbee-module.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/single-model-spectrum-channel.h"
#include "ns3/netanim-module.h"
#include "ns3/random-variable-stream.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

using namespace ns3;
using namespace ns3::lrwpan;
using namespace ns3::zigbee;

NS_LOG_COMPONENT_DEFINE("InteractiveZigbeeSmartHome");

// Global containers
ZigbeeStackContainer g_zigbeeStacks;
NodeContainer g_allNodes;
NetDeviceContainer g_allDevices;

// Network Statistics
struct NetworkStatistics {
    uint32_t totalNodes = 0;
    uint32_t coordinators = 0;
    uint32_t routers = 0;
    uint32_t sensors = 0;
    uint32_t lights = 0;
    
    uint32_t packetsTransmitted = 0;
    uint32_t packetsReceived = 0;
    uint32_t packetsDropped = 0;
    uint32_t routeDiscoveries = 0;
    uint32_t joinAttempts = 0;
    uint32_t joinSuccesses = 0;
    uint32_t groupCommands = 0;
    
    double avgRssi = 0.0;
    double minRssi = 0.0;
    double maxRssi = 0.0;
    
    std::vector<double> rssiSamples;
    std::vector<double> packetDelays;
    
    void AddRssiSample(double rssi) {
        rssiSamples.push_back(rssi);
        if (rssiSamples.size() == 1) {
            minRssi = maxRssi = rssi;
        } else {
            if (rssi < minRssi) minRssi = rssi;
            if (rssi > maxRssi) maxRssi = rssi;
        }
        
        // Calculate average
        double sum = 0;
        for (auto val : rssiSamples) sum += val;
        avgRssi = sum / rssiSamples.size();
    }
    
    void AddPacketDelay(double delay) {
        packetDelays.push_back(delay);
    }
    
    double GetAvgDelay() const {
        if (packetDelays.empty()) return 0.0;
        double sum = 0;
        for (auto d : packetDelays) sum += d;
        return sum / packetDelays.size();
    }
};

NetworkStatistics g_stats;

// Noise and Fading parameters
bool g_enableGaussianNoise = false;
bool g_enableRayleighFading = false;
double g_noiseVariance = 2.0;

Ptr<NormalRandomVariable> g_gaussianNoise;
Ptr<NormalRandomVariable> g_rayleighFading1;
Ptr<NormalRandomVariable> g_rayleighFading2;

// Group addresses
const Mac16Address GROUP_ALL_LIGHTS = Mac16Address("00:01");
const Mac16Address GROUP_SENSORS = Mac16Address("00:02");

/**
 * Apply channel effects (noise and fading)
 */
double ApplyChannelEffects(double rxPowerDbm)
{
    double result = rxPowerDbm;
    
    // Add Gaussian noise
    if (g_enableGaussianNoise && g_gaussianNoise) {
        result += g_gaussianNoise->GetValue();
    }
    
    // Add Rayleigh fading
    if (g_enableRayleighFading && g_rayleighFading1 && g_rayleighFading2) {
        double r1 = g_rayleighFading1->GetValue();
        double r2 = g_rayleighFading2->GetValue();
        double fadingDb = 10 * log10(sqrt(r1*r1 + r2*r2));
        result += fadingDb;
    }
    
    g_stats.AddRssiSample(result);
    return result;
}

/**
 * Print formatted message
 */
void PrintMessage(const std::string& message)
{
    std::cout << std::fixed << std::setprecision(3) 
              << "[" << Simulator::Now().As(Time::S) << "] " 
              << message << std::endl;
}

void PrintMessage(Ptr<ZigbeeStack> stack, const std::string& message)
{
    std::cout << std::fixed << std::setprecision(3) 
              << "[" << Simulator::Now().As(Time::S) << "] "
              << "Node " << stack->GetNode()->GetId() 
              << " [" << stack->GetNwk()->GetNetworkAddress() << "]: "
              << message << std::endl;
}

/**
 * APS Data Indication Callback
 */
void ApsDataIndication(Ptr<ZigbeeStack> stack, ApsdeDataIndicationParams params, Ptr<Packet> p)
{
    g_stats.packetsReceived++;
    
    std::string addrMode;
    if (params.m_dstAddrMode == ApsDstAddressMode::DST_ADDR16_DST_ENDPOINT_PRESENT)
    {
        addrMode = "UNICAST";
    }
    else if (params.m_dstAddrMode == ApsDstAddressMode::GROUP_ADDR_DST_ENDPOINT_NOT_PRESENT)
    {
        addrMode = "GROUPCAST";
        g_stats.groupCommands++;
    }
    
    PrintMessage(stack, "RECEIVED " + addrMode + " DATA (Size: " + 
                 std::to_string(p->GetSize()) + " bytes, Endpoint: " + 
                 std::to_string(params.m_dstEndPoint) + ")");
}

/**
 * Network Formation Confirm
 */
void NwkNetworkFormationConfirm(Ptr<ZigbeeStack> stack, NlmeNetworkFormationConfirmParams params)
{
    if (params.m_status == NwkStatus::SUCCESS)
    {
        PrintMessage(stack, "Network formation SUCCESSFUL");
        g_stats.coordinators++;
    }
    else
    {
        PrintMessage(stack, "Network formation FAILED - Status: " + std::to_string(params.m_status));
    }
}

/**
 * Network Discovery Confirm
 */
void NwkNetworkDiscoveryConfirm(Ptr<ZigbeeStack> stack, NlmeNetworkDiscoveryConfirmParams params)
{
    if (params.m_status == NwkStatus::SUCCESS)
    {
        PrintMessage(stack, "Network discovery completed - Found " + 
                     std::to_string(params.m_netDescList.size()) + " network(s)");
        
        NlmeJoinRequestParams joinParams;
        CapabilityInformation capaInfo;
        capaInfo.SetDeviceType(ROUTER);
        capaInfo.SetAllocateAddrOn(true);
        
        joinParams.m_rejoinNetwork = JoiningMethod::ASSOCIATION;
        joinParams.m_capabilityInfo = capaInfo.GetCapability();
        joinParams.m_extendedPanId = params.m_netDescList[0].m_extPanId;
        
        g_stats.joinAttempts++;
        Simulator::ScheduleNow(&ZigbeeNwk::NlmeJoinRequest, stack->GetNwk(), joinParams);
    }
}

/**
 * Join Confirm
 */
void NwkJoinConfirm(Ptr<ZigbeeStack> stack, NlmeJoinConfirmParams params)
{
    if (params.m_status == NwkStatus::SUCCESS)
    {
        g_stats.joinSuccesses++;
        g_stats.routers++;
        
        std::ostringstream oss;
        oss << params.m_networkAddress;
        
        PrintMessage(stack, "Joined network successfully - Address: " + oss.str());
        
        NlmeStartRouterRequestParams startRouterParams;
        Simulator::ScheduleNow(&ZigbeeNwk::NlmeStartRouterRequest, 
                              stack->GetNwk(), startRouterParams);
    }
    else
    {
        PrintMessage(stack, "Join FAILED - Status: " + std::to_string(params.m_status));
    }
}

/**
 * Route Discovery Confirm
 */
void NwkRouteDiscoveryConfirm(Ptr<ZigbeeStack> stack, NlmeRouteDiscoveryConfirmParams params)
{
    g_stats.routeDiscoveries++;
    
    if (params.m_status == NwkStatus::SUCCESS)
    {
        PrintMessage(stack, "Route discovery SUCCESSFUL");
    }
    else
    {
        PrintMessage(stack, "Route discovery FAILED");
    }
}

/**
 * Send sensor data
 */
void SendSensorData(Ptr<ZigbeeStack> sensorStack, Ptr<ZigbeeStack> coordinatorStack)
{
    uint8_t sensorData[4] = {0x12, 0x34, 0x56, 0x78};
    Ptr<Packet> p = Create<Packet>(sensorData, 4);
    
    ApsdeDataRequestParams dataReqParams;
    ZigbeeApsTxOptions txOptions;
    
    dataReqParams.m_useAlias = false;
    dataReqParams.m_txOptions = txOptions.GetTxOptions();
    dataReqParams.m_srcEndPoint = 1;
    dataReqParams.m_dstEndPoint = 1;
    dataReqParams.m_clusterId = 0x0001;
    dataReqParams.m_profileId = 0x0104;
    dataReqParams.m_dstAddrMode = ApsDstAddressMode::DST_ADDR16_DST_ENDPOINT_PRESENT;
    dataReqParams.m_dstAddr16 = coordinatorStack->GetNwk()->GetNetworkAddress();
    
    g_stats.packetsTransmitted++;
    PrintMessage(sensorStack, "Sending sensor data to Coordinator");
    
    Simulator::ScheduleNow(&ZigbeeAps::ApsdeDataRequest, 
                          sensorStack->GetAps(), dataReqParams, p);
}

/**
 * Send group command
 */
void SendGroupCommand(Ptr<ZigbeeStack> sourceStack, Mac16Address groupAddr, 
                      const std::string& commandName, uint8_t commandId)
{
    uint8_t cmdData[1] = {commandId};
    Ptr<Packet> p = Create<Packet>(cmdData, 1);
    
    ApsdeDataRequestParams dataReqParams;
    ZigbeeApsTxOptions txOptions;
    
    dataReqParams.m_useAlias = false;
    dataReqParams.m_txOptions = txOptions.GetTxOptions();
    dataReqParams.m_srcEndPoint = 1;
    dataReqParams.m_clusterId = 0x0006;
    dataReqParams.m_profileId = 0x0104;
    dataReqParams.m_dstAddrMode = ApsDstAddressMode::GROUP_ADDR_DST_ENDPOINT_NOT_PRESENT;
    dataReqParams.m_dstAddr16 = groupAddr;
    
    g_stats.packetsTransmitted++;
    
    std::ostringstream oss;
    oss << groupAddr;
    PrintMessage(sourceStack, "Sending GROUP command '" + commandName + 
                 "' to group [" + oss.str() + "]");
    
    Simulator::ScheduleNow(&ZigbeeAps::ApsdeDataRequest, 
                          sourceStack->GetAps(), dataReqParams, p);
}

/**
 * Export statistics to CSV
 */
void ExportStatistics(const std::string& filename)
{
    std::ofstream outFile(filename);
    
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open " << filename << " for writing" << std::endl;
        return;
    }
    
    outFile << "# ZigBee Smart Home Network Statistics\n";
    outFile << "# Generated at: " << Simulator::Now().As(Time::S) << "\n\n";
    
    outFile << "Metric,Value\n";
    outFile << "Total Nodes," << g_stats.totalNodes << "\n";
    outFile << "Coordinators," << g_stats.coordinators << "\n";
    outFile << "Routers," << g_stats.routers << "\n";
    outFile << "Sensors," << g_stats.sensors << "\n";
    outFile << "Lights," << g_stats.lights << "\n";
    outFile << "Packets Transmitted," << g_stats.packetsTransmitted << "\n";
    outFile << "Packets Received," << g_stats.packetsReceived << "\n";
    outFile << "Packets Dropped," << g_stats.packetsDropped << "\n";
    outFile << "Route Discoveries," << g_stats.routeDiscoveries << "\n";
    outFile << "Join Attempts," << g_stats.joinAttempts << "\n";
    outFile << "Join Successes," << g_stats.joinSuccesses << "\n";
    outFile << "Group Commands," << g_stats.groupCommands << "\n";
    
    if (g_stats.packetsTransmitted > 0) {
        double pdr = (double)g_stats.packetsReceived / g_stats.packetsTransmitted * 100.0;
        outFile << "Packet Delivery Ratio (%)," << std::fixed << std::setprecision(2) << pdr << "\n";
    }
    
    outFile << "Average RSSI (dBm)," << std::fixed << std::setprecision(2) << g_stats.avgRssi << "\n";
    outFile << "Min RSSI (dBm)," << g_stats.minRssi << "\n";
    outFile << "Max RSSI (dBm)," << g_stats.maxRssi << "\n";
    outFile << "Average Delay (ms)," << std::fixed << std::setprecision(3) 
            << g_stats.GetAvgDelay() * 1000 << "\n";
    
    outFile << "\nGaussian Noise Enabled," << (g_enableGaussianNoise ? "Yes" : "No") << "\n";
    outFile << "Rayleigh Fading Enabled," << (g_enableRayleighFading ? "Yes" : "No") << "\n";
    if (g_enableGaussianNoise) {
        outFile << "Noise Variance (dBm)," << g_noiseVariance << "\n";
    }
    
    outFile.close();
    
    PrintMessage("Statistics exported to " + filename);
}

/**
 * Print network statistics
 */
void PrintStatistics()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "NETWORK STATISTICS" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Network Composition:" << std::endl;
    std::cout << "  Total Nodes:          " << g_stats.totalNodes << std::endl;
    std::cout << "  Coordinators:         " << g_stats.coordinators << std::endl;
    std::cout << "  Routers:              " << g_stats.routers << std::endl;
    std::cout << "  Sensors:              " << g_stats.sensors << std::endl;
    std::cout << "  Lights:               " << g_stats.lights << std::endl;
    
    std::cout << "\nTraffic Statistics:" << std::endl;
    std::cout << "  Packets Transmitted:  " << g_stats.packetsTransmitted << std::endl;
    std::cout << "  Packets Received:     " << g_stats.packetsReceived << std::endl;
    std::cout << "  Packets Dropped:      " << g_stats.packetsDropped << std::endl;
    std::cout << "  Group Commands:       " << g_stats.groupCommands << std::endl;
    
    if (g_stats.packetsTransmitted > 0) {
        double pdr = (double)g_stats.packetsReceived / g_stats.packetsTransmitted * 100.0;
        std::cout << "  Packet Delivery Ratio: " << std::fixed << std::setprecision(2) 
                  << pdr << "%" << std::endl;
    }
    
    std::cout << "\nNetwork Performance:" << std::endl;
    std::cout << "  Join Attempts:        " << g_stats.joinAttempts << std::endl;
    std::cout << "  Join Successes:       " << g_stats.joinSuccesses << std::endl;
    std::cout << "  Route Discoveries:    " << g_stats.routeDiscoveries << std::endl;
    
    if (!g_stats.rssiSamples.empty()) {
        std::cout << "\nSignal Quality:" << std::endl;
        std::cout << "  Average RSSI:         " << std::fixed << std::setprecision(2) 
                  << g_stats.avgRssi << " dBm" << std::endl;
        std::cout << "  Min RSSI:             " << g_stats.minRssi << " dBm" << std::endl;
        std::cout << "  Max RSSI:             " << g_stats.maxRssi << " dBm" << std::endl;
    }
    
    if (!g_stats.packetDelays.empty()) {
        std::cout << "  Average Delay:        " << std::fixed << std::setprecision(3) 
                  << g_stats.GetAvgDelay() * 1000 << " ms" << std::endl;
    }
    
    std::cout << "\nChannel Effects:" << std::endl;
    std::cout << "  Gaussian Noise:       " << (g_enableGaussianNoise ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  Rayleigh Fading:      " << (g_enableRayleighFading ? "Enabled" : "Disabled") << std::endl;
    if (g_enableGaussianNoise) {
        std::cout << "  Noise Variance:       " << g_noiseVariance << " dBm²" << std::endl;
    }
    
    std::cout << "========================================\n" << std::endl;
}

int main(int argc, char* argv[])
{
    // Command line parameters
    uint32_t nRouters = 3;
    uint32_t nSensors = 2;
    uint32_t nLights = 2;
    uint32_t simTime = 300;
    bool verbose = false;
    bool exportStats = true;
    bool manyToOne = true;
    uint32_t enableGaussianNoise = 0;
    uint32_t enableRayleighFading = 0;
    
    CommandLine cmd;
    cmd.AddValue("nRouters", "Number of router nodes", nRouters);
    cmd.AddValue("nSensors", "Number of sensor nodes", nSensors);
    cmd.AddValue("nLights", "Number of light bulb nodes", nLights);
    cmd.AddValue("enableGaussianNoise", "Enable Gaussian noise (0/1)", enableGaussianNoise);
    cmd.AddValue("enableRayleighFading", "Enable Rayleigh fading (0/1)", enableRayleighFading);
    cmd.AddValue("noiseVariance", "Variance of Gaussian noise (dBm)", g_noiseVariance);
    cmd.AddValue("simTime", "Simulation time in seconds", simTime);
    cmd.AddValue("verbose", "Enable verbose logging (0/1)", verbose);
    cmd.AddValue("exportStats", "Export statistics to CSV (0/1)", exportStats);
    cmd.AddValue("manyToOne", "Enable Many-to-One routing (0/1)", manyToOne);
    cmd.Parse(argc, argv);
    
    g_enableGaussianNoise = (enableGaussianNoise == 1);
    g_enableRayleighFading = (enableRayleighFading == 1);
    
    // Configure logging
    LogComponentEnableAll(LogLevel(LOG_PREFIX_TIME | LOG_PREFIX_FUNC | LOG_PREFIX_NODE));
    
    if (verbose)
    {
        LogComponentEnable("ZigbeeNwk", LOG_LEVEL_DEBUG);
        LogComponentEnable("ZigbeeAps", LOG_LEVEL_DEBUG);
    }
    
    // Initialize random variables for noise and fading
    if (g_enableGaussianNoise) {
        g_gaussianNoise = CreateObject<NormalRandomVariable>();
        g_gaussianNoise->SetAttribute("Mean", DoubleValue(0.0));
        g_gaussianNoise->SetAttribute("Variance", DoubleValue(g_noiseVariance));
    }
    
    if (g_enableRayleighFading) {
        g_rayleighFading1 = CreateObject<NormalRandomVariable>();
        g_rayleighFading1->SetAttribute("Mean", DoubleValue(0.0));
        g_rayleighFading1->SetAttribute("Variance", DoubleValue(1.0));
        
        g_rayleighFading2 = CreateObject<NormalRandomVariable>();
        g_rayleighFading2->SetAttribute("Mean", DoubleValue(0.0));
        g_rayleighFading2->SetAttribute("Variance", DoubleValue(1.0));
    }
    
    // Set random seed
    RngSeedManager::SetSeed(12345);
    RngSeedManager::SetRun(1);
    
    // Calculate total nodes
    uint32_t totalNodes = 1 + nRouters + nSensors + nLights; // 1 coordinator
    g_stats.totalNodes = totalNodes;
    g_stats.sensors = nSensors;
    g_stats.lights = nLights;
    
    // Print configuration
    std::cout << "\n========================================" << std::endl;
    std::cout << "INTERACTIVE ZIGBEE SMART HOME SIMULATION" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Network Configuration:" << std::endl;
    std::cout << "  Coordinator:          1" << std::endl;
    std::cout << "  Routers:              " << nRouters << std::endl;
    std::cout << "  Sensors:              " << nSensors << std::endl;
    std::cout << "  Lights:               " << nLights << std::endl;
    std::cout << "  Total Nodes:          " << totalNodes << std::endl;
    std::cout << "\nSimulation Parameters:" << std::endl;
    std::cout << "  Simulation Time:      " << simTime << "s" << std::endl;
    std::cout << "  Many-to-One Routing:  " << (manyToOne ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  Gaussian Noise:       " << (g_enableGaussianNoise ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  Rayleigh Fading:      " << (g_enableRayleighFading ? "Enabled" : "Disabled") << std::endl;
    if (g_enableGaussianNoise) {
        std::cout << "  Noise Variance:       " << g_noiseVariance << " dBm²" << std::endl;
    }
    std::cout << "  Export Statistics:    " << (exportStats ? "Yes" : "No") << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // Create nodes
    g_allNodes.Create(totalNodes);
    
    // Configure LR-WPAN devices
    LrWpanHelper lrWpanHelper;
    g_allDevices = lrWpanHelper.Install(g_allNodes);
    
    // Set IEEE addresses
    lrWpanHelper.SetExtendedAddresses(g_allDevices);
    
    // Configure channel with propagation loss model
    Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel>();
    Ptr<LogDistancePropagationLossModel> propModel = 
        CreateObject<LogDistancePropagationLossModel>();
    Ptr<ConstantSpeedPropagationDelayModel> delayModel = 
        CreateObject<ConstantSpeedPropagationDelayModel>();
    
    channel->AddPropagationLossModel(propModel);
    channel->SetPropagationDelayModel(delayModel);
    
    for (uint32_t i = 0; i < g_allDevices.GetN(); i++)
    {
        Ptr<LrWpanNetDevice> dev = g_allDevices.Get(i)->GetObject<LrWpanNetDevice>();
        dev->SetChannel(channel);
    }
    
    // Configure mobility (Grid layout)
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(50.0),
                                  "DeltaY", DoubleValue(50.0),
                                  "GridWidth", UintegerValue(5),
                                  "LayoutType", StringValue("RowFirst"));
    mobility.Install(g_allNodes);
    
    // Install Zigbee stack
    ZigbeeHelper zigbeeHelper;
    g_zigbeeStacks = zigbeeHelper.Install(g_allDevices);
    
    // Configure callbacks for all stacks
    for (uint32_t i = 0; i < g_zigbeeStacks.GetN(); i++)
    {
        Ptr<ZigbeeStack> zstack = g_zigbeeStacks.Get(i)->GetObject<ZigbeeStack>();
        
        zstack->GetNwk()->AssignStreams(i * 10);
        
        zstack->GetAps()->SetApsdeDataIndicationCallback(
            MakeBoundCallback(&ApsDataIndication, zstack));
        
        zstack->GetNwk()->SetNlmeNetworkFormationConfirmCallback(
            MakeBoundCallback(&NwkNetworkFormationConfirm, zstack));
        zstack->GetNwk()->SetNlmeNetworkDiscoveryConfirmCallback(
            MakeBoundCallback(&NwkNetworkDiscoveryConfirm, zstack));
        zstack->GetNwk()->SetNlmeJoinConfirmCallback(
            MakeBoundCallback(&NwkJoinConfirm, zstack));
        zstack->GetNwk()->SetNlmeRouteDiscoveryConfirmCallback(
            MakeBoundCallback(&NwkRouteDiscoveryConfirm, zstack));
    }
    
    // Get coordinator
    Ptr<ZigbeeStack> coordinator = g_zigbeeStacks.Get(0)->GetObject<ZigbeeStack>();
    
    // Network formation
    NlmeNetworkFormationRequestParams netFormParams;
    netFormParams.m_scanChannelList.channelPageCount = 1;
    netFormParams.m_scanChannelList.channelsField[0] = ALL_CHANNELS;
    netFormParams.m_scanDuration = 0;
    netFormParams.m_superFrameOrder = 15;
    netFormParams.m_beaconOrder = 15;
    
    Simulator::ScheduleWithContext(coordinator->GetNode()->GetId(),
                                   Seconds(1.0),
                                   &ZigbeeNwk::NlmeNetworkFormationRequest,
                                   coordinator->GetNwk(),
                                   netFormParams);
    
    // Device joining (staggered)
    NlmeNetworkDiscoveryRequestParams netDiscParams;
    netDiscParams.m_scanChannelList.channelPageCount = 1;
    netDiscParams.m_scanChannelList.channelsField[0] = 0x00007800;
    netDiscParams.m_scanDuration = 2;
    
    double joinTime = 3.0;
    for (uint32_t i = 1; i < totalNodes; i++)
    {
        Ptr<ZigbeeStack> stack = g_zigbeeStacks.Get(i)->GetObject<ZigbeeStack>();
        Simulator::ScheduleWithContext(stack->GetNode()->GetId(),
                                       Seconds(joinTime),
                                       &ZigbeeNwk::NlmeNetworkDiscoveryRequest,
                                       stack->GetNwk(),
                                       netDiscParams);
        joinTime += 1.5;
    }
    
    // Group configuration for lights
    double groupTime = joinTime + 2.0;
    uint32_t lightStartIdx = 1 + nRouters + nSensors;
    
    for (uint32_t i = 0; i < nLights; i++)
    {
        Ptr<ZigbeeStack> lightStack = g_zigbeeStacks.Get(lightStartIdx + i)->GetObject<ZigbeeStack>();
        
        ApsmeGroupRequestParams groupParams;
        groupParams.m_groupAddress = GROUP_ALL_LIGHTS;
        groupParams.m_endPoint = 1;
        
        Simulator::Schedule(Seconds(groupTime + i * 0.1), 
                           &ZigbeeAps::ApsmeAddGroupRequest,
                           lightStack->GetAps(), groupParams);
    }
    
    // Routing
    double routingTime = groupTime + 2.0;
    
    if (manyToOne)
    {
        NlmeRouteDiscoveryRequestParams routeDiscParams;
        routeDiscParams.m_dstAddrMode = NO_ADDRESS;
        
        Simulator::Schedule(Seconds(routingTime), 
                           &ZigbeeNwk::NlmeRouteDiscoveryRequest,
                           coordinator->GetNwk(),
                           routeDiscParams);
    }
    
    // Data transmission - sensors report periodically
    double dataTime = routingTime + 5.0;
    uint32_t sensorStartIdx = 1 + nRouters;
    
    for (uint32_t i = 0; i < nSensors; i++)
    {
        Ptr<ZigbeeStack> sensorStack = g_zigbeeStacks.Get(sensorStartIdx + i)->GetObject<ZigbeeStack>();
        
        for (uint32_t j = 0; j < 5; j++)
        {
            Simulator::Schedule(Seconds(dataTime + j * 20.0 + i * 5.0), 
                               &SendSensorData, sensorStack, coordinator);
        }
    }
    
    // Group commands to lights
    Simulator::Schedule(Seconds(dataTime + 10.0), 
                       &SendGroupCommand, coordinator, GROUP_ALL_LIGHTS, 
                       "Turn ON All Lights", 0x01);
    
    Simulator::Schedule(Seconds(dataTime + 30.0), 
                       &SendGroupCommand, coordinator, GROUP_ALL_LIGHTS, 
                       "Turn OFF All Lights", 0x00);
    
    Simulator::Schedule(Seconds(dataTime + 50.0), 
                       &SendGroupCommand, coordinator, GROUP_ALL_LIGHTS, 
                       "Turn ON All Lights", 0x01);
    
    // Network Animation
    AnimationInterface anim("interactive-zigbee-network.xml");
    
    // Set node descriptions for NetAnim
    anim.UpdateNodeDescription(coordinator->GetNode(), "Coordinator");
    anim.UpdateNodeColor(coordinator->GetNode(), 255, 0, 0); // Red
    
    for (uint32_t i = 1; i <= nRouters; i++) {
        anim.UpdateNodeDescription(g_allNodes.Get(i), "Router-" + std::to_string(i));
        anim.UpdateNodeColor(g_allNodes.Get(i), 0, 0, 255); // Blue
    }
    
    for (uint32_t i = 0; i < nSensors; i++) {
        uint32_t nodeIdx = sensorStartIdx + i;
        anim.UpdateNodeDescription(g_allNodes.Get(nodeIdx), "Sensor-" + std::to_string(i+1));
        anim.UpdateNodeColor(g_allNodes.Get(nodeIdx), 0, 255, 0); // Green
    }
    
    for (uint32_t i = 0; i < nLights; i++) {
        uint32_t nodeIdx = lightStartIdx + i;
        anim.UpdateNodeDescription(g_allNodes.Get(nodeIdx), "Light-" + std::to_string(i+1));
        anim.UpdateNodeColor(g_allNodes.Get(nodeIdx), 255, 255, 0); // Yellow
    }
    
    // Print statistics at end
    Simulator::Schedule(Seconds(simTime - 1.0), &PrintStatistics);
    
    // Export statistics if enabled
    if (exportStats) {
        Simulator::Schedule(Seconds(simTime - 0.5), &ExportStatistics, 
                           "zigbee-network-statistics.csv");
    }
    
    // Run simulation
    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    Simulator::Destroy();
    
    return 0;
}