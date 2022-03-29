import ROOT
import math
import os
import sys
import CMS_lumi
import array
import tdrstyle

STACKED=0
NORMALIZED=1

class Stackhists:
    def __init__(self, integrlumi):
        self.mcfilelist = []
        self.mclabellist = []
        self.mccolorlist = []
        self.mcpatternlist = []
        self.datafile=None
        self.datafilelist = [] # you can have more than one data file
        self.xseclist = []
        self.sflist = []
        self.resflist = []

        self.mcrootfiles = []
        self.mccounterhistfiles = []
        self.mcfilecounterhistlist = []
        self.datarootfiles = []
        
        self.integrlumi = integrlumi
        # bounding box location for the legends
        self.legend_x1=0.19
        self.legend_y1=0.7
        self.legend_x2=0.88
        self.legend_y2=0.87

        self.histogramlist = []
        self.xtitles = []
        self.ytitles = []
        self.drawmodes = []
        self.drawoptions = []
        self.isLogy = []
        self.ymin = [] # histogram minimum values
        self.ymax = [] # histogram maximum values
        self.binlists = []
        self.tdrStyle=tdrstyle.setTDRStyle()
        ROOT.gROOT.SetStyle("tdrStyle")
        pass

    def __del__(self):
        print("Destruct")

    def prepare(self):
        """Open ROOT files and calculate scale factors for histograms
        """

        #open ROOT files where histograms reside in MC and data
        for afile, cfile in zip(self.mcfilelist, self.mcfilecounterhistlist):
            atfile = ROOT.TFile(afile)
            if cfile=="":
                cfile = afile
            ctfile = ROOT.TFile(cfile)
            self.mcrootfiles.append(atfile)
            self.mccounterhistfiles.append(ctfile)

        """
        for afile in self.datafilelist:
            atfile = ROOT.TFile(afile)
            self.datarootfiles.append(atfile)
        """

        # calculate scaling factors for MC
        # look for 
        for afile, cfile, xsec, id in zip(self.mcfilelist, self.mcfilecounterhistlist, self.xseclist, range(len(self.xseclist))):
            atfile = ROOT.TFile(afile)
            if cfile=="":
                cfile = afile
            ctfile = ROOT.TFile(cfile)
            ahist = ctfile.Get("hcounter_nocut") # this should contain all entries before cuts
            prehist = ctfile.Get("hnevents_pglep_cut000")
            posthist = ctfile.Get("hnevents_cut000")
            if ahist == None and (prehist == None or posthist == None):
                print("counter histogram doesn\'t exist, will proceed with histintegaral=1. Be sure to put 1/histintegral in the scalefactor!")
                self.sflist[id] *= xsec * self.integrlumi 
                self.resflist[id] *= 1.0
            else:
                histintegral = ahist.Integral()
                preevents = prehist.Integral()
                postevents = posthist.Integral()
                resf = preevents / postevents if postevents != 0 else 1.0
                # all histograms should be scaled by this factor
                if "WJetsToLNu_inclHT100" in cfile:
                    histintegral = histintegral*0.96
                self.sflist[id] *= xsec * self.integrlumi / histintgral
                self.resflist[id] *= resf

    def setupStyle(self, colorlist=None, patternlist=None, alpha=1.0):
        self.fillalpha = alpha
        self.colorlist = []
        if colorlist == None:
            self.colorlist = [ ROOT.TColor.GetColor('#cc0000'), ROOT.TColor.GetColor('#ff6666'),
                    ROOT.TColor.GetColor('#660000'), ROOT.TColor.GetColor('#ff9933'),
                    ROOT.TColor.GetColor('#000099'), ROOT.TColor.GetColor('#990099'),
                    ROOT.TColor.GetColor('#00cccc'), ROOT.TColor.GetColor('#ff66ff'),
                    ROOT.TColor.GetColor('#d0cfd4'), ROOT.TColor.GetColor('#000000'),
                    ROOT.TColor.GetColor('#99CC66'), ROOT.TColor.GetColor('#6B8551'),
                    ROOT.TColor.GetColor('#908DCC'), ROOT.TColor.GetColor('#4F4D80') ]
            #self.colorlist = [ROOT.kRed+1, ROOT.kCyan+2, ROOT.kMagenta+2, ROOT.kOrange+1, ROOT.kGreen+1, ROOT.kBlue+2,  ROOT.kSpring+2]
        else:
            self.colorlist = colorlist

        self.patternlist = []
        if patternlist == None:
            for i in range(10):
                self.patternlist.append(1001)
        else:
            self.patternlist = patternlist

        ROOT.gROOT.SetStyle("tdrStyle")
        #tdrstyle.setTDRStyle()
        pass

    def addChannel(self, rootfile, label, colorindex, patternindex=0, isMC=True, xsec=1.0, scalefactor=1.0, counterhistogramroot=""):
        if os.path.isfile(rootfile):
            
            if isMC:
                self.mcfilelist.append(rootfile)
                self.mcfilecounterhistlist.append(counterhistogramroot)
                self.mclabellist.append(label) # if same label, then the histograms will be added together
                self.mccolorlist.append(colorindex)
                self.mcpatternlist.append(patternindex)
                self.xseclist.append(xsec)
                self.sflist.append(scalefactor)
                self.resflist.append(scalefactor)
            else:
                self.datafilelist.append(rootfile)
        else:
            print('Cannot add file %s, it doesn\'t exist'%rootfile)
            print('Please Check')
            #sys.exit(-1)
        pass

    def addHistogram(self, histname, xtitle="", ytitle="", drawmode=STACKED, drawoption="", isLogy=False, ymin=-1111, ymax=-1111, binlist=[]):
        self.histogramlist.append(histname)
        self.xtitles.append(xtitle)
        self.ytitles.append(ytitle)
        self.drawmodes.append(drawmode)
        self.drawoptions.append(drawoption)
        self.isLogy.append(isLogy)
        self.ymin.append(ymin)
        self.ymax.append(ymax)
        self.binlists.append(binlist)

    def MakeSoverB(self, BG_hist_stack, S_hist, label, S_peak_bin, below=None):
        nbins = BG_hist_stack.GetNbinsX()
        BG_peak_bin = BG_hist_stack.GetMaximumBin()

        if below == None :
            if S_peak_bin < BG_peak_bin : below = True
            elif S_peak_bin > BG_peak_bin : below = False
            elif S_peak_bin < nbins/2 : below = True
            else : below = False

        cutinfo = ""
        if below : cutinfo = "x<cut"
        else : cutinfo = "x>cut"

        BG_int = self.MakeCumulative(BG_hist_stack, 1, nbins+1, below)
        S_int = self.MakeCumulative(S_hist, 1, nbins+1, below)

        s_over_b = BG_int.Clone()
        s_over_b.Reset()

        arr = label.split('x', 1)
        M = 1.
        if len(arr)>1:
            M = float(arr[1])

        for ix in range(1, nbins+1):
            if BG_int.GetBinContent(ix) > 0 :
                val = (S_int.GetBinContent(ix)/M)/math.sqrt(BG_int.GetBinContent(ix))
            elif BG_int.GetBinContent(ix) < 0 :
                print("BG integral <0")
                val = 0
            elif BG_int.GetBinContent(ix) == 0 : val = 0

            s_over_b.SetBinContent(ix, val)

        return s_over_b, cutinfo

    def MakeCumulative(self, hist, low, high, below):
        out = hist.Clone(hist.GetName()+'_cumul')
        out.Reset()
        prev = 0
        if below : to_scan = range(low, high)
        else : to_scan = range(high-1, low-1, -1)
        for ix in to_scan :
            val = prev + hist.GetBinContent(ix)
            out.SetBinContent(ix, val)
            prev = val
        return out

    def draw(self, subplot):
        self.prepare()
        for histname, xtitle, ytitle, mode, drawoption, isLogy, ymin, ymax, binlist in zip(self.histogramlist, self.xtitles, self.ytitles, self.drawmodes, self.drawoptions, self.isLogy, self.ymin, self.ymax, self.binlists):
            self.createStacks(histname, xtitle, ytitle, mode, drawoption, isLogy, ymin, ymax, binlist, subplot)

    def createStacks(self, histname, xtitle, ytitle, mode, option="", isLogy=False, ymin=-1111, ymax=-1111, binlist=[], subplot='R'):
        # now stack
        
        hs = ROOT.THStack()
        tl = ROOT.TLegend(self.legend_x1, self.legend_y1, self.legend_x2, self.legend_y2)
        tl.SetNColumns(3)
        tl.SetTextAlign(12)
        tl.SetMargin(0.12)
        tl.SetColumnSeparation(0.01)
        histgroup = dict()
        labellist = []

        # adding signal contribution 
        signalhist = None
        signalhistlist = []
        mchistsum = None

        xbins = []
        nrebins=-1
        if len(binlist)>0:
            xbins = array.array('d', binlist)
            nrebins=len(binlist)-1

