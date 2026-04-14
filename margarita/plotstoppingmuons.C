{
    gROOT->Reset();

    TFile* f = new TFile("g4marg.root");
    TDirectory* dir = (TDirectory*)f->Get("histo");

    // --- Plot 1: Stopping power vs energy (10–100 MeV operational window) ---
    TCanvas* c2 = new TCanvas("c2", "Stopping Power vs Energy", 800, 600);
    TH2D* hSP = (TH2D*)dir->Get("h2.2");
    if (hSP) {
        gStyle->SetPalette(kBird);
        gStyle->SetNumberContours(100);
        hSP->Draw("COLZ");
        TProfile* prof = hSP->ProfileX("hSP_prof");
        prof->SetLineColor(kRed);
        prof->SetLineWidth(2);
        prof->Draw("SAME");
    }

    // --- Existing stop-distribution plots ---
    TCanvas* c1 = new TCanvas("c1", "Stopping muons histograms", 1000, 800);
    c1->Divide(2,2);

    // h1.1 : Stop KE
    c1->cd(1);
    TH1D* h1 = (TH1D*)dir->Get("h1.1");
    if (h1) h1->Draw("HIST");

    // h1.2 : Stop Z
    c1->cd(2);
    TH1D* h2 = (TH1D*)dir->Get("h1.2");
    if (h2) h2->Draw("HIST");

    // h2.1 : Stop XY (2D)
    c1->cd(3);
    TH2D* h3 = (TH2D*)dir->Get("h2.1");
    if (h3) {
        gStyle->SetPalette(kDeepSea);
        gStyle->SetNumberContours(100);
        h3->Draw("COLZ");
    }

    // h1.3 : Stop Initial KE
    c1->cd(4);
    TH1D* h4 = (TH1D*)dir->Get("h1.3");
    if (h4) h4->Draw("HIST");
}
