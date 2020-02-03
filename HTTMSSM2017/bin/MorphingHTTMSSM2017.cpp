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
#include "CombineHarvester/HTTMSSM2017/interface/HttSystematics_MSSMRun2.h"
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

template <typename T>
void To1Bin(T* proc)
{
    std::unique_ptr<TH1> originalHist = proc->ClonedScaledShape();
    TH1F *hist = new TH1F("hist","hist",1,0,1);
    double err = 0;
    double rate =
        originalHist->IntegralAndError(0, originalHist->GetNbinsX() + 1, err);
    hist->SetDirectory(0);
    hist->SetBinContent(1, rate);
    hist->SetBinError(1, err);
    proc->set_shape(*hist, true);  // True means adjust the process rate to the
                                   // integral of the hist
}

//Treatment is different for single bin control regions than for multi-bin cr's
//Introduce a BinIsSBControlRegion to filter out the single bin cr's and
//Let BinIsControlRegion filter all control regions

bool BinIsSBControlRegion(ch::Object const* obj)
{
    return ((boost::regex_search(obj->bin(),boost::regex{"_cr$"})&&(obj->channel()!=std::string("ttbar"))) || (obj->channel() == std::string("zmm")));
}

// Useful to have the inverse sometimes too
bool BinIsNotSBControlRegion(ch::Object const* obj)
{
    return !BinIsSBControlRegion(obj);
}

//BinIsControlRegion filters classic method cr's, z->mumu and ttbar control region
bool BinIsControlRegion(ch::Object const* obj)
{
    return (boost::regex_search(obj->bin(),boost::regex{"_cr$"}) || (obj->channel() == std::string("zmm")));
}

bool BinIsNotControlRegion(ch::Object const* obj)
{
    return !BinIsControlRegion(obj);
}


