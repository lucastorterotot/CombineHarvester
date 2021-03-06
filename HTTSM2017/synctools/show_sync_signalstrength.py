import os
import subprocess as sp
import shlex
import json
import sys
import ROOT as R
R.gStyle.SetOptStat(0)

def main():
    print sys.argv
    era = sys.argv[4]
    datacards = [(sys.argv[1],sys.argv[3]),(sys.argv[2],sys.argv[3])]
    plot_range = 1.0
    channels = ["tt","et","mt","cmb"]
    # channels = ["tt","et","mt"]
    # channels = ["et"]

    limits = {}
    for datacard, real in datacards:
        limits[datacard] = {}

        produceDatacards(datacard, era, real)

        for channel in channels:
            limits[datacard][channel] = calculateLimits(era, channel, real )

    getComparison(limits, datacards[0][0], datacards[1][0] )
    with open( "_".join([era,datacards[0][0], datacards[1][0], datacards[1][1]]) + "_save.json", "w" ) as FSO:
      json.dump( limits, FSO, indent = 4 )

    # with open( "save.json", "r" ) as FSO:
    #   limits = json.load( FSO )

    drawComparison(limits, plot_range, "_".join([era,datacards[0][0], datacards[1][0], datacards[1][1]]) )





def drawComparison(limits, plot_range, era):
    histos = fillHistograms(limits,plot_range)

    cv = createRatioSemiLogCanvas("cv")

    lines = [ R.TLine(x,0,x,plot_range) for x in xrange(2,7,2) ]

    ggL = [ R.TLatex( x, 0.2, "ggH" ) for x in [0.120,0.347, 0.572,0.800] ]
    qqL = [ R.TLatex( x, 0.2, "qqH" ) for x in [0.228,0.455, 0.680,0.908] ]
    chL = [ R.TLatex( x, 0.8, l     ) for x,l in [(0.174, r"#mu#tau"),(0.401, r"e#tau"), (0.628,r"#tau#tau"),(0.855, "Comb"  )] ]

    for g in ggL+qqL:
        g.SetTextSize(0.08)
        g.SetNDC();
    for g in chL:
        g.SetTextSize(0.1)
        g.SetNDC();        
    
    cv.cd(1)
    histos["up"].Draw("E1")
    for l in lines: l.Draw("same")
    for g in chL: g.Draw()

    cv.cd(2)
    histos["mu"].Draw("E1")
    for l in lines: l.Draw("same")

    cv.cd(3)
    histos["down"].Draw("E1")
    for l in lines: l.Draw("same")
    for g in ggL+qqL: g.Draw()

    cv.SaveAs( era +"_sync.png" )

def fillHistograms(limits, plot_range):
    histos = {}
    histos["up"]   = R.TH1D("u","",8,0,8)
    histos["down"] = R.TH1D("d","",8,0,8)
    histos["mu"]   = R.TH1D("m","",8,0,8)

    for i, channel in enumerate( ["mt","et","tt","cmb"] ):
        if channel in limits["comp"] and limits["comp"][channel]:
            histos["up"].SetBinContent(  2*i + 1, limits["comp"][channel]["r_ggH"]["up"] )
            histos["mu"].SetBinContent(  2*i + 1, limits["comp"][channel]["r_ggH"]["mu"] )
            histos["down"].SetBinContent(2*i + 1, limits["comp"][channel]["r_ggH"]["down"] )

            histos["up"].SetBinContent(  2*i + 2, limits["comp"][channel]["r_qqH"]["up"] )
            histos["mu"].SetBinContent(  2*i + 2, limits["comp"][channel]["r_qqH"]["mu"] )
            histos["down"].SetBinContent(2*i + 2, limits["comp"][channel]["r_qqH"]["down"] )

    setHistStyle(histos, plot_range)

    return histos


def setHistStyle(histos, plot_range = 0.15):
    for h in histos:
        for i in xrange( histos[h].GetNbinsX()+1 ):
            if histos[h].GetBinContent(i) < plot_range:
                histos[h].SetBinError(i,0.0000001)
            else:
                histos[h].SetBinContent(i,0.005)
                histos[h].SetBinError(i,10)

            histos[h].GetXaxis().SetLabelFont(63)
            histos[h].GetXaxis().SetLabelSize(14)
            histos[h].GetYaxis().SetLabelFont(63)
            histos[h].GetYaxis().SetLabelSize(14)
            histos[h].GetXaxis().SetLabelSize(0)
            histos[h].GetYaxis().SetNdivisions(4)
            histos[h].SetLineWidth( 2 )   
        histos[h].GetYaxis().SetRangeUser( 0.005 , plot_range )

