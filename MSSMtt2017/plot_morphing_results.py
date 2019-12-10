"""Script for plotting signal templates obtained from morphing."""

# Author: Gael Touquet

"""
# Objectives ###
  takes the ws where the morphing is stored, and plots several mass points of the desired signal.
# Optional parameters ###

# Usage ###
  example usage:
  python plot_morphing_results.py -m 600,800,900 -i 
"""

from optparse import OptionParser


oparser = OptionParser()
oparser.add_option('-m','--masses',action='store')
oparser.add_option('-I','--input', default='output_13Nov/mssm_tt/tt/ws.root' , action='store')
(options, oargs) = oparser.parse_args()

import ROOT

in_file = ROOT.TFile(options.input)

ws = in_file.Get("w")

can = ROOT.TCanvas()

basehists = []
i = 1
leg = ROOT.TLegend(0.65,0.73,0.86,0.87)

for mass in options.masses.split(','):
    basehists.append(ws.var("CMS_th1x").frame())
    ws.var("MH").setVal(float(mass))
    ws.pdf("shapeSig_bbH_htt_tt_8_13TeV").plotOn(basehists[-1],ROOT.RooFit.Name(mass),ROOT.RooFit.LineColor(i))
    leg.AddEntry(mass, mass, "L")
    i+=1


ws.var("CMS_th1x").frame().Draw()
for basehist in basehists:
    basehist.Draw("same")
leg.Draw("same")

can.SaveAs("signals.pdf")
