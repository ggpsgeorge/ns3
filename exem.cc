#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

int main(int argc, char *argv[]){

	CommandLine cmd;
  	cmd.Parse (argc, argv);
  
  	Time::SetResolution (Time::NS);
  	LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  	LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  	NodeContainer nodes;
  	nodes.Create(2);

  	PointToPointHelper pointToPoint;
  	pointToPoint.SetDeviceAttribute("DataRate", StringValue ("10Mbps"));
  	pointToPoint.SetChannelAttribute("Delay", StringValue ("1ms"));

  	NetDeviceContainer devices;
  	devices = pointToPoint.Install(nodes);

  	InternetStackHelper stack;
  	stack.Install(nodes);

  	Ipv4AddressHelper adress;
  	adress.SetBase("10.1.1.0","255.255.255.0");

  	Ipv4InterfaceContainer interfaces = adress.Assign(devices);

  	UdpEchoServerHelper echoServer(9);

  	ApplicationContainer serverApps = echoServer.Install(nodes.Get(2));
  	serverApps.Start(Seconds(1));
  	serverApps.Stop(Seconds(10));

  	

  	Simulator::Run();
  	Simulator::Destroy();

  	return 0;

}