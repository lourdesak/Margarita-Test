// Standalone 1D stopping-depth distribution for stopping muons.
//
// Plots the z-coordinate of each stopping track (one entry per muon
// via stepNum==1 on the stoppingSteps ntuple) so the Bragg-like
// penetration profile is visible directly, and BoxPV vs CylPV can be
// compared side by side from the two output PNGs.
//
// Runs independently of plotstoppingmuons.C / plotspatial.C so no
// other plot is touched. One invocation per ROOT file:
//
//   root -l 'plotzdeposition.C("g4marg.root")'       // -> stopdepth_BoxPV.png
//   root -l 'plotzdeposition.C("g4marg_cyl.root")'   // -> stopdepth_CylPV.png
//
// Axis-label styling mirrors plotspatial.C:
//   * display-only shift `+half` so the geometry spans 0 -> 7.3 cm on
//     the z-axis; underlying ntuple values are untouched.
//   * x-range [-2, 9.3] cm, integer 1 cm ticks via SetNdivisions(11, kTRUE).
//   * dotted gray reference lines at z = 0 and z = 7.3.
//   * red double-headed TArrow + "7.3 cm" diameter callout near the top.
//   * bold black "7.3" tick label in NDC under the z = 7.3 reference line.
//
// Data source: the stoppingSteps ntuple already written by
// MargaritaSteppingAction. One row per stopping muon is selected with
// the cut stepNum == 1, so every entry corresponds to a distinct
// stopping track.

