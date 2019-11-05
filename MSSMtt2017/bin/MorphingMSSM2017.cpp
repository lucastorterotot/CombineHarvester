#include "CombineHarvester/CombinePdfs/interface/MorphFunctions.h"
#include "CombineHarvester/CombineTools/interface/Algorithm.h"
#include "CombineHarvester/CombineTools/interface/AutoRebin.h"
#include "CombineHarvester/CombineTools/interface/BinByBin.h"
#include "CombineHarvester/CombineTools/interface/CardWriter.h"
#include "CombineHarvester/CombineTools/interface/CombineHarvester.h"
#include "CombineHarvester/CombineTools/interface/Observation.h"
#include "CombineHarvester/CombineTools/interface/Process.h"
#include "CombineHarvester/CombineTools/interface/Systematics.h"
#include "CombineHarvester/CombineTools/interface/Utilities.h"
#include "CombineHarvester/MSSMtt2017/interface/HttSystematics_MSSMRun2.h"
#include "RooRealVar.h"
#include "RooWorkspace.h"
#include "TF1.h"
#include "TH2.h"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/program_options.hpp"
#include "boost/regex.hpp"
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <math.h>

using namespace std;
using boost::starts_with;
namespace po = boost::program_options;

int main(int argc, char **argv) {
  typedef vector<string> VString;
  typedef vector<pair<int, string>> Categories;
  using ch::syst::bin_id;
  using ch::JoinStr;

  // Define program options
  string output_folder = "mssm_tt";
  string base_path = string(getenv("CMSSW_BASE")) + "/src/CombineHarvester/MSSMtt2017/shapes";
  string input_folder_tt = "Lyon/";
  string chan = "tt";
  bool ggh_wg1 = true;
  bool auto_rebin = true;
  bool real_data = true;
  bool jetfakes = true;
  bool embedding = true;
  bool mod_dep = false;
  bool classic_bbb = true;
  bool binomial_bbb = false;
  bool verbose = false;
  // string stxs_signals = "stxs_stage0"; // "stxs_stage0" or "stxs_stage1"
  string categories = "MSSMtt"; // "stxs_stage0", "stxs_stage1" or "gof"
  string gof_category_name = "gof";
  po::variables_map vm;
  po::options_description config("configuration");
  config.add_options()
      ("base_path", po::value<string>(&base_path)->default_value(base_path))
      ("input_folder_tt", po::value<string>(&input_folder_tt)->default_value(input_folder_tt))
      ("channel", po::value<string>(&chan)->default_value(chan))
      ("auto_rebin", po::value<bool>(&auto_rebin)->default_value(auto_rebin))
      ("ggh_wg1", po::value<bool>(&ggh_wg1)->default_value(ggh_wg1))
      ("real_data", po::value<bool>(&real_data)->default_value(real_data))
      ("verbose", po::value<bool>(&verbose)->default_value(verbose))
      ("output_folder", po::value<string>(&output_folder)->default_value(output_folder))
      // ("stxs_signals", po::value<string>(&stxs_signals)->default_value(stxs_signals))
      ("categories", po::value<string>(&categories)->default_value(categories))
      ("gof_category_name", po::value<string>(&gof_category_name)->default_value(gof_category_name))
      ("jetfakes", po::value<bool>(&jetfakes)->default_value(jetfakes))
      ("embedding", po::value<bool>(&embedding)->default_value(embedding))
      ("mod_dep", po::value<bool>(&mod_dep)->default_value(mod_dep))
      ("classic_bbb", po::value<bool>(&classic_bbb)->default_value(classic_bbb))
    ("binomial_bbb", po::value<bool>(&binomial_bbb)->default_value(binomial_bbb));
  po::store(po::command_line_parser(argc, argv).options(config).run(), vm);
  po::notify(vm);

  // Define the location of the "auxiliaries" directory where we can
  // source the input files containing the datacard shapes
  std::map<string, string> input_dir;
  input_dir["tt"] = base_path + "/";

  // Define channels
  VString chns;
  if (chan.find("tt") != std::string::npos)
    chns.push_back("tt");

  string mass = "mA";

  if (mod_dep) { 
    mass = "mA"; 
  } else {
    mass = "MH"; 
  }

  RooRealVar mA(mass.c_str(), mass.c_str(), 90., 3200.);
  mA.setConstant(true);
  RooRealVar mH("mH", "mH", 90., 3200.);
  RooRealVar mh("mh", "mh", 90., 3200.);

  // Define background processes
  map<string, VString> bkg_procs;
  VString bkgs, bkgs_em;
  bkgs = {"W", "ZTT", "QCD", "ZL", "ZJ", "TTT", "TTL", "TTJ", "VVJ", "VVT", "VVL"};
  if(embedding){
    bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "ZTT"), bkgs.end());
    bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "TTT"), bkgs.end());
    bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "VVT"), bkgs.end());
    bkgs = JoinStr({bkgs,{"Embedded"}});
  }
  if(jetfakes){
    bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "QCD"), bkgs.end());
    bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "W"), bkgs.end());
    bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "VVJ"), bkgs.end());
    bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "TTJ"), bkgs.end());
    bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "ZJ"), bkgs.end());
    bkgs = JoinStr({bkgs,{"jetFakes"}});
  }
  // when jetfakes and embedded processes are : ZL, TTL, VVL

  std::cout << "[INFO] Considering the following processes:\n";
  if (chan.find("tt") != std::string::npos) {
    std::cout << "For tt channels : \n";
    for (unsigned int i=0; i < bkgs.size(); i++) std::cout << bkgs[i] << std::endl;
  }
  bkg_procs["tt"] = bkgs;

  // Define categories
  map<string, Categories> cats;
  // hack for MSSMtt2017
  if(categories == "MSSMtt"){
    cats["tt"] = {
        { 8, "tt_nobtag"},
        { 9, "tt_btag"},
	// { 7, "tt_inclusive"},
    };
  }
  else if(categories == "gof"){
    cats["et"] = {
        { 100, gof_category_name.c_str() },
    };
    cats["mt"] = {
        { 100, gof_category_name.c_str() },
    };
    cats["tt"] = {
        { 100, gof_category_name.c_str() },
    };
    cats["em"] = {
        { 100, gof_category_name.c_str() },
    };
  }
  else throw std::runtime_error("Given categorization is not known.");

  // Specify signal processes and masses
  vector<string> sig_procs = {"ggH","bbH"};

  vector<string> masses = {"110","120","130","180","250","300","400","450","600","800","900","1200","1400","2300","2600","2900"};//add 1800 for amcatnlo 80 and 90 empty!//{"100","110","120","130","140","180","200","250","300","400","450","600","700","800","900","1200","1400","1500","2300","2600","2900","3200"};
  // vector<string> masses = {"90","100","110","120","130","140","160","180", "200", "250", "350", "400", "450", "500", "600", "700", "800", "900","1000","1200","1400","1600","1800","2000","2300","2600","2900","3200"};

  map<string, VString> signal_types = {
    {"ggH", {"ggH"}},
    {"bbH", {"bbH"}}
  };
  if (mod_dep){
    signal_types = {
      {"ggH", {"ggh_htautau", "ggH_Htautau", "ggA_Atautau"}},
      {"bbH", {"bbh_htautau", "bbH_Htautau", "bbA_Atautau"}}
    };
  }
  // Create combine harverster object
  ch::CombineHarvester cb;

  // Add observations and processes

  for (auto chn : chns) {
    cb.AddObservations({"*"}, {"htt"}, {"13TeV"}, {chn}, cats[chn]);
    cb.AddProcesses({"*"}, {"htt"}, {"13TeV"}, {chn}, bkg_procs[chn], cats[chn],
                    false);
    cb.AddProcesses(masses, {"htt"}, {"13TeV"}, {chn}, signal_types["ggH"], cats[chn],
                    true);
    cb.AddProcesses(masses, {"htt"}, {"13TeV"}, {chn}, signal_types["bbH"], cats[chn],
                    true);
    // Add SM125 as background here
  }

  // Add systematics
  ch::AddMSSMRun2Systematics(cb, jetfakes, embedding, ggh_wg1, 2017);

  // Extract shapes from input ROOT files
  for (string chn : chns) {
    cb.cp().channel({chn}).backgrounds().ExtractShapes(
        input_dir[chn] + "htt_" + chn + ".inputs_datacards_mt_tot" + ".root",
        "$BIN/$PROCESS", "$BIN/$PROCESS_$SYSTEMATIC");
    if (mod_dep) {
      cb.cp().channel({chn}).process(signal_types["bbH"]).ExtractShapes(
	input_dir[chn] + "htt_" + chn + ".inputs_datacards_mt_tot" + ".root",
	"$BIN/bbH$MASS",
	"$BIN/bbH$MASS_$SYSTEMATIC");
      cb.cp().channel({chn}).process(signal_types["ggH"]).ExtractShapes(
	input_dir[chn] + "htt_" + chn + ".inputs_datacards_mt_tot" + ".root",
	"$BIN/ggH$MASS",
	"$BIN/ggH$MASS_$SYSTEMATIC");
    }
    else {
    cb.cp().channel({chn}).process(sig_procs).ExtractShapes(
        input_dir[chn] + "htt_" + chn + ".inputs_datacards_mt_tot" + ".root",
        "$BIN/$PROCESS$MASS", "$BIN/$PROCESS$MASS_$SYSTEMATIC");}
  }

  // Delete processes with 0 yield
  cb.FilterProcs([&](ch::Process *p) {
    bool null_yield = !(p->rate() > 0.0);
    if (null_yield) {
      std::cout << "[WARNING] Removing process with null yield: \n ";
      std::cout << ch::Process::PrintHeader << *p << "\n";
      cb.FilterSysts([&](ch::Systematic *s) {
        bool remove_syst = (MatchingProcess(*p, *s));
        return remove_syst;
      });
    }
    return null_yield;
  });

  // Delete systematics with 0 yield since these result in a bogus norm error in combine
  cb.FilterSysts([&](ch::Systematic *s) {
    if (s->type() == "shape") {
      if (s->shape_u()->Integral() == 0.0) {
        std::cout << "[WARNING] Removing systematic with null yield in up shift:" << std::endl;
        std::cout << ch::Systematic::PrintHeader << *s << "\n";
        return true;
      }
      if (s->shape_d()->Integral() == 0.0) {
        std::cout << "[WARNING] Removing systematic with null yield in down shift:" << std::endl;
        std::cout << ch::Systematic::PrintHeader << *s << "\n";
        return true;
      }
    }
    return false;
  });
  
  // int count_lnN = 0;
  // int count_all = 0;
  // cb.cp().ForEachSyst([&count_lnN, &count_all](ch::Systematic *s) {
  //   if (TString(s->name()).Contains("scale")||TString(s->name()).Contains("CMS_htt_boson_reso_met")){
  //     count_all++;
  //     double err_u = 0.0;
  //     double err_d = 0.0;
  //     int nbins = s->shape_u()->GetNbinsX();
  //     double yield_u = s->shape_u()->IntegralAndError(1,nbins,err_u);
  //     double yield_d = s->shape_d()->IntegralAndError(1,nbins,err_d);
  //     double value_u = s->value_u();
  //     double value_d = s->value_d();
  //     if (std::abs(value_u-1.0)+std::abs(value_d-1.0)<err_u/yield_u+err_d/yield_d){
  //         count_lnN++;
  //         std::cout << "[WARNING] Replacing systematic by lnN:" << std::endl;
  //         std::cout << ch::Systematic::PrintHeader << *s << "\n";
  //         s->set_type("lnN");
  //         bool up_is_larger = (value_u>value_d);
  //         if (value_u < 1.0) value_u = 1.0 / value_u;
  //         if (value_d < 1.0) value_d = 1.0 / value_d;
  //         if (up_is_larger){
  //             value_u = std::sqrt(value_u*value_d);
  //             value_d = 1.0 / value_u;
  //         }else{
  //             value_d = std::sqrt(value_u*value_d);
  //             value_u = 1.0 / value_d;
  //         }
  //         std::cout << "Former relative integral up shift: " << s->value_u() << "; New relative integral up shift: " << value_u << std::endl;
  //         std::cout << "Former relative integral down shift: " << s->value_d() << "; New relative integral down shift: " << value_d << std::endl;
  //         s->set_value_u(value_u);
  //         s->set_value_d(value_d);
  //     }
  //   }
  // });
  // std::cout << "[WARNING] Turned " << count_lnN << " of " << count_all << " checked systematics into lnN:" << std::endl;

  // Replacing observation with the sum of the backgrounds (Asimov data)
  // useful to be able to check this, so don't do the replacement
  // for these
  if (!real_data) {
    for (auto b : cb.cp().bin_set()) {
      std::cout << "[INFO] Replacing data with asimov in bin " << b << "\n";
      cb.cp().bin({b}).ForEachObs([&](ch::Observation *obs) {
        obs->set_shape(cb.cp().bin({b}).backgrounds().GetShape() +
                           cb.cp().bin({b}).signals().GetShape(),
                       true);
      });
    }
  }

  // // rebin categories to predefined binning

  

  // // Rebin background categories
  // for (auto b : cb.cp().bin_set()) {
  //   TString bstr = b;
  //   if (bstr.Contains("unrolled")) continue;
  //   std::cout << "[INFO] Rebin background bin " << b << "\n";
  //   auto shape = cb.cp().bin({b}).backgrounds().GetShape();
  //   auto min = shape.GetBinLowEdge(1);
  //   cb.cp().bin({b}).VariableRebin({min, 0.3, 0.4, 0.5, 0.6, 0.7, 1.0});
  // }
  // // Rebin ggh categories
  // for (auto b : cb.cp().bin_set()) {
  //   TString bstr = b;
  //   if (bstr.Contains("ggh_unrolled")) {
  //     std::cout << "[INFO] Rebin ggh signal bin " << b << "\n";
  //     auto shape = cb.cp().bin({b}).backgrounds().GetShape();
  //     auto min = shape.GetBinLowEdge(1);
  //     auto range = 1.0 - min;
  //     vector<double> raw_binning = {0.3, 0.4, 0.45, 0.5, 0.55, 0.6, 0.7, 1.0};
  //     vector<double> binning = {min};
  //     int n_stage1cats = 9;
  //     if (bstr.Contains("et_")||bstr.Contains("mt_")) n_stage1cats = 7;
  //     for (int i=0; i<n_stage1cats; i++){
  //       for (auto border : raw_binning) {
  //         binning.push_back(i*range+border);
  //       }
  //     }
  //     cb.cp().bin({b}).VariableRebin(binning);
  //   }
  // }
  // // Rebin qqh categories
  // for (auto b : cb.cp().bin_set()) {
  //   TString bstr = b;
  //   if (bstr.Contains("qqh_unrolled")) {
  //     std::cout << "[INFO] Rebin qqh signal bin " << b << "\n";
  //     auto shape = cb.cp().bin({b}).backgrounds().GetShape();
  //     auto min = shape.GetBinLowEdge(1);
  //     auto range = 1.0 - min;
  //     vector<double> raw_binning = {0.4, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.90, 0.95, 1.0};
  //     vector<double> binning = {min};
  //     for (int i=0; i<5; i++){
  //       for (auto border : raw_binning) {
  //         binning.push_back(i*range+border);
  //       }
  //     }
  //     cb.cp().bin({b}).VariableRebin(binning);
  //   }
  // }

  // At this point we can fix the negative bins
  std::cout << "[INFO] Fixing negative bins.\n";
  cb.ForEachProc([](ch::Process *p) {
    if (ch::HasNegativeBins(p->shape())) {
      auto newhist = p->ClonedShape();
      ch::ZeroNegativeBins(newhist.get());
      p->set_shape(std::move(newhist), false);
    }
  });

  cb.ForEachSyst([](ch::Systematic *s) {
    if (s->type().find("shape") == std::string::npos)
      return;
    if (ch::HasNegativeBins(s->shape_u()) ||
        ch::HasNegativeBins(s->shape_d())) {
      auto newhist_u = s->ClonedShapeU();
      auto newhist_d = s->ClonedShapeD();
      ch::ZeroNegativeBins(newhist_u.get());
      ch::ZeroNegativeBins(newhist_d.get());
      s->set_shapes(std::move(newhist_u), std::move(newhist_d), nullptr);
    }
  });

  auto rebin = ch::AutoRebin()
    .SetBinThreshold(0.)
    .SetBinUncertFraction(0.9)
    .SetRebinMode(1)
    .SetPerformRebin(true)
    .SetVerbosity(1);
  if(auto_rebin) rebin.Rebin(cb, cb);
  
  // // Perform auto-rebinning
  // if (auto_rebin) {
  //   const auto threshold = 1.0;
  //   const auto tolerance = 1e-4;
  //   for (auto b : cb.cp().bin_set()) {
  //     std::cout << "[INFO] Rebin bin " << b << "\n";
  //     // Get shape of this category with sum of backgrounds
  //     auto shape = cb.cp().bin({b}).backgrounds().GetShape();
  //     // Push back last bin edge
  //     vector<double> binning;
  //     const auto num_bins = shape.GetNbinsX();
  //     binning.push_back(shape.GetBinLowEdge(num_bins + 1));
  //     // Now, go backwards through bins (from right to left) and merge a bin if
  //     // the background yield is below a given threshold.
  //     auto offset = shape.GetBinLowEdge(1);
  //     auto width = 1.0 - offset;
  //     auto c = 0.0;
  //     for(auto i = num_bins; i > 0; i--) {
  //       // Determine whether this is a boundary of an unrolled category
  //       // if it's a multiple of the width between minimum NN score and 1.0.
  //       auto low_edge = shape.GetBinLowEdge(i);
  //       auto is_boundary = fabs(fmod(low_edge - offset, width)) < tolerance ? true : false;
  //       c += shape.GetBinContent(i);
  //       if (is_boundary) { // If the lower edge is a boundary, set a bin edge.
  //         if (c <= threshold && !(fabs(fmod(binning[0] - offset, width)) < tolerance || fabs(fmod(binning[0] - offset, width)) - width < tolerance)) { // Special case: If this bin is at a boundary but it is below the threshold and the bin above is not again a boundary, merge to the right.
  //           binning.erase(binning.begin());
  //         }
  //         binning.insert(binning.begin(), low_edge);
  //         c = 0.0;
  //       } else { // If this is not a boundary, check whether the content is above the threshold.
  //         if (c > threshold) { // Set lower edge if the bin content is above the threshold.
  //           binning.insert(binning.begin(), low_edge);
  //           c = 0.0;
  //         }
  //       }
  //     }
  //     cb.cp().bin({b}).VariableRebin(binning);
  //   }
  //   // blind subcategories with to little events
  //   for (auto b : cb.cp().bin_set()) {
  //     // Get shape of this category with sum of backgrounds
  //     auto shape = cb.cp().bin({b}).backgrounds().GetShape();
  //     const auto num_bins = shape.GetNbinsX();
  //     for(auto i = num_bins; i > 0; i--) {
  //       if(shape.GetBinContent(i) < threshold){
  //         std::cout << "[INFO] Blind bin " << i << " in " << b << " due to insufficient population!" << "\n";
  //         cb.cp().bin({b}).ForEachProc([i](ch::Process *p) {
  //           auto newhist = p->ClonedShape();
  //           newhist->SetBinContent(i, 0.0);
  //           newhist->SetBinError(i, 0.0);
  //           p->set_shape(std::move(newhist), false);
  //         });
  //         cb.cp().bin({b}).ForEachObs([i](ch::Observation *p) {
  //           auto newhist = p->ClonedShape();
  //           newhist->SetBinContent(i, 0.0);
  //           newhist->SetBinError(i, 0.0);
  //           p->set_shape(std::move(newhist), false);
  //         });
  //         cb.cp().bin({b}).ForEachSyst([i](ch::Systematic *s) {
  //           if (s->type().find("shape") == std::string::npos)
  //               return;
  //           auto newhist_u = s->ClonedShapeU();
  //           auto newhist_d = s->ClonedShapeD();
  //           newhist_u->SetBinContent(i, 0.0);
  //           newhist_u->SetBinError(i, 0.0);
  //           newhist_d->SetBinContent(i, 0.0);
  //           newhist_d->SetBinError(i, 0.0);
  //           s->set_shapes(std::move(newhist_u), std::move(newhist_d), nullptr);
  //         });
  //       }
  //     }
  //   }
  // }

  // Merge bins and set bin-by-bin uncertainties if no autoMCStats is used.
  if (classic_bbb) {
    auto bbb = ch::BinByBinFactory()
      .SetAddThreshold(0.0)
      .SetMergeThreshold(0.5)
      .SetFixNorm(false);
    bbb.MergeBinErrors(cb.cp().backgrounds());
    bbb.AddBinByBin(cb.cp().backgrounds(), cb);
    }
