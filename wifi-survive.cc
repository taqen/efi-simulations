/*
 * wifi-survive.cc
 *
 *  Created on: Jul 23, 2016
 *      Author: kentux
 */

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/propagation-module.h"
#include "ns3/config-store-module.h"
#include "ns3/log.h"
#include "ns3/error-model.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiSurvive");



/**
 *
 * A model for the error rate based on manually set PER values,
 * related to simulating E-FI paper proposal.
 */
class PsrErrorRateModel : public ErrorRateModel
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::PsrErrorRateModel")
      .SetParent<ErrorRateModel> ()
      .SetGroupName ("Wifi")
      .AddConstructor<PsrErrorRateModel> ()
      .AddAttribute("rate", "Error Rate", DoubleValue(0.0),
		    MakeDoubleAccessor(&PsrErrorRateModel::GetRate,
				       &PsrErrorRateModel::SetRate),
		    MakeDoubleChecker<double>(0.0, 1.0))
    ;
    return tid;
  };

  PsrErrorRateModel ()
  : m_rate(0.0)
  {

  };

  double GetChunkSuccessRate (WifiMode mode, WifiTxVector txVector, double snr, uint32_t nbits) const
  {
//    return m_ranvar->GetValue () < m_rate? 0.0:1.0;
    return (1 - m_rate); //TODO too simple, maybe needs some testing
  };

  /**
   * \returns the error rate being applied by the model
   */
  double GetRate (void) const
  {
    return m_rate;
  };
  /**
   * \param rate the error rate to be used by the model
   */
  void SetRate (double rate)
  {
    m_rate = rate;
  };

  /**
   * \param ranvar A random variable distribution to generate random variables
   */
  void SetRandomVariable (Ptr<RandomVariableStream> ranvar)
  {
    m_ranvar = ranvar;
  };

private:
  double m_rate;
  Ptr<RandomVariableStream> m_ranvar;

};

class NodeSpec
{
public:
  NodeSpec ():m_type(STA), m_psr(1){}
  virtual ~NodeSpec(){}

  enum NodeType
  {
    AP = 0,
    RELAY = 1,
    STA = 2
  };

  void SetType (NodeType type)
  {
    m_type = type;
  }

  NodeType GetType (void)
  {
    return m_type;
  }

  void SetPsr (double psr)
  {
    m_psr = psr;
  }

  double GetPsr (void)
  {
    return m_psr;
  }

  void SetPosition (Vector2D pos)
  {
    m_pos = pos;
  }

  Vector2D GetPosition (void)
  {
    return m_pos;
  }

private:
  NodeType m_type;
  double m_psr;
  Vector2D m_pos;
};

Ptr<WifiPhy>
GetWifiPhyPtr (const NetDeviceContainer &nc)
{
  Ptr<WifiNetDevice> wnd = nc.Get (0)->GetObject<WifiNetDevice> ();
  Ptr<WifiPhy> wp = wnd->GetPhy ();
  return wp;
}

void
PrintAttributesIfEnabled (bool enabled)
{
  if (enabled)
    {
      ConfigStore outputConfig;
      outputConfig.ConfigureAttributes ();
    }
}

//Generate a connected cluster based on the Nodes descriptions, cluster[0] is always the group owner
NetDeviceContainer
CreateCluster (std::vector<NodeSpec> clusterNodes, Ptr<SpectrumChannel> wifiChannel)
{
  NodeContainer cluster;
  NetDeviceContainer clusterDevices;

  MobilityHelper mobility; // TODO Specify Mobility

  for(std::vector<NodeSpec>::iterator it = clusterNodes.begin(); it != clusterNodes.end(); it++)
    {
      Ptr<Node> node = Create<Node> ();
      cluster.Add(node);
      mobility.Install(node);
    }

  WifiHelper wifi;
  wifi.SetStandard(WIFI_PHY_STANDARD_80211n_5GHZ);
  wifi.SetRemoteStationManager ("ns3::IdealWifiManager");

  SpectrumWifiPhyHelper wifiPhyHelper = SpectrumWifiPhyHelper::Default ();
  wifiPhyHelper.Set ("RxGain", DoubleValue (0.0));
  wifiPhyHelper.Set ("RxNoiseFigure", DoubleValue (0.0));
  wifiPhyHelper.Set ("EnergyDetectionThreshold", DoubleValue (-110.0));
  wifiPhyHelper.Set ("CcaMode1Threshold", DoubleValue (-110.0));
  wifiPhyHelper.SetChannel (wifiChannel);


  std::stringstream ss;
  Ssid ssid;
  ss << "RelayAP" << cluster.Get(0)->GetId();
  ssid = Ssid (ss.str());

  WifiMacHelper mac;
  mac.SetType ("ns3::ApWifiMac",
                 "QosSupported", BooleanValue (true),
                 "Ssid", SsidValue (ssid),
                 "BeaconGeneration", BooleanValue (true));

  NetDeviceContainer goDevice = wifi.Install (wifiPhyHelper, mac, cluster.Get(0));
  clusterDevices.Add(goDevice);

  for (uint32_t i = 1; i < cluster.GetN(); i++)
    {
      mac.SetType ("ns3::StaWifiMac",
                   "QosSupported", BooleanValue (true),
                   "Ssid", SsidValue (ssid));

      wifiPhyHelper.SetErrorRateModel("ns3::PsrErrorRateModel", "rate", DoubleValue(1.0 - clusterNodes[i].GetPsr()));
      NetDeviceContainer staDevice = wifi.Install (wifiPhyHelper, mac, cluster.Get(i));
      clusterDevices.Add(staDevice);
    }

  return clusterDevices;
}

