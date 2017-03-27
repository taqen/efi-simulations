#include "experiment.h"
#include "psr-error-model.h"
#include "ns3/mobility-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/applications-module.h"
#include "ns3/propagation-module.h"
#include "ns3/config-store-module.h"
#include "ns3/olsr-module.h"
#include "ns3/log.h"
#include "ns3/netanim-module.h"

using namespace ns3;

uint16_t port = 9;

NS_LOG_COMPONENT_DEFINE ("EfiExperiment");

Experiment::Experiment ()
{
	Initialize();
}

Experiment::~Experiment()
{

}

void
LogAssoc (Mac48Address addr)
{
	std::cout << "Associated with " << addr << std::endl;
}

void
Experiment::SetupNode(Ptr<Node> node, NodeSpec::NodeType type, double psr, uint32_t relayId, bool efiActive)
{
	WifiHelper wifiHelper;
	wifiHelper.SetStandard(WIFI_PHY_STANDARD_80211n_5GHZ);
	//TODO Should the FragmentationThreshold be set here or not down for the client nodes?
	wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager", "FragmentationThreshold", UintegerValue(600)); //TODO The actual value should be confirmed

	SpectrumWifiPhyHelper wifiPhyHelper;
	wifiPhyHelper = SpectrumWifiPhyHelper::Default ();
	//	  wifiPhyHelper.Set ("RxGain", DoubleValue (0.0));
	//	  wifiPhyHelper.Set ("RxNoiseFigure", DoubleValue (0.0));
	//	  wifiPhyHelper.Set ("EnergyDetectionThreshold", DoubleValue (-110.0));
	//	  wifiPhyHelper.Set ("CcaMode1Threshold", DoubleValue (-110.0));
	wifiPhyHelper.SetErrorRateModel("ns3::PsrErrorRateModel", "rate", DoubleValue(psr));
	wifiPhyHelper.SetChannel (m_channel);
	WifiMacHelper macHelper;


	InternetStackHelper internet;
	OlsrHelper olsr;
	Ipv4StaticRoutingHelper staticRouting;
	Ipv4ListRoutingHelper list;
	list.Add (staticRouting, 0);
	list.Add (olsr, 10);
	internet.SetRoutingHelper (list);

	internet.Install(node);

	if(!efiActive)
	{
		wifiPhyHelper.Set ("ChannelNumber", UintegerValue(48));
		std::stringstream ss;
		Ssid ssid;
		ss.str("");
		ss << "MasterAP" << 0;
		ssid = Ssid (ss.str());

		macHelper.SetType ("ns3::StaWifiMac",
				"Ssid", SsidValue (ssid),
				"BE_MaxAmpduSize", UintegerValue(0)); //Disable AMPDU (BE_MaxAmpduSize=0) to make sure Fragmentation Threshold is always used
		NetDeviceContainer device = wifiHelper.Install(wifiPhyHelper, macHelper, node);
		m_clusterDevices[0].Add(device);

		if(m_clusterIpAddress.count(0) == 0)
		{
			Ipv4AddressHelper addressHelper;

			addressHelper.SetBase(Ipv4Address("10.0.0.0"), Ipv4Mask("255.255.255.0"));
			addressHelper.Assign(device);
			m_clusterIpAddress[0] = addressHelper;
		}
		else
		{
			m_relayApIpAddress[0].Assign(device);
		}

		SetupReceivePacket(device.Get(0));
	}
	else switch(type)
	{
	case NodeSpec::STA_NORMAL:
	{
		wifiPhyHelper.Set ("ChannelNumber", UintegerValue(48));
		std::stringstream ss;
		Ssid ssid;
		ss.str("");
		ss << "MasterAP" << relayId;
		ssid = Ssid (ss.str());

		macHelper.SetType ("ns3::StaWifiMac",
				"Ssid", SsidValue (ssid),
				"BE_MaxAmpduSize", UintegerValue(0)); //Disable AMPDU (BE_MaxAmpduSize=0) to make sure Fragmentation Threshold is always used
		NetDeviceContainer device = wifiHelper.Install(wifiPhyHelper, macHelper, node);

		m_relayToApDevice.Add(device);


		if(m_relayApIpAddress.count(relayId) == 0)
		{
			ss.str("");
			ss << "10.0." << relayId << ".0";
			Ipv4AddressHelper addressHelper;

			addressHelper.SetBase(Ipv4Address(ss.str().c_str()), Ipv4Mask("255.255.255.0"));
			addressHelper.Assign(device);
			m_relayApIpAddress[relayId] = addressHelper;
		}
		else
		{
			m_relayApIpAddress[relayId].Assign(device);
		}

		SetupReceivePacket(device.Get(0));
		break;
	}

	case NodeSpec::RELAY:
	{
		wifiPhyHelper.Set ("ChannelNumber", UintegerValue(44));
		// Setting a the Relay as an access point to it's clulser's stations.
		std::stringstream ss;
		Ssid ssid;
		ss << "RelayAP" << node->GetId();
		ssid = Ssid (ss.str());

		macHelper.SetType ("ns3::ApWifiMac",
				"Ssid", SsidValue (ssid));

		NetDeviceContainer device = wifiHelper.Install (wifiPhyHelper, macHelper, node);
		m_relayClusterDevice.Add(device);


		if(m_clusterIpAddress.count(node->GetId()) == 0)
		{
			ss.str("");
			ss << "10.0." << node->GetId() << ".0";
			Ipv4AddressHelper addressHelper;

			addressHelper.SetBase(Ipv4Address(ss.str().c_str()), Ipv4Mask("255.255.255.0"));
			addressHelper.Assign(device);
			m_clusterIpAddress[node->GetId()] = addressHelper;
		}
		else
		{
			m_clusterIpAddress[node->GetId()].Assign(device);
		}

		SetupReceivePacket(device.Get(0));


		wifiPhyHelper.Set ("ChannelNumber", UintegerValue(48));
		ss.str("");
		ss << "MasterAP" << relayId;
		ssid = Ssid (ss.str());

		macHelper.SetType ("ns3::StaWifiMac",
				"Ssid", SsidValue (ssid),
				"BE_MaxAmpduSize", UintegerValue(0)); //Disable AMPDU (BE_MaxAmpduSize=0) to make sure Fragmentation Threshold is always used
		device = wifiHelper.Install(wifiPhyHelper, macHelper, node);

		m_relayToApDevice.Add(device);


		if(m_relayApIpAddress.count(relayId) == 0)
		{
			ss.str("");
			ss << "10.0." << relayId << ".0";
			Ipv4AddressHelper addressHelper;

			addressHelper.SetBase(Ipv4Address(ss.str().c_str()), Ipv4Mask("255.255.255.0"));
			addressHelper.Assign(device);
			m_relayApIpAddress[relayId] = addressHelper;
		}
		else
		{
			m_relayApIpAddress[relayId].Assign(device);
		}

		SetupReceivePacket(device.Get(0));

		break;
	}
	case NodeSpec::AP:
	{
		wifiPhyHelper.Set ("ChannelNumber", UintegerValue(48));
		std::stringstream ss;
		Ssid ssid;
		ss << "MasterAP" << node->GetId();
		ssid = Ssid (ss.str());

		macHelper.SetType ("ns3::ApWifiMac",
				"Ssid", SsidValue (ssid));
		NetDeviceContainer device = wifiHelper.Install(wifiPhyHelper, macHelper, node);
		m_apDevice.Add(device);

		//		  internet.Install(node);

		if(m_relayApIpAddress.count(node->GetId()) == 0)
		{
			ss.str("");
			ss << "10.0." << node->GetId() << ".0";
			Ipv4AddressHelper addressHelper;

			addressHelper.SetBase(Ipv4Address(ss.str().c_str()), Ipv4Mask("255.255.255.0"));
			addressHelper.Assign(device);
			m_relayApIpAddress[node->GetId()] = addressHelper;
		}
		else
		{
			m_relayApIpAddress[node->GetId()].Assign(device);
		}

		SetupReceivePacket(device.Get(0));

		break;
	}
	case NodeSpec::STA:
	{

		// Setting a the Relay as an access point to it's clulser's stations.
		std::stringstream ss;
		Ssid ssid;
		ss << "RelayAP" << relayId;
		ssid = Ssid (ss.str());

		macHelper.SetType ("ns3::StaWifiMac",
				"Ssid", SsidValue (ssid),
				"BE_MaxAmpduSize", UintegerValue(0)); //Disable AMPDU (BE_MaxAmpduSize=0) to make sure Fragmentation Threshold is always used
		NetDeviceContainer device = wifiHelper.Install(wifiPhyHelper, macHelper, node);
		m_clusterDevices[relayId].Add(device);

		//		   internet.Install(node);

		if(m_clusterIpAddress.count(relayId) == 0)
		{
			ss.str("");
			ss << "10.0." << relayId << ".0";
			Ipv4AddressHelper addressHelper;

			addressHelper.SetBase(Ipv4Address(ss.str().c_str()), Ipv4Mask("255.255.255.0"));
			addressHelper.Assign(device);
			m_clusterIpAddress[relayId] = addressHelper;
		}
		else
		{
			m_clusterIpAddress[relayId].Assign(device);
		}

		SetupReceivePacket(device.Get(0));

		break;
	}
	default: break;
	}

}

