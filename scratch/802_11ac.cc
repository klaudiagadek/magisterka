/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 SEBASTIEN DERONNE
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
 * Author: Sebastien Deronne <sebastien.deronne@gmail.com>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/nstime.h"
#include "ns3/int64x64-128.h"

#include <iostream> 
#include <fstream>
#include <vector>
#include <string>

// This is a simple example in order to show how to configure an IEEE 802.11n Wi-Fi network.
// This is a simple example in order to show how to configure an IEEE 802.11ac Wi-Fi network.
//
// It ouputs the UDP or TCP goodput for every VHT bitrate value, which depends on the MCS value (0 to 9, where 9 is
// forbidden when the channel width is 20 MHz), the channel width (20, 40, 80 or 160 MHz) and the guard interval (long
// or short). The PHY bitrate is constant over all the simulation run. The user can also specify the distance between
// the access point and the station: the larger the distance the smaller the goodput.
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
NS_LOG_COMPONENT_DEFINE ("vht-wifi-network");

int main (int argc, char *argv[])
{
uint32_t nnode = 10;
  bool udp = true;
  double simulationTime = 100; //seconds
  double distance = 3.0; //meters
        uint32_t maxpack = 10000000; 
  double th[] = {6.5, 7.2, 13.5, 15.0, 29.3, 32.5, 58.5, 65.0, 13.0, 14.4, 27.0, 30.0, 58.5, 65.0, 117.0, 130.0, 19.5, 21.7, 40.5, 45.0, 87.8, 97.5, 175.5, 195.0, 26.0, 28.9, 54.0, 60.0, 117.0, 130.0, 234.0, 260.0, 39.0, 43.3, 81.0, 90.0, 175.5, 195.0, 351.0, 390.0, 52.0, 57.8, 108.0, 120.0, 234.0, 260.0, 468.0, 520.0, 58.5, 65.0, 121.5, 135.0, 263.3, 292.5, 526.5, 585.0, 65.0, 72.2, 135.0, 150.0, 292.5, 325.0, 585.0, 650.0}; //send 
//string phymod[] = {"VhtMcs0", "VhtMcs1", "VhtMcs2", "VhtMcs3", "VhtMcs4", "VhtMcs5", "VhtMcs6", "VhtMcs7", "VhtMcs0", "VhtMcs1", "VhtMcs2", "VhtMcs3", "VhtMcs12", "VhtMcs13", "VhtMcs14", "VhtMcs15", "VhtMcs16", "VhtMcs17", "VhtMcs18", "VhtMcs19", "VhtMcs20", "VhtMcs21", "VhtMcs22", "VhtMcs23", "VhtMcs24", "VhtMcs25", "VhtMcs26", "VhtMcs27", "VhtMcs28", "VhtMcs29", "VhtMcs30", "VhtMcs31"}; //send 
  string ControlMode[] = {"VhtMcs0", "VhtMcs1", "VhtMcs2", "VhtMcs3", "VhtMcs4", "VhtMcs5", "VhtMcs6", "VhtMcs7", "VhtMcs0", "VhtMcs1", "VhtMcs2", "VhtMcs3", "VhtMcs4", "VhtMcs5", "VhtMcs6", "VhtMcs7", "VhtMcs0", "VhtMcs1", "VhtMcs2", "VhtMcs3", "VhtMcs4", "VhtMcs5", "VhtMcs6", "VhtMcs7", "VhtMcs0", "VhtMcs1", "VhtMcs2", "VhtMcs3", "VhtMcs4", "VhtMcs5", "VhtMcs6", "VhtMcs7"}; //send 
  string DataMode[] =    {"VhtMcs0", "VhtMcs0", "VhtMcs0", "VhtMcs0", "VhtMcs0", "VhtMcs0", "VhtMcs0", "VhtMcs0", "VhtMcs1", "VhtMcs1", "VhtMcs1", "VhtMcs1", "VhtMcs1", "VhtMcs1", "VhtMcs1", "VhtMcs1", "VhtMcs2", "VhtMcs2", "VhtMcs2", "VhtMcs2", "VhtMcs2", "VhtMcs2", "VhtMcs2", "VhtMcs2", "VhtMcs3", "VhtMcs3", "VhtMcs3", "VhtMcs3", "VhtMcs3", "VhtMcs3", "VhtMcs3", "VhtMcs3"};
        
   int   CWmin = 15;      //31
    int   CWmax = 1023;    //1023
  
int th_it = -1;
  for (int i = 0; i <= 9; i++) //MCS
    {
      for (int j = 20; j <= 160; ) //channel width
        {
          if (i == 9 && j == 20)
            {
              j *= 2;
              continue;
            }
          for (int k = 0; k < 2; k++) //GI: 0 and 1
            {
                th_it = th_it+1;
              double  thr = th[th_it];
              uint32_t payloadSize; //1500 byte IP packet
              if (udp)
                {
                  payloadSize = 1472; //bytes
                }
              else
                {
                  payloadSize = 1448; //bytes
                  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));
                }
                double inter = (payloadSize * nnode / thr * 8) / 1000000.0;
                std::string interval = to_string(inter);

              NodeContainer wifiStaNode;
              wifiStaNode.Create (nnode);
              NodeContainer wifiApNode;
              wifiApNode.Create (1);
YansWifiChannelHelper wifiChannel;
    wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
        "Exponent", DoubleValue(3.0),
        "ReferenceLoss", DoubleValue(40.0459));  
              YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
              YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
              phy.SetChannel (channel.Create ());

              // Set guard interval
              phy.Set ("ShortGuardEnabled", BooleanValue (k));

              WifiHelper wifi;
              wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
              WifiMacHelper mac;
                
              std::ostringstream oss;
              oss << "VVhtMcs" << i;

              wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue(DataMode[th_it]),
                                "ControlMode", StringValue(ControlMode[th_it]));
                
              Ssid ssid = Ssid ("ns3-80211ac");

    mac.SetType("ns3::StaWifiMac",
                 "Ssid", SsidValue (ssid),
                 "ActiveProbing", BooleanValue (false));

              NetDeviceContainer staDevice;
              staDevice = wifi.Install (phy, mac, wifiStaNode);

    mac.SetType ("ns3::ApWifiMac",
                "Ssid", SsidValue (ssid),
                "BeaconGeneration", BooleanValue (true),
                "BeaconInterval", TimeValue (Seconds (0.1024))
                );

              NetDeviceContainer apDevice;
              apDevice = wifi.Install (phy, mac, wifiApNode);

              // Set channel width
              Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/ChannelWidth", UintegerValue (j));

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

              /* Internet stack*/
              InternetStackHelper stack;
              stack.Install (wifiApNode);
              stack.Install (wifiStaNode);

              Ipv4AddressHelper address;

              address.SetBase ("192.168.1.0", "255.255.255.0");
        Ipv4InterfaceContainer staNodeInterface;
        staNodeInterface = address.Assign (staDevice);
        Ipv4InterfaceContainer apInterfaces;
        apInterfaces =  address.Assign (apDevice);
              /* Setting applications */
              ApplicationContainer serverApp, sinkApp;
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

              Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
             /* 11. Eleventh Step : Install FlowMonitor on all nodes */
     FlowMonitorHelper flowmon;
     Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
    monitor->SetAttribute("StartTime", TimeValue(Seconds(2))); //rozbieg symulatora (po tym czasie rozpocznie sie liczenie statystyk)
    monitor->SetAttribute("DelayBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("JitterBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("PacketSizeBinWidth", DoubleValue(20));

              Simulator::Stop (Seconds (simulationTime + 2));
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
	Ptr<EdcaTxopN> edca;

mac->GetAttribute("VO_EdcaTxopN", ptr);
edca = ptr.Get<EdcaTxopN>();
edca->SetMinCw(CWmin);
edca->SetMaxCw(CWmax);

mac->GetAttribute("VI_EdcaTxopN", ptr);
edca = ptr.Get<EdcaTxopN>();
edca->SetMinCw(CWmin);
edca->SetMaxCw(CWmax);

mac->GetAttribute("BE_EdcaTxopN", ptr);
edca = ptr.Get<EdcaTxopN>();
edca->SetMinCw(CWmin);
edca->SetMaxCw(CWmax);

mac->GetAttribute("BK_EdcaTxopN", ptr);
edca = ptr.Get<EdcaTxopN>();
edca->SetMinCw(CWmin);
edca->SetMaxCw(CWmax);

}
     /* Enable checksum, it is desible by default because it is suppost that we have ideal and perfect network */
     GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
              Simulator::Run ();
              Simulator::Destroy ();

              double throughput = 0;
              if (udp)
                {
                  //UDP
                  uint32_t totalPacketsThrough = DynamicCast<UdpServer> (serverApp.Get (0))->GetReceived ();
                  throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s
                }
              else
                {
                  //TCP
                  uint32_t totalPacketsThrough = DynamicCast<PacketSink> (sinkApp.Get (0))->GetTotalRx ();
                  throughput = totalPacketsThrough * 8 / (simulationTime * 1000000.0); //Mbit/s
                }
                    
     /* 10. Print per flow statistics */
//std::cout << "Cw value = " << CWvalue  << std::endl;
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
        delay = Time(delay_sum/packets);
std::cout <<"MCS "<< i << "   Chann_width " << j << "MHz   short GI " << k << "   Thr " << throughput << "Mbit/s"<< "   delay sum " << delay_sum << "   delay "<< delay << "   lost:" << lost_packets<< "   send:" << send_packets << std::endl;
            }
          j *= 2;
        }
    }
  return 0;
}
