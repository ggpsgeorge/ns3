/************************************************************************************************
    UnB - Universidade de Brasília
    ------------------------------
    Departamento de Ciência da Computação
    Teleinformática e Redes I - Turma A - 2/2016
    Profa. Priscila Solís Barreto 
    Grupo: Gabriel Ferreira Silva – 14/0140131
           Paulo Victor Gonçalves Farias - 13/0144754
           Yuri Ferreira Gomes 12/0043998
 --------------------------------------------------------------------------------------------------------
    Trabalho 2
    ----------
    Objetivo: Implementação de Redes Locais no NS-3 e Análise de Tráfego
    Conteúdo do arquivo: Trabalho2.zip
      > trabalho2.cc
      > relatorioImplementacao.pdf
      > relatorioAnaliseTrafego.pdf (LaTeX)
      > Exemplo:
        > Arquivos de trace (pcap)
        > Imagens do funcionamento
 --------------------------------------------------------------------------------------------------------
    trabalho2.cc
    -------------------------------
    Padrão: O programa, escrito em c++, está em conformidade e usa bibliotecas disponíveis pelo ns3.
    Saída: Simulação de uma rede Ethernet 802.3 se comunicando com clientes de redes Wi-Fi(802.11x).
    Abordagem do problema: Primeiro se cria uma rede Ethernet com 10 clientes e outras duas redes Wi-Fi.
    Com isso é possível criar pontos de acesso nas rede Wi-Fi e estabelecer conexões ponto-a-ponto entre
    a rede Ethernet com os clientes da rede Wi-Fi, ou seja, um servidor. 
    Limitações: A interface gráfica desta versão do ns3 não foi desenvolvida portanto não foi incluida.
    Como compilar e executar: INSIRA ESTE ARQUIVO NA PASTA SCRATCH DO NS3!!!!!!!!!!!!!!
      cd ns-allinone-3.26/ns-3.26 
      ./waf --run scratch/trabalho2 
    Versão do NS3: 3.26 (stable)
    Foram feitos testes no sistema Linux Mint(64 bits), sendo que a versão entregue compilou nos sistema 
    sem warnings e foi executada como esperado.
********************************************************************************************************/

// Bibliotecas usadas 
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

// Topologia da rede Construída em ASCII Art
//                                                  |
//                 Rank 0                           |   Rank 1
// -------------------------------------------------|----------------------------
//
//   Wifi 10.1.4.0 (Wi-Fi 802.11x)
//                                              AP
//  *    *    *    *    *    *    *    *    *    *     
//  |    |    |    |    |    |    |    |    |    |          10.1.1.0
//  n11  n12 n13  n14  n15  n16  n17  n18  n19  n20-------------------------x
//                                                       point-to-point     |
//                                                                  x-----n10   n9  n8    n7  n6  n5  n4   n3   n2   n1
//                                                                  |       |   |    |    |    |   |   |    |    |    |
//                                                                  |     ==============================================
//                                                                  |               LAN 10.1.3.0 (Ethernet 802.3)
//                                                                  |
//   Wifi 10.1.5.0 (Wi-Fi 802.11x)                                  |
//                                              AP                  |
//  *    *    *    *    *    *    *    *    *    *                  |
//  |    |    |    |    |    |    |    |    |    |      10.1.2.0    |
// n21  n22 n23   n24  n25  n26  n27  n28  n29  n30-----------------x
//                                                   point-to-point 

using namespace ns3;

// Definindo o nome do arquivo de log
NS_LOG_COMPONENT_DEFINE ("Trabalho2");