void
Experiment::CreateNodes (std::vector<NodeSpec> nodeSpecs, bool efiActive)
{

	for(uint32_t i = 0; i != nodeSpecs.size(); i++)
	{
		Ptr<Node> node = CreateObject<Node> ();
		Ptr<ConstantPositionMobilityModel> mobility = CreateObject<ConstantPositionMobilityModel> ();
		mobility->SetPosition(nodeSpecs[i].GetPosition());
		node->AggregateObject(mobility);

		m_allNodes.Add(node);
		if(nodeSpecs[i].GetType()==NodeSpec::RELAY)
		{
			m_relayNodes[nodeSpecs[i].GetRelayId()].Add(node);
			SetupNode(node, NodeSpec::RELAY, efiActive? nodeSpecs[i].GetPsr(2): nodeSpecs[i].GetPsr(0), nodeSpecs[i].GetRelayId(), efiActive);
		}
		else if(nodeSpecs[i].GetType()==NodeSpec::STA)
		{
			m_clusterNodes[nodeSpecs[i].GetRelayId()].Add(node);
			SetupNode(node, NodeSpec::STA, efiActive? nodeSpecs[i].GetPsr(2): nodeSpecs[i].GetPsr(0), nodeSpecs[i].GetRelayId(), efiActive);

		}
		else if(nodeSpecs[i].GetType()==NodeSpec::AP)
		{
			m_apNodes.Add(node);
			SetupNode(node, NodeSpec::AP, efiActive? nodeSpecs[i].GetPsr(2): nodeSpecs[i].GetPsr(0), nodeSpecs[i].GetRelayId(), efiActive);
		}
		else if(nodeSpecs[i].GetType()==NodeSpec::STA_NORMAL)
		{
			m_relayNodes[nodeSpecs[i].GetRelayId()].Add(node);
			SetupNode(node, NodeSpec::STA_NORMAL, efiActive? nodeSpecs[i].GetPsr(2): nodeSpecs[i].GetPsr(0), nodeSpecs[i].GetRelayId(), efiActive);
		}
	}

	for(std::map<uint32_t, NetDeviceContainer>::iterator it = m_clusterDevices.begin(); it != m_clusterDevices.end(); it++)
	{
		std::stringstream ss;
		ss.str("");
		ss << "10.0." << it->first << ".2";
	    InstallApplications(m_apDevice, Ipv4Address(ss.str().c_str()));
	}

	m_clusterIpAddress.clear();
	m_relayApIpAddress.clear();
}

