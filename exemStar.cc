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

int main(int argc, char *argv[]){

uint32_t nNodes = 5

PointToPointHelper p2pnode;
p2pnode.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
p2pnode.SetChannelAttribute("Delay", StringValue("2ms"));

//Criando a a topologia
PointToPointStarHelper star (nNodes, p2pnode);

//Instalando bagulhos em cada no
InternetStackHelper internet;
star.InstallStack(internet);

star.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"));

//Criar o hub para a topologia
uint32_t porta = 8000;
Address hubLocalAdress (InternetSocketAdress (Ipv4Address::GetAny(), porta));
PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", hubLocalAdress);
ApplicationContainer hupApp = packetSinkHelper.Install(star.GetHub);

hubApp.Start(Seconds(1.0); 
hubApp.Stop(Seconds(10.0));

ApplicationContainer nodesApp;

for(uint32_t i = 0; i < star.SpokeCount(), ++i){
	AddressValue remoteAddress (InetSocketAdress (star.GetHubIpv4Address(i), porta))
}

nodesApp.Start(Seconds(1.0));
nodesApp.Stop(Seconds(10.0));

//Rotear entre os nos da estrela
Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	return 0;
}