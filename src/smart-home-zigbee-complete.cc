/*
 * Comprehensive ZigBee Smart Home Network Simulation
 * 
 * This example demonstrates a complete smart home network with:
 * - Network formation and association-based joining
 * - Mesh and Many-to-One routing
 * - APS layer data transmission (Unicast and Groupcast)
 * - Multiple device types (Coordinator, Routers, End Devices)
 * - Group-based control (Room-based lighting control)
 * - Sensor reporting and actuator control
 * - Route discovery and table management
 *
 * Network Topology:
 * 
 *  Coordinator (ZC) --- Router1 (ZR1) --- Router2 (ZR2) --- Router3 (ZR3)
 *  [00:00]               |                  |                [Temperature Sensor]
 *                        |                  |
 *                   Router4 (ZR4)      Router5 (ZR5)
 *                   [Living Room]      [Bedroom]
 *                   [Light1, Light2]   [Light3]
 *
 * Groups:
 * - Group 0x0001: Living Room (ZR4 - endpoints 1,2)
 * - Group 0x0002: Bedroom (ZR5 - endpoint 1)
 * - Group 0x0003: All Lights (ZR4 - endpoints 1,2 and ZR5 - endpoint 1)
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

#include <iostream>
#include <iomanip>
#include <sstream>

using namespace ns3;
using namespace ns3::lrwpan;
using namespace ns3::zigbee;

NS_LOG_COMPONENT_DEFINE("SmartHomeZigbee");

// Global container for all Zigbee stacks
ZigbeeStackContainer g_zigbeeStacks;

// Statistics tracking
struct NetworkStats {
    uint32_t packetsTransmitted = 0;
    uint32_t packetsReceived = 0;
    uint32_t routeDiscoveries = 0;
    uint32_t joinAttempts = 0;
    uint32_t joinSuccesses = 0;
    uint32_t groupCommands = 0;
};

NetworkStats g_stats;

// Device role enumeration for easier identification
enum DeviceRole {
    ROLE_COORDINATOR = 0,
    ROLE_ROUTER1,
    ROLE_ROUTER2,
    ROLE_ROUTER3,
    ROLE_ROUTER4_LIVINGROOM,
    ROLE_ROUTER5_BEDROOM,
    NUM_DEVICES
};

// Group addresses
const Mac16Address GROUP_LIVING_ROOM = Mac16Address("00:01");
const Mac16Address GROUP_BEDROOM = Mac16Address("00:02");
const Mac16Address GROUP_ALL_LIGHTS = Mac16Address("00:03");

/**
 * Print a formatted message with timestamp and node info
 */
void PrintMessage(Ptr<ZigbeeStack> stack, const std::string& message)
{
    std::cout << std::fixed << std::setprecision(3) 
              << "[" << Simulator::Now().As(Time::S) << "] "
              << "Node " << stack->GetNode()->GetId() 
              << " [" << stack->GetNwk()->GetNetworkAddress() << "]: "
              << message << std::endl;
}

/**
 * Trace route from source to destination
 */
void TraceRoute(Mac16Address src, Mac16Address dst)
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "TRACE ROUTE at " << Simulator::Now().As(Time::S) << std::endl;
    std::cout << "From: " << src << " To: " << dst << std::endl;
    std::cout << "========================================" << std::endl;
    
    Mac16Address target = src;
    uint32_t hopCount = 0;
    
    while (target != Mac16Address("FF:FF") && target != dst && hopCount < 20)
    {
        Ptr<ZigbeeStack> zstack;
        
        // Find the stack with current target address
        for (auto i = g_zigbeeStacks.Begin(); i != g_zigbeeStacks.End(); i++)
        {
            zstack = *i;
            if (zstack->GetNwk()->GetNetworkAddress() == target)
            {
                break;
            }
        }
        
        bool neighbor = false;
        Mac16Address nextHop = zstack->GetNwk()->FindRoute(dst, neighbor);
        
        if (nextHop == Mac16Address("FF:FF"))
        {
            std::cout << "  " << (hopCount + 1) << ". Node " << zstack->GetNode()->GetId() 
                      << " [" << target << "] - DESTINATION UNREACHABLE" << std::endl;
            break;
        }
        else
        {
            std::cout << "  " << (hopCount + 1) << ". Node " << zstack->GetNode()->GetId() 
                      << " [" << target << "] -> NextHop [" << nextHop << "]";
            
            if (neighbor)
            {
                std::cout << " (Direct Neighbor)";
            }
            
            std::cout << std::endl;
            target = nextHop;
            hopCount++;
        }
    }
    
    if (target == dst)
    {
        std::cout << "  Route Complete! Total hops: " << hopCount << std::endl;
    }
    
    std::cout << "========================================\n" << std::endl;
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
    else
    {
        addrMode = "UNKNOWN";
    }
    
    PrintMessage(stack, "RECEIVED " + addrMode + " DATA (Size: " + 
                 std::to_string(p->GetSize()) + " bytes, Endpoint: " + 
                 std::to_string(params.m_dstEndPoint) + ", Cluster: " + 
                 std::to_string(params.m_clusterId) + ")");
}

