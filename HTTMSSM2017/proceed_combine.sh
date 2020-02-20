#!/bin/bash

# cd $CMSSW_BASE/src/CombineHarvester/HTTMSSM2017/
# rm -rf output/HTT_MSSM_2017_mt/
# scram b -j 35

MorphingHTTMSSM2017 --channel=mt --real_data=true

# for f in $(find ./output/HTT_MSSM_2017_mt/mt/ -type f -iname htt_mt_\*_13TeV.txt)
# do
#     echo "* autoMCStats 0" >> $f
#     #text2workspace.py -o "output/HTT_MSSM_2017_mt/mt/ws.root" $f -P HiggsAnalysis.CombinedLimit.PhysicsModel:multiSignalModel --PO 'map=.*/ggH$:r_ggH[0,0,200]' --PO 'map=.*/bbH$:r_bbH[0,0,200]' $f
# done

combineTool.py -M T2W -o "ws.root" -P HiggsAnalysis.CombinedLimit.PhysicsModel:multiSignalModel --PO '"map=^.*/ggH$:r_ggH[0,0,200]"' --PO '"map=^.*/bbH$:r_bbH[0,0,200]"' -i output/HTT_MSSM_2017_mt/mt/

cd output/HTT_MSSM_2017_mt/
combine -M FitDiagnostics --saveNLL --numToysForShapes 1000 --verbose 1 -d mt/ws.root -m 600 --saveNormalizations --saveShapes --saveWithUncertainties --saveWorkspace -n test_output --plots  --redefineSignalPOIs r_ggH

# python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/test/diffNuisances.py fitDiagnosticstest_output.root -a -g nuisances_plots.root -p r_bbH -f txt > nuisances.txt
# cp fitDiagnosticstest_output.root fitDiagnosticstest_output_600.root