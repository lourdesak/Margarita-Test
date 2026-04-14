{
    gROOT->Reset();

    TFile* f = new TFile("g4marg.root");
    TDirectory* dir = (TDirectory*)f->Get("histo");

    // --- Plot 1: Stopping power vs energy (10–100 MeV operational window) ---
    TCanvas* c2 = new TCanvas("c2", "Stopping Power vs Energy", 800, 600);
    c2->SetLeftMargin(0.14);
    c2->SetBottomMargin(0.12);

    TH2D* hSP = (TH2D*)dir->Get("h2.2");
    if (hSP) {
        // Extract mean dE/dx per KE bin as a clean 1D profile
        TProfile* prof = hSP->ProfileX("hSP_prof");
        prof->SetTitle("Muon Stopping Power in Water");
        prof->GetXaxis()->SetTitle("Kinetic Energy [MeV]");
        prof->GetYaxis()->SetTitle("dE/dx [MeV/cm]");
        prof->GetYaxis()->SetTitleOffset(1.5);
        prof->SetLineColor(kBlue+1);
        prof->SetLineWidth(2);
        prof->SetMarkerStyle(20);
        prof->SetMarkerSize(0.6);
        prof->SetMarkerColor(kBlue+1);
        prof->Draw("E1");

        // Bethe-Bloch theory curve for muons in water (PDG)
        // Parameters: muon mass, electron mass, mean excitation energy I=79.7 eV for water
        TF1* fBB = new TF1("BetheBloch", [](double* x, double* p) -> double {
            const double T   = x[0];          // KE [MeV]
            const double mmu = 105.658;        // muon mass [MeV/c^2]
            const double me  = 0.510999;       // electron mass [MeV/c^2]
            const double I   = 79.7e-6;        // mean excitation energy for water [MeV]
            const double K   = 0.307075;       // [MeV cm^2/g]
            const double ZA  = 0.55509;        // Z/A for water
            const double rho = 1.0;            // density [g/cm^3]
            double gamma  = (T + mmu) / mmu;
            double beta2  = 1.0 - 1.0 / (gamma * gamma);
            double Tmax   = 2.0 * me * beta2 * gamma * gamma /
                            (1.0 + 2.0 * gamma * me / mmu + (me/mmu)*(me/mmu));
            double arg    = 2.0 * me * beta2 * gamma * gamma * Tmax / (I * I);
            return K * ZA * rho / beta2 * (0.5 * TMath::Log(arg) - beta2);
        }, 1., 100., 0);
        fBB->SetLineColor(kRed);
        fBB->SetLineWidth(2);
        fBB->SetLineStyle(2);
        fBB->Draw("SAME");

        TLegend* leg = new TLegend(0.55, 0.70, 0.88, 0.88);
        leg->AddEntry(prof, "Geant4 simulation", "lp");
        leg->AddEntry(fBB,  "Bethe-Bloch (PDG)", "l");
        leg->SetBorderSize(0);
        leg->Draw();
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