/**
 * Network Formation Confirm
 */
void NwkNetworkFormationConfirm(Ptr<ZigbeeStack> stack, NlmeNetworkFormationConfirmParams params)
{
    if (params.m_status == NwkStatus::SUCCESS)
    {
        PrintMessage(stack, "Network formation SUCCESSFUL");
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
        
        // Select the first network and join
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
    else
    {
        PrintMessage(stack, "Network discovery FAILED - Status: " + std::to_string(params.m_status));
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
        PrintMessage(stack, "Joined network successfully");
        std::cout << "  Short Address: " << params.m_networkAddress << std::endl;
        std::cout << "  Extended PAN ID: 0x" << std::hex << params.m_extendedPanId 
                  << std::dec << std::endl;
        
        // Start as router
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
        PrintMessage(stack, "Route discovery FAILED - Status: " + std::to_string(params.m_status));
    }
}

/**
 * Send temperature reading from sensor (Unicast)
 */
void SendTemperatureReading(Ptr<ZigbeeStack> sensorStack, Ptr<ZigbeeStack> coordinatorStack)
{
    // Simulate temperature reading: 23.5°C = 235 in fixed point (x10)
    uint8_t tempData[2] = {0xEB, 0x00}; // 235 in little endian
    Ptr<Packet> p = Create<Packet>(tempData, 2);
    
    ApsdeDataRequestParams dataReqParams;
    ZigbeeApsTxOptions txOptions;
    
    dataReqParams.m_useAlias = false;
    dataReqParams.m_txOptions = txOptions.GetTxOptions();
    dataReqParams.m_srcEndPoint = 1;        // Temperature sensor endpoint
    dataReqParams.m_dstEndPoint = 1;        // Coordinator monitoring endpoint
    dataReqParams.m_clusterId = 0x0402;     // Temperature measurement cluster
    dataReqParams.m_profileId = 0x0104;     // Home Automation profile
    dataReqParams.m_dstAddrMode = ApsDstAddressMode::DST_ADDR16_DST_ENDPOINT_PRESENT;
    dataReqParams.m_dstAddr16 = coordinatorStack->GetNwk()->GetNetworkAddress();
    
    g_stats.packetsTransmitted++;
    PrintMessage(sensorStack, "Sending temperature reading to Coordinator");
    
    Simulator::ScheduleNow(&ZigbeeAps::ApsdeDataRequest, 
                          sensorStack->GetAps(), dataReqParams, p);
}

/**
 * Send group command (Light control)
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
    dataReqParams.m_clusterId = 0x0006;     // On/Off cluster
    dataReqParams.m_profileId = 0x0104;     // Home Automation profile
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
 * Add device endpoint to a group
 */
void AddToGroup(Ptr<ZigbeeStack> stack, Mac16Address groupAddr, uint8_t endpoint, 
                const std::string& groupName)
{
    ApsmeGroupRequestParams groupParams;
    groupParams.m_groupAddress = groupAddr;
    groupParams.m_endPoint = endpoint;
    
    std::ostringstream oss;
    oss << groupAddr;
    PrintMessage(stack, "Adding endpoint " + std::to_string(endpoint) + 
                 " to group '" + groupName + "' [" + oss.str() + "]");
    
    Simulator::ScheduleNow(&ZigbeeAps::ApsmeAddGroupRequest, 
                          stack->GetAps(), groupParams);
}

/**
 * Print all routing tables
 */
void PrintAllRoutingTables()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "ROUTING TABLES at " << Simulator::Now().As(Time::S) << std::endl;
    std::cout << "========================================" << std::endl;
    
    Ptr<OutputStreamWrapper> stream = Create<OutputStreamWrapper>(&std::cout);
    
    for (auto i = g_zigbeeStacks.Begin(); i != g_zigbeeStacks.End(); i++)
    {
        Ptr<ZigbeeStack> zstack = *i;
        std::cout << "\n--- Node " << zstack->GetNode()->GetId() 
                  << " [" << zstack->GetNwk()->GetNetworkAddress() << "] ---" << std::endl;
        zstack->GetNwk()->PrintRoutingTable(stream);
    }
    
    std::cout << "========================================\n" << std::endl;
}

