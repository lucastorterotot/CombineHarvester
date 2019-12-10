```bash
bash <(curl -s https://raw.githubusercontent.com/cms-analysis/CombineHarvester/master/CombineTools/scripts/sparse-checkout-ssh.sh)
export SCRAM_ARCH=slc6_amd64_gcc530 
source $VO_CMS_SW_DIR/cmsset_default.sh
scram project CMSSW CMSSW_8_1_0
cd CMSSW_8_1_0/src
cmsenv
git clone ssh://git@github.com/cms-analysis/HiggsAnalysis-CombinedLimit HiggsAnalysis/CombinedLimit
cd HiggsAnalysis/CombinedLimit
git fetch origin
git checkout v7.0.12
cd ../..
git clone ssh://git@github.com/GaelTouquet/CombineHarvester CombineHarvester -b MSSMtt2017
scram b -j 8
scram b python
```
then to try the fit for the 600 GeV mass point:

```bash
cd CombineHarvester/MSSMtt2017
MorphingMSSM2017 --real_data=true

combineTool.py -M T2W -o "ws.root" -P HiggsAnalysis.CombinedLimit.PhysicsModel:multiSignalModel --PO '"map=^.*/ggH$:r_ggH[0,0,200]"' --PO '"map=^.*/bbH$:r_bbH[0,0,200]"' -i output/mssm_tt/tt/

cd output/mssm_tt/

combine -M FitDiagnostics --saveNLL --numToysForShapes 10000 --verbose 1 -d tt/ws.root -m 600 --saveNormalizations --saveShapes --saveWithUncertainties --saveWorkspace -n test_output --plots  --redefineSignalPOIs r_ggH

```
Then to plot the nuisances:
```
python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/test/diffNuisances.py fitDiagnosticstest_output.root -a -g nuisances_plots.root -p r_bbH -f txt > nuisances.txt
```
The nuisances are plotted in nuisances_plots.root

To make limit plots (after T2W):
ggH
```
combineTool.py -m "110,120,130,140,180,200,250,300,400,450,600,700,800,900,1200,1400,1500,2300,2600,2900" -M Asymptotic --rAbsAcc 0 --rRelAcc 0.0005 --setParameters r_ggH=0,r_bbH=0 --redefineSignalPOIs r_ggH -d tt/ws.root --there -n ".ggH" --parallel 25

combineTool.py -M CollectLimits ./*/higgsCombine.ggH*.root --use-dirs -o "mssm_ggH.json"

python ../../../MSSMFull2016/scripts/plotMSSMLimits.py --logy --logx mssm_ggH_tt.json ~/Combine/MSSM2016/CMSSW_7_4_7/src/CombineHarvester/MSSMFull2016/mssm_ggH_tt.json -o mssm_tt_ggH --process 'gg#phi' --title-right="41.5 fb^{-1}" --title-left="#tau_{h}#tau_{h}" --show 'exp,obs' --plot-exp-points
```
bbH
```
combineTool.py -m "110,120,130,140,180,200,250,300,400,450,600,700,800,900,1200,1400,1500,2300,2600,2900" -M Asymptotic --rAbsAcc 0 --rRelAcc 0.0005 --setParameters r_ggH=0,r_bbH=0 --redefineSignalPOIs r_bbH -d tt/ws.root --there -n ".bbH" --parallel 25

combineTool.py -M CollectLimits ./*/higgsCombine.bbH*.root --use-dirs -o "mssm_bbH.json"

python ../../../MSSMFull2016/scripts/plotMSSMLimits.py --logy --logx mssm_bbH_tt.json ~/Combine/MSSM2016/CMSSW_7_4_7/src/CombineHarvester/MSSMFull2016/mssm_bbH_tt.json -o mssm_tt_bbH --process 'bb#phi' --title-right="41.5 fb^{-1}" --title-left="#tau_{h}#tau_{h}" --show 'exp,obs' --plot-exp-points
```

ma-tanbeta plots successfully tested mssm2016 commands:
```
MorphingMSSMFull2016 --output_folder="mssm_mhmodp" --manual_rebin=true --real_data=false --channel=tt
combineTool.py -M T2W -o "mhmodp.root" -P CombineHarvester.CombinePdfs.MSSM:MSSM --PO filePrefix=$PWD/shapes/Models/ --PO modelFiles=13TeV,mhmodp_mu200_13TeV.root,1 --PO MSSM-NLO-Workspace=$PWD/shapes/Models/higgs_pt_v3_mssm_mode.root -i output/mssm_mhmodp/tt/
combineTool.py -M AsymptoticGrid scripts/mssm_asymptotic_grid_mhmodp.json -d output/mssm_mhmodp/tt/mhmodp.root --parallel 30
combineTool.py -M AsymptoticGrid scripts/mssm_asymptotic_grid_mhmodp.json -d output/mssm_mhmodp/tt/mhmodp.root
python ../CombineTools/scripts/plotLimitGrid.py asymptotic_grid.root --scenario-label "m_{h}^{mod+}"
gnome-open ./limit_grid_output.pdf
```
ma-tanbeta plots commands (!not finished yet!):
```
MorphingMSSM2017 --real_data=true --mod_dep=true
combineTool.py -M T2W -o "mhmodp.root" -P CombineHarvester.CombinePdfs.MSSM:MSSM --PO filePrefix=~/Combine/MSSM2016/CMSSW_7_4_7/src/CombineHarvester/MSSMFull2016/shapes/Models/ --PO modelFiles=13TeV,mhmodp_mu200_13TeV.root,1 --PO MSSM-NLO-Workspace=~/Combine/MSSM2016/CMSSW_7_4_7/src/CombineHarvester/MSSMFull2016/shapes/Models/higgs_pt_v3_mssm_mode.root -i output/mssm_tt/tt/
cd output/mssm_tt/
combineTool.py -M AsymptoticGrid ../../scripts/mssm_asymptotic_grid_mhmodp.json -d tt/mhmodp.root --parallel 30
combineTool.py -M AsymptoticGrid ../../scripts/mssm_asymptotic_grid_mhmodp.json -d tt/mhmodp.root
python ~/Combine/MSSM2016/CMSSW_7_4_7/src/CombineHarvester/CombineTools/scripts/plotLimitGrid.py  asymptotic_grid.root --scenario-label "m_{h}^{mod+}"
gnome-open ./limit_grid_output.pdf
```

no morph commands:
```
MSSM2017 --real_data=true
combineTool.py -M T2W -o "ws.root" -P HiggsAnalysis.CombinedLimit.PhysicsModel:multiSignalModel --PO '"map=^.*/ggH$:r_ggH[0,0,200]"' --PO '"map=^.*/bbH$:r_bbH[0,0,200]"' -i output/mssm_tt/tt/*
combineTool.py  -M Asymptotic --rAbsAcc 0 --rRelAcc 0.0005 --setParameters r_ggH=0,r_bbH=0 --redefineSignalPOIs r_bbH -d */ws.root --there -n ".bbH" --parallel 25
```