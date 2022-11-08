#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
File        : stackhists.py
Developer      : Candan Dozen Altuntas < AT gmail [DOT] com>
Description : stackhists
"""
#from imp import source_from_cache
from ROOT import RDataFrame
import math
import os
import sys
import CMS_lumi
import array
import tdrstyle

STACKED = 0
NORMALIZED = 1


class StackHists:

    def __init__(self, ROOT, tdrStyle, integer_lumi, verbose=False):
        self.verbose = verbose
        self.c1, self.data_file, self.fill_alpha = None, None, None

        self.mc_file_list, self.mc_label_list = [], []
        self.mc_color_list, self.color_list, self.mc_pattern_list, self.pattern_list = [], [], [], []

        self.data_file_list = []  # you can have more than one data file
        self.xsec_list, self.event_num_list, self.sf_list, self.resf_list = [], [], [], []

        self.mc_root_files, self.mc_counter_hist_files, self.mc_file_counter_hist_list = [], [], []
        self.data_root_files = []

        self.integer_lumi = integer_lumi

        # bounding box location for the legends
        #self.legend_x1, self.legend_y1, self.legend_x2, self.legend_y2 = 0.7, 0.19, 0.88, 0.87
        self.legend_x1, self.legend_y1, self.legend_x2, self.legend_y2 = 0.6, 0.76, 0.8, 0.85

        self.histogram_list, self.x_titles, self.y_titles, self.draw_modes, self.draw_options = [], [], [], [], []
        self.is_logy, self.bin_lists = [], []
        self.ymin, self.ymax = [], []  # histogram maximum values

        # Initialize tdrStyle
        self.tdrStyle = tdrstyle.setTDRStyle(ROOT, tdrStyle)

        # Set tdrStyle
        ROOT.gROOT.SetStyle("tdrStyle")
        self.ROOT = ROOT

        # Print all defined variables if verbosity is requested
        if verbose:
            print("~~~~~~~~~~~~~~~~~ StackHists class attributes ~~~~~~~~~~~~~~~~~")
            for k, v in self.__dict__.items():
                print(k, ":", v)
            print("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")

    def __del__(self):
        print("Destruct")

    def prepare_root_files(self):
        """Open ROOT files where histograms reside in MC and data
        """
        for mc_file, mc_cntr_hist_file in zip(self.mc_file_list, self.mc_file_counter_hist_list):
            t_file_mc = self.ROOT.TFile(mc_file)
            if mc_cntr_hist_file == "":
                mc_cntr_hist_file = mc_file
            t_file_mc_cntr_hist = self.ROOT.TFile(mc_cntr_hist_file)
            self.mc_root_files.append(t_file_mc)
            self.mc_counter_hist_files.append(t_file_mc_cntr_hist)

        """
        for mc_file in self.data_file_list:
            t_file_mc = self.ROOT.TFile(mc_file)
            self.data_root_files.append(t_file_mc)
        """

    def prepare_scaling_factors(self):
        """Calculate scaling factors for MC

        TODO what is "look for" :), there was a comment just like : # look for
        """
        
        zipped_lists = zip(self.mc_file_list,
                           self.mc_file_counter_hist_list,
                           self.xsec_list,
                           self.event_num_list,
                           range(len(self.xsec_list))
                           )
        for mc_file, mc_cntr_hist_file, xsec, event_num, range_id in zipped_lists:
            t_file_mc = self.ROOT.TFile(mc_file)
            if mc_cntr_hist_file == "":
                mc_cntr_hist_file = mc_file
            
            t_file_mc_cntr_hist = self.ROOT.TFile(mc_cntr_hist_file)
            #a_hist=None
            a_hist = t_file_mc_cntr_hist.Get("hgenEventSumw_nocut")# this should contain all entries before cuts to UseHistIntegral
            if a_hist == None:
                print("Counter histogram does not exist, will proceed with histintegaral=1. "
                      "Be sure to put 1/histintegral in the scalefactor!")
            #    self.sf_list[range_id] *= xsec * self.integer_lumi
            else:
                hist_integral = a_hist.Integral()
                print("hist integral==", hist_integral)
                self.sf_list[range_id] *= xsec * self.integer_lumi / hist_integral
                print(str(mc_file.split('/')[-1]) + " scaling with hist integral: ", self.sf_list[range_id])
    
            '''    dataFrame = RDataFrame('Runs',mc_cntr_hist_file)
                print (dataFrame.GetColumnNames())
                sumOfWeights = (dataFrame.Sum("genEventSumw"))
                SoW = sumOfWeights.GetValue()
                print("sum of gen weight ==", sumOfWeights.GetValue())
                self.sf_list[range_id] *=xsec * self.integer_lumi / SoW'''
            

    def setupStyle(self, color_list=None, pattern_list=None, alpha=1.0):
        """Setup style
        """
        self.fill_alpha = alpha
        if color_list == None:
            self.color_list = [self.ROOT.TColor.GetColor('#cc0000'), self.ROOT.TColor.GetColor('#ff6666'),
                               self.ROOT.TColor.GetColor('#660000'), self.ROOT.TColor.GetColor('#ff9933'),
                               self.ROOT.TColor.GetColor('#000099'), self.ROOT.TColor.GetColor('#990099'),
                               self.ROOT.TColor.GetColor('#00cccc'), self.ROOT.TColor.GetColor('#ff66ff'),
                               self.ROOT.TColor.GetColor('#d0cfd4'), self.ROOT.TColor.GetColor('#000000'),
                               self.ROOT.TColor.GetColor('#99CC66'), self.ROOT.TColor.GetColor('#6B8551'),
                               self.ROOT.TColor.GetColor('#908DCC'), self.ROOT.TColor.GetColor('#4F4D80')]
            # self.color_list = [self.ROOT.kRed + 1, self.ROOT.kCyan + 2, self.ROOT.kMagenta + 2, self.ROOT.kOrange + 1,
            #                    self.ROOT.kGreen + 1, self.ROOT.kBlue + 2, self.ROOT.kSpring + 2]
        else:
            self.color_list = color_list

        if pattern_list == None:
            for i in range(10):
                self.pattern_list.append(1001)
        else:
            self.pattern_list = pattern_list

        # TODO check if it is still working
        # self.ROOT.gROOT.SetStyle("tdrStyle")
        # tdrstyle.setTDRStyle()
        pass

    def addChannel(self, root_file, label, color_index, pattern_index=0, isMC=True, xsec=1.0, event_num=1.0,
                   scale_factor=1.0, counter_histogram_root=""):
        """Add channel
        """
        if os.path.isfile(root_file):
            if isMC:
                self.mc_file_list.append(root_file)
                self.mc_file_counter_hist_list.append(counter_histogram_root)
                self.mc_label_list.append(label)  # if same label, then the histograms will be added together
                self.mc_color_list.append(color_index)
                self.mc_pattern_list.append(pattern_index)
                self.xsec_list.append(xsec)
                self.event_num_list.append(event_num)
                self.sf_list.append(scale_factor)
                self.resf_list.append(scale_factor)
            else:
                self.data_file_list.append(root_file)
        else:
            print(f'Cannot add file {root_file}, it does not exist')
            print('Please Check')
            # sys.exit(-1)
        pass

    def addHistogram(self, hist_name, x_title="", y_title="", draw_mode=STACKED, draw_option="", is_logy=False,
                     ymin=-1111, ymax=-1111, binlist=[]):
        """Add histogram
        """
        self.histogram_list.append(hist_name)
        self.x_titles.append(x_title)
        self.y_titles.append(y_title)
        self.draw_modes.append(draw_mode)
        self.draw_options.append(draw_option)
        self.is_logy.append(is_logy)
        self.ymin.append(ymin)
        self.ymax.append(ymax)
        self.bin_lists.append(binlist)

    def MakeSoverB(self, BG_hist_stack, S_hist, label, S_peak_bin, below=None):
        """Make Sover B"""
        n_bins = BG_hist_stack.GetNbinsX()
        BG_peak_bin = BG_hist_stack.GetMaximumBin()
        cut_info = "x<cut"
        if below == None:
            if S_peak_bin < BG_peak_bin or S_peak_bin < n_bins / 2:
                below = True
            else:
                below = False
                cut_info = "x>cut"

        BG_int = self.MakeCumulative(BG_hist_stack, 1, n_bins + 1, below)
        S_int = self.MakeCumulative(S_hist, 1, n_bins + 1, below)

        s_over_b = BG_int.Clone()
        s_over_b.Reset()

        arr = label.split('x', 1)
        M = 1.0
        if len(arr) > 1:
            M = float(arr[1])

        for ix in range(1, n_bins + 1):
            val = 0
            bin_ctx = BG_int.GetBinContent(ix)
            if bin_ctx > 0:
                val = (S_int.GetBinContent(ix) / M) / math.sqrt(BG_int.GetBinContent(ix))

            if self.verbose:
                print(f"BG_int bin context: {bin_ctx}, value: {val}")

            s_over_b.SetBinContent(ix, val)

        return s_over_b, cut_info

    def MakeCumulative(self, hist, low, high, below):
        out = hist.Clone(hist.GetName() + '_cumul')
        out.Reset()
        prev = 0
        if below:
            to_scan = range(low, high)
        else:
            to_scan = range(high - 1, low - 1, -1)
        for ix in to_scan:
            val = prev + hist.GetBinContent(ix)
            out.SetBinContent(ix, val)
            prev = val
        return out

    def draw(self, subplot):
        self.prepare_root_files()
        self.prepare_scaling_factors()

        self.c1 = self.ROOT.TCanvas("c1", "c1", 600, 700)
        self.c1.Print("plots.pdf[")
        
        for hist_name, x_title, y_title, mode, draw_option, is_logy, ymin, ymax, binlist in zip(self.histogram_list,
                                                                                                self.x_titles,
                                                                                                self.y_titles,
                                                                                                self.draw_modes,
                                                                                                self.draw_options,
                                                                                                self.is_logy,
                                                                                                self.ymin,
                                                                                                self.ymax,
                                                                                                self.bin_lists):
            self.createStacks(hist_name, x_title, y_title, mode, draw_option, is_logy, ymin, ymax, binlist, subplot)

        self.c1.Print("plots.pdf]")
        #self.c1.SaveAs("plots.C")
        self.c1.Close()

    def createStacks(self, hist_name, x_title, y_title, mode, option="", is_logy=False, ymin=-1111, ymax=-1111,
                     binlist=[], subplot='R'):
        """Create stacks
        """

        hs = self.ROOT.THStack()
        tl = self.ROOT.TLegend(self.legend_x1, self.legend_y1, self.legend_x2, self.legend_y2)
        tl.SetNColumns(1)
        tl.SetTextAlign(12)
        tl.SetMargin(0.12)
        tl.SetColumnSeparation(0.01)
        hist_group = dict()
        label_list = []
        # =============================================================================================================
        # Iterate over files and organize hist files
        # =============================================================================================================
        # adding signal contribution
        #signal_hist = None
        signal_hist_list = []
        mc_hist_sum = None

        xbins = []
        nrebins = -1
        if len(binlist) > 0:
            xbins = array.array('d', binlist)
            nrebins = len(binlist) - 1
        
        
        for ifile in range(len(self.mc_file_list)):
            ahist = self.mc_root_files[ifile].Get(hist_name)
            print("MC root files===",self.mc_file_list[ifile])
            print(f"hist: {hist_name}, entries={ahist.GetEntries()}")

            if nrebins > 0: 
                ahist = ahist.Rebin(nrebins, hist_name + 'rebinned_mc' + str(ifile), xbins)
            ahist.SetBinContent(ahist.GetNbinsX(),ahist.GetBinContent(ahist.GetNbinsX()) + ahist.GetBinContent(ahist.GetNbinsX() + 1))

            if ahist == None:
                print(f"histogram {hist_name} not found in {self.mc_file_list[ifile]}")
                print("quitting...")
                sys.exit(-1)
            else:
                ahist.Scale(self.sf_list[ifile])
                print("scale value per file", self.sf_list[ifile])

                # group by labels
                label = self.mc_label_list[ifile]
                if label not in hist_group:
                    hist_group[label] = ahist
                    label_list.append(label)  # need to take care of the order
                else:
                    hist_group[label].Add(ahist)

                if 'MC_Sig' not in label:
                    if mc_hist_sum == None:
                        mc_hist_sum = ahist.Clone("mchistsum")
                        
                    else:
                        mc_hist_sum.Add(ahist)
                    ahist.SetFillColorAlpha(self.color_list[self.mc_color_list[ifile]], self.fill_alpha)
                    ahist.SetLineColor(self.color_list[self.mc_color_list[ifile]])
                    ahist.SetFillStyle(self.pattern_list[self.mc_pattern_list[ifile]])
                    #print("mchistsum==",mc_hist_sum.GetEntries())
                    # ahist.UseCurrentStyle()
                else:
                    signal_hist = ahist
                    ahist.SetLineColor(self.color_list[self.mc_color_list[ifile]])
                    signal_hist_list.append(ahist)
            
        # =============================================================================================================
        # Organize for MC_Sig
        # =============================================================================================================
        normevts = dict()
        for label in label_list:
            if not "MC_Sig" in label:
                normevts[label] = hist_group[label].Integral()
        renormevts_list = sorted(normevts.items(), key=lambda x: x[1], reverse=True)
        reordered_labellist = [i[0] for i in renormevts_list]
        

        if is_logy:
            reordered_labellist.reverse()

        for label in reordered_labellist:
            ahist = hist_group[label]
            #print(label, ":", float(ahist.Integral()))
            # print(label+" : %f"%ahist.GetEntries())

            if 'MC_Sig' not in label:
                if mode == NORMALIZED:
                    ahistcopy = ahist.Clone()
                    normscale = ahistcopy.Integral()
                    print('normscale_label=', normscale)
                    ahistcopy.Scale(1.0 / normscale)
                    hs.Add(ahistcopy)
                    #tl.AddEntry(ahistcopy, label, "F")
                else:
                    hs.Add(ahist)
                    #tl.AddEntry(ahistcopy, label, "F")
    

        # =============================================================================================================
        # Organize final_data_hist
        # =============================================================================================================
        final_data_hist = None
        if self.data_file_list:
            print("DATA FILE LIST", self.data_file_list)
            for ifile in self.data_file_list:
                t_file_mc = self.ROOT.TFile(ifile)
                #print("datafile===", t_file_mc)
                ahist = t_file_mc.Get(hist_name)
                #print("histogram datafile names:", ahist)
                #print("nbins===", nrebins)
                if nrebins > 0:
                    ahist = ahist.Rebin(nrebins, hist_name + 'rebinned_data', xbins)

                ahist.SetBinContent(ahist.GetNbinsX(),
                                    ahist.GetBinContent(ahist.GetNbinsX()) + ahist.GetBinContent(ahist.GetNbinsX() + 1))

                if ahist == None:
                    print(f"histogram {hist_name} not found in {self.data_file_list[ifile]}")
                    print("exiting...")
                    sys.exit(-1) 
                else:
                    if final_data_hist == None:
                        final_data_hist = ahist.Clone("final_data_hist")
                    else:
                        final_data_hist.Add(ahist)
            print("Data hist Integral : %i"%final_data_hist.Integral())
            if mode == NORMALIZED:
                normscale = final_data_hist.Integral()
                final_data_hist.Scale(1.0 / normscale)
            

            # Legend add entry
            tl.AddEntry(final_data_hist, "Data", "P")

        # inverse_labellist = reversed(labellist)
        # for label in inverse_labellist:
        for label in label_list:
            if not 'MC_Sig' in label:
                ahist = hist_group[label]
                tl.AddEntry(ahist, label, "F")
        for label in label_list:
            if 'MC_Sig' in label:
                ahist = hist_group[label]
                tl.AddEntry(ahist, label, "F")

        if not self.data_file_list:
            c1_top = self.ROOT.TPad("c1_top", "top", 0.01, 0.01, 0.99, 0.99)
        else:
            c1_top = self.ROOT.TPad("c1_top", "top", 0.01, 0.33, 0.99, 0.99)

        c1_top.Draw()
        c1_top.cd()
        c1_top.SetTopMargin(0.05)
        c1_top.SetBottomMargin(0.01)
        if not self.data_file_list:
            c1_top.SetBottomMargin(0.13)
        c1_top.SetRightMargin(0.1)

        # log y scale
        if is_logy:
            c1_top.SetLogy(is_logy)

        if mode == STACKED:
            hs.Draw(option)
        else:
            hs.Draw("nostack " + option)

        
        hs.GetXaxis().SetTitle(x_title)
        hs.GetXaxis().SetNdivisions(6, 5, 0)
        hs.GetXaxis().SetTitleSize(0.05)
        hs.GetXaxis().SetTitleOffset(1.2)
        hs.GetXaxis().SetLabelSize(0.04)
        

        sig_max = -1
        for sighist in signal_hist_list:
            if sig_max < sighist.GetMaximum():
                sig_max = sighist.GetMaximum()
               

        # Set vertical range
        max1 = hs.GetMaximum()
        max2 = -1
        max3 = sig_max
        total_max = max(max(max1, max2), max3)

        if self.data_file_list:
            max2 = final_data_hist.GetMaximum()
           
        if ymin != -1111:
            hs.SetMinimum(ymin)
           
        if ymax != -1111:
            hs.SetMaximum(ymax)
            
        elif is_logy:
            hs.SetMaximum(total_max ** 1.65)
           
        else:
            hs.SetMaximum(total_max * 1.65)
            #hs.SetMaximum(max1)


        for sighist in signal_hist_list:
            sighist.SetLineWidth(3)
            sighist.Draw("same Hist")
           

        if self.data_file_list:
            final_data_hist.SetMarkerStyle(self.ROOT.kFullCircle)
            final_data_hist.Draw("sameerr P")
       
        hs.GetYaxis().SetTitle(y_title)
        hs.GetYaxis().SetNdivisions(6, 5, 0)
        hs.GetYaxis().SetMaxDigits(3)
        hs.GetYaxis().SetLabelSize(0.04)
        hs.GetYaxis().SetTitleSize(0.04)
    

        if not is_logy:
            self.ROOT.TGaxis.SetExponentOffset(-0.08, 0.01, "y")

        # =============================================================================================================
        # Organize final data list
        # =============================================================================================================
        if self.data_file_list:
            # Ratio plot
            if subplot == "R":
                hstackhist = mc_hist_sum
                ratiohist = final_data_hist.Clone("ratiohist")
                
                ratiohist.Divide(hstackhist)
                for n in range(final_data_hist.GetNbinsX()):
                    #print("finaldatahist NbinX",final_data_hist.GetNbinsX() )
                    #print("finaldatahist bincontent",final_data_hist.GetBinContent(n) )
                    if final_data_hist.GetBinContent(n) != 0:
                        new_error = ratiohist.GetBinContent(n) / math.sqrt(final_data_hist.GetBinContent(n))
                        #print("error==", new_error)
                    else:
                        new_error = 0
                    ratiohist.SetBinError(n, new_error)
                ratiohist.SetMinimum(0.3)
                ratiohist.SetMaximum(1.7)

                tl.Draw()
                c1_top.Modified()
                c1_top.Update()
                # CMS_lumi.CMS_lumi(c1_top, 4, 11)
                CMS_lumi.CMS_lumi(self.ROOT, c1_top, 4, 0)
                c1_top.cd() 
                c1_top.RedrawAxis()

                self.c1.cd()
                c1_bottom = self.ROOT.TPad("c1_bottom", "bottom", 0.01, 0.01, 0.99, 0.32)
                c1_bottom.Draw()
                c1_bottom.cd()
                c1_bottom.SetTopMargin(0.02)
                c1_bottom.SetBottomMargin(0.3)
                c1_bottom.SetRightMargin(0.1)
                c1_bottom.SetGridx(1)
                c1_bottom.SetGridy(1)
                ratiohist.Draw("err")

                ratiohist.GetXaxis().SetTitle(x_title)
                ratiohist.GetXaxis().SetNdivisions(6, 5, 0)
                ratiohist.GetXaxis().SetTitleSize(0.12)
                ratiohist.GetXaxis().SetLabelSize(0.10)
                
                ratiohist.GetYaxis().SetTitle("Data/Exp.")
                ratiohist.GetYaxis().SetNdivisions(6, 5, 0)
                ratiohist.GetYaxis().SetTitleSize(0.1)
                ratiohist.GetYaxis().SetLabelSize(0.08)
                ratiohist.GetYaxis().SetTitleOffset(0.7)
                ratiohist.GetYaxis().SetLabelOffset(0.007)
                

            # Significance plot (SoverB hist)
            elif subplot == "S":
                soverbhistlist = []
                M = -1
                S_peak_bin = -1
                
                for label in label_list:
                    if 'MC_Sig' in label:
                        sighist = hist_group[label]
                        soverbhist, cutinfo = self.MakeSoverB(mc_hist_sum, sighist, label, S_peak_bin)
                        #print("cutinfo=======", cutinfo)
                        soverbhistlist.append(soverbhist)
                        tempM = soverbhist.GetMaximum()
                        if M < tempM: M = tempM
            
                tl.Draw()
                c1_top.Modified()
                # CMS_lumi.CMS_lumi(c1_top, 4, 11)
                CMS_lumi.CMS_lumi(self.ROOT, c1_top, 4, 0)
                c1_top.cd()
                c1_top.Update()
                c1_top.RedrawAxis()
                
                self.c1.cd()
                c1_bottom = self.ROOT.TPad("c1_bottom", "bottom", 0.01, 0.01, 0.99, 0.32)
                c1_bottom.Draw()
                c1_bottom.cd()
                c1_bottom.SetTopMargin(0.02)
                c1_bottom.SetBottomMargin(0.3)
                c1_bottom.SetRightMargin(0.1)
                c1_bottom.SetGridx(1)
                c1_bottom.SetGridy(1)

                n = 0
                for soverbh in soverbhistlist:
                    soverbh.SetMinimum(0)
                    soverbh.SetMaximum(M * 1.2)
                    soverbh.SetLineWidth(2)
                    soverbh.SetLineColor(self.color_list[self.mc_color_list[n]])
                    n = n + 1
                    soverbh.Draw("same Hist")
                    if n == 1:
                        soverbtitle = "S/#sqrt{B}" + " ( " + cutinfo + " )"
                        soverbh.GetXaxis().SetTitle(x_title)
                        soverbh.GetXaxis().SetNdivisions(6, 5, 0)
                        soverbh.GetXaxis().SetTitleSize(0.12)
                        soverbh.GetXaxis().SetLabelSize(0.10)
                        
                        soverbh.GetYaxis().SetTitle(soverbtitle)
                        soverbh.GetYaxis().SetNdivisions(6, 5, 0)
                        soverbh.GetYaxis().SetTitleSize(0.1)
                        soverbh.GetYaxis().SetLabelSize(0.08)
                        soverbh.GetYaxis().SetTitleOffset(0.7)
                        soverbh.GetYaxis().SetLabelOffset(0.007)
                       

            c1_bottom.Modified()
            c1_bottom.Update()
            c1_bottom.RedrawAxis()
        else:
            hstackhist = mc_hist_sum

            tl.Draw()
            c1_top.Modified()
            c1_top.Update()
            # CMS_lumi.CMS_lumi(c1_top, 4, 11)
            CMS_lumi.CMS_lumi(self.ROOT, c1_top, 4, 0)
            c1_top.cd()
            c1_top.RedrawAxis()
        # =============================================================================================================

        frame = self.c1.GetFrame()
        if not self.data_file_list:
            frame = c1_top.GetFrame()
        frame.Draw()
        self.c1.Update()
        self.c1.cd()
        path = ""
        if subplot == "R":
            path = "plot_ratio_" + str(self.integer_lumi)
            if not os.path.isdir(path):
                os.mkdir(path)
        elif subplot == "S":
            path = "plot_snb_" + str(self.integer_lumi)
            if not os.path.isdir(path):
                os.mkdir(path)
        outfile = self.ROOT.TFile("stackhist_"+str(self.integer_lumi)+".root","RECREATE")
        outfile.cd()
        if not self.data_file_list:
            if (is_logy):
                c1_top.SaveAs(path + "/" + hist_name + "_logy.pdf")
                self.c1.Print("plots.pdf")
            else:
                c1_top.SaveAs(path + "/" + hist_name + "_nology.pdf")
                self.c1.Print("plots.pdf")
        else:
            if (is_logy):
                self.c1.SaveAs(path + "/" + hist_name + "_logy.pdf")
                self.c1.Print("plots.pdf")
            else:
                self.c1.SaveAs(path + "/" + hist_name + "_nology.pdf")
                self.c1.Print("plots.pdf")
        #pass
        for key, value in hist_group.items():
            pname = key.replace(" ","_")
            tmphist_copy = hist_group[key].Clone(hist_name+"_"+pname)
            tmphist_copy.Write()
        if final_data_hist:
            print("finaldatahist====",final_data_hist)
            final_data_hist_copy = final_data_hist.Clone("hstacked_data_"+hist_name)
            final_data_hist_copy.Write()
        else:
            mc_hist_sum_copy = mc_hist_sum.Clone("hstacked_mc_"+hist_name)
            mc_hist_sum_copy.Write()
        outfile.Save()
        outfile.Close()
        pass





