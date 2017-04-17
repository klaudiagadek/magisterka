
/* *****Graduation Work *******   */
/*
    Theme  : Perfomance of wireless network in second layer (datalink layer)
    Author : Geovani Teca
    Tutor  : Szymon Szott
*/

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/flow-monitor-module.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

/* Default Network Topology IBSS (Independent Basic Service Set)

   Wifi 10.1.1.0

                      AP
  *    *    *   ...   *
  |    |    |   ...   |
 n0   n1   n2   ...  n4 (AP will be always the last node in right side,but node 0 in inner topology created by wifiApNode)
  C                  S
  C = CLIENT         S = Server
  X = Number of nodes

*/
using namespace ns3;                             // namespace where are located all NS_LOG messages
using namespace std;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample"); // Allow to record the program's name, always before main function

int main (int argc, char *argv[])
{

    /* Space for variables declaration */

    bool       verbose = true;
    int        n_stations = 3;
    bool       tracing = false;
    double     simulationTime = 9;
    uint32_t   payloadSize = 1024;
    double     offeredLoad = 5*1000*1000;
    double     intervalSize = (payloadSize*8)/(offeredLoad);
    uint64_t   numberOfPackets = 1000*1000;
    char       standard = 'a';
    bool       enableCtsRts = false;
    uint32_t   CWmin_ofdm = 15;
    uint32_t   CWmax_ofdm = 511;
    uint32_t   CWmin_dsss = 116;
    uint32_t   CWmax_dsss = 116;
    bool       enableCW = false;
    // float    frequency = 0
   /* 254 nodes should be enough, otherwise IP addresses will became an issue */

   if (n_stations > 254)
    {
      cout << "Too many wifi nodes, no more than 254 ." <<endl;
      return 1;
    }

   if ( (CWmin_ofdm < 15) && (CWmax_ofdm > 511) && (CWmin_dsss < 31) && (CWmax_dsss > 1023) )
     {
     cout << "One of CW's parameters are wrong, please check it out " <<endl;
     cout << "CW_ofdm [15;511] and CW_dsss [31;1023]"<<endl;
     return 1;
     }

    /* Specification about wich variable can be changed from command line */

    CommandLine cmd;
    cmd.AddValue ("n_stations", "Number of wifi STATION devices", n_stations);
    cmd.AddValue ("verbose", "Tell echo applications to log messages about their activities if true", verbose);
    cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
    cmd.AddValue ("simulationTime", "Time of simulation", simulationTime);
    cmd.AddValue ("payLoadSize", "The size of Packet to be send by client", payloadSize);
    cmd.AddValue ("offeredLoad","Change the offere load", offeredLoad);
    cmd.AddValue ("standard","Change the standard to b,g,n or c",standard);
    cmd.AddValue ("enableCtsRts","Enable or disable RTS/CTS requests",enableCtsRts);

    cmd.Parse (argc,argv);

    /* Enable or disable option to register activity of client or server */

    if (verbose)
    {
     LogComponentEnable ("UdpTraceClient", LOG_LEVEL_INFO);
     LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
    }

    /* 1.First Step : Create de nodes which will be part of wifi network */

    NodeContainer wifiStaNodes;
    wifiStaNodes.Create (n_stations);

    /* 2.Second Step : Choose which node will be the AP for the network */

    NodeContainer wifiApNode ;
    wifiApNode.Create(1);

    /* 3.Third Step : Build de wifi devices and the channel which will be shared for all devices */
    /* 3.1.Building a channel to be shared without interference */
    /* The helper can be used to create a WifiChannel with a default PropagationLoss and PropagationDelay model */

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();


    /* 3.2.Create a physical layer ( layer 1 ) */

    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();

    /* 3.3.Associate the channel with physical layer belonging to any node, doind so we create a shared channel and the physical layer will decide how fast we can transmite in this channel */

    phy.SetChannel (channel.Create ());

    /* 3.4. Define the standard for physical layer and set up data rate */

    WifiHelper wifi;

    if(standard == 'a')
    {
      wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                            "DataMode", StringValue ("OfdmRate54Mbps"),
                            "ControlMode", StringValue ("OfdmRate6Mbps"),
                            "MaxSsrc", UintegerValue (7),
                            "MaxSlrc", UintegerValue (4),
                            "FragmentationThreshold", UintegerValue (2500));
      if (enableCW)
      {
       Config::Set ("/NodeList/1/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/DcaTxop/MinCw",UintegerValue(CWmin_ofdm));
       Config::Set ("/NodeList/1/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/DcaTxop/MaxCw",UintegerValue(CWmax_ofdm));
      }
    }

    /* 54 max datarate and 6 is the min datarate
       MaxSsrc = The maximum number of retransmission attempts for an RTS
       MaxSLrc = The maximum number of retransmission attempts for a DATA packet
       Fragme ..... = If  the size of the PSDU is bigger than this value, we fragment it such that size of the fragments are equal or smaller
       PSDU (Physical Layer Service Data Unit)
       represents the contents of the PPDU The 802.11a PHY Layer Convergence Procedure (PLCP) transforms each 802.11 frame that a station wishes to send into a PLCP protocol data unit (PPDU).
    */


    else if (standard == 'b')
    {
      wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                   "DataMode",StringValue ("DsssRate11Mbps"),
                                   "ControlMode",StringValue ("DsssRate1Mbps"));

     YansWifiChannelHelper wifiChannel;
     wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
     wifiChannel.AddPropagationLoss  ("ns3::LogDistancePropagationLossModel",
                                      "Exponent", DoubleValue (3.0),
                                      "ReferenceLoss", DoubleValue (40.0459));
      if (enableCW)
        {
          Config::Set ("/NodeList/1/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/DcaTxop/MinCw",UintegerValue(CWmin_dsss));
          Config::Set ("/NodeList/1/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/DcaTxop/MaxCw",UintegerValue(CWmax_dsss));
        }
     }

    /*
     By default propagation loss for standar IEEE 802.11 a is set, so propagation loss is needed for others standards like IEEE 802.11 b,n,g wich work at 2.4 GHz
    */


    else
    {
      wifi.SetStandard (WIFI_PHY_STANDARD_80211g);
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                            "DataMode", StringValue ("ErpOfdmRate54Mbps"),
                            "ControlMode", StringValue ("ErpOfdmRate24Mbps"),
                            "MaxSsrc", UintegerValue (7),
                            "MaxSlrc", UintegerValue (4),
                            "FragmentationThreshold", UintegerValue (2500));

      YansWifiChannelHelper wifiChannel;
      wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
      wifiChannel.AddPropagationLoss  ("ns3::LogDistancePropagationLossModel",
                                       "Exponent", DoubleValue (3.0),
                                       "ReferenceLoss", DoubleValue (40.0459));

        if (enableCW)
           {
             Config::Set ("/NodeList/1/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/DcaTxop/MinCw",UintegerValue(CWmin_ofdm));
             Config::Set ("/NodeList/1/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/DcaTxop/MaxCw",UintegerValue(CWmax_ofdm));
           }
       }

    /*

     ******** STANDARD 802.11n 5GHZ **********
     wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
     YansWifiChannelHelper wifiChannel;
     wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
     wifiChannel.AddPropagationLoss  ("ns3::LogDistancePropagationLossModel",
                                      "Exponent", DoubleValue (3.0),
                                      "ReferenceLoss", DoubleValue (40.0459));
    */


      /* 4. Fourth Step : Create a data link layer a.k.a MAC layer */
      /* 4.1. For all stations */

    WifiMacHelper mac;
    Ssid ssid = Ssid ("Wifinet");
    mac.SetType ("ns3::StaWifiMac",
                 "Ssid", SsidValue (ssid),
                 "ActiveProbing", BooleanValue (false));

     /* 4.2. Set up the network device for each station */

     NetDeviceContainer staDevices;
     staDevices = wifi.Install (phy, mac, wifiStaNodes);


     /*
       ActivingProbing checks if an IP address is really active if A station is transmitting
       SSID = Service Set IDentifier : The name assigned to a Wi-Fi network
    */

     /* Enable or disable RTS/CTS */

       if( enableCtsRts )
      {
       UintegerValue ctsThr = (enableCtsRts ? UintegerValue (100) : UintegerValue (2200));
       Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", ctsThr);
      }

    /* 4.3. Set up the data link layer for AP */

    mac.SetType ("ns3::ApWifiMac",
                "Ssid", SsidValue (ssid),
                "BeaconGeneration", BooleanValue (true),
                "BeaconInterval", TimeValue (Seconds (0.1024))
                );

    /* 4.4. Set up network device for AP */

    NetDeviceContainer apDevices;
    apDevices = wifi.Install (phy, mac, wifiApNode);

    /* 5. Fifth Step : Define mobility of the devices within wifi network */
    /* 5.1. Mobility for stations */

    MobilityHelper mobility;
    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                   "MinX", DoubleValue (0.0),
                                   "MinY", DoubleValue (0.0),
                                   "DeltaX", DoubleValue (1.0),
                                   "DeltaY", DoubleValue (1.0),
                                   "GridWidth", UintegerValue (5),
                                   "LayoutType", StringValue ("RowFirst"));
     /*
      GridPositionAllocator = Allocate positions on a rectangular 2d grid
      MinX = The X coordinate where the grid starts , MinY = The Y coordinate where thr grid starts
      DeltaX = The spaces X ,DeltaY = The spaces Y where the grid starts
      GridWidth = Number of objects layed out on a line , LayoutType = The type of layout ( Row First )
      To summarize : I've defined that the topology will have 5 nodes for each row
    */

    /* 5.2. Kind of mobility and properties of Rectangle */

    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               "Bounds", RectangleValue (Rectangle (-600, 600, -600, 600))
                              );

    /*
     RandomWalk2dMobilityModel = Each station moves with a speed and direction choosen  randomly, if one of them hit the boudaries ( specified by a rectangle)
     of the model, we rebound on the boudary with a reflexive angle and speed ( is a closed box ) .
     Bounds = RectangleValue (Rectangle (Xmin, Xmax, Ymin, Ymax)) .
    */

    /* 5.3. Installing mobility in all stations */

    mobility.Install (wifiStaNodes);

    /* 5.4. Set up mobility of AP (AP will not move) */

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiApNode);

    /* 6. Sixth Step : Install a protocol stack */

    InternetStackHelper stack;

    /* 6.1. Installing in each node */

    stack.Install (wifiApNode);

    /* 6.2. Installing in AP*/

    stack.Install (wifiStaNodes);

    /* 8. Eighth Step : Give the IP address pool to network */

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    address.Assign (staDevices);
    Ipv4InterfaceContainer apInterfaces;
    apInterfaces =  address.Assign (apDevices);

    /* 9. Ninth Step : Set up a UDP Server */

     uint16_t port = 9;
     UdpServerHelper Server (port);
     ApplicationContainer serverApp = Server.Install (wifiApNode.Get (0));
     serverApp.Start (Seconds (1.0));
     serverApp.Stop (Seconds (simulationTime + 1));


     UdpClientHelper Client (apInterfaces.GetAddress (0), port);
     Client.SetAttribute ("MaxPackets", UintegerValue (numberOfPackets));
     Client.SetAttribute ("Interval", TimeValue (Seconds (intervalSize)));
     Client.SetAttribute ("PacketSize", UintegerValue (payloadSize ));

      /* Creating multiple clients */

      for(int iterator = 0 ; iterator < n_stations; ++iterator)
      {
       ApplicationContainer clientApp [n_stations];
       clientApp[iterator]= Client.Install (wifiStaNodes.Get (n_stations - (iterator+1)));
       clientApp[iterator].Start (Seconds (1.0));
       clientApp[iterator].Stop  (Seconds(simulationTime + 1));
      }

      /* 10. Tenth Step : Define a routing protocol */

      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

      if (tracing == true)
      phy.EnablePcap ("third", staDevices);

     /* 11. Eleventh Step : Install FlowMonitor on all nodes */

     FlowMonitorHelper flowmon;
     Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

     Simulator::Stop (Seconds (simulationTime + 1));

     /* Enable checksum, it is desible by default because it is suppost that we have ideal and perfect network */

     GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
     Simulator::Run ();

     /* 10. Print per flow statistics */

     monitor->CheckForLostPackets ();
     Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
     FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
     for (map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {

         if (i->first > 0)
        {
          Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
          cout <<"\n";
          cout << "Flow " << i->first <<" From (Station)"<< " with IP : " << t.sourceAddress << " To (AP) with address : " << t.destinationAddress <<"\n";
          cout << "  Tx Packets: " << i->second.txPackets << "\n";
          cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          cout << "  TxOffered:  " << i->second.txBytes * 8.0 / simulationTime / 1000 / 1000  << " Mbps\n";
          cout << "  Rx Packets: " << i->second.rxPackets << "\n";
          cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
          cout << "  Throughput: " << i->second.rxBytes * 8.0 / simulationTime / 1000 / 1000  << " Mbps\n";
          cout << "\n";
        }
    }

  Simulator::Destroy ();
  return 0;
}




