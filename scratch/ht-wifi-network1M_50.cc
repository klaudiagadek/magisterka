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

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

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
using namespace std;
NS_LOG_COMPONENT_DEFINE("ht-wifi-network");

int main(int argc, char* argv[])
{
//Logowanie
//LogComponentEnable("ArpCache", LOG_LEVEL_ALL);
//LogComponentEnable("Ipv4AddressHelper", LOG_LEVEL_ALL);
//LogComponentEnable("Ipv4AddressGenerator", LOG_LEVEL_ALL);
//LogComponentEnable("ArpHeader", LOG_LEVEL_ALL);
//        LogComponentEnable ("UdpTraceClient", LOG_LEVEL_INFO);
//        LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
//LogComponentEnable ("DcaTxop", LOG_LEVEL_ALL); 

//Zmienne

//Ustawienie wielkosci pakietu
    bool udp = true;
    uint32_t payloadSize; //1500 byte IP packet
    if (udp) {
        payloadSize = 1472; //bytes
    }
    else {
        payloadSize = 1448; //bytes
        Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));
    } 

    double simulationTime = 100;                        //seconds
    double distance = 3.0;                              //meters
    uint32_t nnode = 50;                                 //nodes number
    uint32_t maxpack = 10000000;                         //max packets
    //char interval[] = "0.02";                           //interval between packets
    double inter = (payloadSize * nnode / 1.0 * 8) / 1000000.0;
    std::cout << (payloadSize * nnode / inter * 8) / 1000000.0 << " Mbit/s send" << std::endl;
    std::string interval = to_string(inter);
    std::string phyMode("DsssRate1Mbps");
    int   CWmin = 31;      //31
    int   CWmax = 1023;    //1023

//deklaracja zmiennych z terminala
    CommandLine cmd;
    cmd.AddValue("distance", "Distance in meters between the station and the access point", distance);
    cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
    cmd.AddValue("udp", "UDP if set to 1, TCP otherwise", udp);
    cmd.AddValue("phyMode", "Wifi Phy mode", phyMode);
    cmd.Parse(argc, argv);


/* 1.First Step : Create de nodes which will be part of wifi network */
    NodeContainer wifiStaNode;
    wifiStaNode.Create(nnode);

  /* 2.Second Step : Choose which node will be the AP for the network */

    NodeContainer wifiApNode;
    wifiApNode.Create(1);

    /* 3.Third Step : Build de wifi devices and the channel which will be shared for all devices */
    /* 3.1.Building a channel to be shared without interference */
    /* The helper can be used to create a WifiChannel with a default PropagationLoss and PropagationDelay model */
    YansWifiChannelHelper Channel = YansWifiChannelHelper::Default ();

    /* 3.2.Create a physical layer ( layer 1 ) */
    YansWifiPhyHelper Phy = YansWifiPhyHelper::Default();

/* 3.3.Associate the channel with physical layer belonging to any node, doind so we create a shared channel and the physical layer will decide how fast we can transmite in this channel */
        Phy.SetChannel ( Channel.Create ());

/* 3.4. Define the standard for physical layer and set up data rate */
    WifiHelper wifi;
    wifi.SetStandard(WIFI_PHY_STANDARD_80211b);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
        "DataMode", StringValue(phyMode),
        "ControlMode", StringValue(phyMode));
        YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
        "Exponent", DoubleValue(3.0),
        "ReferenceLoss", DoubleValue(40.0459));

    // Setup the rest of the upper mac
/* 4. Fourth Step : Create a data link layer a.k.a MAC layer */
/* 4.1. For all stations */
        WifiMacHelper Mac;
    Ssid ssid = Ssid("wifi-80211b");
    Mac.SetType("ns3::StaWifiMac",
                 "Ssid", SsidValue (ssid),
                 "ActiveProbing", BooleanValue (false));
 /* 4.2. Set up the network device for each station */
     NetDeviceContainer staDevice;
     staDevice = wifi.Install(Phy, Mac, wifiStaNode);

    /* 4.3. Set up the data link layer for AP */
    Mac.SetType ("ns3::ApWifiMac",
                "Ssid", SsidValue (ssid),
                "BeaconGeneration", BooleanValue (true),
                "BeaconInterval", TimeValue (Seconds (0.1024))
                );

    /* 4.4. Set up network device for AP */
    NetDeviceContainer apDevice;
    apDevice = wifi.Install (Phy, Mac, wifiApNode);


    /* 5. Fifth Step : Define mobility of the devices within wifi network */
    /* 5.1. Mobility for stations */
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
        "MinX", DoubleValue(0.0),
        "MinY", DoubleValue(0.0),
        "DeltaX", DoubleValue(distance),
        "DeltaY", DoubleValue(distance),
        "GridWidth", UintegerValue(5),
        "LayoutType", StringValue("RowFirst"));

