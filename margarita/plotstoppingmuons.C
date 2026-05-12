// ---------------------------------------------------------------------
// Select one clean representative stopping muon and plot its single-
// track dE/dx profile. Selection criteria:
//   * primary mu- that stopped inside the active volume (already the
//     only thing written to the stoppingSteps ntuple)
//   * initial KE within [targetE - Etol, targetE + Etol]
//   * at least minSteps continuous step records
//   * no zero-dEdx interior step (proxy for "no boundary artifact")
// Among matching candidates the one with eKinInit closest to targetE
// is chosen so BoxPV and CylPV land on the same nominal energy.
// Two new canvases (c3, c4) are created; existing c1/c2 are untouched.
// ---------------------------------------------------------------------
void plotSingleStoppingMuon(TFile* f, const TString& geomLabel, const TString& fname,
                            double targetE = 40.0, double Etol = 2.0, int minSteps = 5)
{
    if (!f) return;
    TTree* t = (TTree*)f->Get("ntuple/stoppingSteps");
    if (!t) t = (TTree*)f->Get("stoppingSteps");
    if (!t) {
        ::Warning("plotSingleStoppingMuon",
                  "No stoppingSteps ntuple in %s; skipping single-track plot.",
                  fname.Data());
        return;
    }

    Int_t    eventID = 0, trackID = 0, stepNum = 0, nSteps = 0;
    Double_t eKinInit = 0, kePre = 0, dEdx = 0;
    Double_t x = 0, y = 0, z = 0;
    Double_t stepLen = 0, eDep = 0, cumLen = 0;
    Double_t totalLen = 0, totalEDep = 0;
    Double_t stopX = 0, stopY = 0, stopZ = 0;
    Double_t vtxX = 0, vtxY = 0, vtxZ = 0;
    Double_t dirX = 0, dirY = 0, dirZ = 0;
    t->SetBranchAddress("eventID",   &eventID);
    t->SetBranchAddress("trackID",   &trackID);
    t->SetBranchAddress("stepNum",   &stepNum);
    t->SetBranchAddress("nSteps",    &nSteps);
    t->SetBranchAddress("eKinInit",  &eKinInit);
    t->SetBranchAddress("kePre",     &kePre);
    t->SetBranchAddress("dEdx",      &dEdx);
    t->SetBranchAddress("x",         &x);
    t->SetBranchAddress("y",         &y);
    t->SetBranchAddress("z",         &z);
    t->SetBranchAddress("stepLen",   &stepLen);
    t->SetBranchAddress("eDep",      &eDep);
    t->SetBranchAddress("cumLen",    &cumLen);
    t->SetBranchAddress("totalLen",  &totalLen);
    t->SetBranchAddress("totalEDep", &totalEDep);
    t->SetBranchAddress("stopX",     &stopX);
    t->SetBranchAddress("stopY",     &stopY);
    t->SetBranchAddress("stopZ",     &stopZ);
    t->SetBranchAddress("vtxX",      &vtxX);
    t->SetBranchAddress("vtxY",      &vtxY);
    t->SetBranchAddress("vtxZ",      &vtxZ);
    t->SetBranchAddress("dirX",      &dirX);
    t->SetBranchAddress("dirY",      &dirY);
    t->SetBranchAddress("dirZ",      &dirZ);

    const Long64_t N = t->GetEntries();

    // Currently-buffered candidate
    std::vector<double> curKE, curDEdx, curCum;
    Int_t    curEvent = -1, curTrack = -1, curNSteps = 0;
    Double_t curEKinInit = 0, curTotalLen = 0, curTotalEDep = 0;
    Double_t curStopX = 0, curStopY = 0, curStopZ = 0;
    Double_t curVtxX = 0, curVtxY = 0, curVtxZ = 0;
    Double_t curDirX = 0, curDirY = 0, curDirZ = 0;

    // Best-so-far
    std::vector<double> bestKE, bestDEdx, bestCum;
    Int_t    bestEvent = -1, bestTrack = -1, bestNSteps = 0;
    Double_t bestEKinInit = 0, bestTotalLen = 0, bestTotalEDep = 0;
    Double_t bestStopX = 0, bestStopY = 0, bestStopZ = 0;
    Double_t bestVtxX = 0, bestVtxY = 0, bestVtxZ = 0;
    Double_t bestDirX = 0, bestDirY = 0, bestDirZ = 0;
    Double_t bestDiff = 1e30;

    auto evaluateCurrent = [&]() {
        if (curEvent < 0 || (int)curKE.size() < minSteps) return;
        if (std::fabs(curEKinInit - targetE) > Etol)      return;
        // Sanity: a single mu- cannot deposit more than its initial KE.
        // Reject anything suspicious (allow tiny rounding slack).
        if (curTotalEDep > curEKinInit * 1.05)            return;
        // Physical range cap for a stopping mu- in water at ~40 MeV: ~14 cm.
        // 30 cm is a generous upper bound that still excludes multi-event leaks.
        if (curTotalLen > 30.0)                           return;
        // Cleanness: no zero-dEdx interior step (last step is allowed to be tiny)
        for (size_t i = 0; i + 1 < curDEdx.size(); ++i) {
            if (curDEdx[i] <= 0.) return;
        }
        // Kinematic monotonicity: pre-step KE must be non-increasing along the
        // track (allow tiny up-fluctuations from delta-ray bookkeeping <0.5 MeV).
        for (size_t i = 1; i < curKE.size(); ++i) {
            if (curKE[i] > curKE[i-1] + 0.5) return;
        }
        const double diff = std::fabs(curEKinInit - targetE);
        if (diff < bestDiff) {
            bestDiff       = diff;
            bestKE         = curKE;
            bestDEdx       = curDEdx;
            bestCum        = curCum;
            bestEvent      = curEvent;
            bestTrack      = curTrack;
            bestNSteps     = curNSteps;
            bestEKinInit   = curEKinInit;
            bestTotalLen   = curTotalLen;
            bestTotalEDep  = curTotalEDep;
            bestStopX      = curStopX;
            bestStopY      = curStopY;
            bestStopZ      = curStopZ;
            bestVtxX       = curVtxX;
            bestVtxY       = curVtxY;
            bestVtxZ       = curVtxZ;
            bestDirX       = curDirX;
            bestDirY       = curDirY;
            bestDirZ       = curDirZ;
        }
    };

    for (Long64_t i = 0; i < N; ++i) {
        t->GetEntry(i);
        if (eventID != curEvent || trackID != curTrack) {
            evaluateCurrent();
            curKE.clear(); curDEdx.clear(); curCum.clear();
            curEvent = eventID; curTrack = trackID;
            curNSteps     = nSteps;
            curEKinInit   = eKinInit;
            curTotalLen   = totalLen;
            curTotalEDep  = totalEDep;
            curStopX = stopX; curStopY = stopY; curStopZ = stopZ;
            curVtxX  = vtxX;  curVtxY  = vtxY;  curVtxZ  = vtxZ;
            curDirX  = dirX;  curDirY  = dirY;  curDirZ  = dirZ;
        }
        curKE.push_back(kePre);
        curDEdx.push_back(dEdx);
        curCum.push_back(cumLen);
    }
    evaluateCurrent();

    if (bestEvent < 0) {
        ::Warning("plotSingleStoppingMuon",
                  "No clean stopping mu- in %.1f +- %.1f MeV for %s (tried %lld rows).",
                  targetE, Etol, geomLabel.Data(), (Long64_t)N);
        return;
    }

    // ---- Plot 3: dE/dx vs cumulative path length (Bragg-like profile) ----
    TCanvas* c3 = new TCanvas(Form("c3_%s", geomLabel.Data()),
                              Form("Single track dE/dx vs path length - %s", geomLabel.Data()),
                              800, 600);
    c3->SetLeftMargin(0.12);
    c3->SetBottomMargin(0.12);
    c3->SetRightMargin(0.05);

    // BoxPV gets red points/line; CylPV stays azure
    const Color_t trackColor = (geomLabel == "BoxPV") ? kRed + 1 : (kAzure + 2);

    const int n = (int)bestKE.size();
    TGraph* gL = new TGraph(n, &bestCum[0], &bestDEdx[0]);
    gL->SetMarkerStyle(20);
    gL->SetMarkerSize(0.9);
    gL->SetMarkerColor(trackColor);
    gL->SetLineColor(trackColor);
    gL->SetLineWidth(2);
    TString title3 = Form("Single Stopping #mu^{#minus} dE/dx vs Path Length "
                          "#minus #font[62]{%s};Cumulative path length [cm];dE/dx [MeV/cm]",
                          geomLabel.Data());
    gL->SetTitle(title3);
    gL->Draw("ALP");
    gL->GetXaxis()->SetLimits(0., 8.);
    gL->GetYaxis()->SetRangeUser(0., 60.);

    // Metadata box
    TPaveText* pt = new TPaveText(0.50, 0.55, 0.93, 0.90, "NDC");
    pt->SetFillColor(0);
    pt->SetFillStyle(1001);
    pt->SetBorderSize(1);
    pt->SetTextAlign(12);
    pt->SetTextFont(42);
    pt->SetTextSize(0.028);
    pt->AddText(Form("event / track = %d / %d", bestEvent, bestTrack));
    pt->AddText(Form("E_{kin}^{init} = %.3f MeV (target %.1f #pm %.1f)",
                     bestEKinInit, targetE, Etol));
    pt->AddText(Form("vertex = (%.2f, %.2f, %.2f) cm",
                     bestVtxX, bestVtxY, bestVtxZ));
    pt->AddText(Form("direction = (%g, %g, %g)",
                     bestDirX, bestDirY, bestDirZ));
    pt->AddText(Form("stop = (%.3f, %.3f, %.3f) cm",
                     bestStopX, bestStopY, bestStopZ));
    pt->AddText(Form("E_{dep}^{tot} = %.3f MeV", bestTotalEDep));
    pt->AddText(Form("track length = %.3f cm", bestTotalLen));
    pt->AddText(Form("# steps = %d", bestNSteps));
    pt->Draw();
    c3->Update();

    // ---- Plot 4: dE/dx vs pre-step KE (matches h2.2 axes) ----
    TCanvas* c4 = new TCanvas(Form("c4_%s", geomLabel.Data()),
                              Form("Single track dE/dx vs KE - %s", geomLabel.Data()),
                              800, 600);
    c4->SetLeftMargin(0.12);
    c4->SetBottomMargin(0.12);
    c4->SetRightMargin(0.05);

    // Sort points by pre-step KE for a clean left-to-right line.
    // (Tracking order is high-KE -> low-KE, which would draw right-to-left.)
    std::vector<size_t> order(n);
    for (int i = 0; i < n; ++i) order[i] = (size_t)i;
    std::sort(order.begin(), order.end(),
              [&](size_t a, size_t b){ return bestKE[a] < bestKE[b]; });
    std::vector<double> sortKE(n), sortDEdx(n);
    for (int i = 0; i < n; ++i) {
        sortKE[i]   = bestKE[order[i]];
        sortDEdx[i] = bestDEdx[order[i]];
    }
    TGraph* gK = new TGraph(n, &sortKE[0], &sortDEdx[0]);
    gK->SetMarkerStyle(20);
    gK->SetMarkerSize(0.9);
    gK->SetMarkerColor(trackColor);
    gK->SetLineColor(trackColor);
    gK->SetLineWidth(2);
    TString title4 = Form("Single Stopping #mu^{#minus} dE/dx vs Pre-step KE "
                          "#minus #font[62]{%s};Pre-step KE [MeV];dE/dx [MeV/cm]",
                          geomLabel.Data());
    gK->SetTitle(title4);
    gK->Draw("ALP");
    gK->GetXaxis()->SetLimits(0., 50.);
    gK->GetYaxis()->SetRangeUser(0., 40.);

    TPaveText* pt2 = (TPaveText*)pt->Clone();
    pt2->Draw();
    c4->Update();

    // Save PNGs side-by-side with the existing canvases
    c3->SaveAs(Form("singletrack_path_%s.png", geomLabel.Data()));
    c4->SaveAs(Form("singletrack_ke_%s.png",   geomLabel.Data()));

    // Console summary
    Printf("[SingleTrack-%s] event=%d track=%d  E_init=%.3f MeV"
           "  vtx=(%.2f,%.2f,%.2f)  dir=(%.3f,%.3f,%.3f)"
           "  stop=(%.3f,%.3f,%.3f)  Edep=%.3f MeV  len=%.3f cm  nSteps=%d",
           geomLabel.Data(), bestEvent, bestTrack, bestEKinInit,
           bestVtxX, bestVtxY, bestVtxZ,
           bestDirX, bestDirY, bestDirZ,
           bestStopX, bestStopY, bestStopZ,
           bestTotalEDep, bestTotalLen, bestNSteps);
}

