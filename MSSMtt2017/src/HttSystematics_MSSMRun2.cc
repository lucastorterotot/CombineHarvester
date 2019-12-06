#include "CombineHarvester/MSSMtt2017/interface/HttSystematics_MSSMRun2.h"
#include "CombineHarvester/CombineTools/interface/Process.h"
#include "CombineHarvester/CombineTools/interface/Systematics.h"
#include "CombineHarvester/CombineTools/interface/Utilities.h"
#include <string>
#include <vector>

using namespace std;

namespace ch {

using ch::syst::SystMap;
using ch::syst::SystMapAsymm;
using ch::syst::era;
using ch::syst::channel;
using ch::syst::bin_id;
using ch::syst::process;
using ch::syst::bin;
using ch::JoinStr;

void AddMSSMRun2Systematics(CombineHarvester &cb, bool jetfakes, bool embedding, bool ggh_wg1, int era) {

  // ##########################################################################
  // Define groups of processes
  // ##########################################################################

  // Signal processes
  std::vector<std::string> signals = {"ggH","bbH"};

  // Background processes
  /* // Not used in the function, keep it for documentation purposes.
  std::vector<std::string> backgrounds = {"ZTT",  "W",   "ZL",      "ZJ",
                                          "TTT",  "TTJ", "VVT",     "VVJ",
                                          "EWKZ", "QCD", "jetFakes", "Embedded", "TTL"};
  */

  // All processes being taken from simulation
  // FIXME: Adapt for fake factor and embedding
  std::vector<std::string> mc_processes =
      JoinStr({
              signals,
              {"TTL", "ZL", "VVL"}
              });
  // ##########################################################################
  // Uncertainty: Lumi
  // References:
  // - "CMS Luminosity Measurements for the 2016 Data Taking Period"
  //   (PAS, https://cds.cern.ch/record/2257069)
  // Notes:
  // - FIXME: Adapt for fake factor and embedding
  // ##########################################################################

  float lumi_unc = 1.023;

  cb.cp()
      .channel({"tt"})
      .process(mc_processes)
      .AddSyst(cb, "lumi_", "lnN", SystMap<>::init(lumi_unc));

  // ##########################################################################
  // Uncertainty: Trigger efficiency
  // References:
  // Notes:
  // - FIXME: References?
  // ##########################################################################


  cb.cp()
      .channel({"tt"})
      .process(mc_processes)
      .AddSyst(cb, "CMS_eff_trigger_tt", "lnN", SystMap<>::init(1.10));

  cb.cp()
      .channel({"tt"})
      .process({"Embedded"})
    .AddSyst(cb, "CMS_eff_trigger_emb_tt", "lnN", SystMap<>::init(1.10));

  // ##########################################################################
  // Uncertainty: Electron, muon and tau ID efficiency
  // References:
  // Notes:
  // - FIXME: Adapt for fake factor and embedding
  // - FIXME: Handling of ZL in fully-hadronic channel?
  // - FIXME: References?
  // ##########################################################################

  float ditauID = 1.045; //1.06 with 50% uncorrelated/correlated between MC and embedding

  // MC uncorrelated uncertainty

  // Tau ID: tt with 2 real taus
  cb.cp()
      .channel({"tt"})
      .process(mc_processes)
      .AddSyst(cb, "CMS_eff_mc_t_", "lnN", SystMap<>::init(ditauID));


  // Embedded uncorrelated uncertainty

  // Tau ID: tt with 2 real taus
  cb.cp()
      .channel({"tt"})
      .process({"Embedded"})
      .AddSyst(cb, "CMS_eff_emb_t_", "lnN", SystMap<>::init(ditauID));



  // MC + embedded correlated uncertainty

  // Tau ID: tt with 2 real taus
  cb.cp()
      .channel({"tt"})
      .process(JoinStr({mc_processes, {"Embedded"}}))
      .AddSyst(cb, "CMS_eff_t_", "lnN", SystMap<>::init(ditauID));


  // ##########################################################################
  // Uncertainty: b-tag and mistag efficiency
  // References:
  // Notes:
  // - FIXME: References?
  // ##########################################################################

  cb.cp()
      .channel({"tt"})
      .process(mc_processes)
      .AddSyst(cb, "Btagging_", "shape", SystMap<>::init(1.00));

  // missing efficiency and mistag split? 


  // ##########################################################################
  // Uncertainty: Tau energy scale
  // References:
  // Notes:
  // - Tau energy scale is splitted by decay mode.
  // - FIXME: References?
  // ##########################################################################


  //// per ERA
  // MC uncorrelated uncertainty

  cb.cp()
      .channel({"tt"})
    .process(JoinStr({signals, {"ZTT", "TTT", "TTL", "VVT", "VVL", "jetFakes"}}))
      .AddSyst(cb, "TES_HadronicTau_1prong0pi0_mc_", "shape", SystMap<>::init(0.5));

  cb.cp()
      .channel({"tt"})
      .process(JoinStr({signals, {"ZTT", "TTT", "TTL", "VVT", "VVL", "jetFakes"}}))
      .AddSyst(cb, "TES_HadronicTau_1prong1pi0_mc_", "shape",
               SystMap<>::init(0.5));

  cb.cp()
      .channel({"tt"})
      .process(JoinStr({signals, {"ZTT", "TTT", "TTL", "VVT", "VVL", "jetFakes"}}))
      .AddSyst(cb, "TES_HadronicTau_3prong0pi0_mc_", "shape", SystMap<>::init(0.5));

  cb.cp()
      .channel({"tt"})
      .process(JoinStr({signals, {"ZTT", "TTT", "TTL", "VVT", "VVL", "jetFakes"}}))
      .AddSyst(cb, "TES_promptMuon_1prong0pi0_mc_", "shape", SystMap<>::init(0.5));

  cb.cp()
      .channel({"tt"})
      .process(JoinStr({signals, {"ZTT", "TTT", "TTL", "VVT", "VVL", "jetFakes"}}))
      .AddSyst(cb, "TES_promptEle_1prong0pi0_mc_", "shape", SystMap<>::init(0.5));

  cb.cp()
      .channel({"tt"})
      .process(JoinStr({signals, {"ZTT", "TTT", "TTL", "VVT", "VVL", "jetFakes"}}))
      .AddSyst(cb, "TES_promptEle_1prong1pi0_mc_", "shape", SystMap<>::init(0.5));

  // Embedded uncorrelated uncertainty

  cb.cp()
      .channel({"tt"})
      .process({"Embedded"})
      .AddSyst(cb, "TES_HadronicTau_1prong0pi0_emb_", "shape", SystMap<>::init(0.5));

  cb.cp()
      .channel({"tt"})
      .process({"Embedded"})
      .AddSyst(cb, "TES_HadronicTau_1prong1pi0_emb_", "shape", SystMap<>::init(0.5));

  cb.cp()
      .channel({"tt"})
      .process({"Embedded"})
      .AddSyst(cb, "TES_HadronicTau_3prong0pi0_emb_", "shape", SystMap<>::init(0.5));

  cb.cp()
      .channel({"tt"})
      .process({"Embedded"})
      .AddSyst(cb, "TES_promptMuon_1prong0pi0_emb_", "shape", SystMap<>::init(0.5));

  cb.cp()
      .channel({"tt"})
      .process({"Embedded"})
      .AddSyst(cb, "TES_promptEle_1prong0pi0_emb_", "shape", SystMap<>::init(0.5));

  cb.cp()
      .channel({"tt"})
      .process({"Embedded"})
      .AddSyst(cb, "TES_promptEle_1prong1pi0_emb_", "shape", SystMap<>::init(0.5));

  // MC + embedded correlated uncertainty

  cb.cp()
      .channel({"tt"})
      .process(JoinStr({signals, {"ZTT", "TTT", "TTL", "VVT", "VVL", "jetFakes"}, {"Embedded"}}))
      .AddSyst(cb, "TES_HadronicTau_1prong0pi0_", "shape", SystMap<>::init(0.5));

  cb.cp()
      .channel({"tt"})
      .process(JoinStr({signals, {"ZTT", "TTT", "TTL", "VVT", "VVL", "jetFakes"}, {"Embedded"}}))
      .AddSyst(cb, "TES_HadronicTau_1prong1pi0_", "shape", SystMap<>::init(0.5));

  cb.cp()
      .channel({"tt"})
      .process(JoinStr({signals, {"ZTT", "TTT", "TTL", "VVT", "VVL", "jetFakes"}, {"Embedded"}}))
      .AddSyst(cb, "TES_HadronicTau_3prong0pi0_", "shape", SystMap<>::init(0.5));

  cb.cp()
      .channel({"tt"})
      .process(JoinStr({signals, {"ZTT", "TTT", "TTL", "VVT", "VVL", "jetFakes"}, {"Embedded"}}))
      .AddSyst(cb, "TES_promptMuon_1prong0pi0_", "shape", SystMap<>::init(0.5));

  cb.cp()
      .channel({"tt"})
      .process(JoinStr({signals, {"ZTT", "TTT", "TTL", "VVT", "VVL", "jetFakes"}, {"Embedded"}}))
      .AddSyst(cb, "TES_promptEle_1prong0pi0_", "shape", SystMap<>::init(0.5));

  cb.cp()
      .channel({"tt"})
      .process(JoinStr({signals, {"ZTT", "TTT", "TTL", "VVT", "VVL", "jetFakes"}, {"Embedded"}}))
      .AddSyst(cb, "TES_promptEle_1prong1pi0_", "shape", SystMap<>::init(0.5));

  // ##########################################################################
  // Uncertainty: Jet energy scale
  // References:
  // - Talk in CMS Htt meeting by Daniel Winterbottom about regional JES splits:
  //   https://indico.cern.ch/event/740094/contributions/3055870/
  // Notes:
  // ##########################################################################

  // Regional JES
  // uncorrelated between eras
  cb.cp()
    .channel({"tt"})
    .process(mc_processes)
    .AddSyst(cb, "CMS_scale_j_eta0to3_13TeV_", "shape", SystMap<>::init(1.00));
  
  cb.cp()
    .channel({"tt"})
    .process(mc_processes)
    .AddSyst(cb, "CMS_scale_j_eta0to5_13Tev_", "shape", SystMap<>::init(1.00));
  
  cb.cp()
    .channel({"tt"})
    .process(mc_processes)
    .AddSyst(cb, "CMS_scale_j_eta3to5_13TeV_", "shape", SystMap<>::init(1.00));
  
  cb.cp()
    .channel({"tt"})
    .process(mc_processes)
    .AddSyst(cb, "CMS_scale_j_RelativeBal_13TeV_", "shape", SystMap<>::init(1.00));
  cb.cp()
    .channel({"tt"})
    .process(mc_processes)
    .AddSyst(cb, "CMS_scale_j_RelativeSample_13TeV_", "shape", SystMap<>::init(1.00));

  // ##########################################################################
  // Uncertainty: MET energy scale and Recoil
  // References:
  // Notes:
  // - FIXME: Clustered vs unclustered MET? Inclusion of JES splitting?
  // - FIXME: References?
  // ##########################################################################

  cb.cp()
      .channel({"tt"})
      .process({"ZTT", "TT", "TTT", "TTL", "TTJ", "W", "ZJ", "ZL", "VV", "VVT", "VVL", "VVJ", "ST"})  //Z and W processes are only included due to the EWK fraction. Make sure that there is no contribution to the shift from the DY or Wjets samples.
      .AddSyst(cb, "METunclustered_", "shape", SystMap<>::init(1.00));
  cb.cp()
      .channel({"tt"})
      .process(JoinStr({signals, {"ZTT", "ZL", "ZJ", "W"}}))
      .AddSyst(cb, "METrecoil_response_", "shape", SystMap<>::init(1.00));
  cb.cp()
      .channel({"tt"})
     .process(JoinStr({signals,{"ZTT", "ZL", "ZJ", "W"}}))
      .AddSyst(cb, "METrecoil_resolution_", "shape", SystMap<>::init(1.00));

  // ##########################################################################
  // Uncertainty: Background normalizations
  // References:
  // Notes:
  // - FIXME: Remeasure QCD extrapolation factors for SS and ABCD methods?
  //          Current values are measured by KIT.
  // - FIXME: Adapt for fake factor and embedding
  // - FIXME: W uncertainties: Do we need lnN uncertainties based on the Ersatz
  //          study in Run1 (found in HIG-16043 uncertainty model)
  // - FIXME: References?
  // ##########################################################################

  // VV
  cb.cp()
      .channel({"tt"})
      .process({"VVT", "VVJ", "VVL", "VV", "ST"})
      .AddSyst(cb, "CMS_htt_vvXsec", "lnN", SystMap<>::init(1.05));

  // TT
  cb.cp()
      .channel({"tt"})
      .process({"TTT", "TTL", "TTJ", "TT"})
      .AddSyst(cb, "CMS_htt_tjXsec", "lnN", SystMap<>::init(1.06));

  // W
  cb.cp()
      .channel({"tt"})
      .process({"W"})
      .AddSyst(cb, "CMS_htt_wjXsec", "lnN", SystMap<>::init(1.04));

  // Z
  cb.cp()
      .channel({"tt"})
      .process({"ZTT", "ZL", "ZJ"})
      .AddSyst(cb, "CMS_htt_zjXsec", "lnN", SystMap<>::init(1.04));


  // ##########################################################################
  // Uncertainty: Drell-Yan LO->NLO reweighting
  // References:
  // Notes:
  // - FIXME: References?
  // ##########################################################################
  // is this same as DY pt reweighting?
  cb.cp()
      .channel({"tt"})
      .process({"ZTT", "ZL", "ZJ"})
      .AddSyst(cb, "DY_pT_reweighting_", "shape", SystMap<>::init(1.0));

  // ##########################################################################
  // Uncertainty: TT shape reweighting
  // References:
  // Notes:
  // - FIXME: References?
  // ##########################################################################

  cb.cp()
      .channel({"tt"})
      .process({"TTT", "TTL", "TTJ", "TT"})
      .AddSyst(cb, "top_pT_reweighting_", "shape", SystMap<>::init(1.00));

  // ##########################################################################
  // Uncertainty: Jet to tau fakes
  // References:
  // Notes:
  // - FIXME: Adapt for fake factor and embedding
  // - FIXME: References?
  // ##########################################################################

  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_qcd_syst_", "shape", SystMap<>::init(1.00));

  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_qcd_dm0_njet0_stat_", "shape", SystMap<>::init(1.00));

  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_qcd_dm0_njet1_stat_", "shape", SystMap<>::init(1.00));

  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_w_syst_", "shape", SystMap<>::init(1.00));

  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_tt_syst_", "shape", SystMap<>::init(1.00));

  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_w_frac_syst_", "shape", SystMap<>::init(1.00));

  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_tt_frac_syst_", "shape", SystMap<>::init(1.00));

  // ##########################################################################
  // Uncertainty: Theory uncertainties
  // References:
  // - Gluon-fusion WG1 uncertainty scheme:
  //   https://twiki.cern.ch/twiki/bin/view/CMS/HiggsWG/SignalModelingTools
  // Notes:
  // - FIXME: Add TopMassTreatment from HIG-16043 uncertainty model
  // - FIXME: Compare to HIG-16043 uncertainty model:
  //           - PDF uncertainties splitted by category?
  //           - QCDUnc uncertainties?
  //           - UEPS uncertainties?
  // - FIXME: Check VH QCD scale uncertainty
  // - FIXME: References?
  // ##########################################################################

  // // Uncertainty on branching ratio for HTT at 125 GeV
  // cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //     .process(signals)
  //     .AddSyst(cb, "BR_htt_THU", "lnN", SystMap<>::init(1.017));
  // cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //     .process(signals)
  //     .AddSyst(cb, "BR_htt_PU_mq", "lnN", SystMap<>::init(1.0099));
  // cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //     .process(signals)
  //     .AddSyst(cb, "BR_htt_PU_alphas", "lnN", SystMap<>::init(1.0062));
  // // Uncertainty on branching ratio for HWW at 125 GeV
  // cb.cp()
  //    .channel({"em"})
  //    .process(JoinStr({signals_ggHToWW,signals_qqHToWW}))
  //    .AddSyst(cb, "BR_hww_THU", "lnN", SystMap<>::init(1.0099));   
  // cb.cp()
  //    .channel({"em"})
  //    .process(JoinStr({signals_ggHToWW,signals_qqHToWW}))
  //    .AddSyst(cb, "BR_hww_PU_mq", "lnN", SystMap<>::init(1.0099));
  // cb.cp()
  //    .channel({"em"})
  //    .process(JoinStr({signals_ggHToWW,signals_qqHToWW}))
  //    .AddSyst(cb, "BR_hww_PU_alphas", "lnN", SystMap<>::init(1.0066));
  // // QCD scale
  // if (!ggh_wg1) {
  // cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //    .process(JoinStr({signals_ggH,signals_ggHToWW}))
  //     .AddSyst(cb, "QCDScale_ggH", "lnN", SystMap<>::init(1.039));
  // }
  // cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //    .process(JoinStr({signals_qqH,signals_qqHToWW}))
  //     .AddSyst(cb, "QCDScale_qqH", "lnN", SystMap<>::init(1.005));
  // cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //     .process({"ZH125"})
  //     .AddSyst(cb, "QCDScale_VH", "lnN", SystMap<>::init(1.009));
  // cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //     .process({"WH125"})
  //     .AddSyst(cb, "QCDScale_VH", "lnN", SystMap<>::init(1.008));
  // cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //     .process({"ttH125"})
  //     .AddSyst(cb, "QCDScale_ttH", "lnN", SystMap<>::init(1.08));

  // // PDF
  // cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //    .process(JoinStr({signals_ggH,signals_ggHToWW}))
  //     .AddSyst(cb, "pdf_Higgs_gg", "lnN", SystMap<>::init(1.032));
  // cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //    .process(JoinStr({signals_qqH,signals_qqHToWW}))
  //     .AddSyst(cb, "pdf_Higgs_qq", "lnN", SystMap<>::init(1.021));
  // cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //     .process({"ZH125"})
  //     .AddSyst(cb, "pdf_Higgs_VH", "lnN", SystMap<>::init(1.013));
  // cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //     .process({"WH125"})
  //     .AddSyst(cb, "pdf_Higgs_VH", "lnN", SystMap<>::init(1.018));
  // cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //     .process({"ttH125"})
  //     .AddSyst(cb, "pdf_Higgs_ttH", "lnN", SystMap<>::init(1.036));

  // // Gluon-fusion WG1 uncertainty scheme
  // if (ggh_wg1) {
  //   cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //      .process({signals_ggH})
  //     .AddSyst(cb, "THU_ggH_Mig01", "shape", SystMap<>::init(1.00));
  //   cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //      .process({signals_ggH})
  //     .AddSyst(cb, "THU_ggH_Mig12", "shape", SystMap<>::init(1.00));
  //   cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //      .process({signals_ggH})
  //     .AddSyst(cb, "THU_ggH_Mu", "shape", SystMap<>::init(1.00));
  //   cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //      .process({signals_ggH})
  //     .AddSyst(cb, "THU_ggH_PT120", "shape", SystMap<>::init(1.00));
  //   cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //      .process({signals_ggH})
  //     .AddSyst(cb, "THU_ggH_PT60", "shape", SystMap<>::init(1.00));
  //   cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //      .process({signals_ggH})
  //     .AddSyst(cb, "THU_ggH_Res", "shape", SystMap<>::init(1.00));
  //   cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //      .process({signals_ggH})
  //     .AddSyst(cb, "THU_ggH_VBF2j", "shape", SystMap<>::init(1.00));
  //   cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //      .process({signals_ggH})
  //     .AddSyst(cb, "THU_ggH_VBF3j", "shape", SystMap<>::init(1.00));
  //   cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //      .process({signals_ggH})
  //     .AddSyst(cb, "THU_ggH_qmtop", "shape", SystMap<>::init(1.00));
  //   cb.cp()
  //     .channel({"et", "mt", "tt", "em"})
  //    .process({signals_ggHToWW})
  //     .AddSyst(cb, "QCDScale_ggHWW", "lnN", SystMap<>::init(1.039));
  // }

  cb.cp()
    .channel({"tt"})
    .process({signals})
    .AddSyst(cb, "arbitrary_flat_unc", "lnN", SystMap<>::init(1.10));

  // ##########################################################################
  // Uncertainty: Embedded events
  // References:
  // - https://twiki.cern.ch/twiki/bin/viewauth/CMS/TauTauEmbeddingSamples2016
  // Notes:
  // ##########################################################################

  // Embedded Normalization: No Lumi, Zjxsec information used, instead derived from data using dimuon selection efficiency
  cb.cp()
      .channel({"tt"})
      .process({"Embedded"})
      .AddSyst(cb, "CMS_htt_doublemutrg_", "lnN", SystMap<>::init(1.04));

  // TTbar contamination in embedded events: 10% shape uncertainty of assumed ttbar->tautau event shape
  // cb.cp()
  //   .channel({"tt" })
  //   .process({"Embedded"})
  //   .AddSyst(cb, "CMS_htt_emb_ttbar_", "shape", SystMap<>::init(1.00));

  // Uncertainty of hadronic tau track efficiency correction
  cb.cp()
    .channel({"tt"})
    .process({"Embedded"})
    .AddSyst(cb, "Embedding_tracking_3prong_", "shape", SystMap<>::init(1.00));

  cb.cp()
    .channel({"tt"})
    .process({"Embedded"})
    .AddSyst(cb, "Embedding_tracking_1prong_", "shape", SystMap<>::init(1.00));

  // ##########################################################################
  // Uncertainty: Jet fakes
  // References:
  // - https://twiki.cern.ch/twiki/bin/viewauth/CMS/HiggsToTauTauJet2TauFakes
  // Notes:
  // - FIXME: add 2017 norm uncertainties, and properly correlate across years
  // ##########################################################################
  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_norm_stat_$BIN", "lnN", SystMap<channel, bin_id>::init
  	       ({"tt"}, {8},  1.023) //btag
  	       ({"tt"}, {9}, 1.028) //nobtag
  	       );

  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_norm_syst_$BIN", "lnN", SystMap<channel, bin_id>::init
  	       ({"tt"}, {8},  1.099) //btag
  	       ({"tt"}, {9}, 1.100) //nobtag
  	       );

  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_sub_syst_$BIN", "lnN", SystMap<channel, bin_id>::init
  	       ({"tt"}, {8},  1.03) //btag
  	       ({"tt"}, {9}, 1.03) //nobtag
  	       );

  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_qcd_dm0_njet0_stat_", "shape", SystMap<>::init(1.00));
  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_qcd_dm0_njet1_stat_", "shape", SystMap<>::init(1.00));

  // Shape syst. of different contr
  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_qcd_syst_", "shape", SystMap<>::init(1.00));

  // Shape syst. in tautau due to using QCD FF also for W / tt
  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_w_syst_", "shape", SystMap<>::init(1.00));
  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_tt_syst_", "shape", SystMap<>::init(1.00));

  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_w_frac_syst_", "shape", SystMap<>::init(1.00));

  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_tt_frac_syst_", "shape", SystMap<>::init(1.00));
  // //below: jetFakes norm uncertainties. Current values are for 2016, which are probably a good approx. for 2017. To be updated.

  // // Stat. norm (uncorrelated across years)
  // cb.cp()
  //     .channel({"et", "mt", "tt"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "CMS_ff_norm_stat_$CHANNEL_$BIN_$ERA", "lnN", SystMap<channel, bin_id>::init
  // 	       ({"mt"}, {1},   1.035) //ggh
  // 	       ({"mt"}, {2},   1.048) //qqh
  // 	       ({"mt"}, {11},  1.028) //w
  // 	       ({"mt"}, {12},  1.037) //ztt
  // 	       ({"mt"}, {13},  1.036) //tt
  // 	       ({"mt"}, {14},  1.033) //ss
  // 	       ({"mt"}, {15},  1.028) //zll
  // 	       ({"mt"}, {16},  1.042) //misc
  // 	       ({"mt"}, {100}, 1.026) //incl
  // 	       ({"et"}, {1},   1.052) //ggh
  // 	       ({"et"}, {2},   1.079) //qqh
  // 	       ({"et"}, {11},  1.047) //w
  // 	       ({"et"}, {12},  1.067) //ztt
  // 	       ({"et"}, {13},  1.059) //tt
  // 	       ({"et"}, {14},  1.038) //ss
  // 	       ({"et"}, {15},  1.067) //zll
  // 	       ({"et"}, {16},  1.076) //misc
  // 	       ({"et"}, {100}, 1.046) //incl
  // 	       ({"tt"}, {1},   1.029) //ggh
  // 	       ({"tt"}, {2},   1.037) //qqh
  // 	       ({"tt"}, {12},  1.035) //ztt
  // 	       ({"tt"}, {16},  1.020) //misc
  // 	       ({"tt"}, {17},  1.029) //noniso
  // 	       ({"tt"}, {100}, 1.029) //incl
  // 	       );

  // // Syst. norm: Bin-correlated
  // // uncorrelated between eras
  // cb.cp()
  //     .channel({"et", "mt", "tt"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "CMS_ff_norm_syst_$CHANNEL_$ERA", "lnN", SystMap<channel, bin_id>::init
  // 	       ({"mt"}, {1},   1.049) //ggh
  // 	       ({"mt"}, {2},   1.041) //qqh
  // 	       ({"mt"}, {11},  1.038) //w
  // 	       ({"mt"}, {12},  1.069) //ztt
  // 	       ({"mt"}, {13},  1.037) //tt
  // 	       ({"mt"}, {14},  1.064) //ss
  // 	       ({"mt"}, {15},  1.048) //zll
  // 	       ({"mt"}, {16},  1.064) //misc
  // 	       ({"mt"}, {100}, 1.042) //incl
  // 	       ({"et"}, {1},   1.042) //ggh
  // 	       ({"et"}, {2},   1.040) //qqh
  // 	       ({"et"}, {11},  1.037) //w
  // 	       ({"et"}, {12},  1.062) //ztt
  // 	       ({"et"}, {13},  1.040) //tt
  // 	       ({"et"}, {14},  1.045) //ss
  // 	       ({"et"}, {15},  1.051) //zll
  // 	       ({"et"}, {16},  1.041) //misc
  // 	       ({"et"}, {100}, 1.042) //incl
  // 	       ({"tt"}, {1},   1.068) //ggh
  // 	       ({"tt"}, {2},   1.067) //qqh
  // 	       ({"tt"}, {12},  1.067) //ztt
  // 	       ({"tt"}, {16},  1.078) //misc
  // 	       ({"tt"}, {17},  1.070) //noniso
  // 	       ({"tt"}, {100}, 1.067) //incl
  // 	       );
  // // correlated between eras
  // cb.cp()
  //     .channel({"et", "mt", "tt"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "CMS_ff_norm_syst_$CHANNEL", "lnN", SystMap<channel, bin_id>::init
  // 	       ({"mt"}, {1},   1.049) //ggh
  // 	       ({"mt"}, {2},   1.041) //qqh
  // 	       ({"mt"}, {11},  1.038) //w
  // 	       ({"mt"}, {12},  1.069) //ztt
  // 	       ({"mt"}, {13},  1.037) //tt
  // 	       ({"mt"}, {14},  1.064) //ss
  // 	       ({"mt"}, {15},  1.048) //zll
  // 	       ({"mt"}, {16},  1.064) //misc
  // 	       ({"mt"}, {100}, 1.042) //incl
  // 	       ({"et"}, {1},   1.042) //ggh
  // 	       ({"et"}, {2},   1.040) //qqh
  // 	       ({"et"}, {11},  1.037) //w
  // 	       ({"et"}, {12},  1.062) //ztt
  // 	       ({"et"}, {13},  1.040) //tt
  // 	       ({"et"}, {14},  1.045) //ss
  // 	       ({"et"}, {15},  1.051) //zll
  // 	       ({"et"}, {16},  1.041) //misc
  // 	       ({"et"}, {100}, 1.042) //incl
  // 	       ({"tt"}, {1},   1.068) //ggh
  // 	       ({"tt"}, {2},   1.067) //qqh
  // 	       ({"tt"}, {12},  1.067) //ztt
  // 	       ({"tt"}, {16},  1.078) //misc
  // 	       ({"tt"}, {17},  1.070) //noniso
  // 	       ({"tt"}, {100}, 1.067) //incl
  // 	       );

  // // Syst. norm: Bin-dependent, correlated across years
  // // uncorrelated between eras
  // cb.cp()
  //     .channel({"et", "mt", "tt"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "CMS_ff_sub_syst_$CHANNEL_$BIN_$ERA", "lnN", SystMap<channel, bin_id>::init
  // 	       ({"mt"}, {1},   1.028) //ggh
  // 	       ({"mt"}, {2},   1.028) //qqh
  // 	       ({"mt"}, {11},  1.018) //w
  // 	       ({"mt"}, {12},  1.032) //ztt
  // 	       ({"mt"}, {13},  1.021) //tt
  // 	       ({"mt"}, {14},  1.014) //ss
  // 	       ({"mt"}, {15},  1.028) //zll
  // 	       ({"mt"}, {16},  1.025) //misc
  // 	       ({"mt"}, {100}, 1.025) //incl
  // 	       ({"et"}, {1},   1.028) //ggh
  // 	       ({"et"}, {2},   1.025) //qqh
  // 	       ({"et"}, {11},  1.014) //w
  // 	       ({"et"}, {12},  1.028) //ztt
  // 	       ({"et"}, {13},  1.021) //tt
  // 	       ({"et"}, {14},  1.014) //ss
  // 	       ({"et"}, {15},  1.028) //zll
  // 	       ({"et"}, {16},  1.025) //misc
  // 	       ({"et"}, {100}, 1.025) //incl
  // 	       ({"tt"}, {1},   1.021) //ggh
  // 	       ({"tt"}, {2},   1.021) //qqh
  // 	       ({"tt"}, {12},  1.025) //ztt
  // 	       ({"tt"}, {16},  1.021) //misc
  // 	       ({"tt"}, {17},  1.014) //noniso
  // 	       ({"tt"}, {100}, 1.021) //incl
  // 	       );
  // // correlated between eras
  // cb.cp()
  //     .channel({"et", "mt", "tt"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "CMS_ff_sub_syst_$CHANNEL_$BIN", "lnN", SystMap<channel, bin_id>::init
  // 	       ({"mt"}, {1},   1.028) //ggh
  // 	       ({"mt"}, {2},   1.028) //qqh
  // 	       ({"mt"}, {11},  1.018) //w
  // 	       ({"mt"}, {12},  1.032) //ztt
  // 	       ({"mt"}, {13},  1.021) //tt
  // 	       ({"mt"}, {14},  1.014) //ss
  // 	       ({"mt"}, {15},  1.028) //zll
  // 	       ({"mt"}, {16},  1.025) //misc
  // 	       ({"mt"}, {100}, 1.025) //incl
  // 	       ({"et"}, {1},   1.028) //ggh
  // 	       ({"et"}, {2},   1.025) //qqh
  // 	       ({"et"}, {11},  1.014) //w
  // 	       ({"et"}, {12},  1.028) //ztt
  // 	       ({"et"}, {13},  1.021) //tt
  // 	       ({"et"}, {14},  1.014) //ss
  // 	       ({"et"}, {15},  1.028) //zll
  // 	       ({"et"}, {16},  1.025) //misc
  // 	       ({"et"}, {100}, 1.025) //incl
  // 	       ({"tt"}, {1},   1.021) //ggh
  // 	       ({"tt"}, {2},   1.021) //qqh
  // 	       ({"tt"}, {12},  1.025) //ztt
  // 	       ({"tt"}, {16},  1.021) //misc
  // 	       ({"tt"}, {17},  1.014) //noniso
  // 	       ({"tt"}, {100}, 1.021) //incl
  // 	       );
}
} // namespace ch
