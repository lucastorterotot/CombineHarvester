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
using ch::syst::mass;
using ch::syst::bin;
using ch::JoinStr;

void AddMSSMRun2Systematics(CombineHarvester &cb, bool jetfakes, bool embedding, bool ggh_wg1, int era) {

  // ##########################################################################
  // Define groups of processes
  // ##########################################################################

  // Signal processes
  std::vector<std::string> signals = {"ggH","bbH"};
  std::vector<std::string> SM_procs = {"HiggsVBF125", "HiggsGGH125", "HiggsWplusH125", "HiggsWminusH125", "HiggsTTH125", "HiggsZH125"};
  signals = JoinStr({signals,SM_procs});

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
              {"W", "ZTT", "QCD", "ZL", "ZJ", "TTT", "TTL", "TTJ", "VVJ", "VVT", "VVL"}
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
    .channel({"tt", "mt", "et"})
      .process(mc_processes)
      .AddSyst(cb, "lumi_", "lnN", SystMap<>::init(lumi_unc));

  // ##########################################################################
  // Uncertainty: Trigger efficiency
  // References:
  // Notes:
  // - FIXME: References?
  // - cross trigger unc as shape?
  // ##########################################################################

  float trg_unc_emb = 1.04;
  float trg_unc_tt = 1.10;
  float trg_unc_lt = 1.02;
  float xtrg_unc_lt = 1.054;

  cb.cp()
    .channel({"tt", "mt", "et"})
    .process({"Embedded"})
    .AddSyst(cb, "CMS_eff_trigger_dblmu_emb", "lnN", SystMap<>::init(trg_unc_emb));

  cb.cp()
    .channel({"tt"})
    .process(mc_processes)
    .AddSyst(cb, "CMS_eff_trigger_t", "lnN", SystMap<>::init(trg_unc_tt));
  cb.cp()
    .channel({"tt"})
    .process({"Embedded"})
    .AddSyst(cb, "CMS_eff_trigger_emb_t", "lnN", SystMap<>::init(trg_unc_tt));

  cb.cp()
    .channel({"mt"})
    .process(mc_processes)
    .AddSyst(cb, "CMS_eff_trigger_m", "lnN", SystMap<>::init(trg_unc_lt));
  cb.cp()
    .channel({"mt"})
    .process({"Embedded"})
    .AddSyst(cb, "CMS_eff_trigger_m_emb", "lnN", SystMap<>::init(trg_unc_lt));
  cb.cp()
    .channel({"mt"})
    .process(mc_processes)
    .AddSyst(cb, "CMS_eff_crosstrigger_m", "lnN", SystMap<>::init(xtrg_unc_lt));
  cb.cp()
    .channel({"mt"})
    .process({"Embedded"})
    .AddSyst(cb, "CMS_eff_crosstrigger_m_emb", "lnN", SystMap<>::init(xtrg_unc_lt));

  cb.cp()
    .channel({"et"})
    .process(mc_processes)
    .AddSyst(cb, "CMS_eff_trigger_e", "lnN", SystMap<>::init(trg_unc_lt));
  cb.cp()
    .channel({"et"})
    .process({"Embedded"})
    .AddSyst(cb, "CMS_eff_trigger_e_emb", "lnN", SystMap<>::init(trg_unc_lt));
  cb.cp()
    .channel({"et"})
    .process(mc_processes)
    .AddSyst(cb, "CMS_eff_crosstrigger_e", "lnN", SystMap<>::init(xtrg_unc_lt));
  cb.cp()
    .channel({"et"})
    .process({"Embedded"})
    .AddSyst(cb, "CMS_eff_crosstrigger_e_emb", "lnN", SystMap<>::init(xtrg_unc_lt));

  // ##########################################################################
  // Uncertainty: e -> tauh, mu ->tauh fake rate
  // References:CMS AN 2018/255
  // ##########################################################################

  cb.cp()
    .channel({"tt"})
    .process({"ZL"})
    .AddSyst(cb, "ele_to_tauh_fake", "lnN", SystMap<>::init(1.03)); // 16%?

  cb.cp()
    .channel({"et"})
    .process({"ZL"})
    .AddSyst(cb, "ele_to_tauh_fake", "lnN", SystMap<>::init(1.16));

  cb.cp()
    .channel({"tt"})
    .process({"ZL"})
    .AddSyst(cb, "mu_to_tauh_fake", "lnN", SystMap<>::init(1.05)); // 26%?

  cb.cp()
    .channel({"mt"})
    .process({"ZL"})
    .AddSyst(cb, "mu_to_tauh_fake", "lnN", SystMap<>::init(1.26));

  // ##########################################################################
  // Uncertainty: Electron, muon and tau ID efficiency
  // References:
  // Notes:
  // - FIXME: Adapt for fake factor and embedding
  // - FIXME: Handling of ZL in fully-hadronic channel?
  // - FIXME: References?
  // ##########################################################################

  float tauID_channel_correlated = 1.027;
  float tauID_channel_uncorrelated = 1.012;
  float eleID = 1.02;
  float muID = 1.02;

  // MC uncorrelated uncertainty

  // Tau ID channel correlated
  cb.cp()
    .channel({"tt", "mt", "et"})
      .process(mc_processes)
      .AddSyst(cb, "CMS_eff_mc_chan_cor_tID_", "lnN", SystMap<>::init(tauID_channel_correlated));
  // Tau ID channel uncorrelated
  cb.cp()
    .channel({"tt"})
      .process(mc_processes)
      .AddSyst(cb, "CMS_eff_mc_tt_chan_tID_", "lnN", SystMap<>::init(tauID_channel_uncorrelated));
  cb.cp()
    .channel({"mt"})
      .process(mc_processes)
      .AddSyst(cb, "CMS_eff_mc_mt_chan_tID_", "lnN", SystMap<>::init(tauID_channel_uncorrelated));
  cb.cp()
    .channel({"et"})
      .process(mc_processes)
      .AddSyst(cb, "CMS_eff_mc_et_chan_tID_", "lnN", SystMap<>::init(tauID_channel_uncorrelated));
  // muon ID
  cb.cp()
    .channel({"mt"})
      .process(mc_processes)
      .AddSyst(cb, "CMS_eff_mc_muID_", "lnN", SystMap<>::init(muID));
  // ele ID
  cb.cp()
    .channel({"et"})
      .process(mc_processes)
      .AddSyst(cb, "CMS_eff_mc_eleID_", "lnN", SystMap<>::init(eleID));


  // Embedded uncorrelated uncertainty

  // Tau ID channel correlated
  cb.cp()
    .channel({"tt", "mt", "et"})
      .process({"Embedded"})
      .AddSyst(cb, "CMS_eff_emb_chan_cor_tID_", "lnN", SystMap<>::init(tauID_channel_correlated));
  // Tau ID channel uncorrelated
  cb.cp()
    .channel({"tt"})
      .process({"Embedded"})
      .AddSyst(cb, "CMS_eff_emb_tt_chan_tID_", "lnN", SystMap<>::init(tauID_channel_uncorrelated));
  cb.cp()
    .channel({"mt"})
      .process({"Embedded"})
      .AddSyst(cb, "CMS_eff_emb_mt_chan_tID_", "lnN", SystMap<>::init(tauID_channel_uncorrelated));
  cb.cp()
    .channel({"et"})
      .process({"Embedded"})
      .AddSyst(cb, "CMS_eff_emb_et_chan_tID_", "lnN", SystMap<>::init(tauID_channel_uncorrelated));
  // muon ID
  cb.cp()
    .channel({"mt"})
      .process({"Embedded"})
      .AddSyst(cb, "CMS_eff_emb_muID_", "lnN", SystMap<>::init(muID));
  // ele ID
  cb.cp()
    .channel({"et"})
      .process({"Embedded"})
      .AddSyst(cb, "CMS_eff_emb_eleID_", "lnN", SystMap<>::init(eleID));



  // MC + embedded correlated uncertainty

  // Tau ID channel correlated
  cb.cp()
    .channel({"tt", "mt", "et"})
      .process(JoinStr({mc_processes, {"Embedded"}}))
      .AddSyst(cb, "CMS_eff_chan_cor_tID_", "lnN", SystMap<>::init(tauID_channel_correlated));
  // Tau ID channel uncorrelated
  cb.cp()
    .channel({"tt"})
      .process(JoinStr({mc_processes, {"Embedded"}}))
      .AddSyst(cb, "CMS_eff_tt_chan_tID_", "lnN", SystMap<>::init(tauID_channel_uncorrelated));
  cb.cp()
    .channel({"mt"})
      .process(JoinStr({mc_processes, {"Embedded"}}))
      .AddSyst(cb, "CMS_eff_mt_chan_tID_", "lnN", SystMap<>::init(tauID_channel_uncorrelated));
  cb.cp()
    .channel({"et"})
      .process(JoinStr({mc_processes, {"Embedded"}}))
      .AddSyst(cb, "CMS_eff_et_chan_tID_", "lnN", SystMap<>::init(tauID_channel_uncorrelated));
  // muon ID
  cb.cp()
    .channel({"mt"})
      .process(JoinStr({mc_processes, {"Embedded"}}))
      .AddSyst(cb, "CMS_eff_muID_", "lnN", SystMap<>::init(muID));
  // ele ID
  cb.cp()
    .channel({"et"})
      .process(JoinStr({mc_processes, {"Embedded"}}))
      .AddSyst(cb, "CMS_eff_eleID_", "lnN", SystMap<>::init(eleID));


  // ##########################################################################
  // Uncertainty: b-tag and mistag efficiency
  // References:
  // Notes:
  // - FIXME: References?
  // ##########################################################################

  cb.cp()
    .channel({"tt", "mt", "et"})
      .process(mc_processes)
      .AddSyst(cb, "Btagging_", "shape", SystMap<>::init(1.00));

  // missing efficiency and mistag split? 


  // ##########################################################################
  // Uncertainty: Electron energy scale
  // References:
  // Notes:
  // - FIXME: AN 2018/255
  // - FIXME: add systematic process for heppy!
  // ##########################################################################



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
    .channel({"tt", "mt", "et"})
    .process(JoinStr({mc_processes, {"jetFakes"}}))
    .AddSyst(cb, "TES_HadronicTau_1prong0pi0_mc_", "shape", SystMap<>::init(0.5));
  
  cb.cp()
    .channel({"tt", "mt", "et"})
    .process(JoinStr({mc_processes, {"jetFakes"}}))
    .AddSyst(cb, "TES_HadronicTau_1prong1pi0_mc_", "shape", SystMap<>::init(0.5));
  
  cb.cp()
    .channel({"tt", "mt", "et"})
    .process(JoinStr({mc_processes, {"jetFakes"}}))
    .AddSyst(cb, "TES_HadronicTau_3prong0pi0_mc_", "shape", SystMap<>::init(0.5));
  
  cb.cp()
    .channel({"tt", "mt", "et"})
    .process(JoinStr({mc_processes, {"jetFakes"}}))
    .AddSyst(cb, "TES_promptMuon_1prong0pi0_", "shape", SystMap<>::init(1.));
  
  cb.cp()
    .channel({"tt", "mt", "et"})
    .process(JoinStr({mc_processes, {"jetFakes"}}))
    .AddSyst(cb, "TES_promptEle_1prong0pi0_", "shape", SystMap<>::init(1.));
  
  cb.cp()
    .channel({"tt", "mt", "et"})
    .process(JoinStr({mc_processes, {"jetFakes"}}))
    .AddSyst(cb, "TES_promptEle_1prong1pi0_", "shape", SystMap<>::init(1.));


  // Embedded uncorrelated uncertainty

  cb.cp()
    .channel({"tt", "mt", "et"})
    .process({"Embedded", "jetFakes"})
    .AddSyst(cb, "TES_HadronicTau_1prong0pi0_emb_", "shape", SystMap<>::init(0.5));
  
  cb.cp()
    .channel({"tt", "mt", "et"})
    .process({"Embedded", "jetFakes"})
    .AddSyst(cb, "TES_HadronicTau_1prong1pi0_emb_", "shape", SystMap<>::init(0.5));
  
  cb.cp()
    .channel({"tt", "mt", "et"})
    .process({"Embedded", "jetFakes"})
    .AddSyst(cb, "TES_HadronicTau_3prong0pi0_emb_", "shape", SystMap<>::init(0.5));
  

  // MC + embedded correlated uncertainty

  cb.cp()
    .channel({"tt", "mt", "et"})
    .process(JoinStr({mc_processes, {"Embedded", "jetFakes"}}))
    .AddSyst(cb, "TES_HadronicTau_1prong0pi0_", "shape", SystMap<>::init(0.5));
  
  cb.cp()
    .channel({"tt", "mt", "et"})
    .process(JoinStr({mc_processes, {"Embedded", "jetFakes"}}))
    .AddSyst(cb, "TES_HadronicTau_1prong1pi0_", "shape", SystMap<>::init(0.5));
  
  cb.cp()
    .channel({"tt", "mt", "et"})
    .process(JoinStr({mc_processes, {"Embedded", "jetFakes"}}))
    .AddSyst(cb, "TES_HadronicTau_3prong0pi0_", "shape", SystMap<>::init(0.5));
  
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
    .channel({"tt", "mt", "et"})
    .process(mc_processes)
    .AddSyst(cb, "CMS_scale_j_eta0to3_13TeV_", "shape", SystMap<>::init(1.00));
  
  cb.cp()
    .channel({"tt", "mt", "et"})
    .process(mc_processes)
    .AddSyst(cb, "CMS_scale_j_eta0to5_13Tev_", "shape", SystMap<>::init(1.00));
  
  cb.cp()
    .channel({"tt", "mt", "et"})
    .process(mc_processes)
    .AddSyst(cb, "CMS_scale_j_eta3to5_13TeV_", "shape", SystMap<>::init(1.00));
  
  cb.cp()
    .channel({"tt", "mt", "et"})
    .process(mc_processes)
    .AddSyst(cb, "CMS_scale_j_RelativeBal_13TeV_", "shape", SystMap<>::init(1.00));
  cb.cp()
    .channel({"tt", "mt", "et"})
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
      .channel({"tt", "mt", "et"})
      .process({"TT", "TTT", "TTL", "TTJ", "VV", "VVT", "VVL", "VVJ", "ST"})
      .AddSyst(cb, "METunclustered_", "shape", SystMap<>::init(1.00));
  cb.cp()
      .channel({"tt", "mt", "et"})
      .process(JoinStr({signals, {"ZTT", "ZL", "ZJ", "W"}}))
      .AddSyst(cb, "METrecoil_response_", "shape", SystMap<>::init(1.00));
  cb.cp()
      .channel({"tt", "mt", "et"})
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
      .channel({"tt", "mt", "et"})
      .process({"VVT", "VVJ", "VVL", "VV", "ST"})
      .AddSyst(cb, "CMS_htt_vvXsec", "lnN", SystMap<>::init(1.05));

  // TT
  cb.cp()
      .channel({"tt", "mt", "et"})
      .process({"TTT", "TTL", "TTJ", "TT"})
      .AddSyst(cb, "CMS_htt_tjXsec", "lnN", SystMap<>::init(1.06));

  // W
  cb.cp()
      .channel({"tt", "mt", "et"})
      .process({"W"})
      .AddSyst(cb, "CMS_htt_wjXsec", "lnN", SystMap<>::init(1.04));

  // Z
  cb.cp()
      .channel({"tt", "mt", "et"})
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
      .channel({"tt", "mt", "et"})
      .process({"ZTT", "ZL", "ZJ"})
      .AddSyst(cb, "dy_pt_reweighting_", "shape", SystMap<>::init(1.0));

  // ##########################################################################
  // Uncertainty: TT shape reweighting
  // References:
  // Notes:
  // - FIXME: References?
  // ##########################################################################

  cb.cp()
      .channel({"tt", "mt", "et"})
      .process({"TTT", "TTL", "TTJ", "TT"})
      .AddSyst(cb, "top_pt_reweighting_", "shape", SystMap<>::init(1.00));

  // ##########################################################################
  // Uncertainty: Jet to tau fakes
  // References:
  // Notes:
  // - FIXME: Adapt for fake factor and embedding
  // - FIXME: References?
  // ##########################################################################

  cb.cp()
    .channel({"tt", "mt", "et"})
    .process({"jetFakes"})
    .AddSyst(cb, "CMS_htt_jetFakeTau_$CHANNEL_13TeV", "lnN", SystMap<>::init(1.20)); // from MSSM 2016

  cb.cp()
      .channel({"tt", "mt", "et"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_qcd_syst_", "shape", SystMap<>::init(1.00));
  // cb.cp()
  //     .channel({"tt", "mt", "et"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_qcd_syst_", "lnN", SystMap<>::init(1.03));

  cb.cp()
    .channel({"tt", "mt", "et"})
    .process({"jetFakes"})
    .AddSyst(cb, "ff_qcd_dm0_njet0_stat_", "shape", SystMap<>::init(1.00));
  // cb.cp()
  //     .channel({"tt", "mt", "et"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_qcd_dm0_njet0_stat_", "lnN", SystMap<>::init(1.016));

  cb.cp()
      .channel({"tt", "mt", "et"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_qcd_dm0_njet1_stat_", "shape", SystMap<>::init(1.00));
  // cb.cp()
  //     .channel({"tt", "mt", "et"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_qcd_dm0_njet1_stat_", "lnN", SystMap<>::init(1.016));

  cb.cp()
      .channel({"tt", "mt", "et"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_w_syst_", "shape", SystMap<>::init(1.00));
  // cb.cp()
  //     .channel({"tt", "mt", "et"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_w_syst_", "lnN", SystMap<>::init(1.07));

  cb.cp()
      .channel({"tt", "mt", "et"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_tt_syst_", "shape", SystMap<>::init(1.00));
  // cb.cp()
  //     .channel({"tt", "mt", "et"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_tt_syst_", "lnN", SystMap<>::init(1.013));

  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_w_frac_syst_", "shape", SystMap<>::init(1.00));
  // cb.cp()
  //     .channel({"tt"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_w_frac_syst_", "lnN", SystMap<>::init(1.02));

  cb.cp()
    .channel({"mt", "et"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_w_dm0_njet0_stat_", "shape", SystMap<>::init(1.00));
  // cb.cp()
  //   .channel({"mt", "et"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_w_dm0_njet0_stat_", "lnN", SystMap<>::init(1.02));

  cb.cp()
    .channel({"mt", "et"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_w_dm0_njet1_stat_", "shape", SystMap<>::init(1.00));
  // cb.cp()
  //   .channel({"mt", "et"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_w_dm0_njet1_stat_", "lnN", SystMap<>::init(1.02));

  cb.cp()
      .channel({"tt"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_tt_frac_syst_", "shape", SystMap<>::init(1.00));
  // cb.cp()
  //     .channel({"tt"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_tt_frac_syst_", "lnN", SystMap<>::init(1.011));

  cb.cp()
    .channel({"mt", "et"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_tt_dm0_njet0_stat_", "shape", SystMap<>::init(1.00));
  // cb.cp()
  //   .channel({"mt", "et"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_tt_dm0_njet0_stat_", "lnN", SystMap<>::init(1.011));

  cb.cp()
    .channel({"mt", "et"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_tt_dm0_njet1_stat_", "shape", SystMap<>::init(1.00));
  // cb.cp()
  //   .channel({"mt", "et"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_tt_dm0_njet1_stat_", "lnN", SystMap<>::init(1.011));

  //stat norm uncertainties
  cb.cp()
    .channel({"tt", "mt", "et"})
    .process({"jetFakes"})
    .AddSyst(cb, "ff_norm_stat_$CHANNEL_$BIN", "lnN", SystMap<channel, bin_id>::init
								  ({"mt"}, {7}, 1.040)
								  ({"mt"}, {8}, 1.039)
								  ({"mt"}, {9}, 1.040)
								  ({"mt"}, {10}, 1.038)
								  ({"mt"}, {11}, 1.035)
								  ({"et"}, {7}, 1.071)
								  ({"et"}, {8}, 1.058)
								  ({"et"}, {9}, 1.071)
								  ({"et"}, {10}, 1.044)
								  ({"et"}, {11}, 1.059)
								  ({"tt"}, {7}, 1.028)
								  ({"tt"}, {8}, 1.023)
								  ({"tt"}, {9}, 1.028)
                                                                  );

  //syst norm uncertainties: bin-correlated
  cb.cp()
    .channel({"tt", "mt", "et"})
    .process({"jetFakes"})
    .AddSyst(cb, "ff_norm_syst_$CHANNEL", "lnN", SystMap<channel, bin_id>::init
								  ({"mt"}, {7}, 1.075)
								  ({"mt"}, {8}, 1.075)
								  ({"mt"}, {9}, 1.068)
								  ({"mt"}, {10}, 1.063)
								  ({"mt"}, {11}, 1.055)
								  ({"et"}, {7}, 1.097)
								  ({"et"}, {8}, 1.097)
								  ({"et"}, {9}, 1.089)
								  ({"et"}, {10}, 1.068)
								  ({"et"}, {11}, 1.068)
								  ({"tt"}, {7}, 1.100)
								  ({"tt"}, {8}, 1.099)
								  ({"tt"}, {9}, 1.100)
                                                                  );

  //syst norm uncertainties: bin-specific
  cb.cp()
    .channel({"tt", "mt", "et"})
    .process({"jetFakes"})
    .AddSyst(cb, "ff_sub_syst_$CHANNEL_$BIN", "lnN", SystMap<channel, bin_id>::init
                                                                  ({"mt"}, {7}, 1.04)
                                                                  ({"mt"}, {8}, 1.04)
                                                                  ({"mt"}, {9}, 1.04)
                                                                  ({"mt"}, {10}, 1.04)
                                                                  ({"mt"}, {11}, 1.04)
                                                                  ({"et"}, {7}, 1.04)
                                                                  ({"et"}, {8}, 1.04)
                                                                  ({"et"}, {9}, 1.04)
                                                                  ({"et"}, {10}, 1.04)
                                                                  ({"et"}, {11}, 1.04)
                                                                  ({"tt"}, {7}, 1.03)
                                                                  ({"tt"}, {8}, 1.03)
                                                                  ({"tt"}, {9}, 1.03)
                                                                  );



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

  cb.cp()
    .channel({"tt", "mt", "et"})
    .process({"HiggsGGH125"})
    .AddSyst(cb, "QCDscale_ggH125", "lnN", SystMap<>::init(1.039));

  cb.cp()
    .channel({"tt", "mt", "et"})
    .process({"HiggsVBF125"})
    .AddSyst(cb, "QCDscale_HVBF125", "lnN", SystMapAsymm<>::init(1.004,0.997));

  cb.cp()
    .channel({"tt", "mt", "et"})
    .process({"HiggsZH125", "HiggsWplusH125", "HiggsWminusH125"})
    .AddSyst(cb, "QCDscale_VH", "lnN", SystMapAsymm<process>::init
             ({"HiggsZH125"}, 1.038, 0.969)
             ({"HiggsWplusH125", "HiggsWminusH125"},1.005,0.993));

  cb.cp()
    .channel({"tt", "mt", "et"})
    .process({"HiggsTTH125"})
    .AddSyst(cb, "QCDscale_TTH125", "lnN", SystMapAsymm<>::init(1.058,0.908));

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

  cb.cp()
    .channel({"tt", "mt", "et"})
    .process({"HiggsGGH125"})
    .AddSyst(cb, "pdf_ggH125", "lnN", SystMap<>::init(1.032)); // TODO check value

  cb.cp()
    .channel({"tt", "mt", "et"})
    .process({"HiggsZH125", "HiggsWplusH125", "HiggsWminusH125","HiggsVBF125"})
    .AddSyst(cb, "pdf_VH", "lnN", SystMap<process>::init
             ({"HiggsZH125"},1.016)
             ({"HiggsWplusH125", "HiggsWminusH125"},1.019)
             ({"HiggsVBF125"},1.021)); // TODO check value

  cb.cp()
     .process({"bbH"})
    .AddSyst(cb, "QCDScale_QshScale_bbH","lnN", SystMap<channel,bin_id,mass>::init
     ({"em","et","mt","tt"}, {8,10,12}, {"80"},   1.034)   
     ({"em","et","mt","tt"}, {9,11,13}, {"80"},   0.827)
     ({"em","et","mt","tt"}, {8,10,12}, {"90"},   1.036)   
     ({"em","et","mt","tt"}, {9,11,13}, {"90"},   0.835)
     ({"em","et","mt","tt"}, {8,10,12}, {"100"},  1.036)   
     ({"em","et","mt","tt"}, {9,11,13}, {"100"},  0.847)
     ({"em","et","mt","tt"}, {8,10,12}, {"110"},  1.038)   
     ({"em","et","mt","tt"}, {9,11,13}, {"110"},  0.853)
     ({"em","et","mt","tt"}, {8,10,12}, {"120"},  1.038) 
     ({"em","et","mt","tt"}, {9,11,13}, {"120"},  0.862)
     ({"em","et","mt","tt"}, {8,10,12}, {"130"},  1.04 )
     ({"em","et","mt","tt"}, {9,11,13}, {"130"},  0.867)
     ({"em","et","mt","tt"}, {8,10,12}, {"140"},  1.04 )  
     ({"em","et","mt","tt"}, {9,11,13}, {"140"},  0.872)
     ({"em","et","mt","tt"}, {8,10,12}, {"160"},  1.038)   
     ({"em","et","mt","tt"}, {9,11,13}, {"160"},  0.887)
     ({"em","et","mt","tt"}, {8,10,12}, {"180"},  1.038)   
     ({"em","et","mt","tt"}, {9,11,13}, {"180"},  0.897)
     ({"em","et","mt","tt"}, {8,10,12}, {"200"},  1.038)   
     ({"em","et","mt","tt"}, {9,11,13}, {"200"},  0.902)
     ({"em","et","mt","tt"}, {8,10,12}, {"250"},  1.035)   
     ({"em","et","mt","tt"}, {9,11,13}, {"250"},  0.922)
     ({"em","et","mt","tt"}, {8,10,12}, {"350"},  1.033)   
     ({"em","et","mt","tt"}, {9,11,13}, {"350"},  0.939)
     ({"em","et","mt","tt"}, {8,10,12}, {"400"},  1.036)   
     ({"em","et","mt","tt"}, {9,11,13}, {"400"},  0.934)
     ({"em","et","mt","tt"}, {8,10,12}, {"450"},  1.035)   
     ({"em","et","mt","tt"}, {9,11,13}, {"450"},  0.94 )
     ({"em","et","mt","tt"}, {8,10,12}, {"500"},  1.032)   
     ({"em","et","mt","tt"}, {9,11,13}, {"500"},  0.95 )
     ({"em","et","mt","tt"}, {8,10,12}, {"600"},  1.034)   
     ({"em","et","mt","tt"}, {9,11,13}, {"600"},  0.948)
     ({"em","et","mt","tt"}, {8,10,12}, {"700"},  1.034)   
     ({"em","et","mt","tt"}, {9,11,13}, {"700"},  0.952)
     ({"em","et","mt","tt"}, {8,10,12}, {"800"},  1.036)   
     ({"em","et","mt","tt"}, {9,11,13}, {"800"},  0.948)
     ({"em","et","mt","tt"}, {8,10,12}, {"900"},  1.036)   
     ({"em","et","mt","tt"}, {9,11,13}, {"900"},  0.95 )
     ({"em","et","mt","tt"}, {8,10,12}, {"1000"}, 1.037)   
     ({"em","et","mt","tt"}, {9,11,13}, {"1000"}, 0.949)
     ({"em","et","mt","tt"}, {8,10,12}, {"1200"}, 1.037)   
     ({"em","et","mt","tt"}, {9,11,13}, {"1200"}, 0.951)
     ({"em","et","mt","tt"}, {8,10,12}, {"1400"}, 1.034)   
     ({"em","et","mt","tt"}, {9,11,13}, {"1400"}, 0.957)
     ({"em","et","mt","tt"}, {8,10,12}, {"1600"}, 1.041)   
     ({"em","et","mt","tt"}, {9,11,13}, {"1600"}, 0.943)
     ({"em","et","mt","tt"}, {8,10,12}, {"1800"}, 1.037)   
     ({"em","et","mt","tt"}, {9,11,13}, {"1800"}, 0.952)
     ({"em","et","mt","tt"}, {8,10,12}, {"2000"}, 1.035)   
     ({"em","et","mt","tt"}, {9,11,13}, {"2000"}, 0.956)
     ({"em","et","mt","tt"}, {8,10,12}, {"2300"}, 1.035)   
     ({"em","et","mt","tt"}, {9,11,13}, {"2300"}, 0.956)
     ({"em","et","mt","tt"}, {8,10,12}, {"2600"}, 1.039)   
     ({"em","et","mt","tt"}, {9,11,13}, {"2600"}, 0.95 )
     ({"em","et","mt","tt"}, {8,10,12}, {"2900"}, 1.036)   
     ({"em","et","mt","tt"}, {9,11,13}, {"2900"}, 0.954)
     ({"em","et","mt","tt"}, {8,10,12}, {"3200"}, 1.034) 
     ({"em","et","mt","tt"}, {9,11,13}, {"3200"}, 0.957));

/*  cb.cp().process({"TTH_SM125"}).AddSyst(cb, "pdf_Higgs_ttH","lnN",
    SystMap<>::init(1.036));*/

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

  // cb.cp()
  //   .channel({"tt"})
  //   .process({signals})
  //   .AddSyst(cb, "arbitrary_flat_unc", "lnN", SystMap<>::init(1.10));

  // ##########################################################################
  // Uncertainty: Embedded events
  // References:
  // - https://twiki.cern.ch/twiki/bin/viewauth/CMS/TauTauEmbeddingSamples2016
  // Notes:
  // ##########################################################################

  // Embedded Normalization: No Lumi, Zjxsec information used, instead derived from data using dimuon selection efficiency

  // TTbar contamination in embedded events: 10% shape uncertainty of assumed ttbar->tautau event shape
  // cb.cp()
  //   .channel({"tt", "mt", "et"})
  //   .process({"Embedded"})
  //   .AddSyst(cb, "CMS_htt_emb_ttbar_", "shape", SystMap<>::init(1.00));

  // Uncertainty of hadronic tau track efficiency correction
  cb.cp()
    .channel({"tt", "mt", "et"})
    .process({"Embedded"})
    .AddSyst(cb, "Embedding_tracking_3prong_", "shape", SystMap<>::init(1.00));

  cb.cp()
    .channel({"tt", "mt", "et"})
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
      .channel({"tt", "mt", "et"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_norm_stat_$BIN", "lnN", SystMap<channel, bin_id>::init
  	       ({"tt"}, {8},  1.023) //btag
  	       ({"tt"}, {9}, 1.028) //nobtag
  	       ({"mt", "et"}, {8},  1.023) //btag tight
  	       ({"mt", "et"}, {9}, 1.028) //nobtag tight
  	       ({"mt", "et"}, {10},  1.023) //btag loose
  	       ({"mt", "et"}, {11}, 1.028) //nobtag loose
  	       );

  cb.cp()
      .channel({"tt", "mt", "et"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_norm_syst_$BIN", "lnN", SystMap<channel, bin_id>::init
  	       ({"tt"}, {8},  1.099) //btag
  	       ({"tt"}, {9}, 1.100) //nobtag
  	       ({"mt", "et"}, {8},  1.099) //btag tight
  	       ({"mt", "et"}, {9}, 1.100) //nobtag tight
  	       ({"mt", "et"}, {10},  1.099) //btag loose
  	       ({"mt", "et"}, {11}, 1.100) //nobtag loose
  	       );

  cb.cp()
      .channel({"tt", "mt", "et"})
      .process({"jetFakes"})
      .AddSyst(cb, "ff_sub_syst_$BIN", "lnN", SystMap<channel, bin_id>::init
  	       ({"tt"}, {8},  1.03) //btag
  	       ({"tt"}, {9}, 1.03) //nobtag
  	       ({"mt", "et"}, {8},  1.03) //btag tight
  	       ({"mt", "et"}, {9}, 1.03) //nobtag tight
  	       ({"mt", "et"}, {10},  1.03) //btag loose
  	       ({"mt", "et"}, {11}, 1.03) //nobtag loose
  	       );

  // cb.cp()
  //     .channel({"tt", "mt", "et"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_qcd_dm0_njet0_stat_", "shape", SystMap<>::init(1.00));
  // cb.cp()
  //     .channel({"tt", "mt", "et"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_qcd_dm0_njet1_stat_", "shape", SystMap<>::init(1.00));

  // // Shape syst. of different contr
  // cb.cp()
  //     .channel({"tt", "mt", "et"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_qcd_syst_", "shape", SystMap<>::init(1.00));

  // // Shape syst. in tautau due to using QCD FF also for W / tt
  // cb.cp()
  //     .channel({"tt", "mt", "et"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_w_syst_", "shape", SystMap<>::init(1.00));
  // cb.cp()
  //     .channel({"tt", "mt", "et"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_tt_syst_", "shape", SystMap<>::init(1.00));

  // cb.cp()
  //     .channel({"tt", "mt", "et"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_w_frac_syst_", "shape", SystMap<>::init(1.00));

  // cb.cp()
  //     .channel({"tt", "mt", "et"})
  //     .process({"jetFakes"})
  //     .AddSyst(cb, "ff_tt_frac_syst_", "shape", SystMap<>::init(1.00));
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