void
Experiment::SetupReceivePacket (Ptr<NetDevice> device)
{
	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	Ptr<Ipv4L3Protocol> ipv4 = device->GetNode()->GetObject<Ipv4L3Protocol> ();
	Ipv4InterfaceAddress addr = ipv4->GetInterface(ipv4->GetInterfaceForDevice(device))->GetAddress(0);

	InetSocketAddress local = InetSocketAddress (addr.GetLocal(), port);

	Ptr <Socket> sink = Socket::CreateSocket (device->GetNode(), tid);

	sink->Bind (local);
	sink->BindToNetDevice(device);
	sink->SetRecvCallback (MakeCallback ( &Experiment::ReceivePacket, this));

	m_packetsTotal[device] = 0;
}

void LogBusy (Time start, Time duration, WifiPhy::State state)
{
	std::string stateString;
	switch(state)
	{
	case WifiPhy::CCA_BUSY:
		stateString = "CCA_BUSY";
		std::cout << stateString.c_str() << " since " << start.GetSeconds() << " s duration " << duration.GetSeconds() << " s" << std::endl;
		break;
	case WifiPhy::IDLE:
		stateString = "IDLE";
		break;
	case WifiPhy::SLEEP:
		stateString = "SLEEP";
		break;
	case WifiPhy::TX:
		stateString = "TX";
		break;
	case WifiPhy::RX:
		stateString = "RX";
		break;
	case WifiPhy::SWITCHING:
		stateString = "SWITCHING";
		break;
	default:
		NS_FATAL_ERROR("Impossible state");
	}
	//  std::cout << stateString.c_str() << " since " << start.GetSeconds() << " s duration " << duration.GetSeconds() << " s" << std::endl;
}

