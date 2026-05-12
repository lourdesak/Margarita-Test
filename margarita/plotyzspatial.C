// Standalone YZ spatial-distribution plot for stopping muons.
//
// Run independently of plotstoppingmuons.C so it cannot affect any
// existing plot:
//
//   root -l 'plotyzspatial.C("g4marg.root")'        // BoxPV
//   root -l 'plotyzspatial.C("g4marg_cyl.root")'    // CylPV
//
// Produces a single canvas + PNG:
//   spatial_yz_BoxPV.png   /   spatial_yz_CylPV.png
//
// Coordinate convention (per spec):
//   detector occupies 0..7.3 cm in both displayed axes;
//   ~1 cm empty margin around it on all sides; equal aspect ratio.
// Geant4 stores positions centered on the origin (half-extent 3.65 cm),
// so we display
//     z_disp = 3.65 - stopZ   (0 at muon entry face, 7.3 at exit face)
//     y_disp = stopY + 3.65   (0 at lower edge,       7.3 at upper)
//
// Only stopping muons appear because the stoppingSteps ntuple is only
// written for tracks that stopped inside the active volume. We further
// take one row per track via stepNum==1 so the histogram counts each
// stopping muon exactly once.

void plotyzspatial(const char* fname = "g4marg.root")
{
    // Save current global style so this script doesn't leak settings into
    // any other ROOT session state.
    TStyle* savedStyle = gStyle ? (TStyle*)gStyle->Clone("savedStyleBeforeYZ") : nullptr;

    TFile* f = TFile::Open(fname);
    if (!f || f->IsZombie()) {
        ::Error("plotyzspatial", "Cannot open %s", fname);
        return;
    }

    TTree* t = (TTree*)f->Get("ntuple/stoppingSteps");
    if (!t) t = (TTree*)f->Get("stoppingSteps");
    if (!t) {
        ::Error("plotyzspatial",
                "No stoppingSteps ntuple in %s. Rebuild and rerun the macro first.",
                fname);
        f->Close();
        return;
    }

    // Infer geometry label from filename.
    TString fn(fname);
    const bool isCyl = fn.Contains("cyl");
    const TString geomLabel = isCyl ? "CylPV" : "BoxPV";

    // Coordinate window.
    const double half   = 3.65;          // detector half-extent [cm]
    const double margin = 1.0;           // empty margin [cm]
    const double lo     = -margin;       // -1.0
    const double hi     = 2.0 * half + margin; // 8.3

    // Local style — won't leak because we restore savedStyle at the end.
    gStyle->SetPalette(kDeepSea);
    gStyle->SetNumberContours(100);
    gStyle->SetOptStat(0);

    // Build the y-z histogram with shifted coordinates via TTree::Draw.
    // The histogram is created in gDirectory; we immediately detach it.
    const TString hname = Form("hYZ_%s_%lld",
                               geomLabel.Data(), (Long64_t)gRandom->Integer(1<<30));
    const TString expr  = Form("(stopY+%g):(%g-stopZ)>>%s(90,%g,%g,90,%g,%g)",
                               half, half, hname.Data(), lo, hi, lo, hi);
    const Long64_t nFilled = t->Draw(expr, "stepNum==1", "goff");
    TH2D* hYZ = (TH2D*)gDirectory->Get(hname);
    if (!hYZ || nFilled == 0) {
        ::Error("plotyzspatial",
                "No stopping-track rows to fill the y-z plot for %s.",
                geomLabel.Data());
        f->Close();
        return;
    }
    hYZ->SetDirectory(nullptr);
    hYZ->SetStats(0);
    hYZ->SetTitle(Form("Stopping Muon Spatial Distribution (y-z) "
                       "#minus #font[62]{%s};z [cm];y [cm]",
                       geomLabel.Data()));
    hYZ->GetXaxis()->SetTitleOffset(1.05);
    hYZ->GetYaxis()->SetTitleOffset(1.15);
    hYZ->GetXaxis()->CenterTitle(true);
    hYZ->GetYaxis()->CenterTitle(true);

    // Dedicated square canvas (unique name; will not collide with c1..c4 from
    // plotstoppingmuons.C even if both scripts were ever run in the same
    // ROOT session).
    TCanvas* cYZ = new TCanvas(Form("cYZ_%s", geomLabel.Data()),
                               Form("Stopping muons y-z (%s)", geomLabel.Data()),
                               720, 720);
    cYZ->SetLeftMargin(0.13);
    cYZ->SetRightMargin(0.15);
    cYZ->SetBottomMargin(0.12);
    cYZ->SetTopMargin(0.10);
    cYZ->SetFixedAspectRatio(kTRUE);

    hYZ->Draw("COLZ");

    // Clean detector outline at the active region (0..7.3 cm in both axes).
    TBox* outline = new TBox(0., 0., 2.0 * half, 2.0 * half);
    outline->SetLineColor(kWhite);
    outline->SetLineWidth(2);
    outline->SetFillStyle(0);
    outline->Draw();

    cYZ->Update();
    cYZ->SaveAs(Form("spatial_yz_%s.png", geomLabel.Data()));

    Printf("[YZ-%s] filled %lld stopping-muon entries  (z_disp = 3.65 - stopZ,  y_disp = stopY + 3.65)",
           geomLabel.Data(), nFilled);

    // Restore prior style so nothing persists if the user later sources
    // another script in the same ROOT process.
    if (savedStyle) {
        savedStyle->cd();
    }
}
