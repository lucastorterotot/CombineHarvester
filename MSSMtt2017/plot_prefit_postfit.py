
import json
with open('impacts.json', 'r') as myfile:
    res=myfile.read()
obj = json.loads(res)

import ROOT

cb = ROOT.TCanvas("cb","cb",10,10,900,500)
cb.SetBottomMargin(0.15)
#inihist = ROOT.TH1F("inihist","inihist",len(obj["params"]),-0.5,len(obj["params"])-0.5)
p_fit = "prefit"
g = ROOT.TGraphAsymmErrors()
g.SetLineWidth(2)
g.SetLineColor(ROOT.kGray)
g.SetMaximum(2) 	
g.SetMinimum(-2)	
for i,o in enumerate(obj["params"]):
	#print(i,o["name"], o["fit"],o["prefit"])
	g.SetPoint(i,float(i),o[p_fit][1])
	g.SetPointError(i,0.03,0.03,-1*(o[p_fit][0]),o[p_fit][2])
	#inihist.GetXaxis().SetBinLabel(i,o["name"])

p_fit = "fit"

g_postfit = ROOT.TGraphAsymmErrors()
g_postfit.SetLineWidth(2)
g_postfit.SetLineColor(ROOT.kBlue)
g_postfit.SetMaximum(2) 	
g_postfit.SetMinimum(-2)	
for i,o in enumerate(obj["params"]):
	#print(i,o["name"], o["fit"],o["prefit"])
	g_postfit.SetPoint(i,float(i),o[p_fit][1])
	g_postfit.SetPointError(i,0.01,0.01,-1*(o[p_fit][0]),o[p_fit][2])
	#inihist.GetXaxis().SetBinLabel(i,o["name"])

g.Draw()
g_postfit.Draw("e2 z same")


#inihist.Draw("same")
cb.Update()
cb.SaveAs("nuisances_my.png")
cb.SaveAs("nuisances_my.pdf")



