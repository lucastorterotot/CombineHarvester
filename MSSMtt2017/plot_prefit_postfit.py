"""Script for generating prefit post plot from the impacts.json."""

# Author: Ece Asilar easilar@gmail.com

"""
# Objectives ###
  This script loops over the "params" keys in the given json file. 
  saves plot in formats: png, pdf, tex
# Optional parameters ###
  1) input json file path, default is : impacts.json
  2) output file path
# Usage ###
  example usage:
  python plot_prefit_postfit.py --jsonpath=<location_of_json>
"""


from optparse import OptionParser


oparser = OptionParser()
oparser.add_option('--basepath', default='impacts.json' , action='store')
oparser.add_option('--output', default='./nuisances_my' , action='store')
(options, oargs) = oparser.parse_args()

import json
with open(options.basepath, 'r') as myfile:
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
	print(i,o["name"], o["fit"],o["prefit"])
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
cb.SaveAs(options.output+".png")
cb.SaveAs(options.output+".pdf")



