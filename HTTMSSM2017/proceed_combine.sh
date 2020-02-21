#!/bin/bash

# cd $CMSSW_BASE/src/CombineHarvester/HTTMSSM2017/
# rm -rf output/HTT_MSSM_2017_mt/
# scram b -j 35

MorphingHTTMSSM2017 --channel=mt --real_data=true
combineTool.py -M T2W -o "ws.root" -P HiggsAnalysis.CombinedLimit.PhysicsModel:multiSignalModel --PO '"map=^.*/ggH$:r_ggH[0,0,200]"' --PO '"map=^.*/bbH$:r_bbH[0,0,200]"' -i output/HTT_MSSM_2017_mt/mt/

cd output/HTT_MSSM_2017_mt/
combine -M FitDiagnostics --saveNLL --numToysForShapes 1000 --verbose 1 -d mt/ws.root -m 600 --saveNormalizations --saveShapes --saveWithUncertainties --saveWorkspace -n test_output --plots  --redefineSignalPOIs r_ggH

python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/test/diffNuisances.py fitDiagnosticstest_output.root -a -g nuisances_plots.root -p r_bbH -f txt > nuisances.txt
cp fitDiagnosticstest_output.root fitDiagnosticstest_output_600.root

for p in gg bb
do
    combineTool.py -m "110,120,130,140,180,200,250,300,400,450,600,700,800,900,1200,1400,1500,2300,2600,2900" -M AsymptoticLimits --rAbsAcc 0 --rRelAcc 0.0005 --setParameters r_ggH=0,r_bbH=0 --redefineSignalPOIs r_${p}H -d mt/ws.root --there -n ".${p}H" --parallel 25
    combineTool.py -M CollectLimits ./*/higgsCombine.${p}H*.root --use-dirs -o "mssm_${p}H.json"
    python ../../../MSSMFull2016/scripts/plotMSSMLimits.py --logy --logx mssm_${p}H_mt.json -o mssm_mt_${p}H --process '${p}#phi' --title-right="41.5 fb^{-1}" --title-left="#mu#tau_{h}" --show 'exp,obs' --plot-exp-points --use-hig-17-020-style
done