/**
 * Connect a cluster Group Owner (Relay) to AP
 * TODO: Note here that WifiPhy uses the default NistErrorRateModel,
 * This should be checked if it fits the scenario.
 */

void
Experiment::ReceivePacket (Ptr <Socket> socket)
{
	//  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << " Received one packet!");
	Ptr <Packet> packet;
	Address addr;
	socket->GetSockName (addr);
	InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (addr);
	Ptr<Ipv4L3Protocol> ipv4l3p = socket->GetNode()->GetObject<Ipv4L3Protocol>();
	Ptr<NetDevice> device = ipv4l3p->GetInterface(ipv4l3p->GetInterfaceForAddress(iaddr.GetIpv4()))->GetDevice();

	while (packet = socket->Recv ())
	{
		m_packetsTotal[device]++;
	}
}

//TODO This install an app sending traffic from every netdevice in src to every netdevice in dst
void
Experiment::InstallApplications (NetDeviceContainer src, NetDeviceContainer dst)
{

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();
	ApplicationContainer apps;

	for(NetDeviceContainer::Iterator it1 = src.Begin(); it1 != src.End(); it1++)
	{
	    for(NetDeviceContainer::Iterator it2 = dst.Begin(); it2 != dst.End(); it2++)
	      {
		Ptr<Ipv4L3Protocol> ipv4_dst = (*it2)->GetNode()->GetObject<Ipv4L3Protocol> ();
		Ipv4InterfaceAddress addr_dst = ipv4_dst->GetInterface(ipv4_dst->GetInterfaceForDevice(*it2))->GetAddress(0);

		Ptr<Ipv4L3Protocol> ipv4_src = (*it1)->GetNode()->GetObject<Ipv4L3Protocol> ();
		Ipv4InterfaceAddress addr_src = ipv4_src->GetInterface(ipv4_src->GetInterfaceForDevice(*it1))->GetAddress(0);

		OnOffHelper onoff ("ns3::UdpSocketFactory",
				Address (InetSocketAddress (addr_dst.GetLocal(), port))); // TODO Send traffic to AP?
		onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
		onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

		apps.Add(onoff.Install((*it1)->GetNode()));

		std::cout << "address " << addr_src.GetLocal() << " sending packets to address " << addr_dst.GetLocal() << std::endl;

	      }
	}

	apps.Start (Seconds (15));
	apps.Stop (Seconds (100));
}

void
Experiment::InstallApplications (NetDeviceContainer src, Ipv4Address address)
{

	Ipv4GlobalRoutingHelper::PopulateRoutingTables();
	ApplicationContainer apps;

	for(NetDeviceContainer::Iterator it1 = src.Begin(); it1 != src.End(); it1++)
	  {

	    Ptr<Ipv4L3Protocol> ipv4_src = (*it1)->GetNode()->GetObject<Ipv4L3Protocol> ();
	    Ipv4InterfaceAddress addr_src = ipv4_src->GetInterface(ipv4_src->GetInterfaceForDevice(*it1))->GetAddress(0);

	    OnOffHelper onoff ("ns3::UdpSocketFactory",
			       Address (InetSocketAddress (address, port))); // TODO Send traffic to AP?
	    onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
	    onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

	    apps.Add(onoff.Install((*it1)->GetNode()));

	    std::cout << "address " << addr_src.GetLocal() << " sending packets to address " << address << std::endl;

	  }

	apps.Start (Seconds (15));
	apps.Stop (Seconds (100));
}

