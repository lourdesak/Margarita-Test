// Standalone 2D *absolute* stopping-efficiency map vs the muon
// transverse entry coordinates.
//
// One plot per geometry. Runs independently of every other macro so
// no existing plot is touched. One invocation per ROOT file:
//
//   root -l 'plotzxdeposition.C("g4marg.root")'      // -> stopeff_xy_BoxPV.png
//   root -l 'plotzxdeposition.C("g4marg_cyl.root")'  // -> stopeff_xy_CylPV.png
//
// Beam geometry note
// ------------------
// In this simulation the beam fires along -z: every primary has
// vtxZ ~= +10 cm (source plane), while vtxX and vtxY each vary in
// [-3.65, +3.65] cm to cover the front face of the detector. The two
// coordinates that actually parametrise the entry point are therefore
// (x, y), NOT (z, x). The macro file name stays `plotzxdeposition.C`
// (left unchanged so existing scripts still find it) but the plot
// itself is an x-y entry-point efficiency map.
//
// Physics
// -------
// Two ntuples produced by MargaritaSteppingAction / HistoManager are
// used:
//
//   * stoppingSteps  — one row per *step* of every stopping mu-.
//                      We pick stepNum==1 to get one entry per
//                      stopping track. Its vertex (vtxX, vtxY) is the
//                      *numerator* of the efficiency map.
//
//   * incidentMuons  — one row per primary mu- that enters the
//                      active detector volume. Its (vtxX, vtxY) is
//                      the *denominator*.
//
// Efficiency(x, y) = N_stop(x, y) / N_incident(x, y).
//
// Bins where N_incident == 0 are masked (left blank) so they don't
// alias into a phantom 100 %.
//
// The BoxPV map fills the 7.3 x 7.3 cm square; the CylPV map vanishes
// outside the disk x^2 + z^2 <= (3.65 cm)^2 — the geometric edge
// effect we want to visualise.
//
// Axis-label styling mirrors plotspatial.C exactly:
//   * display-only shift `+half` so the geometry spans 0 -> 7.3 cm.
//   * range [-2, 9.3] cm, integer 1 cm ticks via SetNdivisions(11, kTRUE).
//   * dotted gray reference lines at z={0,7.3} and x={0,7.3}.
//   * red double-headed TArrow + "7.3 cm" diameter callout below y=0.
//   * bold black "7.3" tick labels (NDC) on each axis.
//   * white detector outline on top (square for BoxPV, circle for CylPV).

