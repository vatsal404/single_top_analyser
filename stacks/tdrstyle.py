#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
File        : tdrstyle.py
Description : tdrStyle plot settings
"""


# def tdrGrid(gridOn):
#     tdrStyle.SetPadGridX(gridOn)
#     tdrStyle.SetPadGridY(gridOn)
#
#
# # fixOverlay: Redraws the axis
# def fixOverlay():
#     gPad.RedrawAxis()


def setTDRStyle(rt, style):
    # for the canvas:
    style.SetCanvasBorderMode(0)
    style.SetCanvasColor(rt.kWhite)
    style.SetCanvasDefH(600)  # Height of canvas
    style.SetCanvasDefW(600)  # Width of canvas
    style.SetCanvasDefX(0)  # Position on screen
    style.SetCanvasDefY(0)

    style.SetPadBorderMode(0)
    # tdrStyle.SetPadBorderSize(Width_t size = 1)
    style.SetPadColor(rt.kWhite)
    style.SetPadGridX(False)
    style.SetPadGridY(False)
    style.SetGridColor(0)
    style.SetGridStyle(3)
    style.SetGridWidth(1)

    # For the frame:
    style.SetFrameBorderMode(0)
    style.SetFrameBorderSize(1)
    style.SetFrameFillColor(0)
    style.SetFrameFillStyle(0)
    style.SetFrameLineColor(1)
    style.SetFrameLineStyle(1)
    style.SetFrameLineWidth(1)

    # For the histo:
    # tdrStyle.SetHistFillColor(1)
    # tdrStyle.SetHistFillStyle(0)
    style.SetHistLineColor(1)
    style.SetHistLineStyle(0)
    style.SetHistLineWidth(1)
    style.SetHistTopMargin(0.15)
    # tdrStyle.SetLegoInnerR(Float_t rad = 0.5)
    # tdrStyle.SetNumberContours(Int_t number = 20)

    style.SetEndErrorSize(2)
    # tdrStyle.SetErrorMarker(20)
    # tdrStyle.SetErrorX(0.)

    style.SetMarkerStyle(20)

    # For the fit/function:
    style.SetOptFit(1)
    style.SetFitFormat("5.4g")
    style.SetFuncColor(2)
    style.SetFuncStyle(1)
    style.SetFuncWidth(1)

    # For the date:
    style.SetOptDate(0)
    # tdrStyle.SetDateX(Float_t x = 0.01)
    # tdrStyle.SetDateY(Float_t y = 0.01)

    # For the statistics box:
    style.SetOptFile(0)
    style.SetOptStat(0)  # To display the mean and RMS:   SetOptStat("mr")
    style.SetStatColor(rt.kWhite)
    style.SetStatFont(42)
    style.SetStatFontSize(0.025)
    style.SetStatTextColor(1)
    style.SetStatFormat("6.4g")
    style.SetStatBorderSize(1)
    style.SetStatH(0.1)
    style.SetStatW(0.15)
    # tdrStyle.SetStatStyle(Style_t style = 1001)
    # tdrStyle.SetStatX(Float_t x = 0)
    # tdrStyle.SetStatY(Float_t y = 0)

    # Margins:
    style.SetPadTopMargin(0.05)
    style.SetPadBottomMargin(0.13)
    style.SetPadLeftMargin(0.16)
    style.SetPadRightMargin(0.02)

    # For the Global title:

    style.SetOptTitle(0)
    style.SetTitleFont(42)
    style.SetTitleColor(1)
    style.SetTitleTextColor(1)
    style.SetTitleFillColor(10)
    style.SetTitleFontSize(0.05)
    # tdrStyle.SetTitleH(0) # Set the height of the title box
    # tdrStyle.SetTitleW(0) # Set the width of the title box
    # tdrStyle.SetTitleX(0) # Set the position of the title box
    # tdrStyle.SetTitleY(0.985) # Set the position of the title box
    # tdrStyle.SetTitleStyle(Style_t style = 1001)
    # tdrStyle.SetTitleBorderSize(2)

    # For the axis titles:

    style.SetTitleColor(1, "XYZ")
    style.SetTitleFont(42, "XYZ")
    style.SetTitleSize(0.06, "XYZ")
    # tdrStyle.SetTitleXSize(Float_t size = 0.02) # Another way to set the size?
    # tdrStyle.SetTitleYSize(Float_t size = 0.02)
    style.SetTitleXOffset(0.9)
    style.SetTitleYOffset(1.25)
    # tdrStyle.SetTitleOffset(1.1, "Y") # Another way to set the Offset

    # For the axis labels:

    style.SetLabelColor(1, "XYZ")
    style.SetLabelFont(42, "XYZ")
    style.SetLabelOffset(0.007, "XYZ")
    style.SetLabelSize(0.05, "XYZ")

    # For the axis:

    style.SetAxisColor(1, "XYZ")
    style.SetStripDecimals(True)
    style.SetTickLength(0.03, "XYZ")
    style.SetNdivisions(510, "XYZ")
    style.SetPadTickX(1)  # To get tick marks on the opposite side of the frame
    style.SetPadTickY(1)

    # Change for log plots:
    style.SetOptLogx(0)
    style.SetOptLogy(0)
    style.SetOptLogz(0)

    # Postscript options:
    style.SetPaperSize(20., 20.)
    # tdrStyle.SetLineScalePS(Float_t scale = 3)
    # tdrStyle.SetLineStyleString(Int_t i, const char* text)
    # tdrStyle.SetHeaderPS(const char* header)
    # tdrStyle.SetTitlePS(const char* pstitle)

    # tdrStyle.SetBarOffset(Float_t baroff = 0.5)
    # tdrStyle.SetBarWidth(Float_t barwidth = 0.5)
    # tdrStyle.SetPaintTextFormat(const char* format = "g")
    # tdrStyle.SetPalette(Int_t ncolors = 0, Int_t* colors = 0)
    # tdrStyle.SetTimeOffset(Double_t toffset)
    # tdrStyle.SetHistMinimumZero(kTRUE)

    style.SetHatchesLineWidth(5)
    style.SetHatchesSpacing(0.05)

    style.cd()
