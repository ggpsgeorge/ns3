#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/yans-wifi-helper.h"

#include "ns3/csma-star-helper.h"
#include "ns3/node-list.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/vector.h"
#include "ns3/log.h"

#include <iostream>
#include <sstream>

// Topologia
//
//            n2 	 +          +     n3          .
//             | ... |\        /| ... |           .
//             ======= \      / =======           .
//              CSMA    \    /   CSMA             .
//                       \  /                     .
//            n1     +--- n0 ---+     n4
//						(Hub)          
//             | ... |          | ... |           .
//             =======          =======           .
//              CSMA         	  CSMA             .                 .
//
// Onde cada nodo n1, ..., n4 possui 10 nodos extras

NS_LOG_COMPONENT_DEFINE("trabTr1");

using namespace ns3;

int main(int argc, char *argv[]){

	Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(137));
	Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("14kb/s"));

	uint32_t nNodes = 5;
	uint32_t nWifi1 = 10;
	uint32_t nWifi2 = 10;


	CsmaHelper csma;
	csma.SetChannelAttribute("DataRate", StringValue("5Mbps"));
	csma.SetChannelAttribute("Delay", StringValue("2ms"));

	CsmaStarHelper star(nNodes, csma);

	NodeContainer fillNodes;

	NetDeviceContainer fillDevices;

	uint32_t fNodes = 11;

	//Criando nos da LAN e preenchendo
	for(uint32_t i = 0; i < star.GetSpokeDevices().GetN();++i){
		Ptr<Channel> channel = star.GetSpokeDevices().Get(i)->GetChannel();
		Ptr<CsmaChannel> csmaChannel = channel -> GetObject<CsmaChannel>();
		NodeContainer newNodes;
		newNodes.Create(fNodes);
		fillNodes.Add(newNodes);
		fillDevices.Add(csma.Install(newNodes, csmaChannel));
	}

	InternetStackHelper internet;
	star.InstallStack(internet);
	internet.Install(fillNodes);

	star.AssignIpv4Addresses (Ipv4AddressHelper("10.1.0.0", "255.255.255.0"));

	Ipv4AddressHelper address;

	//Dando a cada no um endereco
	for(uint32_t i = 0; i < star.SpokeCount(); ++i){
		std::ostringstream subnet;
	  	subnet << "10.1." << i << ".0";
	    
	    address.SetBase (subnet.str ().c_str (), "255.255.255.0", "0.0.0.3");
	    
	    for (uint32_t j = 0; j < fNodes; ++j){
			address.Assign (fillDevices.Get (i * fNodes + j));
		}
	}

	//Criar nos wifi

	NodeContainer p2pNodes1, p2pNodes2;
	p2pNodes1.Create(2); 
	p2pNodes2.Create(2); 

	
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
	pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

	NetDeviceContainer p2pDevices1, p2pDevices2;
	p2pDevices1 = pointToPoint.Install (p2pNodes1);
	p2pDevices2 = pointToPoint.Install (p2pNodes2);

	NodeContainer wifiNodes1, wifiNodes2;

	wifiNodes1.Create(nWifi1);
	wifiNodes2.Create(nWifi2);

	NodeContainer wifiApNodes1 = p2pNodes1.Get(1);
	NodeContainer wifiApNodes2 = p2pNodes2.Get(1);

	YansWifiChannelHelper channel1 = YansWifiChannelHelper::Default();
	YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default();

	YansWifiPhyHelper phy1 = YansWifiPhyHelper::Default();
	YansWifiPhyHelper phy2 = YansWifiPhyHelper::Default();

	phy1.SetChannel(channel1.Create());
	phy2.SetChannel(channel2.Create());

	WifiHelper wifi1, wifi2;

	wifi1.SetRemoteStationManager("ns3::AarfWifiManager");
	wifi2.SetRemoteStationManager("ns3::AarfWifiManager");

	WifiMacHelper mac1, mac2;

	Ssid ssid = Ssid("ns-3-ssid");

	mac1.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
	mac2.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));

	NetDeviceContainer staDevices1, staDevices2;
	staDevices1 = wifi1.Install(phy1, mac1, wifiNodes1);
	staDevices2 = wifi2.Install(phy2, mac2, wifiNodes2);

	mac1.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
	mac2.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));

	NetDeviceContainer appDevices1, appDevices2;
	appDevices1 = wifi1.Install(phy1, mac1, wifiApNodes1);
	appDevices2 = wifi1.Install(phy2, mac2, wifiApNodes2);

	MobilityHelper mobility1, mobility2;

	mobility1.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  	mobility2.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  	mobility1.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds", 
  								RectangleValue(Rectangle(-50,50,-50,50)));

  	mobility2.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds", 
  								RectangleValue(Rectangle(-50,50,-50,50)));

  	mobility1.Install(wifiNodes1);
  	mobility2.Install(wifiNodes2);

  	mobility1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  	mobility1.Install (wifiApNodes1);

  	mobility2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  	mobility2.Install (wifiApNodes2);

  	internet.Install(wifiNodes1);
  	internet.Install(wifiNodes2);
  	internet.Install(wifiApNodes1);
  	internet.Install(wifiApNodes2);

  	address.SetBase ("10.2.4.0", "255.255.255.0");
  	address.Assign (staDevices1);
  	address.Assign (appDevices1);

  
  	address.SetBase ("10.2.5.0", "255.255.255.0");
  	address.Assign (staDevices2);
  	address.Assign (appDevices2);

  	UdpEchoServerHelper echoServer(10);
  	ApplicationContainer serverApps = echoServer.Install(fillNodes.Get(fNodes));
  	serverApps.Start(Seconds(1.0));
  	serverApps.Stop(Seconds(10.0));

	uint16_t port = 8000;

	//Hub ira pegar para ele qualquer endereco Ipv4
	Address hubLocalAddress (InetSocketAddress(Ipv4Address::GetAny(), port));
	PacketSinkHelper packetSinker ("ns3::TcpSocketFactory", hubLocalAddress);
	ApplicationContainer hubApp = packetSinker.Install(star.GetHub());
	hubApp.Start(Seconds(1.0));
	hubApp.Stop(Seconds(10.0));

	
	OnOffHelper onOff("ns3::TcpSocketFactory", Address());
	onOff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
	onOff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

	ApplicationContainer spokeApps;

	for (uint32_t i = 0; i < star.SpokeCount (); ++i){
        AddressValue remoteAddress (InetSocketAddress (star.GetHubIpv4Address (i), port));
 	    onOff.SetAttribute ("Remote", remoteAddress);
  		spokeApps.Add (onOff.Install (star.GetSpokeNode (i)));
    }

  	spokeApps.Start (Seconds (1.0));
  	spokeApps.Stop (Seconds (10.0));

  	ApplicationContainer fillApps;

	for (uint32_t i = 0; i < fillNodes.GetN(); ++i){
        AddressValue remoteAddress (InetSocketAddress (star.GetHubIpv4Address (i/fNodes), port));
 	    onOff.SetAttribute ("Remote", remoteAddress);
  		fillApps.Add (onOff.Install (fillNodes.Get(i)));

    }

  	fillApps.Start (Seconds (1.0));
  	fillApps.Stop (Seconds (10.0));

  	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  	csma.EnablePcapAll("csma-star");
  	phy1.EnablePcapAll("wifi1");
  	phy2.EnablePcapAll("wifi2");
  	pointToPoint.EnablePcapAll("p2p");

    Simulator::Run ();
    Simulator::Stop(Seconds(10.0));
    Simulator::Destroy();


	return 0;

}