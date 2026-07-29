#!/usr/bin/env python3

import ROOT
import argparse

ROOT.gROOT.SetBatch(False)
ROOT.gStyle.SetOptStat(0)

parser = argparse.ArgumentParser()
parser.add_argument("-i", "--input", required=True,
                    help="Input ROOT file")
parser.add_argument("-c", "--category", required=True,
                    help="Category, e.g. cat_1j1b")
parser.add_argument("-p", "--process", required=True,
                    help="Process, e.g. single_tW")
parser.add_argument("-s", "--syst", required=True,
                    help="Systematic, e.g. btag_lf_corr")
parser.add_argument("--noNorm", action="store_true",
                    help="Don't normalize histograms")
args = parser.parse_args()

ROOT.TH1.SetDefaultSumw2()

f = ROOT.TFile.Open(args.input)

nom = f.Get(f"{args.category}/{args.process}")
up = f.Get(f"{args.category}/{args.process}_{args.syst}Up")
down = f.Get(f"{args.category}/{args.process}_{args.syst}Down")

if not nom:
    raise RuntimeError("Missing nominal histogram")
if not up:
    raise RuntimeError("Missing Up histogram")
if not down:
    raise RuntimeError("Missing Down histogram")

nom = nom.Clone("nom")
up = up.Clone("up")
down = down.Clone("down")

print("\n========== Integrals ==========")
print(f"Nominal : {nom.Integral():.6f}")
print(f"Up      : {up.Integral():.6f}   ({up.Integral()/nom.Integral():.4f})")
print(f"Down    : {down.Integral():.6f}   ({down.Integral()/nom.Integral():.4f})")
print("===============================\n")

if not args.noNorm:
    for h in [nom, up, down]:
        if h.Integral() != 0:
            h.Scale(1./h.Integral())

nom.SetLineColor(ROOT.kBlack)
nom.SetLineWidth(3)

up.SetLineColor(ROOT.kRed+1)
up.SetLineWidth(2)

down.SetLineColor(ROOT.kBlue+1)
down.SetLineWidth(2)

#----------------------------------
# Ratio histograms
#----------------------------------

ratio_up = up.Clone("ratio_up")
ratio_down = down.Clone("ratio_down")

ratio_up.Divide(nom)
ratio_down.Divide(nom)

ratio_up.SetLineColor(ROOT.kRed+1)
ratio_down.SetLineColor(ROOT.kBlue+1)

ratio_up.SetLineWidth(2)
ratio_down.SetLineWidth(2)

#----------------------------------
# Canvas
#----------------------------------

c = ROOT.TCanvas("c","",900,850)

pad1 = ROOT.TPad("pad1","",0,0.30,1,1)
pad2 = ROOT.TPad("pad2","",0,0,1,0.30)

pad1.SetBottomMargin(0.02)

pad2.SetTopMargin(0.03)
pad2.SetBottomMargin(0.32)
pad2.SetGridy()

pad1.Draw()
pad2.Draw()

#==================================
# Upper pad
#==================================

pad1.cd()

maximum = max(
    nom.GetMaximum(),
    up.GetMaximum(),
    down.GetMaximum()
)

nom.SetMaximum(1.25*maximum)

nom.SetTitle("")

nom.GetYaxis().SetTitle("Events")

nom.Draw("hist")
up.Draw("hist same")
down.Draw("hist same")

leg = ROOT.TLegend(0.63,0.72,0.88,0.88)
leg.SetBorderSize(0)
leg.SetFillStyle(0)
leg.AddEntry(nom,"Nominal","l")
leg.AddEntry(up,"Up","l")
leg.AddEntry(down,"Down","l")
leg.Draw()

latex = ROOT.TLatex()
latex.SetNDC()
latex.SetTextSize(0.035)

latex.DrawLatex(0.14,0.92,args.category)
latex.DrawLatex(0.14,0.87,args.process)
latex.DrawLatex(0.14,0.82,args.syst)

#==================================
# Ratio pad
#==================================

pad2.cd()

ratio_up.SetTitle("")

ratio_up.GetYaxis().SetTitle("Var/Nom")
ratio_up.GetXaxis().SetTitle("Bin")

ratio_up.GetYaxis().SetNdivisions(505)

ratio_up.GetYaxis().SetTitleSize(0.10)
ratio_up.GetYaxis().SetTitleOffset(0.45)
ratio_up.GetYaxis().SetLabelSize(0.08)

ratio_up.GetXaxis().SetTitleSize(0.12)
ratio_up.GetXaxis().SetLabelSize(0.10)

ratio_up.SetMinimum(0.6)
ratio_up.SetMaximum(1.4)

ratio_up.Draw("hist")
ratio_down.Draw("hist same")

line = ROOT.TLine(
    ratio_up.GetXaxis().GetXmin(),
    1.0,
    ratio_up.GetXaxis().GetXmax(),
    1.0
)
line.SetLineStyle(2)
line.Draw()

c.SaveAs(f"{args.category}_{args.process}_{args.syst}.pdf")
c.SaveAs(f"{args.category}_{args.process}_{args.syst}.png")

print("Saved:")
print(f"  {args.category}_{args.process}_{args.syst}.pdf")
print(f"  {args.category}_{args.process}_{args.syst}.png")

input("Press Enter to exit...")