/**
 * Print network statistics
 */
void PrintStatistics()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "NETWORK STATISTICS" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "  Join Attempts:        " << g_stats.joinAttempts << std::endl;
    std::cout << "  Join Successes:       " << g_stats.joinSuccesses << std::endl;
    std::cout << "  Route Discoveries:    " << g_stats.routeDiscoveries << std::endl;
    std::cout << "  Packets Transmitted:  " << g_stats.packetsTransmitted << std::endl;
    std::cout << "  Packets Received:     " << g_stats.packetsReceived << std::endl;
    std::cout << "  Group Commands:       " << g_stats.groupCommands << std::endl;
    std::cout << "  Packet Success Rate:  ";
    
    if (g_stats.packetsTransmitted > 0)
    {
        double successRate = (double)g_stats.packetsReceived / g_stats.packetsTransmitted * 100.0;
        std::cout << std::fixed << std::setprecision(2) << successRate << "%" << std::endl;
    }
    else
    {
        std::cout << "N/A" << std::endl;
    }
    
    std::cout << "========================================\n" << std::endl;
}

int main(int argc, char* argv[])
{
    // Command line parameters
    uint32_t simTime = 300;
    bool verbose = false;
    bool manyToOne = true;
    
    CommandLine cmd;
    cmd.AddValue("simTime", "Simulation time in seconds", simTime);
    cmd.AddValue("verbose", "Enable verbose logging", verbose);
    cmd.AddValue("manyToOne", "Enable Many-to-One routing", manyToOne);
    cmd.Parse(argc, argv);
    
    // Configure logging
    LogComponentEnableAll(LogLevel(LOG_PREFIX_TIME | LOG_PREFIX_FUNC | LOG_PREFIX_NODE));
    
    if (verbose)
    {
        LogComponentEnable("ZigbeeNwk", LOG_LEVEL_DEBUG);
        LogComponentEnable("ZigbeeAps", LOG_LEVEL_DEBUG);
    }
    
    // Set random seed for reproducibility
    RngSeedManager::SetSeed(12345);
    RngSeedManager::SetRun(1);
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "SMART HOME ZIGBEE NETWORK SIMULATION" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Devices: " << NUM_DEVICES << std::endl;
    std::cout << "Simulation Time: " << simTime << "s" << std::endl;
    std::cout << "Many-to-One Routing: " << (manyToOne ? "Enabled" : "Disabled") << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // Create nodes
    NodeContainer nodes;
    nodes.Create(NUM_DEVICES);
    
    // Configure LR-WPAN devices
    LrWpanHelper lrWpanHelper;
    NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(nodes);
    
    // Set IEEE addresses
    lrWpanHelper.SetExtendedAddresses(lrwpanDevices);
    
    // Configure channel
    Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel>();
    Ptr<LogDistancePropagationLossModel> propModel = 
        CreateObject<LogDistancePropagationLossModel>();
    Ptr<ConstantSpeedPropagationDelayModel> delayModel = 
        CreateObject<ConstantSpeedPropagationDelayModel>();
    
    channel->AddPropagationLossModel(propModel);
    channel->SetPropagationDelayModel(delayModel);
    
    for (uint32_t i = 0; i < lrwpanDevices.GetN(); i++)
    {
        Ptr<LrWpanNetDevice> dev = lrwpanDevices.Get(i)->GetObject<LrWpanNetDevice>();
        dev->SetChannel(channel);
    }
    
    // Configure mobility (Linear topology)
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(80.0),
                                  "DeltaY", DoubleValue(60.0),
                                  "GridWidth", UintegerValue(3),
                                  "LayoutType", StringValue("RowFirst"));
    mobility.Install(nodes);
    
    // Install Zigbee stack
    ZigbeeHelper zigbeeHelper;
    g_zigbeeStacks = zigbeeHelper.Install(lrwpanDevices);
    
    // Configure callbacks for all stacks
    for (uint32_t i = 0; i < g_zigbeeStacks.GetN(); i++)
    {
        Ptr<ZigbeeStack> zstack = g_zigbeeStacks.Get(i)->GetObject<ZigbeeStack>();
        
        // Assign streams for reproducibility
        zstack->GetNwk()->AssignStreams(i * 10);
        
        // APS callbacks
        zstack->GetAps()->SetApsdeDataIndicationCallback(
            MakeBoundCallback(&ApsDataIndication, zstack));
        
        // NWK callbacks
        zstack->GetNwk()->SetNlmeNetworkFormationConfirmCallback(
            MakeBoundCallback(&NwkNetworkFormationConfirm, zstack));
        zstack->GetNwk()->SetNlmeNetworkDiscoveryConfirmCallback(
            MakeBoundCallback(&NwkNetworkDiscoveryConfirm, zstack));
        zstack->GetNwk()->SetNlmeJoinConfirmCallback(
            MakeBoundCallback(&NwkJoinConfirm, zstack));
        zstack->GetNwk()->SetNlmeRouteDiscoveryConfirmCallback(
            MakeBoundCallback(&NwkRouteDiscoveryConfirm, zstack));
    }
    
    // Get individual stacks for easier reference
    Ptr<ZigbeeStack> coordinator = g_zigbeeStacks.Get(ROLE_COORDINATOR)->GetObject<ZigbeeStack>();
    Ptr<ZigbeeStack> router1 = g_zigbeeStacks.Get(ROLE_ROUTER1)->GetObject<ZigbeeStack>();
    Ptr<ZigbeeStack> router2 = g_zigbeeStacks.Get(ROLE_ROUTER2)->GetObject<ZigbeeStack>();
    Ptr<ZigbeeStack> router3 = g_zigbeeStacks.Get(ROLE_ROUTER3)->GetObject<ZigbeeStack>();
    Ptr<ZigbeeStack> router4 = g_zigbeeStacks.Get(ROLE_ROUTER4_LIVINGROOM)->GetObject<ZigbeeStack>();
    Ptr<ZigbeeStack> router5 = g_zigbeeStacks.Get(ROLE_ROUTER5_BEDROOM)->GetObject<ZigbeeStack>();
    
    // ===== NETWORK FORMATION =====
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
    
    // ===== DEVICE JOINING (Staggered) =====
    NlmeNetworkDiscoveryRequestParams netDiscParams;
    netDiscParams.m_scanChannelList.channelPageCount = 1;
    netDiscParams.m_scanChannelList.channelsField[0] = 0x00007800; // Channels 11-14
    netDiscParams.m_scanDuration = 2;
    
    double joinTime = 3.0;
    for (uint32_t i = 1; i < NUM_DEVICES; i++)
    {
        Ptr<ZigbeeStack> stack = g_zigbeeStacks.Get(i)->GetObject<ZigbeeStack>();
        Simulator::ScheduleWithContext(stack->GetNode()->GetId(),
                                       Seconds(joinTime),
                                       &ZigbeeNwk::NlmeNetworkDiscoveryRequest,
                                       stack->GetNwk(),
                                       netDiscParams);
        joinTime += 2.0;
    }
    
    // ===== GROUP CONFIGURATION =====
    double groupTime = joinTime + 2.0;
    
    // Living Room group (Router 4, endpoints 1 and 2)
    Simulator::Schedule(Seconds(groupTime), &AddToGroup, 
                       router4, GROUP_LIVING_ROOM, 1, "Living Room");
    Simulator::Schedule(Seconds(groupTime + 0.1), &AddToGroup, 
                       router4, GROUP_LIVING_ROOM, 2, "Living Room");
    
    // Bedroom group (Router 5, endpoint 1)
    Simulator::Schedule(Seconds(groupTime + 0.2), &AddToGroup, 
                       router5, GROUP_BEDROOM, 1, "Bedroom");
    
    // All Lights group
    Simulator::Schedule(Seconds(groupTime + 0.3), &AddToGroup, 
                       router4, GROUP_ALL_LIGHTS, 1, "All Lights");
    Simulator::Schedule(Seconds(groupTime + 0.4), &AddToGroup, 
                       router4, GROUP_ALL_LIGHTS, 2, "All Lights");
    Simulator::Schedule(Seconds(groupTime + 0.5), &AddToGroup, 
                       router5, GROUP_ALL_LIGHTS, 1, "All Lights");
    
    // ===== ROUTING =====
    double routingTime = groupTime + 2.0;
    
    if (manyToOne)
    {
        // Many-to-One routing
        NlmeRouteDiscoveryRequestParams routeDiscParams;
        routeDiscParams.m_dstAddrMode = NO_ADDRESS;
        
        Simulator::Schedule(Seconds(routingTime), 
                           &ZigbeeNwk::NlmeRouteDiscoveryRequest,
                           coordinator->GetNwk(),
                           routeDiscParams);
    }
    else
    {
        // Mesh routing from coordinator to router 3
        NlmeRouteDiscoveryRequestParams routeDiscParams;
        routeDiscParams.m_dstAddr = router3->GetNwk()->GetNetworkAddress();
        routeDiscParams.m_dstAddrMode = UCST_BCST;
        routeDiscParams.m_radius = 0;
        
        Simulator::Schedule(Seconds(routingTime),
                           &ZigbeeNwk::NlmeRouteDiscoveryRequest,
                           coordinator->GetNwk(),
                           routeDiscParams);
    }
    
    // ===== DATA TRANSMISSION =====
    double dataTime = routingTime + 5.0;
    
    // Temperature sensor reports (periodic)
    for (uint32_t i = 0; i < 5; i++)
    {
        Simulator::Schedule(Seconds(dataTime + i * 20.0), 
                           &SendTemperatureReading, router3, coordinator);
    }
    
    // Group commands (light control)
    Simulator::Schedule(Seconds(dataTime + 5.0), 
                       &SendGroupCommand, coordinator, GROUP_LIVING_ROOM, 
                       "Turn ON Living Room", 0x01);
    
    Simulator::Schedule(Seconds(dataTime + 10.0), 
                       &SendGroupCommand, coordinator, GROUP_BEDROOM, 
                       "Turn ON Bedroom", 0x01);
    
    Simulator::Schedule(Seconds(dataTime + 15.0), 
                       &SendGroupCommand, coordinator, GROUP_ALL_LIGHTS, 
                       "Turn OFF All Lights", 0x00);
    
    Simulator::Schedule(Seconds(dataTime + 25.0), 
                       &SendGroupCommand, coordinator, GROUP_ALL_LIGHTS, 
                       "Turn ON All Lights", 0x01);
    
    // ===== DIAGNOSTICS =====
    // Trace route
    Simulator::Schedule(Seconds(dataTime - 2.0), &TraceRoute,
                       coordinator->GetNwk()->GetNetworkAddress(),
                       router3->GetNwk()->GetNetworkAddress());
    
    // Print routing tables
    Simulator::Schedule(Seconds(dataTime - 1.0), &PrintAllRoutingTables);
    
    // Print statistics
    Simulator::Schedule(Seconds(simTime - 1.0), &PrintStatistics);
    
    // ===== RUN SIMULATION =====
    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    Simulator::Destroy();
    
    return 0;
}