void plotzxdeposition(const char* fname = "g4marg.root")
{
    // Force interactive display so the canvas pops up in addition to
    // the PNG being saved.
    gROOT->SetBatch(kFALSE);

    // Snapshot current global style so this script never leaks state
    // back into a long-running ROOT session.
    TStyle* savedStyle = gStyle ? (TStyle*)gStyle->Clone("savedStyleBeforeZXDep")
                                : nullptr;

    TFile* f = TFile::Open(fname);
    if (!f || f->IsZombie()) {
        ::Error("plotzxdeposition", "Cannot open %s", fname);
        return;
    }

    TTree* tStop = (TTree*)f->Get("ntuple/stoppingSteps");
    if (!tStop) tStop = (TTree*)f->Get("stoppingSteps");

    TTree* tInc = (TTree*)f->Get("ntuple/incidentMuons");
    if (!tInc)  tInc  = (TTree*)f->Get("incidentMuons");

    if (!tStop || !tInc) {
        ::Error("plotzxdeposition",
                "Missing ntuples in %s (need stoppingSteps AND incidentMuons). "
                "Rebuild and rerun the simulation first.",
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
    const double axisLo  = -2.0;
    const double axisHi  = 9.3;
    // 0.5 cm per bin: span 11.3 cm / 0.5 ~= 23 bins on each axis.
    // With ~100 k incident muons over the 7.3 x 7.3 cm active face
    // this gives ~500 events per bin -> per-bin binomial error
    // sqrt(p(1-p)/N) ~= 0.02 at p~0.4, so the cylindrical chord-
    // length gradient is no longer buried under counting noise.
    const int    nB      = 23;

    // Warm yellow -> orange -> red palette for high contrast against
    // the empty (white) background. kDarkBodyRadiator runs from dark
    // at low values, through red and orange, up to bright yellow at
    // the maximum — so high-efficiency bins glow visibly.
    gStyle->SetPalette(kDarkBodyRadiator);
    gStyle->SetNumberContours(100);
    gStyle->SetOptStat(0);

    // --------------------------------------------------------------
    // Numerator: stopping muons binned by their entry vertex (z, x).
    // --------------------------------------------------------------
    const TString hnameStop = Form("hStopVtx_%s_%lld",
                                   geomLabel.Data(),
                                   (Long64_t)gRandom->Integer(1 << 30));
    // y axis = vtxY+half, x axis = vtxX+half (transverse entry plane).
    const TString exprStop  = Form("(vtxY+%g):(vtxX+%g)>>%s(%d,%g,%g,%d,%g,%g)",
                                   half, half, hnameStop.Data(),
                                   nB, axisLo, axisHi, nB, axisLo, axisHi);
    const Long64_t nStop = tStop->Draw(exprStop, "stepNum==1", "goff");
    TH2D* hStop = (TH2D*)gDirectory->Get(hnameStop);

    // --------------------------------------------------------------
    // Denominator: every incident primary mu- by its entry vertex.
    // --------------------------------------------------------------
    const TString hnameInc = Form("hIncVtx_%s_%lld",
                                  geomLabel.Data(),
                                  (Long64_t)gRandom->Integer(1 << 30));
    const TString exprInc  = Form("(vtxY+%g):(vtxX+%g)>>%s(%d,%g,%g,%d,%g,%g)",
                                  half, half, hnameInc.Data(),
                                  nB, axisLo, axisHi, nB, axisLo, axisHi);
    const Long64_t nInc = tInc->Draw(exprInc, "", "goff");
    TH2D* hInc = (TH2D*)gDirectory->Get(hnameInc);

    if (!hStop || !hInc || nStop == 0 || nInc == 0) {
        ::Warning("plotzxdeposition",
                  "Empty ntuple(s) for %s (nStop=%lld, nInc=%lld).",
                  geomLabel.Data(), (Long64_t)nStop, (Long64_t)nInc);
        if (savedStyle) savedStyle->cd();
        f->Close();
        return;
    }
    hStop->SetDirectory(nullptr);
    hInc ->SetDirectory(nullptr);

    // --------------------------------------------------------------
    // Build efficiency map = numerator / denominator, masking bins
    // with no incident statistics so they show as empty (kDeepSea
    // background) rather than aliasing to 100 %.
    // --------------------------------------------------------------
    TH2D* hEff = (TH2D*)hStop->Clone(Form("hEff_%s", geomLabel.Data()));
    hEff->SetDirectory(nullptr);
    hEff->Reset();
    for (int ix = 1; ix <= nB; ++ix) {
        for (int iy = 1; iy <= nB; ++iy) {
            const double d = hInc ->GetBinContent(ix, iy);
            const double n = hStop->GetBinContent(ix, iy);
            if (d <= 0.) {
                hEff->SetBinContent(ix, iy, 0.);
                continue;
            }
            const double eff = n / d;
            const double err = std::sqrt(eff * (1. - eff) / d);  // binomial
            hEff->SetBinContent(ix, iy, eff);
            hEff->SetBinError  (ix, iy, err);
        }
    }
    hEff->SetStats(0);
    hEff->SetMinimum(0.0);
    hEff->SetMaximum(1.0);

    // Axis labels mirror plotspatial.C's "z-x" convention: the
    // horizontal axis is called "z entry" and the vertical "x entry".
    // Mapping to the underlying simulation columns is just a rename
    // (the two transverse coordinates that actually vary at the
    // entry plane are Geant4-x and Geant4-y; we relabel y -> "z" and
    // x -> "x" so the figure axes match the other margarita plots).
    hEff->SetTitle(Form("Stopping Efficiency vs (z, x) Entry Point "
                        "#minus #font[62]{%s};z entry [cm];x entry [cm]",
                        geomLabel.Data()));
    hEff->GetXaxis()->SetTitleOffset(1.05);
    hEff->GetYaxis()->SetTitleOffset(1.15);
    hEff->GetXaxis()->CenterTitle(true);
    hEff->GetYaxis()->CenterTitle(true);
    hEff->GetZaxis()->SetTitle("Stopping efficiency  N_{stop}/N_{inc}");
    hEff->GetZaxis()->SetTitleOffset(1.10);
    hEff->GetXaxis()->SetNdivisions(11, kTRUE);
    hEff->GetYaxis()->SetNdivisions(11, kTRUE);

    // --------------------------------------------------------------
    // Helper: dotted reference lines + diameter arrow + "7.3"
    // tick labels (same conventions as plotspatial.C).
    // --------------------------------------------------------------
    auto stampGeometryRefs = [&](TCanvas* c) {
        auto mkLine = [](double x1, double y1, double x2, double y2) {
            TLine* L = new TLine(x1, y1, x2, y2);
            L->SetLineStyle(2);
            L->SetLineColor(kGray + 2);
            L->SetLineWidth(1);
            L->Draw();
        };
        mkLine(0.0,    axisLo, 0.0,    axisHi);   // z = 0
        mkLine(geomHi, axisLo, geomHi, axisHi);   // z = 7.3
        mkLine(axisLo, 0.0,    axisHi, 0.0);      // x = 0
        mkLine(axisLo, geomHi, axisHi, geomHi);   // x = 7.3

        const double yArrow = -1.0;
        TArrow* arr = new TArrow(geomLo, yArrow, geomHi, yArrow,
                                 0.012, "<|>");
        arr->SetLineColor(kRed + 1);
        arr->SetFillColor(kRed + 1);
        arr->SetLineWidth(2);
        arr->SetAngle(40);
        arr->Draw();

        TLatex* arrLab = new TLatex();
        arrLab->SetTextFont(42);
        arrLab->SetTextSize(0.030);
        arrLab->SetTextColor(kRed + 1);
        arrLab->SetTextAlign(21);
        arrLab->DrawLatex(geomCtr, yArrow + 0.25, "7.3 cm");

        c->Update();
        const double xRange = axisHi - axisLo;
        const double yRange = axisHi - axisLo;
        const double lm = c->GetLeftMargin();
        const double rm = c->GetRightMargin();
        const double bm = c->GetBottomMargin();
        const double tm = c->GetTopMargin();
        const double xNDC = lm + (geomHi - axisLo) / xRange * (1.0 - lm - rm);
        const double yNDC = bm + (geomHi - axisLo) / yRange * (1.0 - bm - tm);

        TLatex* tx = new TLatex();
        tx->SetNDC();
        tx->SetTextFont(62);                  // bold
        tx->SetTextSize(0.028);
        tx->SetTextColor(kBlack);
        tx->SetTextAlign(23);
        tx->DrawLatex(xNDC, bm - 0.055, "7.3");
        tx->SetTextAlign(32);
        tx->DrawLatex(lm - 0.030, yNDC, "7.3");
    };

    // --------------------------------------------------------------
    // Canvas + draw
    // --------------------------------------------------------------
    TCanvas* c = new TCanvas(Form("cZXEff_%s", geomLabel.Data()),
                             Form("Stopping efficiency z-x (%s)",
                                  geomLabel.Data()),
                             720, 720);
    c->SetLeftMargin(0.13);
    c->SetRightMargin(0.15);
    c->SetBottomMargin(0.12);
    c->SetTopMargin(0.10);
    c->SetFixedAspectRatio(kTRUE);
    hEff->Draw("COLZ");

    stampGeometryRefs(c);

    // Both geometries project to the same 7.3 x 7.3 cm square in the
    // transverse entry plane:
    //   BoxPV  — the front face is literally a 7.3 cm square.
    //   CylPV  — axis along y, so the (x, y) projection of the cylinder
    //            is { |x|<=R, |y|<=R } which is again a 7.3 cm square.
    // The geometric "edge effect" the user wants to visualise shows up
    // as a *gradient* in stopping efficiency across that square (high
    // near x=0 where the chord through the cylinder is the full
    // diameter, low near |x|=R where the chord goes to zero) rather
    // than as a different outline shape.
    TBox* outline = new TBox(geomLo, geomLo, geomHi, geomHi);
    outline->SetLineColor(kWhite);
    outline->SetLineWidth(2);
    outline->SetFillStyle(0);
    outline->Draw();

    c->Update();
    c->SaveAs(Form("stopeff_zx_%s.png", geomLabel.Data()));

    // Console summary
    const double totalEff = hInc->Integral() > 0
        ? hStop->Integral() / hInc->Integral()
        : 0.0;
    Printf("[StoppingEff-%s] N_stop=%lld  N_inc=%lld  overall efficiency=%.3f"
           "   saved 2D (z,x)-entry stopping-efficiency map.",
           geomLabel.Data(),
           (Long64_t)hStop->Integral(),
           (Long64_t)hInc ->Integral(),
           totalEff);

    if (savedStyle) {
        savedStyle->cd();
    }
}
