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
NS_LOG_COMPONENT_DEFINE ("ht-wifi-network");

int main (int argc, char *argv[])
{
//LogComponentEnable("EdcaTxopN", LOG_LEVEL_ALL);
uint32_t nnode = 3;
  bool udp = true;
  double simulationTime = 2; //seconds
  double distance = 3.0; //meters
  double frequency = 5.0; //whether 2.4 or 5.0 GHz
  double th[] = {6.5, 7.2, 13.5, 15.0, 13.0, 14.4, 27.0, 30.0, 19.5, 21.7, 40.5, 45.0, 26.0, 28.9, 54.0, 60.0, 39.0, 43.3, 81.0, 90.0, 52.0, 57.8, 108.0, 120.0, 58.5, 65.0, 121.5, 135.0, 65.0, 72.2, 135.0, 150.0}; //send 
    int   CWmin = 15;      //31
    int   CWmax = 1023;    //1023
  CommandLine cmd;
  cmd.AddValue ("frequency", "Whether working in the 2.4 or 5.0 GHz band (other values gets rejected)", frequency);
  cmd.AddValue ("distance", "Distance in meters between the station and the access point", distance);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("udp", "UDP if set to 1, TCP otherwise", udp);
  cmd.Parse (argc,argv);

//Ustawienie parametrw Cwmin i Cwmax
int th_it = -1;
  for (int i = 0; i <= 7; i++) //MCS do 7
    {
      for (int j = 20; j <= 40; ) //channel width
        {
          for (int k = 0; k < 2; k++) //GI: 0 and 1     800 potem 400
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
                



              YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
              YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
              phy.SetChannel (channel.Create ());

              // Set guard interval
              phy.Set ("ShortGuardEnabled", BooleanValue (k));

              WifiMacHelper mac;
              WifiHelper wifi;
              if (frequency == 5.0)
                {
                  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
                }
              else if (frequency == 2.4)
                {
                  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);
                  Config::SetDefault ("ns3::LogDistancePropagationLossModel::ReferenceLoss", DoubleValue (40.046));
                }
              else
                {
                  std::cout<<"Wrong frequency value!"<<std::endl;
                  return 0;
                }

              std::ostringstream oss;
              oss << "HtMcs" << i;
              wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue (oss.str ()),
                                            "ControlMode", StringValue (oss.str ()));
                
              Ssid ssid = Ssid ("ns3-80211n");

              mac.SetType ("ns3::StaWifiMac",
                           "Ssid", SsidValue (ssid));

              NetDeviceContainer staDevice;
              staDevice = wifi.Install (phy, mac, wifiStaNode);

              mac.SetType ("ns3::ApWifiMac",
                           "Ssid", SsidValue (ssid));

              NetDeviceContainer apDevice;
              apDevice = wifi.Install (phy, mac, wifiApNode);

              // Set channel width
              Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/ChannelWidth", UintegerValue (j));

              // mobility.
              MobilityHelper mobility;
              Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

              positionAlloc->Add (Vector (0.0, 0.0, 0.0));
              positionAlloc->Add (Vector (distance, 0.0, 0.0));
              mobility.SetPositionAllocator (positionAlloc);

              mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

              mobility.Install (wifiApNode);
              mobility.Install (wifiStaNode);

              /* Internet stack*/
              InternetStackHelper stack;
              stack.Install (wifiApNode);
              stack.Install (wifiStaNode);

              Ipv4AddressHelper address;

              address.SetBase ("192.168.1.0", "255.255.255.0");
              Ipv4InterfaceContainer staNodeInterface;
              Ipv4InterfaceContainer apNodeInterface;

              staNodeInterface = address.Assign (staDevice);
              apNodeInterface = address.Assign (apDevice);

              /* Setting applications */
              ApplicationContainer serverApp, sinkApp;
              if (udp)
                {
                  //UDP flow
                  UdpServerHelper myServer (9);
                  serverApp = myServer.Install (wifiStaNode.Get (0));
                  serverApp.Start (Seconds (2.0));
                  serverApp.Stop (Seconds (simulationTime + 2));

                  UdpClientHelper myClient (staNodeInterface.GetAddress (0), 9);
                  myClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
                  myClient.SetAttribute ("Interval", TimeValue (Time (interval))); //packets/s
                  myClient.SetAttribute ("PacketSize", UintegerValue (payloadSize));
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
std::cout <<"MCS "<< i << "   Chann_width " << j << "MHz   short GI " << k << "   Thr " << throughput << "Mbit/s"<< "   delay sum " << delay_sum << "   delay "<< delay << "   lost:" << lost_packets << "   send:" << send_packets <<std::endl;
            }
          j *= 2;
        }
    }
  return 0;
}