void plotstoppingmuons(const char* fname = "g4marg.root")
{
    gROOT->Reset();

    TFile* f = new TFile(fname);
    if (!f || f->IsZombie()) {
        ::Error("plotstoppingmuons", "Cannot open %s", fname);
        return;
    }
    TDirectory* dir = (TDirectory*)f->Get("histo");

    // Infer geometry label from filename
    TString fn(fname);
    const bool isCyl = fn.Contains("cyl");
    const TString geomLabel = isCyl ? "CylPV" : "BoxPV";

    // --- Plot 1: Stopping power vs energy (COLZ + profile) ---
    TCanvas* c2 = new TCanvas("c2", "Stopping Power vs Energy", 800, 600);
    c2->SetLeftMargin(0.12);
    c2->SetBottomMargin(0.12);
    c2->SetRightMargin(0.13);

    TH2D* hSP = (TH2D*)dir->Get("h2.2");
    if (hSP) {
        gStyle->SetPalette(kBird);
        // Keep the stats box (Entries / Mean x / Mean y / Std Dev x / Std Dev y)
        // but drop the histogram-name line ("h2.2") at the top of the box.
        // SetOptStat format is the 9-digit code "ksiourmen": each digit selects
        // kurtosis, skewness, integral, overflow, underflow, rms, mean, entries,
        // name (left-to-right). The default is 1111 (n+e+m+r). To drop the name
        // we use 1110 (e+m+r): Entries, Mean (x & y for 2D), Std Dev (x & y).
        gStyle->SetOptStat(1110);
        gStyle->SetNumberContours(100);
        TString title = "Stopping Power vs Kinetic Energy #minus #font[62]{" + geomLabel + "}";
        hSP->SetTitle(title);
        hSP->SetStats(1);                    // ensure the box is drawn
        hSP->GetXaxis()->SetTitle("Initial Kinetic Energy [MeV]");
        hSP->GetYaxis()->SetTitle("dE/dx [MeV/cm]");
        hSP->GetZaxis()->SetTitle("dE/dx [MeV/cm]");
        hSP->Draw("COLZ");

        TProfile* prof = hSP->ProfileX("hSP_prof");
        prof->SetLineColor(kRed);
        prof->SetLineWidth(2);
        prof->Draw("SAME");
    }

    // --- Plot 2: stop distributions (2x2) ---
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

    // h2.1 : Stop XY (2D) — cleaned-up spatial distribution with detector outline
    //                       and a small margin of empty space around it.
    // (Only this sub-pad is modified; c2, c3, c4 and the other c1 sub-pads stay
    //  exactly as before. The original h2.1 in memory is not altered — we copy
    //  its contents into a wider view histogram for display.)
    c1->cd(3);
    TH2D* h3 = (TH2D*)dir->Get("h2.1");
    if (h3) {
        gStyle->SetPalette(kDeepSea);
        gStyle->SetNumberContours(100);

        // Detector half-extent in x and y is 3.65 cm for both BoxPV and the
        // side-on CylPV projection. Show ~1.5 cm of empty space on each side.
        const double half   = 3.65;
        const double margin = 1.5;
        const double R      = half + margin;
        const int    nB     = 130;

        TH2D* hView = new TH2D(
            Form("h2_1_view_%s", geomLabel.Data()),
            Form("Stopping Muon Spatial Distribution (x-y) "
                 "#minus #font[62]{%s};x [cm];y [cm]",
                 geomLabel.Data()),
            nB, -R, R, nB, -R, R);
        hView->SetStats(0);
        hView->SetDirectory(nullptr);

        // Copy bin contents from h2.1 into the wider view histogram so that
        // the colour map is shown with extra space around the active region.
        for (int ix = 1; ix <= h3->GetNbinsX(); ++ix) {
            const double xc = h3->GetXaxis()->GetBinCenter(ix);
            const int jx = hView->GetXaxis()->FindBin(xc);
            if (jx < 1 || jx > nB) continue;
            for (int iy = 1; iy <= h3->GetNbinsY(); ++iy) {
                const double yc = h3->GetYaxis()->GetBinCenter(iy);
                const int jy = hView->GetYaxis()->FindBin(yc);
                if (jy < 1 || jy > nB) continue;
                const double c = h3->GetBinContent(ix, iy);
                if (c == 0) continue;
                hView->SetBinContent(jx, jy,
                                     hView->GetBinContent(jx, jy) + c);
            }
        }

        // Equal aspect ratio and clean pad margins (this sub-pad only).
        gPad->SetFixedAspectRatio(kTRUE);
        gPad->SetLeftMargin(0.14);
        gPad->SetRightMargin(0.15);
        gPad->SetBottomMargin(0.12);
        gPad->SetTopMargin(0.10);

        // Axis cosmetics
        hView->GetXaxis()->SetTitleOffset(1.05);
        hView->GetYaxis()->SetTitleOffset(1.25);
        hView->GetXaxis()->CenterTitle(true);
        hView->GetYaxis()->CenterTitle(true);

        hView->Draw("COLZ");

        // Active-detector boundary: 7.3 x 7.3 cm square in the x-y projection
        // for both geometries. Drawn in white over the dark palette so it is
        // clearly visible without dominating the figure.
        TBox* outline = new TBox(-half, -half, half, half);
        outline->SetLineColor(kWhite);
        outline->SetLineWidth(2);
        outline->SetFillStyle(0);
        outline->Draw();
    }

    // h1.3 : Stop Initial KE
    c1->cd(4);
    TH1D* h4 = (TH1D*)dir->Get("h1.3");
    if (h4) h4->Draw("HIST");

    // -----------------------------------------------------------------
    // NEW: single-track diagnostic (new canvases c3 and c4; existing
    // c1/c2 above are untouched).
    // -----------------------------------------------------------------
    plotSingleStoppingMuon(f, geomLabel, fn);
}
