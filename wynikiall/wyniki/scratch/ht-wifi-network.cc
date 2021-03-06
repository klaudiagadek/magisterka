/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 MIRKO BANCHI
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Mirko Banchi <mk.banchi@gmail.com>
 *          Sebastien Deronne <sebastien.deronne@gmail.com>
 */

#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/config-store-module.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/onoff-application.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/dca-txop.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/dcf-manager.h"
#include "ns3/arp-cache.h"
#include "ns3/arp-header.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address-generator.h"
#include "ns3/ipv4-address-helper.h"
// This is a simple example in order to show how to configure an IEEE 802.11n Wi-Fi network.
//
// It ouputs the UDP or TCP goodput for every VHT bitrate value, which depends on the MCS value (0 to 7), the
// channel width (20 or 40 MHz) and the guard interval (long or short). The PHY bitrate is constant over all
// the simulation run. The user can also specify the distance between the access point and the station: the
// larger the distance the smaller the goodput.
//
// The simulation assumes a single station in an infrastructure network:
//
//  STA     AP
//    *     *
//    |     |
//   n1     n2
//
//Packets in this simulation aren't marked with a QosTag so they are considered
//belonging to BestEffort Access Class (AC_BE).

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ht-wifi-network");

int main(int argc, char* argv[])
{

    //uint32_t cw_Min = 31;
    //uint32_t cw_Max = 1023;
    //LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    //LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    //LogComponentEnable("DcaTxop", LOG_LEVEL_ALL);
        LogComponentEnable("ArpCache", LOG_LEVEL_ALL);
        LogComponentEnable("Ipv4AddressHelper", LOG_LEVEL_ALL);
LogComponentEnable("Ipv4AddressGenerator", LOG_LEVEL_ALL);
LogComponentEnable("ArpHeader", LOG_LEVEL_ALL);
 


    bool udp = true;
    double simulationTime = 100;                        //seconds
    double distance = 1.0;                              //meters
    uint32_t nnode = 10;                                 //nodes number
    uint32_t maxpack = 1000000;                         //max packets
    char interval[] = "0.01";                           //interval between packets
    double inter = 0.01;
    std::string phyMode("DsssRate11Mbps");

    CommandLine cmd;
    cmd.AddValue("distance", "Distance in meters between the station and the access point", distance);
    cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
    cmd.AddValue("udp", "UDP if set to 1, TCP otherwise", udp);
    cmd.AddValue("phyMode", "Wifi Phy mode", phyMode);
    cmd.Parse(argc, argv);

    uint32_t payloadSize; //1500 byte IP packet
    if (udp) {
        payloadSize = 1472; //bytes
    }
    else {
        payloadSize = 1448; //bytes
        Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));
    }

    NodeContainer wifiStaNode;
    wifiStaNode.Create(nnode);
    NodeContainer wifiApNode;
    wifiApNode.Create(1);

    WifiHelper wifi;
    wifi.SetStandard(WIFI_PHY_STANDARD_80211b);

    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
    // ns-3 supports RadioTap and Prism tracing extensions for 802.11
    wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

    YansWifiChannelHelper wifiChannel;
    // reference loss must be changed since 802.11b is operating at 2.4GHz
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
        "Exponent", DoubleValue(3.0),
        "ReferenceLoss", DoubleValue(400.0459));
    wifiPhy.SetChannel(wifiChannel.Create());

    // Add a non-QoS upper mac, and disable rate control
    WifiMacHelper wifiMac;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
        "DataMode", StringValue(phyMode),
        "ControlMode", StringValue(phyMode));
    // Setup the rest of the upper mac
    Ssid ssid = Ssid("wifi-80211b");
    wifiMac.SetType("ns3::ApWifiMac",
        "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevice = wifi.Install(wifiPhy, wifiMac, wifiApNode);

    NetDeviceContainer apDevices = apDevice;

    // setup sta.
    wifiMac.SetType("ns3::StaWifiMac",
        "Ssid", SsidValue(ssid),
        "ActiveProbing", BooleanValue(false));
    NetDeviceContainer staDevice = wifi.Install(wifiPhy, wifiMac, wifiStaNode);

    // Set channel width
    //Config::Set("/NodeList/*/DeviceList/0/$ns3::WifiNetDevice/Phy/ChannelWidth", UintegerValue(20));
    //Config::Set("/NodeList/*/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/DcaTxop/MinCw", UintegerValue(cw_Min));
    //Config::Set("/NodeList/*/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/DcaTxop/MaxCw", UintegerValue(cw_Max));

    // mobility.
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

    positionAlloc->Add(Vector(0.0, 0.0, 0.0));

    //setting distance between ap and nodes
    //for (uint32_t i = 0; i < nnode; i++) {
    //    positionAlloc->Add(Vector(distance, 0.0, 0.0));
    //}
    //mobility.SetPositionAllocator (positionAlloc);
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
        "MinX", DoubleValue(0.0),
        "MinY", DoubleValue(0.0),
        "DeltaX", DoubleValue(distance),
        "DeltaY", DoubleValue(distance),
        "GridWidth", UintegerValue(5),
        "LayoutType", StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    mobility.Install(wifiApNode);
    mobility.Install(wifiStaNode);

    /* Internet stack*/
    InternetStackHelper stack;
    stack.Install(wifiApNode);
    stack.Install(wifiStaNode);

    Ipv4AddressHelper address;

    address.SetBase("192.168.0.0", "255.255.255.0");

    Ipv4InterfaceContainer apNodeInterface;
    Ipv4InterfaceContainer staNodeInterface;
    apNodeInterface = address.Assign(apDevice);

    staNodeInterface = address.Assign(staDevice);



    // Tracing and logging.
    FlowMonitorHelper flowmon_helper;
    Ptr<FlowMonitor> monitor = flowmon_helper.InstallAll();
    monitor->SetAttribute("StartTime", TimeValue(Seconds(1))); //rozbieg symulatora (po tym czasie rozpocznie sie liczenie statystyk)
    monitor->SetAttribute("DelayBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("JitterBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("PacketSizeBinWidth", DoubleValue(20));

    /* Setting applications */
    ApplicationContainer serverApp, sinkApp;

    //UDP flow
if (udp)
{
        UdpServerHelper myServer(9);
        serverApp = myServer.Install(wifiApNode);
        serverApp.Start(Seconds(1.0));
        serverApp.Stop(Seconds(simulationTime + 2));


        UdpClientHelper myClient(apNodeInterface.GetAddress (0), 9);
        myClient.SetAttribute("MaxPackets", UintegerValue(maxpack));
        myClient.SetAttribute("Interval", TimeValue(Time(interval)));
        myClient.SetAttribute("PacketSize", UintegerValue(payloadSize));
                for (uint16_t i = 0; i < nnode; ++i)
                {
                        ApplicationContainer clientApp[nnode];
                        clientApp[i] = myClient.Install(wifiStaNode.Get(nnode - (i+1)));
                        clientApp[i].Start(Seconds(1.0));
                        clientApp[i].Stop(Seconds(simulationTime + 1));
                }
}
else
{
                    //TCP flow
                    uint16_t port = 50000;
                   Address apLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
                    PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", apLocalAddress);
                        sinkApp = packetSinkHelper.Install (wifiStaNode.Get (0));
  
                                sinkApp.Start (Seconds (0.0));
                                sinkApp.Stop (Seconds (simulationTime + 1));

                   OnOffHelper onoff ("ns3::TcpSocketFactory",Ipv4Address::GetAny ());
                    onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
                    onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
                   onoff.SetAttribute ("PacketSize", UintegerValue (payloadSize));
                   onoff.SetAttribute ("DataRate", DataRateValue (1000000000)); //bit/s
                   ApplicationContainer apps;
                 for (uint16_t i = 0; i < nnode; i++)
                {
                   AddressValue remoteAddress (InetSocketAddress (staNodeInterface.GetAddress (nnode - (i+1)), port));
                   onoff.SetAttribute ("Remote", remoteAddress);
                   apps.Add (onoff.Install (wifiApNode.Get (0)));
                   apps.Start (Seconds (1.0));
                   apps.Stop (Seconds (simulationTime + 1));
                }                }
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    wifiPhy.EnablePcap("SimpleHtHiddenStations_Ap", apDevice.Get(0));
    wifiPhy.EnablePcap("SimpleHtHiddenStations_Sta", staDevice.Get(0));
    wifiPhy.EnablePcap ("SimpleHtHiddenStations_Sta", staDevice.Get (1));
    //wifiPhy.EnablePcap ("SimpleHtHiddenStations_Sta", staDevice.Get (2));
    //wifiPhy.EnablePcap ("SimpleHtHiddenStations_Sta", staDevice.Get (3));
    //wifiPhy.EnablePcap ("SimpleHtHiddenStations_Sta", staDevice.Get (4));
    //wifiPhy.EnablePcap ("SimpleHtHiddenStations_Sta", staDevice.Get (5));
    //wifiPhy.EnablePcap ("SimpleHtHiddenStations_Sta", staDevice.Get (6));
    //wifiPhy.EnablePcap ("SimpleHtHiddenStations_Sta", staDevice.Get (7));
    //wifiPhy.EnablePcap ("SimpleHtHiddenStations_Sta", staDevice.Get (8));
    //wifiPhy.EnablePcap ("SimpleHtHiddenStations_Sta", staDevice.Get (9));



    Simulator::Stop(Seconds(simulationTime + 1));
    Simulator::Run();
    Simulator::Destroy();
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon_helper.GetClassifier());
    //monitor->SerializeToXmlFile ("logi/btb/8/results.xml", true, true);

    //Printing results
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

    for (std::map<FlowId, FlowMonitor::FlowStats>::iterator flow = stats.begin(); flow != stats.end(); flow++) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(flow->first);

        std::cout << "FlowID: " << flow->first << "("
                  << "TCP"
                  << " "
                  << t.sourceAddress << "/" << t.sourcePort << " --> "
                  << t.destinationAddress << "/" << t.destinationPort << ")" << std::endl;

        std::cout << "  Tx bytes: " << flow->second.txBytes << std::endl;
        std::cout << "  Rx bytes: " << flow->second.rxBytes << std::endl;
        std::cout << "  Tx packets: " << flow->second.txPackets << std::endl;
        std::cout << "  Rx packets: " << flow->second.rxPackets << std::endl;
        std::cout << "  Lost packets: " << flow->second.lostPackets << std::endl;
        std::cout << "  Throughput: " << flow->second.rxBytes * 8.0 / (flow->second.timeLastRxPacket.GetSeconds() - flow->second.timeFirstTxPacket.GetSeconds()) / 1024 << " Kbps";
        std::cout << "  Jitter sum: " << flow->second.jitterSum << std::endl;
        std::cout << "  Delay sum: " << flow->second.delaySum << std::endl;
    }
    double throughput = 0;

    uint32_t totalPacketsThrough = DynamicCast<UdpServer>(serverApp.Get(0))->GetReceived();
    throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s

    std::cout << nnode << " nnode" << std::endl;
    std::cout << (payloadSize * nnode / inter * 8) / 1000000.0 << " Mbit/s send" << std::endl;
    std::cout << throughput << " Mbit/s" << std::endl;
    return 0;
}
