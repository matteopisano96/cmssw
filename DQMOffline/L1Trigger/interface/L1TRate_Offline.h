#ifndef DQMOFFLINE_L1TRIGGER_L1TRATE_OFFLINE_H
#define DQMOFFLINE_L1TRIGGER_L1TRATE_OFFLINE_H

// system include files
#include <memory>
#include <unistd.h>

// user include files
#include "DQMServices/Core/interface/DQMOneEDAnalyzer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DQMServices/Core/interface/DQMStore.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

//DataFormats
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutRecord.h"
#include "DataFormats/Scalers/interface/LumiScalers.h"
#include "DataFormats/Scalers/interface/Level1TriggerRates.h"
#include "DataFormats/Scalers/interface/Level1TriggerScalers.h"
#include "DataFormats/Common/interface/ConditionsInEdm.h"  // Parameters associated to Run, LS and Event
#include "DataFormats/Luminosity/interface/LumiDetails.h"  // Luminosity Information
#include "DataFormats/Luminosity/interface/LumiSummary.h"  // Luminosity Information

#include "L1Trigger/GlobalTriggerAnalyzer/interface/L1GtUtils.h"

#include "DQM/L1TMonitor/interface/L1TMenuHelper.h"

#include <TString.h>

#include <iostream>
#include <fstream>
#include <vector>

//
// class declaration
//

class L1TRate_Offline : public DQMOneEDAnalyzer<edm::one::WatchLuminosityBlocks> {
public:
  enum Errors { UNKNOWN = 1, WARNING_PY_MISSING_FIT = 2 };

public:
  L1TRate_Offline(const edm::ParameterSet& ps);  // Constructor
  ~L1TRate_Offline() override;                   // Destructor

protected:
  void analyze(const edm::Event& e, const edm::EventSetup& c) override;  // Analyze
  void bookHistograms(DQMStore::IBooker& ibooker, const edm::Run& run, const edm::EventSetup& iSetup) override;

  void beginLuminosityBlock(edm::LuminosityBlock const& lumiBlock, edm::EventSetup const& c) override;
  void endLuminosityBlock(edm::LuminosityBlock const& lumiBlock, edm::EventSetup const& c) override;

  // Private methods
  //private:

  //    bool getXSexFitsOMDS  (const edm::ParameterSet& ps);
  bool getXSexFitsPython(const edm::ParameterSet& ps);

  // Private variables
private:
  // bool
  bool m_verbose;

  // int
  int m_refPrescaleSet;  // What is the reference prescale index to use for trigger choice
  int m_maxNbins;        // Maximum number of bins for MonitorElement
  int m_lsShiftGTRates;  // What shift (if any) to be applied to GT Rates LS number

  // string
  std::string m_outputFile;  // External output file name (for testiting)

  // Vectors
  const std::vector<std::vector<int> >* m_listsPrescaleFactors;  // Collection os all sets of prescales

  // Maps
  std::map<int, double> m_lsDeadTime;                           // Map of dead time for each LS
  std::map<int, int> m_lsPrescaleIndex;                         // Map of precale index for each LS
  std::map<int, double> m_lsLuminosity;                         // Map of luminosity recorded for each LS
  std::map<int, std::map<TString, double> > m_lsRates;          // Map of rates (by bit) recorded for each LS
  std::map<TString, int> m_algoBit;                             // Map of bit associated with a L1 Algo alias
  std::map<TString, TF1*> m_algoFit;                            // Map of bit associated with a L1 Algo alias
  std::map<std::string, bool> m_inputCategories;                // Map of categories to monitor
  std::map<std::string, std::string> m_selectedTriggers;        // Map of what trigger to monitor for each category
  std::map<TString, MonitorElement*> m_xSecObservedToExpected;  // Monitor Elements for Observed to Expected Algo XSec
  std::map<TString, MonitorElement*> m_xSecVsInstLumi;          // Monitor Elements for Algo XSec vs Instant Luminosity

  std::map<TString, MonitorElement*> m_xSecObservedVsDelivLumi;
  std::map<TString, MonitorElement*> m_xSecObservedVsRecorLumi;

  std::map<TString, MonitorElement*> m_CountsVsLS;      // Monitor Elements for counts
  std::map<TString, MonitorElement*> m_InstLumiVsLS;    // Monitor Elements for Instant Lumi
  std::map<TString, MonitorElement*> m_PrescIndexVsLS;  // Monitor Elements for Prescale Index
  //  std::map<TString,MonitorElement*>       m_DeadTimeVsLS;         // Monitor Elements (Check Purpose)

  std::map<TString, MonitorElement*> m_xSecObservedVsLS;
  std::map<TString, MonitorElement*> m_DelivLumiVsLS;
  std::map<TString, MonitorElement*> m_RecorLumiVsLS;

  std::map<TString, TF1*> m_templateFunctions;  // For each trigger template f(InstLumi)=XSec

  std::map<int, std::map<TString, double> > m_lsCounts;  // Map of counts (by bit) recorded for each LS

  // Input tags
  edm::EDGetTokenT<LumiScalersCollection> m_scalersSource_LSCollection;             // Where to get L1 Scalers
  edm::EDGetTokenT<Level1TriggerScalersCollection> m_scalersSource_L1TSCollection;  // Where to get L1 Scalers
  edm::EDGetTokenT<L1GlobalTriggerReadoutRecord> m_l1GtDataDaqInputTag;             // Where to get L1 GT Data DAQ

  // ParameterSet
  edm::ParameterSet m_parameters;

  // MonitorElement
  MonitorElement* m_ErrorMonitor;

  const edm::ESGetToken<L1GtTriggerMenu, L1GtTriggerMenuRcd> m_menuToken;
  const edm::ESGetToken<L1GtPrescaleFactors, L1GtPrescaleFactorsAlgoTrigRcd> m_l1GtPfAlgoToken;
  L1TMenuHelper::Tokens m_helperTokens;
  L1GtUtils m_l1GtUtils;
};

#endif
