#include "ROOT/RDataFrame.hxx"
#include "TCanvas.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"

void mirage_plot(std::string inputFile="input.root", std::string outputFile="output.root"){
    ROOT::RDataFrame df("mirage", inputFile);

    auto df_valid = df.Filter("daughterE > 0");

    auto h_energy = df_valid.Histo1D(
        {"h_energy", "Neutrino Energy; Energy (MeV);Counts", 100, 0, 5000},
        "daughterE"
    );

    auto h_profile = df_valid.Histo2D(
        {"h_profile", "Neutrino Profile at ND(574 m); x [mm]; y [mm]", 100, -500, 500, 100, -500, 500},
        "projXat574m", "projYat574m"
    );

    TFile out(outputFile.c_str(), "RECREATE");
    h_energy->Write();
    h_profile->Write();
    out.Close();

    std::cout << ">>> Analysis complete: " << outputFile << std::endl;
}