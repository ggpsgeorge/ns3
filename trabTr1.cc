#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"

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

	CsmaHelper csma;
	csma.SetChannelAttribute("DataRate", StringValue("5Mbps"));
	csma.SetChannelAttribute("Delay", StringValue("2ms"));

	CsmaStarHelper star(nNodes, csma);

	//Completar cada LAN com mais nos
	NodeContainer fillNodes;

	NetDeviceContainer fillDevices;

	uint32_t fNodes = 10;

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

	for(uint32_t i = 0; i < star.SpokeCount(); ++i){
		std::ostringstream subnet;
	  	subnet << "10.1." << i << ".0";
	    
	    address.SetBase (subnet.str ().c_str (), "255.255.255.0", "0.0.0.3");
	    
	    for (uint32_t j = 0; j < fNodes; ++j){
			address.Assign (fillDevices.Get (i * fNodes + j));
		}
	}

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

    Simulator::Run ();
    Simulator::Destroy ();


	return 0;

}