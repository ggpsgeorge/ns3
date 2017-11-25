#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"


/*Topologia Estrela
			 	n2
	   p2p	   /
n4	---	---	n0 - n1
			   \
			    n3
n4(server) fora da topologia

*/

NS_LOG_COMPONENT_DEFINE("exemStar");
using namespace ns3;

int main(int argc, char *argv[]){

	uint32_t nNodes = 10;

	Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (250));
	Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("5kb/s"));

	NodeContainer servNode;
	NodeContainer clientNodes;

	//###############################################################################
	NS_LOG_INFO("Contruir Star");

	PointToPointHelper p2p;
	p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
	p2p.SetChannelAttribute("Delay", StringValue("2ms"));
	PointToPointStarHelper star(nNodes, p2p);

	//###############################################################################
	NS_LOG_INFO("Instalar internet stack em todos os nNodes");

	InternetStackHelper internet;
	star.InstallStack(internet);

	//###############################################################################
	NS_LOG_INFO("Delega Ipv4");

	star.AssignIpv4Addresses (Ipv4AddressHelper("10.1.1.0", "255.255.255.0"));

	//###############################################################################
	NS_LOG_INFO("Cria Apps");

	uint16_t port = 8000;
	//Hub ira pegar para ele qualquer endereco Ipv4
	Address hubLocalAddress (InetSocketAddress(Ipv4Address::GetAny(), port));
	PacketSinkHelper packetSinker ("ns3::TcpSocketFactory", hubLocalAddress);
	ApplicationContainer hubApp = packetSinker.Install(star.GetHub());
	hubApp.Start(Seconds(1.0));
	hubApp.Stop(Seconds(10.0));

	//###############################################################################

	OnOffHelper onOff("ns3::TcpSocketFactory", Address());
	onOff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
	onOff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

	ApplicationContainer servApps;

	for (uint32_t i = 0; i < star.SpokeCount (); ++i){
        AddressValue remoteAddress (InetSocketAddress (star.GetHubIpv4Address (i), port));
 	    onOff.SetAttribute ("Remote", remoteAddress);
  		servApps.Add (onOff.Install (star.GetSpokeNode (i)));
    }

  	servApps.Start (Seconds (1.0));
  	servApps.Stop (Seconds (10.0));
	
	//###############################################################################	
	NS_LOG_INFO ("Static Routing Star");
  
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    NS_LOG_INFO ("Enable pcap tracing");
 
    p2p.EnablePcapAll ("star");
 
    NS_LOG_INFO ("Run Simulation");

    Simulator::Run ();
    Simulator::Destroy ();

    NS_LOG_INFO ("Done.");








	

	return 0;
}