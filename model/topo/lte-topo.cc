/******************************************************************************
 * Copyright 2016-2017 Cisco Systems, Inc.                                    *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License");            *
 * you may not use this file except in compliance with the License.           *
 *                                                                            *
 * You may obtain a copy of the License at                                    *
 *                                                                            *
 *     http://www.apache.org/licenses/LICENSE-2.0                             *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 ******************************************************************************/

/**
 * @file
 * Wired network topology setup implementation for rmcat ns3 module.
 *
 * @version 0.1.0
 * @author Jiantao Fu
 * @author Sergio Mena
 * @author Xiaoqing Zhu
 */

#include "lte-topo.h"

NS_LOG_COMPONENT_DEFINE ("LteTopo");

namespace ns3 {

static void CourseChange (std::string context, Ptr<const MobilityModel> mobility)
{
    Vector pos = mobility->GetPosition ();
    Vector vel = mobility->GetVelocity ();
    NS_LOG_INFO(context << ", model=" << mobility << ", POS: x=" << pos.x << ", y=" << pos.y
                << ", z=" << pos.z << "; VEL: x=" << vel.x << ", y=" << vel.y
                << ", z=" << vel.z);
}

Ptr<PointToPointEpcHelper> LteTop::epcHelper;

void LteTop::Build(uint16_t numberOfUEs, double speedKmPerHour, uint32_t msDelay)
{
    NS_LOG_INFO("LteTop::Build");
    // setup core network
    // PointToPoint links for the connection between the eNBs and the SGW (S1-U interface) and among eNBs (X2-U and X2-C interfaces)
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
    lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisPropagationLossModel"));

    epcHelper = CreateObject<PointToPointEpcHelper> ();
    lteHelper->SetEpcHelper (epcHelper);

    // Create packet data network gateway
    Ptr<Node> pgw = epcHelper->GetPgwNode ();

    // Create a single RemoteHost on Internet
    m_remoteNode.Create (1);
    Ptr<Node> remoteHost = m_remoteNode.Get (0);
    InternetStackHelper internet;
    internet.Install (m_remoteNode);

    // Create p2p channel between pgw and remote host
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
    p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
    p2ph.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (msDelay)));
    NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
    ipv4h.Assign (internetDevices);

    // routing setup SGW/PGW -> RemoteHost, by default UEs will be assigned with ip address in 7.0.0.0 network
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
    remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

    // setup radio access network (E-UTRAN)
    m_enbNodes.Create(1);
    m_ueNodes.Create(numberOfUEs);

    // Install Mobility Model
    MobilityHelper mobility;
    ObjectFactory pos;
    pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
    pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=1000.0|Max=3000.0]"));
    pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=1000.0|Max=3000.0]"));
    Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();

    std::ostringstream speedVariableStream;
    speedVariableStream << "ns3::ConstantRandomVariable[Constant="
                        << speedKmPerHour * 1000.0 / 3600
                        << "]";

    mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                               "Speed", StringValue (speedVariableStream.str ()),
                               "PositionAllocator", PointerValue (taPositionAlloc));

    mobility.Install(m_ueNodes);

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (m_enbNodes);


    // Install LTE Devices to the nodes
    NetDeviceContainer m_enbDevices = lteHelper->InstallEnbDevice (m_enbNodes);
    NetDeviceContainer m_ueDevices = lteHelper->InstallUeDevice (m_ueNodes);

    // Install the IP stack on the UEs, assign ip and setup routing
    internet.Install (m_ueNodes);
    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = epcHelper->AssignUeIpv4Address (m_ueDevices);
    for (uint32_t u = 0; u < m_ueNodes.GetN (); ++u) {
        Ptr<Node> ueNode = m_ueNodes.Get (u);
        // Set the default gateway for the UE
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

    // Attach UEs to eNodeB
    for (uint16_t i = 0; i < numberOfUEs; i++) {
        lteHelper->Attach (m_ueDevices.Get(i), m_enbDevices.Get(0));
    }

    Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (1024 * 1024));
    Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChange));

    lteHelper->EnablePhyTraces ();
    lteHelper->EnableMacTraces ();
    lteHelper->EnableRlcTraces ();
    lteHelper->EnablePdcpTraces ();
}

ApplicationContainer LteTop::InstallRMCAT (const std::string& flowId,
                                           uint32_t nodeId,
                                           uint16_t serverPort,
                                           bool downstream)
{
    NS_ASSERT (nodeId <  m_ueNodes.GetN ());

    auto sender = m_ueNodes.Get (nodeId);
    auto receiver = m_remoteNode.Get (0);

    if (downstream) {
        std::swap (sender, receiver);
    }

    return Topo::InstallRMCAT (flowId,
                               sender,
                               receiver,
                               serverPort);
}

ApplicationContainer LteTop::InstallCBR(uint32_t nodeId,
                                        uint16_t serverPort,
                                        uint64_t bitrate,
                                        uint32_t packetSize,
                                        bool downstream)
{
    NS_ASSERT (nodeId <  m_ueNodes.GetN ());

    auto sender = m_ueNodes.Get (nodeId);
    auto receiver = m_remoteNode.Get (0);

    if (downstream) {
        std::swap (sender, receiver);
    }

    return Topo::InstallCBR (sender,
                             receiver,
                             serverPort,
                             bitrate,
                             packetSize);
}

ApplicationContainer LteTop::InstallTCP(const std::string& flowId,
                                        uint32_t nodeId,
                                        uint16_t serverPort,
                                        bool downstream)
{
    NS_ASSERT (nodeId <  m_ueNodes.GetN ());

    auto sender = m_ueNodes.Get (nodeId);
    auto receiver = m_remoteNode.Get (0);

    if (downstream) {
        std::swap (sender, receiver);
    }

    return Topo::InstallTCP (flowId,
                             sender,
                             receiver,
                             serverPort);
}

void LteTop::EnableLTELogComponents()
{
    LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_LEVEL_ALL);
    LogComponentEnable ("LteHelper", logLevel);
    LogComponentEnable ("EpcHelper", logLevel);
    LogComponentEnable ("EpcEnbApplication", logLevel);
    LogComponentEnable ("EpcSgwPgwApplication", logLevel);
    LogComponentEnable ("EpcMme", logLevel);
    LogComponentEnable ("LteEnbRrc", logLevel);
}

}