void Experiment::Initialize ()
{

	/// Initialize Channel and PHY parameters

	SpectrumChannelHelper channelHelper;
	channelHelper.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	channelHelper.AddSpectrumPropagationLoss("ns3::FriisSpectrumPropagationLossModel");
	channelHelper.SetChannel("ns3::MultiModelSpectrumChannel");
	m_channel = channelHelper.Create();


}

//TODO fix return variables
NodeContainer Experiment::GetNodes(NodeSpec::NodeType type) const
{
  NodeContainer nodes;
  switch(type)
  {
    case NodeSpec::STA:
      //      nodes = m_clusterNodes;
      break;
    case NodeSpec::STA_NORMAL:
      break;
    case NodeSpec::AP:
      //      nodes = m_apNodes;
      break;
    case NodeSpec::RELAY:
      //      nodes = m_relayNodes;
      break;
    default:
      std::cout << "Wrong NodeType provided" << std::endl;
  }
  return nodes;
}

NetDeviceContainer Experiment::GetNetDevices(NodeSpec::NodeType type) const
{
  NetDeviceContainer devices;
  switch(type)
  {
    case NodeSpec::STA:
      //      devices = m_clusterDevices;
      break;
    case NodeSpec::STA_NORMAL:
      break;
    case NodeSpec::AP:
      //      devices = m_apDevice;
      break;
    case NodeSpec::RELAY:
      //      devices = NetDeviceContainer(m_relayClusterDevice, m_relayToApDevice);
      break;
    default:
      std::cout << "Wrong NodeType provided" << std::endl;
  }
  return devices;
}

std::map<Ptr<NetDevice>, uint64_t> Experiment::GetPacketsTotal ()
{
	return m_packetsTotal;
}

void
Experiment::Run(int argc, char *argv[])
{
	double simTime = 20.0;
	std::string animFile = "animation.xml";

	CommandLine cmd;
	cmd.AddValue("simTime","Simulation Time", simTime);
	cmd.AddValue("animFile","Animation xml filename", animFile);


	Simulator::Stop(Seconds(simTime));

	AnimationInterface anim (animFile.c_str());
	anim.EnablePacketMetadata (true);
	anim.SetMobilityPollInterval (Seconds (1));

	for(NodeList::Iterator it = NodeList::Begin(); it != NodeList::End(); it++)
	{
		Ptr<ConstantPositionMobilityModel> mobility = (*it)->GetObject<ConstantPositionMobilityModel>();
		Vector position = mobility->GetPosition();
		anim.SetConstantPosition(*it, position.x, position.y, position.z);
	}
	anim.EnableWifiPhyCounters(Seconds(0), Seconds(simTime), Seconds(1));
	anim.EnableWifiMacCounters(Seconds(0), Seconds(simTime), Seconds(1));
	anim.EnableQueueCounters(Seconds(0), Seconds(simTime), Seconds(1));

	Simulator::Run();


	std::map<Ptr<NetDevice>, uint64_t> packetsTotal = GetPacketsTotal();
	for(std::map<Ptr<NetDevice>, uint64_t>::iterator it = packetsTotal.begin(); it != packetsTotal.end(); it++)
	{
		std::cout <<
				" at Node " << it->first->GetNode()->GetId() <<
				" MAC type " << it->first->GetObject<WifiNetDevice>()->GetMac()->GetInstanceTypeId().GetName() <<
				" Ipv4 Address " << it->first->GetNode()->GetObject<Ipv4L3Protocol>()->
				GetInterface(it->first->GetNode()->GetObject<Ipv4L3Protocol>()->GetInterfaceForDevice(it->first->GetObject<WifiNetDevice>()))->GetAddress(0).GetLocal () <<
				" received " << it->second << " packets" << std::endl;
	}



	Simulator::Destroy();

	//  std::cout << experiment.GetPacketsTotal().size() << std::endl;

}
