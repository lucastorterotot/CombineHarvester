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

combineTool.py -M T2W -o "ws.root" -P HiggsAnalysis.CombinedLimit.PhysicsModel:multiSignalModel --PO '"map=^.*/ggH$:r_ggH[0,0,200]"' --PO '"map=^.*/bbH$:r_bbH[0,0,200]"' -i output/mssm_tt/tt/\*/

cd output/mssm_tt/

combine -M FitDiagnostics --saveNLL --numToysForShapes 10000 --verbose 1 -d tt/\*/ws.root -m 600 --saveNormalizations --saveShapes --saveWithUncertainties --saveWorkspace -n test_output --plots  --redefineSignalPOIs r_ggH

```
Then to plot the nuisances:
```
python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/test/diffNuisances.py fitDiagnosticstest_output.root -a -g nuisances_plots.root -p r_bbH -f txt > nuisances.txt
```
The nuisances are plotted in nuisances_plots.root