/* 5.2. Kind of mobility and properties of Rectangle */
    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               "Bounds", RectangleValue (Rectangle (-600, 600, -600, 600))
                              );
    /* 5.3. Installing mobility in all stations */
    mobility.Install(wifiStaNode);

    /* 5.4. Set up mobility of AP (AP will not move) */
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiApNode);

    /* 6. Sixth Step : Install a protocol stack */
    InternetStackHelper stack;

        /* 6.1. Installing in each node */
    stack.Install(wifiApNode);

 /* 6.2. Installing in AP*/
    stack.Install(wifiStaNode);

    /* 8. Eighth Step : Give the IP address pool to network */
        Ipv4AddressHelper address;
        address.SetBase("192.168.0.0", "255.255.255.0");
        Ipv4InterfaceContainer staNodeInterface;
        staNodeInterface = address.Assign (staDevice);
        Ipv4InterfaceContainer apInterfaces;
        apInterfaces =  address.Assign (apDevice);

    /* Setting applications */
    ApplicationContainer serverApp, sinkApp;
 /* 9. Ninth Step : Set up a UDP Server */

    //UDP flow
if (udp)
{
        UdpServerHelper myServer(9);
        serverApp = myServer.Install(wifiApNode.Get (0));
        serverApp.Start(Seconds(2.0));
        serverApp.Stop(Seconds(simulationTime + 2));


        UdpClientHelper myClient(apInterfaces.GetAddress (0), 9);
        myClient.SetAttribute("MaxPackets", UintegerValue(maxpack));
        myClient.SetAttribute("Interval", TimeValue(Time(interval)));
        myClient.SetAttribute("PacketSize", UintegerValue(payloadSize));
      /* Creating multiple clients */
                for (uint16_t i = 0; i < nnode; ++i)
                {
                        ApplicationContainer clientApp[nnode];
                        clientApp[i] = myClient.Install(wifiStaNode.Get(nnode - (i+1)));
                        clientApp[i].Start(Seconds(2.0));
                        clientApp[i].Stop(Seconds(simulationTime + 2));
                }
}
else
{
                    //TCP flow
                    uint16_t port = 50000;
                   Address apLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
                    PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", apLocalAddress);
                        sinkApp = packetSinkHelper.Install (wifiStaNode.Get (0));
  
                                sinkApp.Start (Seconds (2.0));
                                sinkApp.Stop (Seconds (simulationTime + 2));

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
                   apps.Start (Seconds (2.0));
                   apps.Stop (Seconds (simulationTime + 2));
                }
                }
        /* 10. Tenth Step : Define a routing protocol */
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

     /* 11. Eleventh Step : Install FlowMonitor on all nodes */
     FlowMonitorHelper flowmon;
     Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
    monitor->SetAttribute("StartTime", TimeValue(Seconds(2))); //rozbieg symulatora (po tym czasie rozpocznie sie liczenie statystyk)
    monitor->SetAttribute("DelayBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("JitterBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("PacketSizeBinWidth", DoubleValue(20));

     Simulator::Stop (Seconds (simulationTime + 2));

//Ustawienie parametrw Cwmin i Cwmax
for (uint16_t i = 0; i < nnode+1; ++i)
{
                
Ptr<Node> node;

if (i==nnode){
        node = wifiApNode.Get(0);
}
else{
        node = wifiStaNode.Get(nnode - (i+1)); // Get station from node container 
}
	Ptr<NetDevice> dev = node->GetDevice(0);

	Ptr<WifiNetDevice> wifi_dev = DynamicCast<WifiNetDevice>(dev);

	Ptr<WifiMac> mac = wifi_dev->GetMac();

	

        PointerValue ptr;

	mac->GetAttribute("DcaTxop", ptr);

	

        Ptr<DcaTxop> dca = ptr.Get<DcaTxop>();

	dca->SetMinCw(CWmin);

	dca->SetMaxCw(CWmax);
}
     /* Enable checksum, it is desible by default because it is suppost that we have ideal and perfect network */
     GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
     Simulator::Run ();

     /* 10. Print per flow statistics */

     monitor->CheckForLostPackets ();
     Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
     FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
        Time delay_sum = Time(0);
        Time delay = Time(0);
        Time packets = Time(0);
        int lost_packets = 0;
        int send_packets = 0;
     for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator flow = stats.begin (); flow != stats.end (); ++flow)
    {

         if (flow->first > 0)
        {
/*          Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(flow->first);

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
          std::cout << "\n";
*/
        delay_sum = delay_sum + Time(flow->second.delaySum);
        packets = packets + (Time(flow->second.rxPackets));
        lost_packets = lost_packets + flow->second.rxPackets;
        send_packets = send_packets + flow->second.txPackets;
        }
}
        delay = Time(delay_sum/packets);delay = Time(delay_sum/packets);

            
//Obliczenie thoughput-u
double throughput = 0;

    uint32_t totalPacketsThrough = DynamicCast<UdpServer>(serverApp.Get(0))->GetReceived();
    throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s

std::cout <<"phyMode "<<phyMode<< "   Thr " << throughput << "Mbit/s"<< "   delay sum " << delay_sum << "   delay "<< delay << "   lost:" << lost_packets << "   send:" << send_packets <<std::endl;


//Sprzatanie po symulacji 
  Simulator::Destroy ();

    return 0;
}