////if (binomial_bbb) {
////  auto bbb = ch::BinByBinFactory()
////                 .SetPattern("CMS_$ANALYSIS_$CHANNEL_$BIN_$ERA_$PROCESS_binomial_bin_$#")
////                 .SetBinomialP(0.022)
////                 .SetBinomialErrors(true)
////                 .SetBinomialN(1000.0)
////                 .SetFixNorm(false);
////  bbb.AddBinByBin(cb.cp().backgrounds(), cb);
////}


  // This function modifies every entry to have a standardised bin name of
  // the form: {analysis}_{channel}_{bin_id}_{era}
  ch::SetStandardBinNames(cb, "$ANALYSIS_$CHANNEL_$BINID_$ERA");

  RooWorkspace ws("htt", "htt");
 
  TFile demo("htt_mssm_demo.root", "RECREATE");


  map<string, RooAbsReal *> mass_var = {
    {"ggh_htautau", &mh}, {"ggH_Htautau", &mH}, {"ggA_Atautau", &mA},
    {"bbh_htautau", &mh}, {"bbH_Htautau", &mH}, {"bbA_Atautau", &mA}
  };

  if (!mod_dep) {
    mass_var = {
      {"ggH", &mA}, {"bbH", &mA}
  };
  }


  std::string norm = "norm";
  auto bins = cb.bin_set();
  for (auto b : bins) {
    auto procs = cb.cp().bin({b}).process(ch::JoinStr({signal_types["ggH"], signal_types["bbH"]})).process_set();
    for (auto p : procs) {
      // for (auto t : signal_types["ggH"]) {std::cout<<t<<std::endl;}
      // for (auto t : signal_types["bbH"]) {std::cout<<t<<std::endl;}
      std::string pdf_name = ch::BuildRooMorphing(ws, cb, b, p, *(mass_var[p]), norm, true, false, false, &demo);
    }
  }
  demo.Close();
  cb.AddWorkspace(ws);
  cb.cp().process(ch::JoinStr({signal_types["ggH"], signal_types["bbH"]})).ExtractPdfs(cb, "htt", "$BIN_$PROCESS_morph");


  // Write out datacards. Naming convention important for rest of workflow. We
  // make one directory per chn-cat, one per chn and cmb. In this code we only
  // store the individual datacards for each directory to be combined later.
  string output_prefix = "output/";
  ch::CardWriter writer(output_prefix + output_folder + "/$TAG/$BIN.txt",
                        output_prefix + output_folder +
                            "/$TAG/htt_input_" + "2017" + ".root");

  // We're not using mass as an identifier - which we need to tell the
  // CardWriter
  // otherwise it will see "*" as the mass value for every object and skip it
  writer.SetWildcardMasses({});

  // Set verbosity
  if (verbose)
    writer.SetVerbosity(1);

  // Write datacards combined and per channel
  // writer.WriteCards("cmb", cb);

  for (auto chn : chns) {
    writer.WriteCards(chn, cb.cp().channel({chn}));
  }

  if (verbose)
    cb.PrintAll();

  cout << "[INFO] Done producing datacards.\n";
}