int main(int argc, char** argv) {
  typedef vector<string> VString;
  typedef vector<pair<int, string>> Categories;
  using ch::syst::bin_id;
  using ch::JoinStr;

  // Define program options
  string chan = "mt";
  string output_folder_base = "HTT_MSSM_2017";
  string base_path = string(getenv("CMSSW_BASE")) + "/src/CombineHarvester/HTTMSSM2017/shapes";
  string input_folder_tt = "Lyon_tt/";
  string input_folder_mt = "Lyon_mt/";
  string input_folder_et = "Lyon_et/";
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
  string categories = "HTTMSSM"; // "stxs_stage0", "stxs_stage1" or "gof"
  string gof_category_name = "gof";
  po::variables_map vm;
  po::options_description config("configuration");
  config.add_options()
      ("base_path", po::value<string>(&base_path)->default_value(base_path))
      ("input_folder_tt", po::value<string>(&input_folder_tt)->default_value(input_folder_tt))
      ("input_folder_mt", po::value<string>(&input_folder_mt)->default_value(input_folder_mt))
      ("input_folder_et", po::value<string>(&input_folder_et)->default_value(input_folder_et))
      ("channel", po::value<string>(&chan)->default_value(chan))
      ("auto_rebin", po::value<bool>(&auto_rebin)->default_value(auto_rebin))
      ("ggh_wg1", po::value<bool>(&ggh_wg1)->default_value(ggh_wg1))
      ("real_data", po::value<bool>(&real_data)->default_value(real_data))
      ("verbose", po::value<bool>(&verbose)->default_value(verbose))
      ("output_folder", po::value<string>(&output_folder_base)->default_value(output_folder_base))
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

  string output_folder = output_folder_base+"_"+chan;



  // First define the location of the "auxiliaries" directory where we can
  // source the input files containing the datacard shapes
  std::map<string, string> input_dir;
  input_dir["tt"] = base_path + "/" + input_folder_tt + "/" ;
  input_dir["mt"] = base_path + "/" + input_folder_mt + "/" ;
  input_dir["et"] = base_path + "/" + input_folder_et + "/" ;

  // Define channels
  VString chns;
  if ( chan.find("tt") != std::string::npos ) chns.push_back("tt");
  if ( chan.find("mt") != std::string::npos ) chns.push_back("mt");
  if ( chan.find("et") != std::string::npos ) chns.push_back("et");
  if ( chan=="all" ) chns = {"tt","mt","et"};

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
    if ( chan.find("tt") != std::string::npos ) bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "ZTT"), bkgs.end());
    if ( chan.find("tt") != std::string::npos ) bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "TTT"), bkgs.end());
    if ( chan.find("tt") != std::string::npos ) bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "VVT"), bkgs.end());
    bkgs = JoinStr({bkgs,{"Embedded"}});
  }
  if(jetfakes){
    bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "QCD"), bkgs.end());
    if ( chan.find("tt") != std::string::npos ) bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "W"), bkgs.end());
    if ( chan.find("tt") != std::string::npos ) bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "VVJ"), bkgs.end());
    if ( chan.find("tt") != std::string::npos ) bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "TTJ"), bkgs.end());
    if ( chan.find("mt") != std::string::npos ) bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "ZJ"), bkgs.end());
    if ( chan.find("et") != std::string::npos ) bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "ZJ"), bkgs.end());
    bkgs = JoinStr({bkgs,{"jetFakes"}});
  }

  if ( chan.find("tt") == std::string::npos ) bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "VVL"), bkgs.end());
  if ( chan.find("tt") == std::string::npos ) bkgs.erase(std::remove(bkgs.begin(), bkgs.end(), "TTL"), bkgs.end());

  std::cout << "[INFO] Considering the following processes:\n";
  if (chan.find("tt") != std::string::npos) {
    std::cout << "For tt channels : \n";
    for (unsigned int i=0; i < bkgs.size(); i++) std::cout << bkgs[i] << std::endl;
  }
  if (chan.find("mt") != std::string::npos) {
    std::cout << "For mt channels : \n";
    for (unsigned int i=0; i < bkgs.size(); i++) std::cout << bkgs[i] << std::endl;
  }
  if (chan.find("et") != std::string::npos) {
    std::cout << "For et channels : \n";
    for (unsigned int i=0; i < bkgs.size(); i++) std::cout << bkgs[i] << std::endl;
  }
  bkg_procs["tt"] = bkgs;
  bkg_procs["mt"] = bkgs;
  bkg_procs["et"] = bkgs;

  // Define categories
  map<string, Categories> cats;
  if(categories == "HTTMSSM"){
    cats["tt"] = {
       { 8, "tt_nobtag"},
       { 9, "tt_btag"},
      // { 7, "tt_inclusive"},
    };
    cats["mt"] = {
      // {8, "mt_nobtag_tight"},
      // {9, "mt_btag_tight"},
      // {10, "mt_nobtag_loosemt"},
      // {11, "mt_btag_loosemt"},
      { 7, "mt_inclusive"},
    };
    cats["et"] = {
      // {8, "et_nobtag_tight"},
      // {9, "et_btag_tight"},
      // {10, "et_nobtag_loosemt"},
      // {11, "et_btag_loosemt"},
      { 7, "et_inclusive"},
    };
  }
  else if(categories == "gof"){
    cats["tt"] = {
      { 100, gof_category_name.c_str() },
    };
    cats["mt"] = {
      { 100, gof_category_name.c_str() },
    };
    cats["et"] = {
      { 100, gof_category_name.c_str() },
    };
  }
  else throw std::runtime_error("Given categorization is not known.");

  // Specify signal processes and masses
  vector<string> sig_procs = {"ggH","bbH"};

  vector<string> masses = {"80","90","110","120","130","180","250","300","400","450","600","800","900","1200","1400","1800","2000","2300","2600","2900"};//{"110","120","130","180","250","300","400","450","600","800","900","1200","1400","2300","2600","2900"};//add 1800 for amcatnlo 80 and 90 empty!//{"100","110","120","130","140","180","200","250","300","400","450","600","700","800","900","1200","1400","1500","2300","2600","2900","3200"};,"800","450",

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