void plotzdeposition(const char* fname = "g4marg.root")
{
    // Force interactive display so the canvas pops up in addition to
    // the PNG being saved.
    gROOT->SetBatch(kFALSE);

    // Snapshot current global style so this script never leaks state
    // back into a long-running ROOT session (same convention as
    // plotspatial.C).
    TStyle* savedStyle = gStyle ? (TStyle*)gStyle->Clone("savedStyleBeforeZDep")
                                : nullptr;

    TFile* f = TFile::Open(fname);
    if (!f || f->IsZombie()) {
        ::Error("plotzdeposition", "Cannot open %s", fname);
        return;
    }

    TTree* t = (TTree*)f->Get("ntuple/stoppingSteps");
    if (!t) t = (TTree*)f->Get("stoppingSteps");
    if (!t) {
        ::Error("plotzdeposition",
                "No stoppingSteps ntuple in %s. Rebuild and rerun the macro first.",
                fname);
        f->Close();
        return;
    }

    const TString fn(fname);
    const bool    isCyl     = fn.Contains("cyl");
    const TString geomLabel = isCyl ? "CylPV" : "BoxPV";

    // Geometry / axis constants — identical to plotspatial.C.
    const double half    = 3.65;          // detector half-extent  [cm]
    const double geomLo  = 0.0;
    const double geomHi  = 2.0 * half;    // 7.3 cm
    const double geomCtr = 0.5 * (geomLo + geomHi);
    const double axisLo  = -2.0;          // [cm]
    const double axisHi  = 9.3;           // [cm]
    const int    nBins   = 113;           // ~0.1 cm/bin across span 11.3

    gStyle->SetOptStat(0);

    // ------------------------------------------------------------------
    // Build the 1D histogram of (stopZ + half) with one entry per
    // stopping track (stepNum==1).
    // ------------------------------------------------------------------
    const TString hname = Form("hStopZ_%s_%lld",
                               geomLabel.Data(),
                               (Long64_t)gRandom->Integer(1 << 30));
    const TString expr  = Form("(stopZ+%g)>>%s(%d,%g,%g)",
                               half, hname.Data(),
                               nBins, axisLo, axisHi);
    const Long64_t nFilled = t->Draw(expr, "stepNum==1", "goff");
    TH1D* h = (TH1D*)gDirectory->Get(hname);
    if (!h || nFilled == 0) {
        ::Warning("plotzdeposition",
                  "No stopping-track rows to fill depth plot for %s.",
                  geomLabel.Data());
        if (savedStyle) savedStyle->cd();
        f->Close();
        return;
    }
    h->SetDirectory(nullptr);
    h->SetStats(0);

    // BoxPV red, CylPV azure — same colour convention used elsewhere.
    const Color_t fillColor = (geomLabel == "BoxPV") ? (kRed   + 1)
                                                     : (kAzure + 2);
    h->SetLineColor(fillColor);
    h->SetLineWidth(2);
    h->SetFillColorAlpha(fillColor, 0.30);

    h->SetTitle(Form("Stopping Muon Depth Distribution "
                     "#minus #font[62]{%s};Stopping depth z [cm];Counts",
                     geomLabel.Data()));
    h->GetXaxis()->SetTitleOffset(1.05);
    h->GetYaxis()->SetTitleOffset(1.55);   // pushed further from the axis
    h->GetXaxis()->CenterTitle(true);
    h->GetYaxis()->CenterTitle(true);
    // Optimised ~11 divisions -> integer 1 cm ticks across [-2, 9].
    h->GetXaxis()->SetNdivisions(11, kTRUE);

    TCanvas* c = new TCanvas(Form("cZDep_%s", geomLabel.Data()),
                             Form("Stopping depth (%s)", geomLabel.Data()),
                             800, 600);
    c->SetLeftMargin(0.16);   // a bit wider so the leftward-pushed
                              // y-axis title isn't clipped by the pad edge
    c->SetRightMargin(0.05);
    c->SetBottomMargin(0.13);
    c->SetTopMargin(0.10);
    h->Draw("HIST");

    // Headroom for the two stacked callouts (7.3 cm diameter + beam
    // direction), then commit y-range.  yMax was peak*1.15 when only
    // the diameter callout lived at the top; bumping to 1.35 leaves
    // room for the beam-direction arrow stacked above it without
    // moving the diameter callout's absolute height.
    c->Update();
    const double yMax = h->GetMaximum() * 1.35;
    h->SetMaximum(yMax);

    // Dotted reference lines at z = 0 and z = 7.3.
    auto mkLine = [](double x1, double y1, double x2, double y2) {
        TLine* L = new TLine(x1, y1, x2, y2);
        L->SetLineStyle(2);
        L->SetLineColor(kGray + 2);
        L->SetLineWidth(1);
        L->Draw();
    };
    mkLine(0.0,    0.0, 0.0,    yMax);     // z = 0
    mkLine(geomHi, 0.0, geomHi, yMax);     // z = 7.3

    // Red double-headed diameter arrow + "7.3 cm" label.
    // 0.92 * old_yMax(=peak*1.15) == 0.78 * new_yMax(=peak*1.35), so
    // this arrow stays at the exact same absolute height as before.
    const double yArrow = yMax * 0.78;
    TArrow* arr = new TArrow(geomLo, yArrow, geomHi, yArrow, 0.012, "<|>");
    arr->SetLineColor(kRed + 1);
    arr->SetFillColor(kRed + 1);
    arr->SetLineWidth(2);
    arr->SetAngle(40);
    arr->Draw();

    TLatex* arrLab = new TLatex();
    arrLab->SetTextFont(42);
    arrLab->SetTextSize(0.032);
    arrLab->SetTextColor(kRed + 1);
    arrLab->SetTextAlign(21);              // center-bottom: above the arrow
    arrLab->DrawLatex(geomCtr, yArrow + yMax * 0.018, "7.3 cm");

    // Beam-direction arrow stacked above the diameter callout. Same
    // style and direction as in plotspatial.C: single-headed black
    // arrow pointing LEFT, since the beam travels along -z and the
    // +z entry face sits at the right edge of the geometry outline
    // (z_display = 7.3 cm). Numerical confirmation: mean stopZ > 0,
    // mean (dirX, dirY, dirZ) = (0, 0, -1), mean (stop - vtx)_z ~ -9 cm.
    const double yBeam = yMax * 0.93;
    TArrow* beam = new TArrow(8.5, yBeam, 5.5, yBeam, 0.020, "|>");
    beam->SetLineColor(kBlack);
    beam->SetFillColor(kBlack);
    beam->SetLineWidth(2);
    beam->SetAngle(35);
    beam->Draw();

    TLatex* beamLab = new TLatex();
    beamLab->SetTextFont(42);
    beamLab->SetTextSize(0.030);
    beamLab->SetTextColor(kBlack);
    beamLab->SetTextAlign(21);
    beamLab->DrawLatex(0.5 * (8.5 + 5.5), yBeam + yMax * 0.020, "beam direction");

    // Bold black "7.3" axis label in NDC, matching plotspatial.C.
    const double xRange = axisHi - axisLo;
    const double lm = c->GetLeftMargin();
    const double rm = c->GetRightMargin();
    const double bm = c->GetBottomMargin();
    const double xNDC = lm + (geomHi - axisLo) / xRange * (1.0 - lm - rm);

    TLatex* tx = new TLatex();
    tx->SetNDC();
    tx->SetTextFont(62);                   // bold
    tx->SetTextSize(0.028);
    tx->SetTextColor(kBlack);
    tx->SetTextAlign(23);
    tx->DrawLatex(xNDC, bm - 0.055, "7.3");

    c->Update();
    c->SaveAs(Form("stopdepth_%s.png", geomLabel.Data()));

    Printf("[StoppingDepth-%s] saved 1D stopping-depth distribution.",
           geomLabel.Data());

    if (savedStyle) {
        savedStyle->cd();
    }
}