//Connect a cluster Group Owner to AP
//TODO: Note here that WifiPhy uses the default NistErrorRateModel,
// This should be checked if it fits the scenario.
void
ConnectGoAp(Ptr<Node> goNode, Ptr<SpectrumChannel> wifiChannel )
{
  WifiHelper wifi;
  wifi.SetStandard(WIFI_PHY_STANDARD_80211n_5GHZ);
  wifi.SetRemoteStationManager ("ns3::IdealWifiManager");

  SpectrumWifiPhyHelper wifiPhy = SpectrumWifiPhyHelper::Default ();
  wifiPhy.Set ("RxGain", DoubleValue (0.0));
  wifiPhy.Set ("RxNoiseFigure", DoubleValue (0.0));
  wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue (-110.0));
  wifiPhy.Set ("CcaMode1Threshold", DoubleValue (-110.0));
  wifiPhy.SetChannel (wifiChannel);

  WifiMacHelper mac;
  Ssid ssid = Ssid("MasterAP");
  mac.SetType ("ns3::StaWifiMac",
                 "QosSupported", BooleanValue (true),
                 "Ssid", SsidValue (ssid));
}

int main (int argc, char *argv[])
{
  CommandLine cmd;

  double psrTh = 0.8;

  cmd.AddValue("psrTh","PSR Threshold that defines the zones", psrTh);

  Ptr<Node> masterAp = Create<Node> ();
  NodeContainer AllNodes;

  SpectrumChannelHelper channelHelper;
  channelHelper.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  channelHelper.AddPropagationLoss("ns3::FixedRssLossModel");
  channelHelper.SetChannel("ns3::MultiModelSpectrumChannel");


  Ptr<SpectrumChannel> wifiChannel = channelHelper.Create();

  WifiHelper wifi;
  wifi.SetStandard(WIFI_PHY_STANDARD_80211n_5GHZ);
  wifi.SetRemoteStationManager ("ns3::IdealWifiManager");

  SpectrumWifiPhyHelper wifiPhy = SpectrumWifiPhyHelper::Default(); // Note: This will set a default NiistErrorRateModel
  wifiPhy.Set ("RxGain", DoubleValue (0.0));
  wifiPhy.Set ("RxNoiseFigure", DoubleValue (0.0));
  wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue (-110.0));
  wifiPhy.Set ("CcaMode1Threshold", DoubleValue (-110.0));
  wifiPhy.SetChannel (wifiChannel);

  Ssid ssid = Ssid ("MasterAP");
  WifiMacHelper mac;
  mac.SetType ("ns3::ApWifiMac",
                 "QosSupported", BooleanValue (true),
                 "Ssid", SsidValue (ssid),
                 "BeaconGeneration", BooleanValue (true));

  NetDeviceContainer masterApDevice = wifi.Install (wifiPhy, mac, masterAp);

  std::vector<NodeSpec> clusterNodes;
  NodeSpec ns1;
  ns1.SetPosition(Vector2D(0,0));
  ns1.SetPsr(1);
  ns1.SetType(NodeSpec::RELAY);

  NodeSpec ns2;
  ns2.SetPosition(Vector2D(20,0));
  ns2.SetPsr(1);
  ns2.SetType(NodeSpec::STA);

  NodeSpec ns3;
  ns3.SetPosition(Vector2D(0,20));
  ns3.SetPsr(1);
  ns3.SetType(NodeSpec::STA);

  clusterNodes.push_back(ns1);
  clusterNodes.push_back(ns2);
  clusterNodes.push_back(ns3);

  NetDeviceContainer clusterDevices = CreateCluster(clusterNodes, wifiChannel);
  ConnectGoAp(clusterDevices.Get(0)->GetNode(), wifiChannel);

//  PointerValue ptr;
//  Ptr<EdcaTxopN> edca;
//  Ptr<WifiNetDevice> wifiDev = DynamicCast<WifiNetDevice>(clusterDevices.Get(0));
//  Ptr<WifiMac> wifiMac = wifiDev->GetMac();
//  wifiMac->GetAttribute("BE_EdcaTxopN", ptr);
//  edca = ptr.Get<EdcaTxopN>();
////  edca->SetManager();
}