//   VString SM_procs = {"ggH_SM125", "qqH_SM125", "ZH_SM125", "WminusH_SM125","WplusH_SM125"};

//   //Example - could fill this map with hardcoded binning for different
//   //categories if manual_rebin is turned on
//   map<string, vector<double> > binning;
//   binning["et_nobtag_tight"] = {0,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,225,250,275,300,325,350,400,500,700,900,1100,3900};
//   binning["et_nobtag_loosemt"] = {0,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,225,250,275,300,325,350,400,500,700,900,3900};
//   binning["et_btag_loosemt"] = {0,60,80,100,120,140,160,180,200,250,300,350,400,500,3900};
//   binning["et_btag_tight"] = {0,20,40,60,80,100,120,140,160,180,200,250,300,350,400,500,3900};
//   binning["mt_nobtag_tight"] = {0,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,225,250,275,300,325,350,400,500,700,900,3900};
//   binning["mt_nobtag_loosemt"] = {0,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,225,250,275,300,325,350,400,500,700,3900};
//   binning["mt_btag_tight"] = {0,20,40,60,80,100,120,140,160,180,200,250,300,350,400,500,700,3900};
//   binning["mt_btag_loosemt"] = {0,60,80,100,120,140,160,180,200,250,300,350,400,500,700,3900};
//   binning["tt_nobtag"] = {0,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,225,250,275,300,325,350,400,500,700,900,3900};
//   binning["tt_btag"] = {0,20,40,60,80,100,120,140,160,180,200,250,300,350,400,500,700,900,3900};
//   binning["em_nobtag_lowPzeta"] = {0,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,225,250,275,300,325,350,400,500,700,900,4000};
//   binning["em_nobtag_mediumPzeta"] = {0,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,225,250,275,300,325,350,400,500,700,900,4000};
//   binning["em_nobtag_highPzeta"] = {0,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,225,250,275,300,325,350,400,500,700,900,4000};
//   binning["ttbar_cr"] = {0,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,225,250,275,300,325,350,400,500,700,900,4000};
//   binning["em_btag_lowPzeta"] = {0,20,40,60,80,100,120,140,160,180,200,250,300,350,400,500,700,4000};
//   binning["em_btag_mediumPzeta"] = {0,20,40,60,80,100,120,140,160,180,200,250,300,350,400,500,700,4000};
//   binning["em_btag_highPzeta"] = {0,20,40,60,80,100,120,140,160,180,200,250,300,350,400,500,700,4000};
// /*  binning["zmm_nobtag"] = {60,70,80,90,100,110,120};
//   binning["zmm_btag"] = {60,70,80,90,100,110,120};*/

  // Create an empty CombineHarvester instance that will hold all of the
  // datacard configuration and histograms etc.
  ch::CombineHarvester cb;
  // Uncomment this next line to see a *lot* of debug information
  // cb.SetVerbosity(3);

  // if (control_region > 0){
  //     // for each channel use the categories >= 10 for the control regions
  //     // the control regions are ordered in triples (10,11,12),(13,14,15)...
  //     for (auto chn : chns){
  //       // for em or tt do nothing
  //       if (ch::contains({"em", "tt", "zmm","ttbar"}, chn)) {
  //         std::cout << " - Skipping extra control regions for channel " << chn << "\n";
  //         continue;
  //       }
  //       // if ( chn == "em") continue;
  //       Categories queue;
  //       int binid = 14;
  //       for (auto cat:cats[chn+"_13TeV"]){
  //         if(binid != 23 && binid!=20){ //Fairly shit way of dropping no b-tag loose mT wjets CR
  //           queue.push_back(make_pair(binid,cat.second+"_wjets_cr"));
  //         }
  //         queue.push_back(make_pair(binid+1,cat.second+"_qcd_cr"));
  //         if(binid != 23 && binid!=20){ //Fairly shit way of dropping no b-tag loose mT wjets ss CR
  //           queue.push_back(make_pair(binid+2,cat.second+"_wjets_ss_cr"));
  //         }
  //         binid += 3;
  //       }
  //       cats[chn+"_13TeV"].insert(cats[chn+"_13TeV"].end(),queue.begin(),queue.end());
  //     }
  // }

  // // List the mappings from the NLO gluon fusion process names to the histogram
  // // names in the shape files
  // map<string, string> gg_shapes = {
  //   {"gght_htautau", "ggh_t"}, {"gghb_htautau", "ggh_b"}, {"gghi_htautau", "ggh_i"},
  //   {"ggHt_Htautau", "ggH_t"}, {"ggHb_Htautau", "ggH_b"}, {"ggHi_Htautau", "ggH_i"},
  //   {"ggAt_Atautau", "ggA_t"}, {"ggAb_Atautau", "ggA_b"}, {"ggAi_Atautau", "ggA_i"},
  //   {"ggHt", "gg"+mod_indep_h+"_t"},
  //   {"ggHb", "gg"+mod_indep_h+"_b"},
  //   {"ggHi", "gg"+mod_indep_h+"_i"},
  //   {"ggH", "ggH"}  // old LO case
  // };

  // if(ggHatNLO){
  //   signal_types = {
  //     {"ggH", {"gght_htautau", "gghb_htautau", "gghi_htautau", "ggHt_Htautau", "ggHb_Htautau", "ggHi_Htautau", "ggAt_Atautau", "ggAb_Atautau", "ggAi_Atautau"}},
  //     {"bbH", {"bbh_htautau", "bbH_Htautau", "bbA_Atautau"}}
  //   };
  // }
  // if(mass=="MH"){
  //   if(ggHatNLO){
  //     signal_types = {
  //       {"ggH", {"ggHt", "ggHb", "ggHi"}},
  //       {"bbH", {"bbH"}}
  //     };
  //   }else{
  //     signal_types = {
  //       {"ggH", {"ggH"}},
  //       {"bbH", {"bbH"}}
  //     };
  //   }
  // }



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
    // if(SM125==string("bkg_SM125") && chn!="zmm") cb.AddProcesses({"*"}, {"htt"}, {"13TeV"}, {chn}, SM_procs, cats[chn+"_13TeV"], false);
    // if(SM125==string("signal_SM125") && chn!="zmm") cb.AddProcesses({"*"}, {"htt"}, {"13TeV"}, {chn}, SM_procs, cats[chn+"_13TeV"], true);
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