// x --------------------------- Início do programa -------------------------------- x 
int main (){

  // Habilitando o logging
  bool verbose = true;

  // Definindo número de clientes em cada rede
  uint32_t nCsma = 9; // 1 extra será o nosso servidor
  uint32_t nWifi = 10; 
  uint32_t nWifi2 = 10; 

  // Habilitando logs
  if (verbose){
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  }

  // Cria dois nós p2p para cada rede Wi-Fi 
  NodeContainer p2pNodes, p2pNodes2;
  p2pNodes.Create(2);  // Wi-Fi 1 -------- Ethernet
  p2pNodes2.Create(2); // Wi-Fi 2 -------- Ethernet

  // Definindo características da conexão p2p
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  // Instalação dos nodes p2p (n10(WIFI1) --- n10(Ethernet),  n10(WIFI2)---- n10(Ethernet))
  NetDeviceContainer p2pDevices, p2pDevices2;
  p2pDevices = pointToPoint.Install (p2pNodes);
  p2pDevices2 = pointToPoint.Install (p2pNodes2);

  // Criando os nodes LAN 
  NodeContainer csmaNodes;
  csmaNodes.Add(p2pNodes.Get(0));
  csmaNodes.Add(p2pNodes2.Get(0));
  csmaNodes.Create(nCsma);

  // Definindo características da rede LAN
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  // Instalando as novas configurações aos nodes
  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install(csmaNodes);

  // Agora criaremos os nós Wifi
  NodeContainer wifiStaNodes, wifiStaNodes2;
  // WIFI 1 
  wifiStaNodes.Create(nWifi);
  // WIFI 2
  wifiStaNodes2.Create(nWifi2);

  // AP da rede 1 e o da rede 2
  // WIFI 1
  NodeContainer wifiApNodes = p2pNodes.Get(1);
  // WIFI 2
  NodeContainer wifiApNodes2 = p2pNodes2.Get(1);

  // Criando o mesmo canal de Wifi para todos os nós da mesma rede
  // WIFI1
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  // WIFI2 
  YansWifiPhyHelper phy2 = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());
  phy2.SetChannel (channel2.Create ());

  // Utilizando non-Qos MAC
  WifiHelper wifi, wifi2;

  // Esse metodo determina o typo de algoritmo utilizado para controle. Algoritmo AARF
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
  wifi2.SetRemoteStationManager ("ns3::AarfWifiManager");

  // Setando MAC
  WifiMacHelper mac, mac2;
  // Aqui é criado o modelo 802.11. Cria-se um objeto ns-3-ssid. 
  Ssid ssid = Ssid ("ns-3-ssid");
  // WIFI 1 - MAC
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));
  // WIFI 2 - MAC
  mac2.SetType ("ns3::StaWifiMac",
             "Ssid", SsidValue (ssid),
             "ActiveProbing", BooleanValue (false));

  // Aqui ele termina de criar os nodes wifi, ou seja,
  // estabelece a conexão entre os nós da mesma rede.
  NetDeviceContainer staDevices, staDevices2;
  // WIFI 1
  staDevices = wifi.Install (phy, mac, wifiStaNodes);
  // WIFI 2
  staDevices2 = wifi2.Install (phy2, mac2, wifiStaNodes2);

  // Nodes criados, aqui é iniciada a configuração do AP (Access Point).
  // WIFI 1
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));
  // WIFI 2
  mac2.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  // Criando app devices e instalando as redes WIFI
  NetDeviceContainer apDevices, apDevices2;
  // WIFI 1
  apDevices = wifi.Install (phy, mac, wifiApNodes);
  // WIFI 2
  apDevices2 = wifi.Install (phy2, mac2, wifiApNodes2);

   // Objeto que vai dar "mobilidade" aos nós que não são APs.
  MobilityHelper mobility, mobility2;

  // Espalhando os nodes pelo grid. 
  // WIFI 1
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
  // WIFI 2 
  mobility2.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));


  // Movimentos aleatorios dos nodes.
    // WIFI 1
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  // WIFI 2
  mobility2.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));

  // Instalação desse modelo nos nós wifi.  
  // WIFI 1                          
  mobility.Install (wifiStaNodes);
  // WIFI 2                    
  mobility2.Install (wifiStaNodes2);

  // Aqui ele deixa o AP fixo no grid, sem se movimentar.
  // WIFI 1
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNodes);
  // WIFI 2
  mobility2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility2.Install (wifiApNodes2);

  // Criando protocolos e inicializando a rede
  InternetStackHelper stack;
  Ipv4AddressHelper address;
  Ipv4InterfaceContainer p2pInterfaces, p2pInterfaces2;
  Ipv4InterfaceContainer csmaInterfaces, csmaInterfaces2;

  stack.Install (csmaNodes);
  stack.Install (wifiApNodes);
  stack.Install (wifiStaNodes);
  stack.Install (wifiApNodes2);
  stack.Install (wifiStaNodes2);


  // Designa a rede 10.1.1.0 e 10.1.2.0 para os nós p2p 
  address.SetBase ("10.1.1.0", "255.255.255.0");
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  p2pInterfaces2 = address.Assign (p2pDevices2);

  // Designa a rede 10.1.3.0 para os nós CSMA
  address.SetBase ("10.1.3.0", "255.255.255.0");
  csmaInterfaces = address.Assign (csmaDevices);
   csmaInterfaces2 = address.Assign (csmaDevices);
 
  // Designa a rede 10.1.4.0 para os dispositivos wifi e AP
  address.SetBase ("10.1.4.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);

  // Designa a rede 10.1.5.0 para os dispositivos wifi2 e AP
  address.SetBase ("10.1.5.0", "255.255.255.0");
  address.Assign (staDevices2);
  address.Assign (apDevices2);
      
  
  // Escuta nas portas 21 e 31
  UdpEchoServerHelper echoServer (21);
  UdpEchoServerHelper echoServer2 (31);

  // Inicializando ethernet
  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma));
  serverApps.Start (Seconds (0.0));
  serverApps.Stop (Seconds (30.0));
  
  ApplicationContainer serverApps2 = echoServer2.Install (csmaNodes.Get (nCsma));
  serverApps2.Start (Seconds (0.0));
  serverApps2.Stop (Seconds (30.0));
  
  // Sempre utiliza as portas correspondentes para se comunicar com Ethernet
  // WIFI 1
  UdpEchoClientHelper  echoClient (csmaInterfaces.GetAddress(nCsma), 21);
  // WIFI 2
  UdpEchoClientHelper echoClient2 (csmaInterfaces2.GetAddress (nCsma), 31);

  // Setando atributos da transferencia para os clientes de cada rede WIFI
  // WIFI 1
  echoClient.SetAttribute ("MaxPackets", UintegerValue (24));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
  // WIFI 2
  echoClient2.SetAttribute ("MaxPackets", UintegerValue (30));
  echoClient2.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient2.SetAttribute ("PacketSize", UintegerValue (256));

  // O echo client aponta para o server da rede CSMA
  // WIFI 1
  ApplicationContainer clientApps = 
   echoClient.Install (wifiStaNodes.Get (nWifi - 1));
  // WIFI 2
  ApplicationContainer clientApps2 = 
   echoClient2.Install (wifiStaNodes2.Get (nWifi2 - 1));

  // Tempo da simulação de cada cliente
  // WIFI 1
  clientApps.Start (Seconds (0.0));
  clientApps.Stop (Seconds (30.0));
  // WIFI 2 
  clientApps2.Start (Seconds (0.0));
  clientApps2.Stop (Seconds (30.0));

  // Enable internetworking. 
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // A simulação parar em 30s.
  Simulator::Stop (Seconds (30.0));

  // Permite visualizar o trafego. 
  pointToPoint.EnablePcapAll ("p2p");
  phy.EnablePcap ("wifi1", apDevices.Get(0));
  phy2.EnablePcap ("wifi2", apDevices2.Get (0));
  csma.EnablePcap ("ethernet", csmaDevices.Get (0), true);

  // Roda e encerra
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

// x --------------------------- Fim do programa -------------------------------- x 