#        if "cut000" in histname: print("Rescaled histogram : "+histname)
        for ifile in range(len(self.mcfilelist)):
            #afile = ROOT.TFile(self.mcfilelist[ifile])
            #ahist = afile.Get(histname)
            ahist = self.mcrootfiles[ifile].Get(histname)
            if nrebins>0: ahist=ahist.Rebin(nrebins,histname+'rebinned_mc'+str(ifile),xbins)
            ahist.SetBinContent(ahist.GetNbinsX(), ahist.GetBinContent(ahist.GetNbinsX()) + ahist.GetBinContent(ahist.GetNbinsX()+1))

            if ahist == None:
                print("histogram %s not found in %s"%(histname, self.mcfilelist[ifile]))
                print("quitting")
                sys.exit(-1)
            else:
                if "cut000" in histname:
                    ahist.Scale(self.sflist[ifile]*self.resflist[ifile])
                else:
                    ahist.Scale(self.sflist[ifile])

                # group by labels
                label = self.mclabellist[ifile]
                if label not in histgroup:
                    histgroup[label] = ahist
                    labellist.append(label) # need to take care of the order
                else:
                    histgroup[label].Add(ahist)

                if 'LFV' not in label:
                    if mchistsum == None:
                        mchistsum = ahist.Clone("mchistsum")
                    else:
                        mchistsum.Add(ahist)
                    ahist.SetFillColorAlpha(self.colorlist[self.mccolorlist[ifile]], self.fillalpha)
                    ahist.SetLineColor(self.colorlist[self.mccolorlist[ifile]])
                    ahist.SetFillStyle(self.patternlist[self.mcpatternlist[ifile]])
                    #ahist.UseCurrentStyle()
                else:
                    signalhist = ahist
                    ahist.SetLineColor(self.colorlist[self.mccolorlist[ifile]])
                    signalhistlist.append(ahist)

        normevts = dict()
        for label in labellist:
            if not "LFV" in label:
                normevts[label] = histgroup[label].Integral()
        renormevts_list=sorted(normevts.items(),key=lambda x:x[1],reverse=True)
        reordered_labellist = [i[0] for i in renormevts_list]
        if isLogy: reordered_labellist.reverse()

        for label in reordered_labellist:
            ahist = histgroup[label]
            #print(label+" : %f"%ahist.Integral())
            if 'LFV' not in label:
                if mode == NORMALIZED:
                    ahistcopy = ahist.Clone()
                    normscale = ahistcopy.Integral()
                    ahistcopy.Scale(1.0/normscale)
                    hs.Add(ahistcopy)
                    #tl.AddEntry(ahistcopy, label, "F")
                else:
                    hs.Add(ahist)
                    #tl.AddEntry(ahist, label, "F")
            #else:
                #tl.AddEntry(ahist, label, "L")

        finaldatahist = None
        if self.datafilelist:
            for ifile in self.datafilelist:
                atfile = ROOT.TFile(ifile)
                ahist = atfile.Get(histname)
                if nrebins>0: ahist=ahist.Rebin(nrebins,histname+'rebinned_data',xbins)
                ahist.SetBinContent(ahist.GetNbinsX(), ahist.GetBinContent(ahist.GetNbinsX()) + ahist.GetBinContent(ahist.GetNbinsX()+1))
                if ahist is None:
                    print("histogram %s not found in %s"%(histname, self.datafilelist[ifile]))
                    sys.exit(-1)
                else:
                    if finaldatahist == None:
                        finaldatahist = ahist.Clone("finaldata")
                    else:
                        finaldatahist.Add(ahist)
            #print("Data : %i"%finaldatahist.Integral())
            if mode == NORMALIZED:
                normscale = finaldatahist.Integral()
                finaldatahist.Scale(1.0/normscale)
        
            # Legend add entry
            tl.AddEntry(finaldatahist, "Data", "P")
        #inverse_labellist = reversed(labellist)
        #for label in inverse_labellist:
        for label in labellist:
            if not 'LFV' in label:
                ahist = histgroup[label]
                tl.AddEntry(ahist, label, "F")
        for label in labellist:
            if 'LFV' in label:
                ahist = histgroup[label]
                tl.AddEntry(ahist, label, "F")