def createRatioSemiLogCanvas(name):

    cv = R.TCanvas(name, name, 10, 10, 800, 600)

    # this is the tricky part...
    # Divide with correct margins
    cv.Divide(1, 3, 0.0, 0.0)

    # Set Pad sizes
    cv.GetPad(1).SetPad(0.0, 0.66, 1., 1.0)
    cv.GetPad(2).SetPad(0.0, 0.33, 1., 0.66)
    cv.GetPad(3).SetPad(0.0, 0.00, 1., 0.33)

    cv.GetPad(1).SetFillStyle(4000)
    cv.GetPad(2).SetFillStyle(4000)
    cv.GetPad(3).SetFillStyle(4000)

    # Set pad margins 1
    cv.cd(1)
    R.gPad.SetTopMargin(0.3)
    R.gPad.SetBottomMargin(0.05)
    R.gPad.SetLeftMargin(0.15)
    R.gPad.SetRightMargin(0.02)
    R.gPad.SetGridy()
    R.gPad.SetLogy()

    cv.cd(2)
    R.gPad.SetTopMargin(0.05)
    R.gPad.SetLeftMargin(0.15)
    R.gPad.SetBottomMargin(0.05)
    R.gPad.SetRightMargin(0.02)
    R.gPad.SetGridy()
    R.gPad.SetLogy()

    # # Set pad margins 2
    cv.cd(3)
    R.gPad.SetTopMargin(0.05)
    R.gPad.SetBottomMargin(0.3)
    R.gPad.SetLeftMargin(0.15)
    R.gPad.SetRightMargin(0.02)
    R.gPad.SetGridy()
    R.gPad.SetLogy()

    cv.cd(1)
    return cv

def getComparison(limits, first, second):

    limits["comp"] = {}
    for channel in limits[first].keys():
        limits["comp"][channel] = {}
        for poi in limits[first][channel].keys():
            limits["comp"][channel][poi] = {}
            limits["comp"][channel][poi]["mu"] = getDeviation(limits[first][channel][poi]["mu"], limits[second][channel][poi]["mu"])
            limits["comp"][channel][poi]["up"] = getDeviation(limits[first][channel][poi]["up"], limits[second][channel][poi]["up"])
            limits["comp"][channel][poi]["down"] = getDeviation(limits[first][channel][poi]["down"], limits[second][channel][poi]["down"])

            limits["comp"][channel][poi]["mu_div"] = getDeviation(limits[first][channel][poi]["mu"], limits[second][channel][poi]["mu"], True )
            limits["comp"][channel][poi]["up_div"] = getDeviation(limits[first][channel][poi]["up"], limits[second][channel][poi]["up"], True )
            limits["comp"][channel][poi]["down_div"] = getDeviation(limits[first][channel][poi]["down"], limits[second][channel][poi]["down"], True )

    for l in limits.keys():
        if l != "comp":
            limits.pop(l,None)


def getDeviation(a,b, div = False):
    if div: return abs( a - b )
    return abs( a - b ) / ( 0.5*( a + b ) )

def produceDatacards(shapes, era, real):
    print "Producing Datacards for",shapes, era,"real data: ",real
    p = sp.Popen(shlex.split("sh make_datacards.sh {0} {1} {2}".format(shapes, era, real) ) ,stdout = sp.PIPE, stderr = sp.PIPE, shell=False)
    out, err = p.communicate()

def calculateLimits(era, channel, real):
    print "Make limits for", era, channel
    if real == "true":
        p = sp.Popen(shlex.split("sh stage0_obs.sh {0} {1}".format(era, channel) ) ,stdout = sp.PIPE, stderr = sp.PIPE, shell=False)
    else:
        p = sp.Popen(shlex.split("sh stage0_exp.sh {0} {1}".format(era, channel) ) ,stdout = sp.PIPE, stderr = sp.PIPE, shell=False)

    out, err = p.communicate()
    return getResult(out)


def getResult(prompt):
    # print out 
    save = False
    results = {}
    for line in prompt.splitlines():
        if "--- MultiDimFit ---" in line or "-- Significance --" in line: save = True
        if "Done" in line: save = False

        if save and "(68%)" in line:
            values = extractConstraints(line) 
            results[ values["name"] ] = values        
        else:
            print line
    return results

def extractConstraints(line):

    values = [ value for value in line.replace(":","").split(" ") if value ]

    info = { "name": values[0] }

    info["mu"] = abs(float( values[1] ) )
    info["down"] = abs(float( values[2].split("/")[0] ) )
    info["up"] = abs(float( values[2].split("/")[1] ) )

    return info

if __name__ == '__main__':
    main()