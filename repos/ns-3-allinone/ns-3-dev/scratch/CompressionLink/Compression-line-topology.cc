/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
 // Network topology
 //                           Compression link
 //         8 Mb/s, 2ms      1,2,..10 Mb/s, 2ms      8 Mb/s, 2ms
 //     n0---------------n1----------------------n2---------------n3
 //   Server           Router                  Router           Client

 #include <iostream>
 #include <fstream>
 #include <string>
 #include <cassert>
 
 #include "ns3/core-module.h"
 #include "ns3/network-module.h"
 #include "ns3/internet-module.h"
 #include "ns3/point-to-point-module.h"
 #include "ns3/applications-module.h"
 #include "ns3/flow-monitor-helper.h"
 #include "ns3/ipv4-global-routing-helper.h"
 #include "ns3/queue.h"
 #include "ns3/packet.h"
 #include "ns3/config-store.h"

using namespace std;
using namespace ns3;
 
 NS_LOG_COMPONENT_DEFINE ("CompressionLineTopology");
 
 int 
 main (int argc, char *argv[])
 {

     CommandLine cmd;
	 string capacity = "";
	 cmd.AddValue("capacity", "Middle Link Capacity", capacity);
	 cmd.Parse (argc, argv);

	 ConfigStore inputConfig;
	 inputConfig.ConfigureDefaults ();
	 Config::SetDefault ("ns3::PointToPointNetDevice::m_protocol", UintegerValue (0x0021));
	 cmd.Parse (argc, argv);

	 Time::SetResolution (Time::MS);
	 //LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
	 //LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
	 Packet packet;
	 packet.EnablePrinting();
	 NodeContainer nodes;
	 nodes.Create (4);

	 InternetStackHelper stack;
	 stack.Install (nodes);

	 PointToPointHelper p0p1;
	 p0p1.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));
	 p0p1.SetChannelAttribute ("Delay", StringValue ("2ms"));

	 PointToPointHelper p1p2;
	 p1p2.SetDeviceAttribute ("DataRate", StringValue (capacity));
	 p1p2.SetChannelAttribute ("Delay", StringValue ("2ms"));

	 PointToPointHelper p2p3;
	 p2p3.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));
	 p2p3.SetChannelAttribute ("Delay", StringValue ("2ms"));

	 Ipv4AddressHelper address;
	 NetDeviceContainer devices;
	 AsciiTraceHelper ascii;

	 devices = p0p1.Install (nodes.Get(0), nodes.Get(1));
	 address.SetBase ("10.1.1.0", "255.255.255.0");
	 Ipv4InterfaceContainer interface1 = address.Assign (devices);
	 p0p1.EnableAsciiAll (ascii.CreateFileStream ("myfirst.tr"));
	 p0p1.EnablePcap("UDPsender", devices.Get(0), false);

	 devices = p1p2.Install (nodes.Get(1), nodes.Get(2));
	 address.SetBase ("10.1.2.0", "255.255.255.0");
	 Ipv4InterfaceContainer interface2 = address.Assign (devices);
	 p1p2.EnableAsciiAll (ascii.CreateFileStream ("mysecond.tr"));
	 p1p2.EnablePcap("Compression",devices.Get(0), false);

	 PointerValue ptr;
	 Ptr<PointToPointNetDevice> net0 = DynamicCast<PointToPointNetDevice>(devices.Get(0));
	 net0->EnableCompression(true);
	 Ptr<PointToPointNetDevice> net1 = DynamicCast<PointToPointNetDevice>(devices.Get(1));
	 net1->EnableDeCompression(true);

	 devices = p2p3.Install (nodes.Get(2), nodes.Get(3));
	 address.SetBase ("10.1.3.0", "255.255.255.0");
	 Ipv4InterfaceContainer interface3 = address.Assign (devices);
	 p2p3.EnableAsciiAll(ascii.CreateFileStream("mythird.tr"));
	 //p2p3.EnablePcap("thirdone");
	 p2p3.EnablePcap("Decompression",devices.Get(0), false);
	 p2p3.EnablePcap("UDPreceiver", devices.Get(1), false);
	 Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	 UdpServerHelper server (4000);
	 ApplicationContainer apps = server.Install (nodes.Get (3));
	 apps.Start (Seconds (1.0));
	 apps.Stop (Seconds (65.0));
	 //this is variable for changing entropy high- ture and low- false
	 uint32_t MaxPacketSize = 1100;

	 uint32_t maxPacketCount = 6000;
	 UdpClientHelper client (interface3.GetAddress(1), 4000);
	 client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
	 client.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
	 client.SetAttribute("SetEntropy", BooleanValue (false));
	 apps = client.Install (nodes.Get (0));
	 apps.Start (Seconds (2.0));
	 apps.Stop (Seconds (65.0));
	 Ptr<UdpServer> udpServer = server.GetServer();
	 UdpServerHelper server2 (6000);
	 apps = server2.Install (nodes.Get (3));
	 apps.Start (Seconds (67.0));
	 apps.Stop (Seconds (130.0));
     UdpClientHelper client2 (interface3.GetAddress(1), 6000);
	 client2.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
	 client2.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
	 client2.SetAttribute("SetEntropy", BooleanValue (true));
	 apps = client2.Install (nodes.Get (0));
	 apps.Start (Seconds (68.0));
	 apps.Stop (Seconds (130.0));
	 Ptr<UdpServer> udpServer2 = server2.GetServer();
//	 delay = udpServer->GetTimeDiff();
//	 cout << "Delay High Entropy: " << delay << endl;
//	 apps.Start (Seconds (11.0));
//	 apps.Stop (Seconds (20.0));
	 Simulator::Run ();
	 Simulator::Destroy ();
	 Time delay = udpServer->GetTimeDiff();
	 cout << "Delay Low Entropy: " << delay << endl;
	 Time delay2 = udpServer2->GetTimeDiff();
	 cout << "Delay High Entropy: " << delay2 << endl;
	 Time deltaLH = delay2 - delay;
	 if(deltaLH > 100) {
		 cout << "Compression detected!" << endl;
	 }
	 else {
		 cout << "No compression was detected." << endl;
	 }
	 return 0;
 }