#            if 'LFV' not in label:
#                tl.AddEntry(ahist, label, "F")
#            else:
#                tl.AddEntry(ahist, label, "L")

        c1 = None
        if not self.datafilelist:
            c1 = ROOT.TCanvas("c1", "c1", 600, 600)
        else:
            c1 = ROOT.TCanvas("c1", "c1", 600, 700)

        """
        c1.SetLeftMargin(0.15)
        c1.SetBottomMargin(0.15)
        c1.SetTopMargin(0.06)
        c1.SetRightMargin(0.10)
        """

        c1_top = None
        if not self.datafilelist:
            c1_top = ROOT.TPad("c1_top", "top", 0.01, 0.01, 0.99, 0.99)
        else:
            c1_top = ROOT.TPad("c1_top", "top", 0.01, 0.33, 0.99, 0.99)
        c1_top.Draw()
        c1_top.cd()
        c1_top.SetTopMargin(0.1)
        c1_top.SetBottomMargin(0.01)
        if not self.datafilelist:
            c1_top.SetBottomMargin(0.13)
        c1_top.SetRightMargin(0.1)
        
        # log y scale
        if isLogy:
            c1_top.SetLogy(isLogy)

        if mode == STACKED:
            hs.Draw(option)
        else:
            hs.Draw("nostack " + option)
        xaxis = hs.GetXaxis()
        xaxis.SetTitle(xtitle)
        xaxis.SetNdivisions(6,5,0)
        xaxis.SetTitleSize(0.05)
        xaxis.SetTitleOffset(1.2)
        #xaxis.SetMaxDigits(4)
        
        sig_max = -1
        for sighist in signalhistlist:
            if sig_max<sighist.GetMaximum():
                sig_max = sighist.GetMaximum()

        # Set vertical range
        max1 = hs.GetMaximum()
        max2 = -1
        max3 = sig_max
        total_max = max(max(max1,max2),max3)
        if self.datafilelist:
            max2 = finaldatahist.GetMaximum()
        if ymin != -1111:
            hs.SetMinimum(ymin)
        if ymax != -1111:
            hs.SetMaximum(ymax)
        elif isLogy:
            hs.SetMaximum(total_max**1.65)
        else:
            hs.SetMaximum(total_max*1.65)

        #if signalhist is not None:        
        #    signalhist.SetLineWidth(3)
        #    signalhist.Draw("same Hist")

        for sighist in signalhistlist:
            sighist.SetLineWidth(3)
            sighist.Draw("same Hist")

        if self.datafilelist:
            finaldatahist.SetMarkerStyle(ROOT.kFullCircle)
            finaldatahist.Draw("sameerr P")

        yaxis = hs.GetYaxis()
        yaxis.SetTitle(ytitle)
        yaxis.SetNdivisions(6,5,0)
        yaxis.SetMaxDigits(3)
        
        if not isLogy:
            ROOT.TGaxis.SetExponentOffset(-0.08,0.01,"y")

        if self.datafilelist:
            # Ratio plot
            if subplot == "R":
                hstackhist = mchistsum
                ratiohist = finaldatahist.Clone("ratiohist")
                ratiohist.Divide(hstackhist)
                for n in range(finaldatahist.GetNbinsX()):
                    newerror=ratiohist.GetBinContent(n)/math.sqrt(finaldatahist.GetBinContent(n)) if finaldatahist.GetBinContent(n)!=0 else 0
                    ratiohist.SetBinError(n,newerror)
                ratiohist.SetMinimum(0.3)
                ratiohist.SetMaximum(1.7)

                tl.Draw()
                c1_top.Modified()
                #CMS_lumi.CMS_lumi(c1_top, 4, 11)
                CMS_lumi.CMS_lumi(c1_top, 4, 0)
                c1_top.cd()
                c1_top.Update()
                c1_top.RedrawAxis()

                c1.cd()
                c1_bottom = ROOT.TPad("c1_bottom", "bottom", 0.01, 0.01, 0.99, 0.32)
                c1_bottom.Draw()
                c1_bottom.cd()
                c1_bottom.SetTopMargin(0.02)
                c1_bottom.SetBottomMargin(0.3)
                c1_bottom.SetRightMargin(0.1)
                c1_bottom.SetGridx(1)
                c1_bottom.SetGridy(1)
                ratiohist.Draw("err")

                xaxis = ratiohist.GetXaxis()
                xaxis.SetTitle(xtitle)
                xaxis.SetNdivisions(6,5,0)
                xaxis.SetTitleSize(0.12)
                xaxis.SetLabelSize(0.10)

                yaxis = ratiohist.GetYaxis()
                yaxis.SetTitle("Data/Exp.")
                yaxis.SetNdivisions(6,5,0)
                yaxis.SetTitleSize(0.1)
                yaxis.SetLabelSize(0.08)
                yaxis.SetTitleOffset(0.7)
                yaxis.SetLabelOffset(0.007)

            #Significance plot (SoverB hist)
            elif subplot == "S":
                soverbhistlist = []
                M = -1
                S_peak_bin = -1
                for label in labellist :
                    if 'LFV T' in label:
                        Thist = histgroup[label]
                        S_peak_bin = Thist.GetMaximumBin()
                        break
                for label in labellist :
                    if 'LFV' in label:
                        sighist = histgroup[label]
                        soverbhist, cutinfo = self.MakeSoverB(mchistsum, sighist, label, S_peak_bin)
                        soverbhistlist.append(soverbhist)
                        tempM = soverbhist.GetMaximum()
                        if M < tempM : M = tempM

                tl.Draw()
                c1_top.Modified()
                #CMS_lumi.CMS_lumi(c1_top, 4, 11)
                CMS_lumi.CMS_lumi(c1_top, 4, 0)
                c1_top.cd()
                c1_top.Update()
                c1_top.RedrawAxis()

                c1.cd()
                c1_bottom = ROOT.TPad("c1_bottom", "bottom", 0.01, 0.01, 0.99, 0.32)
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
                    soverbh.SetMaximum(M*1.2)
                    soverbh.SetLineWidth(2)
                    soverbh.SetLineColor(self.colorlist[self.mccolorlist[n]])
                    n = n+1
                    soverbh.Draw("same Hist")
                    if n == 1 :
                        soverbtitle = "S/#sqrt{B}"+" ( "+cutinfo+" )"
                        xaxis = soverbh.GetXaxis()
                        xaxis.SetTitle(xtitle)
                        xaxis.SetNdivisions(6,5,0)
                        xaxis.SetTitleSize(0.12)
                        xaxis.SetLabelSize(0.10)

                        yaxis = soverbh.GetYaxis()
                        yaxis.SetTitle(soverbtitle)
                        yaxis.SetNdivisions(6,5,0)
                        yaxis.SetTitleSize(0.1)
                        yaxis.SetLabelSize(0.08)
                        yaxis.SetTitleOffset(0.7)
                        yaxis.SetLabelOffset(0.007)

            c1_bottom.Modified()
            c1_bottom.RedrawAxis()
        else:
            hstackhist = mchistsum

            tl.Draw()
            c1_top.Modified()
            #CMS_lumi.CMS_lumi(c1_top, 4, 11)
            CMS_lumi.CMS_lumi(c1_top, 4, 0)
            c1_top.cd()
            c1_top.Update()
            c1_top.RedrawAxis()

        frame = c1.GetFrame()
        if not self.datafilelist:
            frame = c1_top.GetFrame()
        frame.Draw()
        path = ""
        if subplot == "R":
            path = "plot_ratio_"+str(self.integrlumi)
            if not os.path.isdir(path):
                os.mkdir(path)
        elif subplot == "S":
            path = "plot_snb_"+str(self.integrlumi)
            if not os.path.isdir(path):
                os.mkdir(path)
        if not self.datafilelist:
            if(isLogy):
                c1_top.SaveAs(path+"/"+histname+"_logy.pdf")
            else:
                c1_top.SaveAs(path+"/"+histname+"_nology.pdf")
        else:
            if(isLogy):
                c1.SaveAs(path+"/"+histname+"_logy.pdf")
            else:
                c1.SaveAs(path+"/"+histname+"_nology.pdf")
        c1.Close()
        pass
