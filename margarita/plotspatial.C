// Standalone y-z and z-x stopping-muon spatial-distribution plots.
//
// Run independently of plotstoppingmuons.C so it cannot affect any
// existing plot. One invocation per ROOT file, two canvases + PNGs:
//
//   root -l 'plotspatial.C("g4marg.root")'       // -> spatial_yz_BoxPV.png
//                                                //    spatial_zx_BoxPV.png
//   root -l 'plotspatial.C("g4marg_cyl.root")'   // -> spatial_yz_CylPV.png
//                                                //    spatial_zx_CylPV.png
//
// Styling deliberately mirrors the c1->cd(3) x-y sub-pad in
// plotstoppingmuons.C:
//   * detector half-extent  half   = 3.65 cm, centred on the origin
//   * display range  R = half + 1.5 = 5.15 cm  ( ~1.5 cm margin )
//   * 130 bins per axis, kDeepSea palette, equal aspect ratio
//   * white detector outline drawn on top
//
// Outline shape per plane:
//   y-z : 7.3 x 7.3 cm square for both BoxPV and CylPV
//         (the rotated CylPV has axis along Y with half-length 3.65
//          and a disk cross-section of radius 3.65 cm in the X-Z
//          plane -> the y-z projection of the active region is a
//          rectangle 7.3 x 7.3 cm, indistinguishable from BoxPV).
//   z-x : 7.3 x 7.3 cm square for BoxPV.
//         Circle of radius 3.65 cm for CylPV (the cyl cross-section
//          in z-x is x^2 + z^2 <= R^2, a disk).
//
// Data source: the stoppingSteps ntuple already written by
// MargaritaSteppingAction. One row per stopping muon is selected with
// the cut stepNum == 1, so every entry corresponds to a distinct
// stopping track.

