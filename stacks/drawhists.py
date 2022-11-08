#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
File        : drawhist.py
Developper     : Candan Dozen Altuntas < AT gmail [DOT] com>
Description : drawhist.py
"""
import copy
import ROOT
from importlib.resources import path
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
        rlumi["18"] = 150.0 / 137.65
        runs = ["16pre", "16post", "17", "18"]
 
    
    # for all DATA (2016+2017+2018)
    if year == "run2":
        print("run2")
        #s.addChannel("Run2.root", "data", 999, isMC=False)
    else:
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

def main(ratio, significance,logstyle, verbose, year):
    click.echo(f'Input Arguments: ratio:{ratio},significance:{significance} logstyle:{logstyle}, year:{year}')

    # Subplot ( Ratio plot : option -R, --ratio (default) / significance plot :  option -S, --significance )
    initial_tdrStyle = ROOT.TStyle("tdrStyle", "Style for P-TDR")
    tdrstyle.setTDRStyle(ROOT, initial_tdrStyle)

    sys = "_norm"
    # Lumi ratio dictionary for integrated Run2
    rlumi = {"16pre": 1., "16post": 1., "17": 1., "18": 1.}
    s, CMS_lumi.lumi_13TeV, rlumi, runs = get_prep_stack_hist(year, initial_tdrStyle, rlumi, verbose)

    s.setupStyle(alpha=1)
    CMS_lumi.extraText = ""
    #CMS_lumi.extraText = "Simulation"
    
    for run in runs:
        if run == "16pre":
            print("Run2016_pre")
            #s.addChannel(run+"/testw_jets.root", "W+jets", 3, isMC=True, xsec=rlumi[run]*61526700*0.9647, counterhistogramroot=run+"/testw_jets.root")
        
        elif run == "16post":
            print("Run2016_post")
        
        elif run == "17":
            print("Run2017")
            #s.addChannel(run+"/WJetsToLNu_inclHT100.root", "W+jets", 3, isMC=True, xsec=rlumi[run]*61526700*0.9645, counterhistogramroot=run+"/WJetsToLNu_inclHT100.root")
        
        elif run == "18":
            print("Run2018")

            #MC_Signal test in local
            s.addChannel("/home/cms/dozen-altuntas/FLY_project/CMSSW_12_3_2/src/fly/analyzed/Chris_test.root","MC_1",1,isMC=True, xsec=137,  counter_histogram_root="/home/cms/dozen-altuntas/FLY_project/CMSSW_12_3_2/src/fly/analyzed/Chris_test.root")
            s.addChannel("/home/cms/dozen-altuntas/FLY_project/CMSSW_12_3_2/src/fly/analyzed/Chris_test2.root","MC_2",2,isMC=True, xsec=137,  counter_histogram_root="/home/cms/dozen-altuntas/FLY_project/CMSSW_12_3_2/src/fly/analyzed/Chris_test2.root")
            s.addChannel("/home/cms/dozen-altuntas/FLY_project/CMSSW_12_3_2/src/fly/analyzed/Chris_test3.root","MC_3",3,isMC=True, xsec=137,  counter_histogram_root="/home/cms/dozen-altuntas/FLY_project/CMSSW_12_3_2/src/fly/analyzed/Chris_test3.root")
            s.addChannel("/home/cms/dozen-altuntas/FLY_project/CMSSW_12_3_2/src/fly/analyzed/Chris_test4.root","MC_4",4,isMC=True, xsec=137,  counter_histogram_root="/home/cms/dozen-altuntas/FLY_project/CMSSW_12_3_2/src/fly/analyzed/Chris_test4.root")
            
            #Data
            s.addChannel("/home/cms/dozen-altuntas/FLY_project/CMSSW_12_3_2/src/fly/analyzed/Chris_data.root","data", 999, isMC=False)
            s.addChannel("/home/cms/dozen-altuntas/FLY_project/CMSSW_12_3_2/src/fly/analyzed/Chris_data2.root","data", 999, isMC=False)
            s.addChannel("/home/cms/dozen-altuntas/FLY_project/CMSSW_12_3_2/src/fly/analyzed/Chris_data3.root","data", 999, isMC=False)
            s.addChannel("/home/cms/dozen-altuntas/FLY_project/CMSSW_12_3_2/src/fly/analyzed/Chris_data4.root","data", 999, isMC=False)
            
    # Muon Histograms
    #s.addHistogram('tightMuon_leading_pt_cut1', 'tight muon p_{T} [GeV]', 'Entries', draw_mode=stackhists.STACKED, draw_option='hist')
    #s.addHistogram('tightMuon_leading_pt_cut10', 'tight muon p_{T} [GeV]', 'Entries', draw_mode=stackhists.STACKED, draw_option='hist')

    # Jet Histograms
    s.addHistogram("hgood_jetpt_nocut", "good1 jet p_{T} [GeV]", "Entries", draw_mode=stackhists.STACKED, draw_option="hist",is_logy=logstyle, ymin=0.1)
    s.addHistogram("hgood_jetpt_cut0", "good2 jet p_{T} [GeV]", "Entries", draw_mode=stackhists.STACKED, draw_option="hist",is_logy=logstyle,ymin=0.1)
    s.addHistogram("hgood_jetpt_cut00", "good3 jet p_{T} [GeV]", "Entries", draw_mode=stackhists.STACKED, draw_option="hist",is_logy=logstyle,ymin=0.1)
    

    subplot = "R"
    if ratio:
        subplot = "R"
    elif significance:
        subplot = "S"

    s.draw(subplot)


if __name__ == '__main__':
    main()
