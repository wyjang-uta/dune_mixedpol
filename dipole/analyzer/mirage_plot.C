#include "ROOT/RDataFrame.hxx"
#include "TCanvas.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"

void mirage_plot(std::string inputFile="input.root", std::string outputFile="output.root"){
    TFile* f = TFile::Open(inputFile.c_str());
    if (!f || f->IsZombie()) return;
    TString treeName = f->GetListOfKeys()->At(0)->GetName();
    ROOT::RDataFrame df(treeName, inputFile);

    auto df_valid = df.Filter("daughterE > 0");

    auto df_numu = df_valid.Filter("daughterPDG == 14 && daughterPz > 0");
    auto df_numubar = df_valid.Filter("daughterPDG == -14 && daughterPz > 0");
    auto df_nue = df_valid.Filter("daughterPDG == 12 && daughterPz > 0");
    auto df_nuebar = df_valid.Filter("daughterPDG == -12 && daughterPz > 0");

    auto h_energy = df_valid.Histo1D(
        {"h_energy", "Neutrino Energy; Energy (MeV);Counts", 100, 0, 5000},
        "daughterE"
    );

    auto h_profile = df_valid.Histo2D(
        {"h_profile", "Neutrino Profile at ND(574 m); x [mm]; y [mm]", 100, -500, 500, 100, -500, 500},
        "projXat574m", "projYat574m"
    );

    auto h_numu_10m_x_10m = df_numu.Histo2D({"h_numu_10m_x_10m", "Neutrino Profile at ND(10 m); x [mm]; y [mm]", 500, -5000, 5000, 500, -5000, 5000},"projXat574m", "projYat574m");
    auto h_numubar_10m_x_10m = df_numubar.Histo2D({"h_numubar_10m_x_10m", "Neutrino Profile at ND(10 m); x [mm]; y [mm]", 500, -5000, 5000, 500, -5000, 5000},"projXat574m", "projYat574m");
    auto h_nue_10m_x_10m = df_nue.Histo2D({"h_nue_10m_x_10m", "Neutrino Profile at ND(10 m); x [mm]; y [mm]", 500, -5000, 5000, 500, -5000, 5000},"projXat574m", "projYat574m");
    auto h_nuebar_10m_x_10m = df_nuebar.Histo2D({"h_nuebar_10m_x_10m", "Neutrino Profile at ND(10 m); x [mm]; y [mm]", 500, -5000, 5000, 500, -5000, 5000},"projXat574m", "projYat574m");

    auto h_numu_100m_x_100m = df_numu.Histo2D({"h_numu_100m_x_100m", "Neutrino Profile at ND(574 m); x [mm]; y [mm]", 500, -50000, 50000, 500, -50000, 50000},"projXat574m", "projYat574m");
    auto h_numubar_100m_x_100m = df_numubar.Histo2D({"h_numubar_100m_x_100m", "Neutrino Profile at ND(574 m); x [mm]; y [mm]", 500, -50000, 50000, 500, -50000, 50000},"projXat574m", "projYat574m");
    auto h_nue_100m_x_100m = df_nue.Histo2D({"h_nue_100m_x_100m", "Neutrino Profile at ND(574 m); x [mm]; y [mm]", 500, -50000, 50000, 500, -50000, 50000},"projXat574m", "projYat574m");
    auto h_nuebar_100m_x_100m = df_nuebar.Histo2D({"h_nuebar_100m_x_100m", "Neutrino Profile at ND(574 m); x [mm]; y [mm]", 500, -50000, 50000, 500, -50000, 50000},"projXat574m", "projYat574m");

    auto h_numu_10000m_x_10000m = df_numu.Histo2D({"h_numu_10000m_x_10000m", "Neutrino Profile at ND(10000 m); x [mm]; y [mm]", 500, -5000000, 5000000, 500, -5000000, 5000000},"projXat574m", "projYat574m");
    auto h_numubar_10000m_x_10000m = df_numubar.Histo2D({"h_numubar_10000m_x_10000m", "Neutrino Profile at ND(10000 m); x [mm]; y [mm]", 500, -5000000, 5000000, 500, -5000000, 5000000},"projXat574m", "projYat574m");
    auto h_nue_10000m_x_10000m = df_nue.Histo2D({"h_nue_10000m_x_10000m", "Neutrino Profile at ND(10000 m); x [mm]; y [mm]", 500, -5000000, 5000000, 500, -5000000, 5000000},"projXat574m", "projYat574m");
    auto h_nuebar_10000m_x_10000m = df_nuebar.Histo2D({"h_nuebar_10000m_x_10000m", "Neutrino Profile at ND(10000 m); x [mm]; y [mm]", 500, -5000000, 5000000, 500, -5000000, 5000000},"projXat574m", "projYat574m");

    auto df_numu_ff = df_numu.Filter("projXat574m > -3500 && projXat574m < 3500 && projYat574m > -1750 && projYat574m < 1750");
    auto df_numubar_ff = df_numubar.Filter("projXat574m > -3500 && projXat574m < 3500 && projYat574m > -1750 && projYat574m < 1750");
    auto df_nue_ff = df_nue.Filter("projXat574m > -3500 && projXat574m < 3500 && projYat574m > -1750 && projYat574m < 1750");
    auto df_nuebar_ff = df_nuebar.Filter("projXat574m > -3500 && projXat574m < 3500 && projYat574m > -1750 && projYat574m < 1750");

    auto h_numu_ff_daughterE = df_numu_ff.Histo1D({"h_numu_ff_daughterE", "Numu daughterE after FF; daughterE; Events", 200, 0, 20},"daughterE");
    auto h_numubar_ff_daughterE = df_numubar_ff.Histo1D({"h_numubar_ff_daughterE", "Numubar daughterE after FF; daughterE; Events", 200, 0, 20},"daughterE");
    auto h_nue_ff_daughterE = df_nue_ff.Histo1D({"h_nue_ff_daughterE", "Nue daughterE after FF; daughterE; Events", 200, 0, 20},"daughterE");
    auto h_nuebar_ff_daughterE = df_nuebar_ff.Histo1D({"h_nuebar_ff_daughterE", "Nuebar daughterE after FF; daughterE; Events", 200, 0, 20},"daughterE");

    TFile out(outputFile.c_str(), "RECREATE");
    h_energy->Write();
    h_profile->Write();

    h_numu_10m_x_10m->Write();
    h_numubar_10m_x_10m->Write();
    h_nue_10m_x_10m->Write();
    h_nuebar_10m_x_10m->Write();

    h_numu_100m_x_100m->Write();
    h_numubar_100m_x_100m->Write();
    h_nue_100m_x_100m->Write();
    h_nuebar_100m_x_100m->Write();

    h_numu_10000m_x_10000m->Write();
    h_numubar_10000m_x_10000m->Write();
    h_nue_10000m_x_10000m->Write();
    h_nuebar_10000m_x_10000m->Write();

    h_numu_ff_daughterE->Write();
    h_numubar_ff_daughterE->Write();
    h_nue_ff_daughterE->Write();
    h_nuebar_ff_daughterE->Write();

    out.Close();

    std::cout << ">>> Analysis complete: " << outputFile << std::endl;
}