void plotspatial(const char* fname = "g4marg.root")
{
    // Snapshot current global style so this script never leaks state
    // back into a long-running ROOT session.
    TStyle* savedStyle = gStyle ? (TStyle*)gStyle->Clone("savedStyleBeforeSpatial")
                                : nullptr;

    TFile* f = TFile::Open(fname);
    if (!f || f->IsZombie()) {
        ::Error("plotspatial", "Cannot open %s", fname);
        return;
    }

    TTree* t = (TTree*)f->Get("ntuple/stoppingSteps");
    if (!t) t = (TTree*)f->Get("stoppingSteps");
    if (!t) {
        ::Error("plotspatial",
                "No stoppingSteps ntuple in %s. Rebuild and rerun the macro first.",
                fname);
        f->Close();
        return;
    }

    const TString fn(fname);
    const bool    isCyl     = fn.Contains("cyl");
    const TString geomLabel = isCyl ? "CylPV" : "BoxPV";

    const double half   = 3.65;          // detector half-extent [cm]
    const double margin = 1.5;           // visible empty margin [cm] (legacy)
    const double R      = half + margin; // 5.15 (legacy)
    // Axis range: -2 to 9.3 cm on both axes.
    // Geometry outline spans 0 to 7.3 cm on both axes.
    const double axisLo  = -2.0;         // [cm] (shared lower bound)
    const double axisHi  = 9.3;          // [cm] x upper bound
    const double yAxisHi = 9.3;          // [cm] y upper bound
    const double geomLo = 0.0;           // [cm]
    const double geomHi = 2.0 * half;    // 7.3 cm
    const double geomCtr = 0.5 * (geomLo + geomHi); // 3.65 cm (for cylinder)
    const int    nB     = 130;

    gStyle->SetPalette(kDeepSea);
    gStyle->SetNumberContours(100);
    gStyle->SetOptStat(0);

    // --------------------------------------------------------------
    // Helper: stamp dotted reference lines at x={0, 7.3}, y={0, 7.3}
    // and add explicit "7.3" tick labels on the X and Y axes. The
    // helper is reused by every plot below so styling stays in sync.
    // --------------------------------------------------------------
    auto stampGeometryRefs = [&](TCanvas* c) {
        auto mkLine = [](double x1, double y1, double x2, double y2) {
            TLine* L = new TLine(x1, y1, x2, y2);
            L->SetLineStyle(2);             // dotted
            L->SetLineColor(kGray + 2);
            L->SetLineWidth(1);
            L->Draw();
        };
        // Dotted reference lines spanning the full visible axis range.
        mkLine(0.0,    axisLo, 0.0,    yAxisHi);  // x = 0
        mkLine(geomHi, axisLo, geomHi, yAxisHi);  // x = 7.3
        mkLine(axisLo, 0.0,    axisHi, 0.0);      // y = 0
        mkLine(axisLo, geomHi, axisHi, geomHi);   // y = 7.3

        // Diameter caller: red double-headed arrow along z (horizontal,
        // x-axis in the plot) just below the geometry, with a red
        // "7.3 cm" label centred above it.
        const double yArrow = -1.0;   // sits in the margin strip below y=0
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
        arrLab->SetTextAlign(21);     // center-bottom: label sits above arrow
        arrLab->DrawLatex(geomCtr, yArrow + 0.25, "7.3 cm");

        // Explicit "7.3" labels on each axis. Drawn in NDC so they
        // land cleanly in the canvas margin and never get clipped by
        // the data frame. Bold black, to match the regular tick
        // labels while still being visibly emphasised.
        c->Update();  // ensure pad margins/ranges are committed
        const double xRange = axisHi  - axisLo;
        const double yRange = yAxisHi - axisLo;
        const double lm = c->GetLeftMargin();
        const double rm = c->GetRightMargin();
        const double bm = c->GetBottomMargin();
        const double tm = c->GetTopMargin();
        const double xNDC = lm + (geomHi - axisLo) / xRange * (1.0 - lm - rm);
        const double yNDC = bm + (geomHi - axisLo) / yRange * (1.0 - bm - tm);

        TLatex* tx = new TLatex();
        tx->SetNDC();
        tx->SetTextFont(62);                        // bold
        tx->SetTextSize(0.028);
        tx->SetTextColor(kBlack);
        tx->SetTextAlign(23);                       // center-top  (x-axis)
        tx->DrawLatex(xNDC, bm - 0.055, "7.3");      // nudged downward
        tx->SetTextAlign(32);                       // right-center (y-axis)
        tx->DrawLatex(lm - 0.030, yNDC, "7.3");      // nudged leftward
    };

    // --------------------------------------------------------------
    // Helper: stamp a "beam direction" arrow in the empty strip
    // above the detector outline. Both spatial plots (y-z and z-x)
    // have z on the horizontal axis; the source plane is at z ~ +10
    // cm and stopping muons are biased toward +z (mean stopZ ≈ +0.9
    // cm in BoxPV, +0.75 cm in CylPV, with mean stopX, stopY ≈ 0),
    // so the beam travels along -z and the arrow points LEFT in
    // the display-shifted axes (the +z entry face sits at the right-
    // hand edge of the geometry outline at z_display = 7.3 cm).
    // The arrow lives at y_display ≈ 8.0 cm, well above the 0 -> 7.3
    // cm geometry outline and below the canvas title, so it cannot
    // overlap any plotted data.
    // --------------------------------------------------------------
    auto stampBeamArrow = [&](TCanvas* /*c*/) {
        const double yArrow = 8.0;        // empty strip above the 7.3 cm outline
        const double xTail  = 8.5;        // right (source side)
        const double xHead  = 5.5;        // left (direction of propagation)
        TArrow* beam = new TArrow(xTail, yArrow, xHead, yArrow, 0.020, "|>");
        beam->SetLineColor(kBlack);
        beam->SetFillColor(kBlack);
        beam->SetLineWidth(2);
        beam->SetAngle(35);
        beam->Draw();

        TLatex* lab = new TLatex();
        lab->SetTextFont(42);
        lab->SetTextSize(0.030);
        lab->SetTextColor(kBlack);
        lab->SetTextAlign(21);            // center-bottom: label above arrow
        lab->DrawLatex(0.5 * (xTail + xHead), yArrow + 0.45, "beam direction");
    };

    // ------------------------------------------------------------------
    // y-z plot
    // ------------------------------------------------------------------
    {
        const TString hname = Form("hYZ_%s_%lld",
                                   geomLabel.Data(),
                                   (Long64_t)gRandom->Integer(1 << 30));
        // Display-only shift: data is centred at origin in the geometry,
        // but we relabel axes so the detector spans 0 -> 7.3 cm. Add
        // `half` to each plotted coordinate; the stored ntuple values
        // are untouched.
        const TString expr  = Form("(stopY+%g):(stopZ+%g)>>%s(%d,%g,%g,%d,%g,%g)",
                                   half, half, hname.Data(),
                                   nB, axisLo, axisHi, nB, axisLo, yAxisHi);
        const Long64_t nFilled = t->Draw(expr, "stepNum==1", "goff");
        TH2D* h = (TH2D*)gDirectory->Get(hname);
        if (h && nFilled > 0) {
            h->SetDirectory(nullptr);
            h->SetStats(0);
            h->SetTitle(Form("Stopping Muon Spatial Distribution (y-z) "
                             "#minus #font[62]{%s};z [cm];y [cm]",
                             geomLabel.Data()));
            h->GetXaxis()->SetTitleOffset(1.05);
            h->GetYaxis()->SetTitleOffset(1.15);
            h->GetXaxis()->CenterTitle(true);
            h->GetYaxis()->CenterTitle(true);
            // Integer ticks every 1 cm -> 0 is always labelled.
            // Both spans are 11.3 (non-integer), so let ROOT optimise
            // to round labels by asking for ~11 divisions with
            // optimisation enabled; this yields integer ticks from
            // -2 to 9 on both axes.
            h->GetXaxis()->SetNdivisions(11, kTRUE);
            h->GetYaxis()->SetNdivisions(11, kTRUE);

            TCanvas* c = new TCanvas(Form("cYZ_%s", geomLabel.Data()),
                                     Form("Stopping muons y-z (%s)",
                                          geomLabel.Data()),
                                     720, 720);
            c->SetLeftMargin(0.13);
            c->SetRightMargin(0.15);
            c->SetBottomMargin(0.12);
            c->SetTopMargin(0.10);
            c->SetFixedAspectRatio(kTRUE);
            h->Draw("COLZ");

            // Dotted reference lines + explicit 7.3 tick labels.
            stampGeometryRefs(c);
            stampBeamArrow(c);

            TBox* outline = new TBox(geomLo, geomLo, geomHi, geomHi);
            outline->SetLineColor(kWhite);
            outline->SetLineWidth(2);
            outline->SetFillStyle(0);
            outline->Draw();

            c->Update();
            c->SaveAs(Form("spatial_yz_%s.png", geomLabel.Data()));
        } else {
            ::Warning("plotspatial",
                      "No stopping-track rows to fill y-z plot for %s.",
                      geomLabel.Data());
        }
    }

    // ------------------------------------------------------------------
    // z-x plot
    // ------------------------------------------------------------------
    {
        const TString hname = Form("hZX_%s_%lld",
                                   geomLabel.Data(),
                                   (Long64_t)gRandom->Integer(1 << 30));
        // Display-only shift (see y-z block above).
        const TString expr  = Form("(stopX+%g):(stopZ+%g)>>%s(%d,%g,%g,%d,%g,%g)",
                                   half, half, hname.Data(),
                                   nB, axisLo, axisHi, nB, axisLo, yAxisHi);
        const Long64_t nFilled = t->Draw(expr, "stepNum==1", "goff");
        TH2D* h = (TH2D*)gDirectory->Get(hname);
        if (h && nFilled > 0) {
            h->SetDirectory(nullptr);
            h->SetStats(0);
            h->SetTitle(Form("Stopping Muon Spatial Distribution (z-x) "
                             "#minus #font[62]{%s};z [cm];x [cm]",
                             geomLabel.Data()));
            h->GetXaxis()->SetTitleOffset(1.05);
            h->GetYaxis()->SetTitleOffset(1.15);
            h->GetXaxis()->CenterTitle(true);
            h->GetYaxis()->CenterTitle(true);
            // Integer ticks every 1 cm -> 0 is always labelled.
            // Both spans are 11.3 (non-integer), so let ROOT optimise
            // to round labels by asking for ~11 divisions with
            // optimisation enabled; this yields integer ticks from
            // -2 to 9 on both axes.
            h->GetXaxis()->SetNdivisions(11, kTRUE);
            h->GetYaxis()->SetNdivisions(11, kTRUE);

            TCanvas* c = new TCanvas(Form("cZX_%s", geomLabel.Data()),
                                     Form("Stopping muons z-x (%s)",
                                          geomLabel.Data()),
                                     720, 720);
            c->SetLeftMargin(0.13);
            c->SetRightMargin(0.15);
            c->SetBottomMargin(0.12);
            c->SetTopMargin(0.10);
            c->SetFixedAspectRatio(kTRUE);
            h->Draw("COLZ");

            // Dotted reference lines + explicit 7.3 tick labels.
            stampGeometryRefs(c);
            stampBeamArrow(c);

            if (isCyl) {
                // CylPV cross-section in z-x: disk of radius `half`,
                // now centred at (geomCtr, geomCtr) so the geometry
                // spans 0 -> 7.3 cm on each axis.
                TEllipse* outline = new TEllipse(geomCtr, geomCtr, half, half);
                outline->SetLineColor(kWhite);
                outline->SetLineWidth(2);
                outline->SetFillStyle(0);
                outline->Draw();
            } else {
                TBox* outline = new TBox(geomLo, geomLo, geomHi, geomHi);
                outline->SetLineColor(kWhite);
                outline->SetLineWidth(2);
                outline->SetFillStyle(0);
                outline->Draw();
            }

            c->Update();
            c->SaveAs(Form("spatial_zx_%s.png", geomLabel.Data()));
        } else {
            ::Warning("plotspatial",
                      "No stopping-track rows to fill z-x plot for %s.",
                      geomLabel.Data());
        }
    }

    Printf("[Spatial-%s] saved y-z and z-x stopping-muon distributions.",
           geomLabel.Data());

    if (savedStyle) {
        savedStyle->cd();
    }
}
