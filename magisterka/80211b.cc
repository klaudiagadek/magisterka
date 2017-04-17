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

NS_LOG_COMPONENT_DEFINE ("ht-wifi-network");

int main (int argc, char *argv[])
{
  bool udp = true;
  double simulationTime = 5; //seconds
  double distance = 1.0; //meters
  uint32_t nnode = 50;
  std::string phyMode ("DsssRate1Mbps");
Ptr<DcaTxop> m_beaconDca = CreateObject<DcaTxop> ();
        m_beaconDca->SetMinCw(100);
        m_beaconDca->SetMaxCw(100);
 CommandLine cmd;
  cmd.AddValue ("distance", "Distance in meters between the station and the access point", distance);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("udp", "UDP if set to 1, TCP otherwise", udp);
  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.Parse (argc,argv);
LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
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

              NodeContainer wifiStaNode;
              wifiStaNode.Create (nnode);
              NodeContainer wifiApNode;
              wifiApNode.Create (1);


                WifiHelper wifi;
                wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

                YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
                // ns-3 supports RadioTap and Prism tracing extensions for 802.11
                wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

                YansWifiChannelHelper wifiChannel;
                // reference loss must be changed since 802.11b is operating at 2.4GHz
                wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
                wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel",
                                                "Exponent", DoubleValue (3.0),
                                                "ReferenceLoss", DoubleValue (40.0459));
                wifiPhy.SetChannel (wifiChannel.Create ());

                // Add a non-QoS upper mac, and disable rate control
                WifiMacHelper wifiMac;
                wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                              "DataMode",StringValue (phyMode),
                                              "ControlMode",StringValue (phyMode));
                // Setup the rest of the upper mac
                Ssid ssid = Ssid ("wifi-80211b");
                wifiMac.SetType ("ns3::ApWifiMac",
                                 "Ssid", SsidValue (ssid));
                NetDeviceContainer apDevice = wifi.Install (wifiPhy, wifiMac, wifiApNode);
                NetDeviceContainer apDevices = apDevice;

                // setup sta.
                wifiMac.SetType ("ns3::StaWifiMac",
                                 "Ssid", SsidValue (ssid),
                                 "ActiveProbing", BooleanValue (false));
                NetDeviceContainer staDevice = wifi.Install (wifiPhy, wifiMac, wifiStaNode);

              // Set channel width
              Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/ChannelWidth", UintegerValue (20));

              // mobility.
              MobilityHelper mobility;
              Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

              positionAlloc->Add (Vector (0.0, 0.0, 0.0));
              positionAlloc->Add (Vector (distance, 0.0, 0.0));
              positionAlloc->Add (Vector (distance, distance, 0.0));
              positionAlloc->Add (Vector (0.0, distance, 0.0));
              positionAlloc->Add (Vector (distance,distance, distance));
              positionAlloc->Add (Vector (distance, 0.0, distance));
              positionAlloc->Add (Vector (0.0, 0.0, distance));
              positionAlloc->Add (Vector (2*distance, 0.0, 0.0));
              positionAlloc->Add (Vector (distance, 2*distance, 0.0));
              positionAlloc->Add (Vector (0.0, 2*distance, distance));
              positionAlloc->Add (Vector (distance, distance,2*distance));
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


                  //UDP flow
                  UdpServerHelper myServer (9);

                  serverApp = myServer.Install (wifiApNode.Get(0));
                  serverApp.Start (Seconds (0.0));
                  serverApp.Stop (Seconds (simulationTime + 1));

                  UdpClientHelper myClient (apNodeInterface.GetAddress (0), 9);
                  myClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
                  myClient.SetAttribute ("Interval", TimeValue (Time ("0.00001"))); //packets/s
                  myClient.SetAttribute ("PacketSize", UintegerValue (payloadSize));
                  ApplicationContainer clientApp = myClient.Install (wifiStaNode);
                        
                  clientApp.Start (Seconds (1.0));
                  clientApp.Stop (Seconds (simulationTime + 1));
                        

              Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
                wifiPhy.EnablePcap ("SimpleHtHiddenStations_Ap", apDevice.Get (0));
        wifiPhy.EnablePcap ("SimpleHtHiddenStations_Sta1", staDevice.Get (0));
        wifiPhy.EnablePcap ("SimpleHtHiddenStations_Sta2", staDevice.Get (1));
        wifiPhy.EnablePcap ("SimpleHtHiddenStations_Sta3", staDevice.Get (2));
              Simulator::Stop (Seconds (simulationTime + 1));
              Simulator::Run ();
              Simulator::Destroy ();

              double throughput = 0;
                  //UDP
                  uint32_t totalPacketsThrough = DynamicCast<UdpServer> (serverApp.Get (0))->GetReceived ();
                  throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s

              std::cout << throughput << " Mbit/s" << std::endl;
  return 0;
}