//   if(SM125!=string("")) {
//      cb.cp().process(SM_procs).ForEachProc([&](ch::Process * proc) {
//        proc->set_rate(proc->rate()*0.0627);
//       });
//      cb.cp().process({"ggH_SM125"}).ForEachProc([&](ch::Process *proc){
//        proc->set_rate(proc->rate()*48.58);
//      });
//      cb.cp().process({"qqH_SM125"}).ForEachProc([&](ch::Process *proc){
//        proc->set_rate(proc->rate()*3.782);
//      });
//      cb.cp().process({"WplusH_SM125"}).ForEachProc([&](ch::Process *proc){
//        proc->set_rate(proc->rate()*0.8400);
//      });
//      cb.cp().process({"WminusH_SM125"}).ForEachProc([&](ch::Process *proc){
//        proc->set_rate(proc->rate()*0.5328);
//      });
//      cb.cp().process({"ZH_SM125"}).ForEachProc([&](ch::Process *proc){
//        proc->set_rate(proc->rate()*0.8839);
//      });
// /*     cb.cp().process({"TTH_SM125"}).ForEachProc([&](ch::Process *proc){
//        proc->set_rate(proc->rate()*0.5071);
//      });*/
//   }

//   // And convert any shapes in the CRs to lnN:
//   // Convert all shapes to lnN at this stage
//   cb.cp().FilterSysts(BinIsNotSBControlRegion).syst_type({"shape"}).ForEachSyst([](ch::Systematic *sys) {
//     sys->set_type("lnN");
//   });

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


  // // Merge to one bin for control region bins
  // cb.cp().FilterAll(BinIsNotSBControlRegion).ForEachProc(To1Bin<ch::Process>);
  // cb.cp().FilterAll(BinIsNotSBControlRegion).ForEachObs(To1Bin<ch::Observation>);

  // // Rebinning
  // // --------------------
  // // Keep track of shapes before and after rebinning for comparison
  // // and for checking the effect of negative bin contents
  // std::map<std::string, TH1F> before_rebin;
  // std::map<std::string, TH1F> after_rebin;
  // std::map<std::string, TH1F> after_rebin_neg;
  // if (check_neg_bins) {
  //   for (auto b : bins) {
  //     before_rebin[b] = cb.cp().bin({b}).backgrounds().GetShape();
  //   }
  // }


  // auto rebin = ch::AutoRebin()
  //   .SetBinThreshold(0.)
  //   .SetBinUncertFraction(0.9)
  //   .SetRebinMode(1)
  //   .SetPerformRebin(true)
  //   .SetVerbosity(1);
  // if(auto_rebin) rebin.Rebin(cb, cb);

  // if(manual_rebin) {
  //   for(auto b : cb.cp().FilterAll(BinIsSBControlRegion).bin_set()) {
  //     std::cout << "Rebinning by hand for bin: " << b <<  std::endl;
  //     cb.cp().bin({b}).VariableRebin(binning[b]);
  //   }
  // }

  // if (check_neg_bins) {
  //   for (auto b : bins) {
  //     after_rebin[b] = cb.cp().bin({b}).backgrounds().GetShape();
  //     // std::cout << "Bin: " << b << " (before)\n";
  //     // before_rebin[b].Print("range");
  //     // std::cout << "Bin: " << b << " (after)\n";
  //     // after_rebin[b].Print("range");
  //     // Build a sum-of-bkgs TH1 that doesn't truncate the negative yields
  //     // like the CH GetShape does
  //     for (auto p : cb.cp().bin({b}).backgrounds().process_set()) {
  //       TH1F proc_hist;
  //       cb.cp().bin({b}).process({p}).ForEachProc([&](ch::Process *proc) {
  //         proc_hist = proc->ShapeAsTH1F();
  //         proc_hist.Scale(proc->no_norm_rate());
  //         for (int i = 1; i <= proc_hist.GetNbinsX(); ++i) {
  //           if (proc_hist.GetBinContent(i) < 0.) {
  //             std::cout << p << " bin " << i << ": " << proc_hist.GetBinContent(i) << "\n";
  //           }
  //         }
  //       });
  //       if (after_rebin_neg.count(b)) {
  //         after_rebin_neg[b].Add(&proc_hist);
  //       } else {
  //         after_rebin_neg[b] = proc_hist;
  //       }
  //     }
  //     std::cout << "Bin: " << b << "\n";
  //     for (int i = 1; i <= after_rebin[b].GetNbinsX(); ++i) {
  //       double offset = after_rebin[b].GetBinContent(i) - after_rebin_neg[b].GetBinContent(i);
  //       double offset_by_yield = offset / after_rebin[b].GetBinContent(i);
  //       double offset_by_err = offset / after_rebin[b].GetBinError(i);
  //       printf("%-2i offset %-10.4f tot %-10.4f err %-10.4f off/tot %-10.4f off/err %-10.4f\n", i , offset, after_rebin[b].GetBinContent(i), after_rebin[b].GetBinError(i), offset_by_yield, offset_by_err);
  //     }
  //   }
  // }

  // // Uncomment this to inject 1 obs event in the last bin of every signal-region
  // // category
  // // if(!real_data){
  // //     for (auto b : cb.cp().FilterAll(BinIsControlRegion).bin_set()) {
  // //       std::cout << " - Adjusting data in bin " << b << "\n";
  // //         cb.cp().bin({b}).ForEachObs([&](ch::Observation *obs) {
  // //           TH1F new_obs = cb.cp().bin({b}).GetObservedShape();
  // //           new_obs.SetBinContent(new_obs.GetNbinsX(), 1.);
  // //           new_obs.Print("range");
  // //           obs->set_shape(new_obs, true);
  // //         });
  // //       }
  // // }

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

  cout << "Generating bbb uncertainties...";
  auto bbb = ch::BinByBinFactory()
    .SetPattern("CMS_$ANALYSIS_$BIN_$ERA_$PROCESS_bin_$#")
    .SetAddThreshold(0.)
    .SetMergeThreshold(0.4)
    .SetFixNorm(true)
    .SetMergeSaturatedBins(false)
    .SetPoissonErrors(true);
  for (auto chn : chns) {
    std::cout << " - Doing bbb for channel " << chn << "\n";
    bbb.MergeAndAdd(cb.cp().channel({chn}).backgrounds().FilterAll([](ch::Object const* obj) {
                return BinIsSBControlRegion(obj);
                }), cb);
  }
  // And now do bbb for the control region with a slightly different config:
  auto bbb_ctl = ch::BinByBinFactory()
    .SetPattern("CMS_$ANALYSIS_$BIN_$ERA_$PROCESS_bin_$#")
    .SetAddThreshold(0.)
    .SetMergeThreshold(0.4)
    .SetFixNorm(false)  // contrary to signal region, bbb *should* change yield here
    .SetVerbosity(1);
  // Will merge but only for non W and QCD processes, to be on the safe side
  bbb_ctl.MergeBinErrors(cb.cp().process({"QCD", "W"}, false).FilterProcs(BinIsNotSBControlRegion));
  bbb_ctl.AddBinByBin(cb.cp().process({"QCD", "W"}, false).FilterProcs(BinIsNotSBControlRegion), cb);
  cout << " done\n";

  // if(postfit_plot){
  //   cb.ForEachSyst([](ch::Systematic *s) {
  //     if (s->type().find("shape") == std::string::npos) return;
  //     if (s->name().find("CMS_htt_tt_btag_13TeV_ZTT_bin_18") == std::string::npos) return;
  //       std::cout<<"Adjusting down shape for nuisance "<<s->name()<<std::endl;
  //       auto newhist_u = s->ClonedShapeU();
  //       auto newhist_d = s->ClonedShapeD();
  //       newhist_d.get()->SetBinContent(18,0.0003);
  //       // Set the new shape but do not change the rate, we want the rate to still
  //       // reflect the total integral of the events
  //       s->set_shapes(std::move(newhist_u), std::move(newhist_d), nullptr);
  //   });
  // }


  // //Switch JES over to lnN:
  // /*cb.cp().syst_name({"CMS_scale_j_13TeV"}).ForEachSyst([](ch::Systematic *sys) { sys->set_type("lnN");});
  // cb.cp().syst_name({"CMS_scale_b_13TeV"}).ForEachSyst([](ch::Systematic *sys) { sys->set_type("lnN");});
  // cb.cp().syst_name({"CMS_fake_b_13TeV"}).ForEachSyst([](ch::Systematic *sys) { sys->set_type("lnN");});*/

  // //Need to rename norm_ff_tt_dm0_njet0_stat and norm_ff_tt_dm1_njet0_stat to contain the channel directives
  // cb.cp().channel({"mt"}).RenameSystematic(cb,"norm_ff_tt_dm0_njet0_stat","norm_ff_tt_dm0_mt_stat");
  // cb.cp().channel({"mt"}).RenameSystematic(cb,"norm_ff_tt_dm1_njet0_stat","norm_ff_tt_dm1_mt_stat");
  // cb.cp().channel({"et"}).RenameSystematic(cb,"norm_ff_tt_dm0_njet0_stat","norm_ff_tt_dm0_et_stat");
  // cb.cp().channel({"et"}).RenameSystematic(cb,"norm_ff_tt_dm1_njet0_stat","norm_ff_tt_dm1_et_stat");


  
  // if (classic_bbb) {
  //   auto bbb = ch::BinByBinFactory()
  //     .SetAddThreshold(0.0)
  //     .SetMergeThreshold(0.5)
  //     .SetFixNorm(false);
  //   bbb.MergeBinErrors(cb.cp().backgrounds());
  //   bbb.AddBinByBin(cb.cp().backgrounds(), cb);
  //   }
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
  // which is commonly used in the htt analyses
  ch::SetStandardBinNames(cb, "$ANALYSIS_$CHANNEL_$BINID_$ERA");

  // First we generate a set of bin names:
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

  // if (do_morphing) {
  //   std::string norm = "norm";
  //   // For model-independent NLO we will add an extra normalisation term
  //   if (mass == "MH" && ggHatNLO) {
  //     if (mod_indep_use_sm) {
  //       TFile f_sm(sm_gg_fractions.c_str());
  //       RooWorkspace *w_sm = (RooWorkspace*)f_sm.Get("w");
  //       w_sm->var(("m"+mod_indep_h).c_str())->SetName("MH");
  //       RooAbsReal *t_frac = w_sm->function(("gg" + mod_indep_h + "_t_SM_frac").c_str());
  //       RooAbsReal *b_frac = w_sm->function(("gg" + mod_indep_h + "_b_SM_frac").c_str());
  //       RooAbsReal *i_frac = w_sm->function(("gg" + mod_indep_h + "_i_SM_frac").c_str());
  //       t_frac->SetName("ggHt_frac");
  //       b_frac->SetName("ggHb_frac");
  //       i_frac->SetName("ggHi_frac");
  //       ws.import(mA);
  //       ws.import(*t_frac, RooFit::RecycleConflictNodes());
  //       ws.import(*b_frac, RooFit::RecycleConflictNodes());
  //       ws.import(*i_frac, RooFit::RecycleConflictNodes());
  //       f_sm.Close();
  //     } else {
  //       ws.factory("ggHt_frac[1.0,-10,10]");
  //       ws.factory("ggHb_frac[0.0,-10,10]");
  //       ws.factory("expr::ggHi_frac(\"1-@0-@1\",ggHt_frac,ggHb_frac)");
  //       ws.var("ggHt_frac")->setConstant(true);
  //       ws.var("ggHb_frac")->setConstant(true);
  //     }
  //   }
  //   auto bins = cb.bin_set();
  //   for (auto b : bins) {
  //     auto procs = cb.cp().bin({b}).process(ch::JoinStr({signal_types["ggH"], signal_types["bbH"]})).process_set();
  //     for (auto p : procs) {
  //       std::string norm = "norm";
  //       if (mass == "MH" && ggHatNLO && ch::contains(signal_types["ggH"], p)) {
  //         norm = "prenorm";
  //       }
  //       std::string pdf_name = ch::BuildRooMorphing(
  //           ws, cb, b, p, *(mass_var[p]), norm, true, false, false, &demo);
  //       if (mass == "MH" && ggHatNLO && ch::contains(signal_types["ggH"], p)) {
  //         ws.factory(TString::Format("expr::%s_norm('@0*@1',%s, %s_frac)",
  //                                    pdf_name.c_str(),
  //                                    (pdf_name + "_" + norm).c_str(), p.c_str()));
  //       }
  //     }
  //   }
  // }
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

  // if (partial_unblinding) {
  //   cb.FilterAll([&](ch::Object *obj) {
  //     if (obj->channel() == "et" || obj->channel() == "mt") return !ch::contains({10, 11}, obj->bin_id());
  //     if (obj->channel() == "em") return !ch::contains({8, 9, 12 ,13}, obj->bin_id());
  //     if (obj->channel() == "tt") return true;
  //     return false;
  //   });
  // }

 //Write out datacards. Naming convention important for rest of workflow. We
 //make one directory per chn-cat, one per chn and cmb. In this code we only
 //store the individual datacards for each directory to be combined later, but
 //note that it's also possible to write out the full combined card with CH
  string output_prefix = "output/";
  ch::CardWriter writer(output_prefix + output_folder + "/$TAG/$BIN.txt",
                        output_prefix + output_folder +
                            "/$TAG/htt_input_" + "2017" + ".root");
  // We're not using mass as an identifier - which we need to tell the CardWriter
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



  // writer.WriteCards("cmb", cb);
  // for (auto chn : chns) {
  //   if(chn == std::string("zmm") || chn == std::string("ttbar"))
  //   {
  //       continue;
  //   }
  //   // per-channel
  //   writer.WriteCards(chn, cb.cp().channel({chn, "zmm","ttbar"}));
  //   // And per-channel-category
  //   if(chn == std::string("et") || chn ==std::string("mt")){
  //     writer.WriteCards("htt_"+chn+"_8_13TeV", cb.cp().channel({chn,"zmm","ttbar"}).attr({"tight","high","all"},"mtsel").attr({"nobtag","all"},"cat"));
  //     writer.WriteCards("htt_"+chn+"_9_13TeV", cb.cp().channel({chn,"zmm","ttbar"}).attr({"tight","high","all"},"mtsel").attr({"btag","all"},"cat"));
  //     writer.WriteCards("htt_"+chn+"_10_13TeV", cb.cp().channel({chn,"zmm","ttbar"}).attr({"loose","high","all"},"mtsel").attr({"nobtag","all"},"cat"));
  //     writer.WriteCards("htt_"+chn+"_11_13TeV", cb.cp().channel({chn,"zmm","ttbar"}).attr({"loose","high","all"},"mtsel").attr({"btag","all"},"cat"));
  //   }
  //   if(chn == std::string("tt")){
  //     writer.WriteCards("htt_"+chn+"_8_13TeV", cb.cp().channel({chn,"zmm","ttbar"}).attr({"nobtag","all"},"cat"));
  //     writer.WriteCards("htt_"+chn+"_9_13TeV", cb.cp().channel({chn,"zmm","ttbar"}).attr({"btag","all"},"cat"));
  //   }
  //   if(chn == std::string("em")){
  //     writer.WriteCards("htt_"+chn+"_8_13TeV", cb.cp().channel({chn,"zmm","ttbar"}).attr({"low","all"},"pzeta").attr({"nobtag","all"},"cat"));
  //     writer.WriteCards("htt_"+chn+"_9_13TeV", cb.cp().channel({chn,"zmm","ttbar"}).attr({"low","all"},"pzeta").attr({"btag","all"},"cat"));
  //     writer.WriteCards("htt_"+chn+"_10_13TeV", cb.cp().channel({chn,"zmm","ttbar"}).attr({"medium","all"},"pzeta").attr({"nobtag","all"},"cat"));
  //     writer.WriteCards("htt_"+chn+"_11_13TeV", cb.cp().channel({chn,"zmm","ttbar"}).attr({"medium","all"},"pzeta").attr({"btag","all"},"cat"));
  //     writer.WriteCards("htt_"+chn+"_12_13TeV", cb.cp().channel({chn,"zmm","ttbar"}).attr({"high","all"},"pzeta").attr({"nobtag","all"},"cat"));
  //     writer.WriteCards("htt_"+chn+"_13_13TeV", cb.cp().channel({chn,"zmm","ttbar"}).attr({"high","all"},"pzeta").attr({"btag","all"},"cat"));
  //   }
  //  //b-tag and no b-tag per channel:
  //  writer.WriteCards("htt_"+chn+"_nobtag_13TeV", cb.cp().channel({chn,"zmm","ttbar"}).attr({"nobtag","all"},"cat"));
  //  writer.WriteCards("htt_"+chn+"_btag_13TeV", cb.cp().channel({chn,"zmm","ttbar"}).attr({"btag","all"},"cat"));
  // }
  // // For btag/nobtag areas want to include control regions. This will
  // // work even if the extra categories aren't there.
  // writer.WriteCards("htt_cmb_btag_13TeV", cb.cp().attr({"btag","all"},"cat"));
  // writer.WriteCards("htt_cmb_nobtag_13TeV", cb.cp().attr({"nobtag","all"},"cat"));

  // cb.PrintAll();
  // cout << " done\n";
}
