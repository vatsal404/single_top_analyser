#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
File        : drawhist.py
Developper     : Candan Dozen Altuntas < AT gmail [DOT] com>
Description : drawhist.py
"""
import copy
import ROOT
import stackhists
import CMS_lumi
import tdrstyle
import click


def get_prep_stack_hist(year, initial_tdrStyle, rlumi, verbose):
    """Prepare necessary variables with given year

    Args:
        year: given year
        initial_tdrStyle: initialized tdrStyle instance
        rlumi list
    Return:
        s, CMS_lumi.lumi_13TeV, rlumi, runs
    """
    runs = []
    s = None
    if year == "16pre":
        CMS_lumi.lumi_13TeV = "19.5 fb^{-1}"
        s = stackhists.StackHists(ROOT, initial_tdrStyle, 19.5, verbose)
        runs = ["16pre"]
    
    if year == "16post":
        CMS_lumi.lumi_13TeV = "16.8 fb^{-1}"
        s = stackhists.StackHists(ROOT, initial_tdrStyle, 16.8, verbose)
        runs = ["16post"]
    
    elif year == "17":
        CMS_lumi.lumi_13TeV = "41.5 fb^{-1}"
        s = stackhists.StackHists(ROOT, initial_tdrStyle, 41.48, verbose)
        runs = ["17"]
    
    elif year == "18":
        CMS_lumi.lumi_13TeV = "150.0 fb^{-1}"
        s = stackhists.StackHists(ROOT, initial_tdrStyle, 150.0, verbose)
        runs = ["18"]
    
    elif year == "run2":
        CMS_lumi.lumi_13TeV = "138 fb^{-1}"
        s = stackhists.StackHists(ROOT, initial_tdrStyle, 137.65, verbose)
        rlumi["16pre"] = 19.5 / 137.65
        rlumi["16post"] = 16.8 / 137.65
        rlumi["17"] = 41.48 / 137.65
        # rlumi["18"] = 59.83/137.65
        rlumi["18"] = 150.0 / 137.65
        runs = ["16pre", "16post", "17", "18"]

    # DATA
    if year == "run2":
        print("run2")
        #s.addChannel("Run2"+sys+".root", "data", 999, isMC=False)
    else:
       
        s.addChannel("18/Signal_tightID_miniall_tight_SumPtTwoMuons160_DR18_BJetone.root", "data", 999, isMC=False)
        print("no run2")
        #s.addChannel("/home/cms/dozen-altuntas/FLY_project/CMSSW_12_3_2/src/fly/analyzed/test_nano_v1.root", "data", 999,isMC=False)
    #return s, CMS_lumi.lumi_13TeV, rlumi, runs
    return copy.copy(s), CMS_lumi.lumi_13TeV, rlumi, runs 


@click.command()
@click.option('--ratio', '-R', is_flag=True, default=False, required=False)
@click.option('--significance', '-S', is_flag=True, default=False, required=False)
@click.option('--logstyle', '-L', is_flag=True, default=False, required=False)
@click.option('--verbose', '-v', is_flag=True, default=False, required=False)
@click.option('--year', '-Y', default="", type=str)

def main(ratio, significance, logstyle, verbose, year):
    click.echo(f'Input Arguments: ratio:{ratio}, significance:{significance}, logstyle:{logstyle}, year:{year}')

    # Subplot ( Ratio plot : option -R, --ratio (default) / significance plot :  option -S, --significance )
    initial_tdrStyle = ROOT.TStyle("tdrStyle", "Style for P-TDR")
    tdrstyle.setTDRStyle(ROOT, initial_tdrStyle)

    sys = "_norm"
    # Lumi ratio dictionary for integrated Run2
    rlumi = {"16pre": 1., "16post": 1., "17": 1., "18": 1.}
    s, CMS_lumi.lumi_13TeV, rlumi, runs = get_prep_stack_hist(year, initial_tdrStyle, rlumi, verbose)

    s.setupStyle(alpha=1)
    CMS_lumi.extraText = ""
    # CMS_lumi.extraText = "Simulation"

    for run in runs:
        if run == "16pre":
            print("Run2016_pre")
            #s.addChannel(run+"/Signal_tightID_miniall_tight_SumPtTwoMuons160_DR18_BJetone.root", "W+jets", 3, isMC=True, xsec=rlumi[run]*61526700*0.9647, counterhistogramroot=run+"/Signal_Candan.root")
        
        elif run == "16post":
            print("Run2016_post")
            #s.addChannel(run+"/WJetsToLNu_inclHT100_"+run+sys+".root", "W+jets", 3, isMC=True, xsec=rlumi[run]*61526700*0.9647, counterhistogramroot=run+"/WJetsToLNu_inclHT100_"+run+sys+".root")
        
        elif run == "17":
            print("Run2017")
            #s.addChannel(run+"/WJetsToLNu_inclHT100_"+run+sys+".root", "W+jets", 3, isMC=True, xsec=rlumi[run]*61526700*0.9645, counterhistogramroot=run+"/WJetsToLNu_inclHT100_"+run+sys+".root")
        
        elif run == "18":
            print("Run2018")
            #MC_Signal
            s.addChannel(run + "/Signal_tightID_miniall_tight_SumPtTwoMuons160_DR18_BJetone.root", "MC_Sig", 1,
                         isMC=True, xsec=89.0,  counter_histogram_root=run + "/Signal_tightID_miniall_tight_SumPtTwoMuons160_DR18_BJetone.root")
            s.addChannel(run + "/WW_DS_tightID_miniall_tight_SumPtTwoMuons160_DR18_BJetone.root", "ww_ds", 2,
                         isMC=True, xsec=89.0, counter_histogram_root=run + "/WW_DS_tightID_miniall_tight_SumPtTwoMuons160_DR18_BJetone.root")
            s.addChannel(run + "/WW_tightID_miniall_tight_SumPtTwoMuons160_DR18_BJetone.root", "ww_tight", 3,
                         isMC=True, xsec=89.0, counter_histogram_root=run + "/WW_tightID_miniall_tight_SumPtTwoMuons160_DR18_BJetone.root")



    # Muon Histograms
    s.addHistogram("Pt_Muons_cut00", "muons p_{T} cut00", "Entries", draw_mode=stackhists.STACKED, draw_option="hist",is_logy=logstyle, ymin=0.1)
    s.addHistogram("Pt_Muons_nocut", "muons p_{T} nocut", "Entries", draw_mode=stackhists.STACKED, draw_option="hist",is_logy=logstyle, ymin=0.1)
    s.addHistogram("Pt_Muons_cut0000", "muons p_{T} cut0000", "Entries", draw_mode=stackhists.STACKED,draw_option="hist", is_logy=logstyle, ymin=0.1)

    s.addHistogram("Subleading_Pt_Muons_cut0", "Subleadinhg muons p_{T}", "Entries", draw_mode=stackhists.STACKED,draw_option="hist", is_logy=logstyle, ymin=0.1)
    s.addHistogram("Subleading_Pt_Muons_nocut", "Subleadinhg muons p_{T}", "Entries", draw_mode=stackhists.STACKED,draw_option="hist", is_logy=logstyle, ymin=0.1)


    # Jet Histograms
    # Leading Jet
    # s.addHistogram("h1jet1pt_cut000", "p_{T} of Leading Jet (GeV)", "Entries", drawmode=stackhists.STACKED, drawoption="hist", isLogy=logstyle, ymin=0.1)

    subplot = "R"
    if ratio:
        subplot = "R"
    elif significance:
        subplot = "S"

    s.draw(subplot)


if __name__ == '__main__':
    main()
