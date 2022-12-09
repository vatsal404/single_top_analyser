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
        CMS_lumi.lumi_13TeV = "41.48 fb^{-1}"
        s = stackhists.StackHists(ROOT, initial_tdrStyle, 41480, verbose)
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
           
            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-11-30/otherMC/PROC_ST_t-channel_top.root", "t-channel", 0, isMC=True, xsec=136.02, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_ST_t-channel_top.root")
            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_ST_t-channel_antitop.root", "t-channel", 0, isMC=True, xsec=80.95, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_ST_t-channel_antitop.root")
            # s-channel+tW
            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_ST_s-channel.root", "s-channel + tW", 1, isMC=True, xsec=3.68, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_ST_s-channel.root")
            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_ST_tW_top.root", "s-channel + tW", 1, isMC=True, xsec=35.85, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_ST_tW_top.root")
            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_ST_tW_antitop.root", "s-channel + tW", 1, isMC=True, xsec=35.85, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_ST_tW_antitop.root")
            # ttbar
            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_TTToSemiLeptonic.root", "ttbar", 2, isMC=True, xsec=364.351, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_TTToSemiLeptonic.root")
            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_TTTo2L2Nu.root", "ttbar", 2, isMC=True, xsec=87.31, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_TTTo2L2Nu.root")


            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_TTWJetsToLNu.root", "tt + X", 3, isMC=True, xsec=0.2045, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_TTWJetsToLNu.root")
            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_TTWJetsToQQ.root", "tt + X", 3, isMC=True, xsec=0.4062, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_TTWJetsToQQ.root")

            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_TTZToLLNuNu.root", "tt + X", 3, isMC=True, xsec=0.2529, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_TTZToLLNuNu.root")

            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_TTZToQQ.root", "tt + X", 3, isMC=True, xsec=0.5297, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_TTZToQQ.root")

            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_WJetsToLNu.root", " W-Jets", 4, isMC=True, xsec=61334.9, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_WJetsToLNu.root")

            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_WW.root", "Diboson", 5, isMC=True, xsec=118.7, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_WW.root")

            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_WZ.root", "Diboson", 5, isMC=True, xsec=47.13, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_WZ.root")
            
            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_ZZ.root", "Diboson", 5, isMC=True, xsec= 16.523, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_ZZ.root")

            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_DYJetsToLL_M-10to50.root", "Drell-Yan", 6, isMC=True, xsec=18610, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_DYJetsToLL_M-10to50.root")

            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_DYJetsToLL_M-50.root", "Drell-Yan", 6, isMC=True, xsec= 6025.2, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_DYJetsToLL_M-50.root")
#
            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-15To20_MuEnriched.root", "QCD", 7, isMC=True, xsec= 3336011, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-15To20_MuEnriched.root")

            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-20To30_MuEnriched.root", "QCD", 7, isMC=True, xsec= 3987854.9, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-20To30_MuEnriched.root")
            
            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-30To50_MuEnriched.root", "QCD", 7, isMC=True, xsec= 1705381, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-30To50_MuEnriched.root")

            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-50To80_MuEnriched.root", "QCD", 7, isMC=True, xsec=395178 , counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-50To80_MuEnriched.root")


            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-80To120_MuEnriched.root", "QCD", 7, isMC=True, xsec= 106889.4, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-80To120_MuEnriched.root")


            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-120To170_MuEnriched.root", "QCD", 7, isMC=True, xsec= 23773.61, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-120To170_MuEnriched.root")


            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-170To300_MuEnriched.root", "QCD", 7, isMC=True, xsec=  8292.9, counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-170To300_MuEnriched.root")

            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-300To470_MuEnriched.root", "QCD", 7, isMC=True, xsec=797.4  , counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-300To470_MuEnriched.root")

            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-470To600_MuEnriched.root", "QCD", 7, isMC=True, xsec= 56.6 , counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-470To600_MuEnriched.root")

            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-600To800_MuEnriched.root", "QCD", 7, isMC=True, xsec=25.1  , counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-600To800_MuEnriched.root")

            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-800To1000_MuEnriched.root", "QCD", 7, isMC=True, xsec=4.7  , counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-800To1000_MuEnriched.root")

            s.addChannel("/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-1000_MuEnriched.root", "QCD", 7, isMC=True, xsec=1.6  , counter_histogram_root="/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/PROC_QCD_Pt-1000_MuEnriched.root")

#
            #s.addChannel('/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-11-30/otherMC/Run2017E_SingleMuon.root', 'data', 9, isMC=False)
            s.addChannel('/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-12-08/genWeight/DATA17_SingleMuon.root', 'data', 9, isMC=False)
            #s.addChannel('/gridgroup/cms/greenberg/analysis/CMSSW_12_3_0/src/fly/results/2022-11-30/otherMC/Run2017D_SingleMuon.root', 'data', 9, isMC=False)

        elif run == "18":
            print("Run2018")

    ##Add Histograms
    s.addHistogram("tightMuon_leading_eta_cut1002", "tleadmueta", "Entries", draw_mode=stackhists.STACKED, draw_option="hist", is_logy=False)
    s.addHistogram("tightMuon_leading_pt_cut1002", "tmuonl_pt [GeV]", "Entries", draw_mode=stackhists.STACKED, draw_option="hist", is_logy=False)
    #s.addHistogram("tightMuon_leading_pt_cut1003", "tmuonl_pt [GeV]", "Entries", draw_mode=stackhists.STACKED, draw_option="hist", is_logy=False)
    #s.addHistogram("tightMuon_leading_eta_cut1003", "tmuonl_eta", "Entries", draw_mode=stackhists.STACKED, draw_option="hist", is_logy=False)
    

    subplot = "R"
    if ratio:
        subplot = "R"
    elif significance:
        subplot = "S"

    s.draw(subplot)


if __name__ == '__main__':
    main()
