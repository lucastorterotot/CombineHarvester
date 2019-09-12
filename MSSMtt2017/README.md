```bash
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
then to try the fits:

```bash
cd CombineHarvester/MSSMtt2017
MorphingMSSM2017 --real_data=true

combineTool.py -M T2W -o "ws.root" -P HiggsAnalysis.CombinedLimit.PhysicsModel:multiSignalModel --PO '"map=^.*/ggh$:r_ggh[0,-1,200]"' --PO '"map=^.*/bbh$:r_bbh[0,-1,200]"' -i output/mssm_tt/tt/*
cd output/mssm_tt/

combineTool.py -t -1 -M Impacts -d tt/600/ws.root -m 600 --doInitialFit --robustFit 1 --redefineSignalPOIs r_ggh --setParameters r_bbh=0.0,r_ggh=0.1 --setParameterRanges r_ggh=0,0.1 --freezeParameters r_bbh

combineTool.py -t -1 -M Impacts -d tt/600/ws.root -m 600 --doFits --robustFit 1 --redefineSignalPOIs r_ggh --setParameters r_bbh=0.0,r_ggh=0 --setParameterRanges r_ggh=-1,1 --allPars  --freezeParameters r_bbh --parallel 32

combineTool.py -M Impacts -d tt/600/ws.root -m 600 --redefineSignalPOIs r_ggh --allPars --exclude r_bbh -o impacts_ggH_600.json
plotImpacts.py -i impacts_ggH_600.json -o impacts_ggH_600 --transparent
```

Below Find the recipe for low mt\_tot region fit. Fit works, we have the prefit and b-only uncertainities.

```bash
cd CombineHarvester/MSSMtt2017
MorphingMSSM2017 --real_data=true 
combineTool.py -M T2W -o "ws.root" -P HiggsAnalysis.CombinedLimit.PhysicsModel:multiSignalModel --PO '"map=^.*/ggH$:r_ggH[0,-2,200]"' --PO '"map=^.*/bbH$:r_bbH[0,-2,200]"' -i output/mssm_tt/tt/*
combine -M FitDiagnostics --saveNLL --numToysForShapes 1000 --verbose 1 -d output/mssm_tt/tt/600/ws.root -m 600 --autoMaxPOIs "*" --autoBoundsPOIs "*" --saveNormalizations --saveShapes --saveWithUncertainties --saveWorkspace -n test_1 --plots
```
