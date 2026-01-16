{
    gROOT->Reset();

    // Open the ROOT file produced by Geant4
    TFile* f = new TFile("test01.root");

    // Grab the histogram directory
    TDirectory* dir = (TDirectory*)f->Get("histo");

    // Canvas
    TCanvas* c1 = new TCanvas("c1", "Stopping muons histograms", 1000, 800);
    c1->Divide(2,2);  // 2x2 pad layout

    // h3.1 : Stop KE
    c1->cd(1);
    TH1D* h31 = (TH1D*)dir->Get("h3.1");
    if (h31) h31->Draw("HIST");

    // h3.2 : Stop Z
    c1->cd(2);
    TH1D* h32 = (TH1D*)dir->Get("h3.2");
    if (h32) h32->Draw("HIST");

    // h3.5 : Stop XY (2D)
    c1->cd(3);
    TH2D* h35 = (TH2D*)dir->Get("h3.5");
    if (h35) {
        gStyle->SetPalette(kDeepSea); 
        gStyle->SetNumberContours(100);   // smoother gradient
        h35->Draw("COLZ");                // "COLZ" draws color scale
    }

    // h3.4 : Stop Initial KE
    c1->cd(4);
    TH1D* h34 = (TH1D*)dir->Get("h3.4");
    if (h34) h34->Draw("HIST");
}
