import ROOT
from ROOT import *

runs = ["16pre","16post","17","18"]
datafiles = ["PileupDATA_UL16pre","PileupDATA_UL16post","PileupDATA_UL17","PileupDATA_UL18"]
mcfiles = ["PileupMC_UL16","PileupMC_UL17","PileupMC_UL18"]

for run in runs:
    datarootfile = "PileupDATA_UL"+run+".root"
    mcrootfile = ""
    if "16" in run:
        mcrootfile = "PileupMC_UL16.root"
    else:
        mcrootfile = "PileupMC_UL"+run+".root"

    fdata = TFile(datarootfile)
    h_pu = fdata.Get("pileup")
    h_pu_plus = fdata.Get("pileup_plus")
    h_pu_minus = fdata.Get("pileup_minus")
    fmc = TFile(mcrootfile)
    fmc.cd()
    h_pu_mc = fmc.Get("pu_mc")

    h_pu.SetLineStyle(1)
    h_pu_plus.SetLineStyle(3)
    h_pu_minus.SetLineStyle(7)
    h_pu_mc.SetLineStyle(1)
    h_pu.SetLineColor(1)
    h_pu_plus.SetLineColor(1)
    h_pu_minus.SetLineColor(1)
    h_pu_mc.SetLineColor(2)
    h_pu.SetLineWidth(2)
    h_pu_plus.SetLineWidth(2)
    h_pu_minus.SetLineWidth(2)
    h_pu_mc.SetLineWidth(2)

    h_pu.Scale(1/h_pu.Integral())
    h_pu_plus.Scale(1/h_pu_plus.Integral())
    h_pu_minus.Scale(1/h_pu_minus.Integral())
    h_pu_mc.Scale(1/h_pu_mc.Integral())
    
    m = max([h_pu.GetMaximum(),h_pu_plus.GetMaximum(),h_pu_minus.GetMaximum(),h_pu_mc.GetMaximum()])
    h_pu.SetMaximum(m*1.2)

    c1 = TCanvas('c','c',600,600)
    c1.cd()
    c1.SetMargin(0.1,0.05,0.1,0.05)
    gStyle.SetOptStat(0)
    gStyle.SetOptTitle(0)
    h_pu.Draw("hist")
    h_pu_plus.Draw("histsame")
    h_pu_minus.Draw("histsame")
    h_pu_mc.Draw("histsame")
    
    leg = TLegend(0.55,0.6,0.9,0.9)
    gStyle.SetLegendTextSize(0.04)
    leg.SetBorderSize(0)
    leg.AddEntry(h_pu,"Pileup Data")
    leg.AddEntry(h_pu_plus,"Pileup Data up")
    leg.AddEntry(h_pu_minus,"Pileup Data down")
    leg.AddEntry(h_pu_mc,"Pileup MC")
    leg.Draw()
    
    c1.Print("h_pileup"+run+".pdf")
    c1.Close()
    fdata.Close()
    fmc.Close()
