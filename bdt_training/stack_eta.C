void stack_eta() {
    // Main MC and Data file
    TFile *file = TFile::Open("eta_ele2j1b_histograms.root");
    if (!file || file->IsZombie()) {
        std::cerr << "Cannot open ROOT file!" << std::endl;
        return;
    }

    // QCD (Multijet) file
    TFile *qcdFile = TFile::Open("multijet_template_ele2j1t_eta.root");
    if (!qcdFile || qcdFile->IsZombie()) {
        std::cerr << "Cannot open multijet ROOT file!" << std::endl;
        return;
    }

    // Retrieve histograms
    TH1D* signal_top     = (TH1D*)file->Get("signal_top");
    TH1D* signal_antitop = (TH1D*)file->Get("signal_antitop");
    TH1D* ttbar          = (TH1D*)file->Get("ttbar");
    TH1D* wjets          = (TH1D*)file->Get("wjets");
    TH1D* data           = (TH1D*)file->Get("data");
    TH1D* qcd            = (TH1D*)qcdFile->Get("multijet");

    if (!signal_top || !signal_antitop || !ttbar || !wjets || !data || !qcd) {
        std::cerr << "One or more histograms are missing!" << std::endl;
        return;
    }

    // Apply scale factors
    double sf_signal_top     = 1.0089;
    double sf_signal_antitop = 1.0089;
    double sf_ttbar          = 1.0089;
    double sf_wjets          = 1.0089;
    double sf_qcd            = 1.4192;

    signal_top->Scale(sf_signal_top);
    signal_antitop->Scale(sf_signal_antitop);
    ttbar->Scale(sf_ttbar);
    wjets->Scale(sf_wjets);
    qcd->Scale(sf_qcd);

    // Set fill colors
    signal_top->SetFillColor(kRed + 1);
    signal_antitop->SetFillColor(kRed - 4);
    ttbar->SetFillColor(kOrange - 2);
    wjets->SetFillColor(kGreen + 1);
    qcd->SetFillColor(kGray + 2);

    // Create stack
    THStack* stack = new THStack("stack", "BDT Score;BDT Score;Events");
    stack->Add(qcd); // QCD added first
    stack->Add(wjets);
    stack->Add(ttbar);
    stack->Add(signal_antitop);
    stack->Add(signal_top);

    // MC total for ratio
    TH1D* mc_total = (TH1D*)qcd->Clone("mc_total");
    mc_total->Add(wjets);
    mc_total->Add(ttbar);
    mc_total->Add(signal_antitop);
    mc_total->Add(signal_top);

    // Canvas and pads
    TCanvas* c = new TCanvas("c", "BDT Stack Plot", 800, 800);
    TPad* pad1 = new TPad("pad1", "Main", 0, 0.3, 1, 1.0);
    TPad* pad2 = new TPad("pad2", "Ratio", 0, 0.05, 1, 0.3);
    pad1->SetBottomMargin(0.02);
    pad2->SetTopMargin(0.05);
    pad2->SetBottomMargin(0.3);
    pad1->Draw();
    pad2->Draw();

    // Draw stack + data
    pad1->cd();
    stack->Draw("HIST");

    data->SetMarkerStyle(20);
    data->SetMarkerSize(1.0);
    data->SetMarkerColor(kBlack);
    data->SetLineColor(kBlack);
    data->SetLineWidth(1);
    data->Draw("E1 SAME");

    double max_value = std::max(stack->GetMaximum(), data->GetMaximum());
    stack->SetMaximum(max_value * 1.3);
    stack->SetTitle("BDT Score;BDT score;Events");

    // Legend
    TLegend* legend = new TLegend(0.1, 0.75, 0.9, 0.9);
    legend->SetNColumns(3);
    legend->SetTextSize(0.03);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);

    legend->AddEntry(signal_top, "t-channel top", "f");
    legend->AddEntry(signal_antitop, "t-channel antitop", "f");
    legend->AddEntry(ttbar, "TTbar", "f");
    legend->AddEntry(wjets, "W+jets", "f");
    legend->AddEntry(qcd, "QCD", "f");
    legend->AddEntry(data, "Data", "lep");
    legend->Draw();

    // Ratio plot
    pad2->cd();
    TH1D* ratio = (TH1D*)data->Clone("ratio");
    ratio->Divide(mc_total);
    ratio->SetTitle(";BDT score;Data / MC");
    ratio->SetLineColor(kBlack);
    ratio->SetMarkerStyle(20);
    ratio->SetMarkerSize(0.8);
    ratio->SetMinimum(0.5);
    ratio->SetMaximum(1.5);
    ratio->GetYaxis()->SetNdivisions(505);
    ratio->GetYaxis()->SetLabelSize(0.1);
    ratio->GetXaxis()->SetLabelSize(0.1);
    ratio->GetXaxis()->SetTitleSize(0.12);
    ratio->GetYaxis()->SetTitleSize(0.12);
    ratio->GetYaxis()->SetTitleOffset(0.5);
    ratio->Draw("E1P");

    TLine* line = new TLine(ratio->GetXaxis()->GetXmin(), 1.0,
                            ratio->GetXaxis()->GetXmax(), 1.0);
    line->SetLineStyle(2);
    line->SetLineColor(kGray + 2);
    line->Draw();

    c->SaveAs("eta_ele_prefit.png");

    // Close files
    file->Close();
    qcdFile->Close();
}

