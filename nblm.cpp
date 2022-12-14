#include <cstdlib>
#include <fstream>
#include <iostream>

#include <TApplication.h>
#include <TCanvas.h>
#include <TH1F.h>

#include "Garfield/AvalancheMC.hh"
#include "Garfield/AvalancheMicroscopic.hh"
#include "Garfield/ComponentComsol.hh"
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/Random.hh"
#include "Garfield/Sensor.hh"
#include "Garfield/TrackHeed.hh"
#include "Garfield/ViewFEMesh.hh"
#include "Garfield/ViewField.hh"
#include "Garfield/ViewSignal.hh"

using namespace Garfield;

double transfer(double t) {
  constexpr double tau = 2.;
  return (t / tau) * exp(1 - t / tau);
}

int main(int argc, char *argv[]) {

  TApplication app("app", &argc, argv);

  // Load the field map.
  ComponentComsol fm;
  fm.Initialise("data/umegas_nblm_mesh_11.mphtxt", "data/nBLM_dielectrics.dat",
                "data/umegas_nblm_field_11.txt", "mm");

  const std::string label = "ReadoutPlane";
  fm.SetWeightingPotential("data/umegas_nblm_ramo_11.txt", label);

  fm.EnableMirrorPeriodicityX();
  fm.EnableMirrorPeriodicityY();
  // fm.EnableMirrorPeriodicityZ();
  fm.PrintRange();

  constexpr double pitch = 0.006;

  ViewField fieldView;
  constexpr bool plotField = true;
  if (plotField) {
    fieldView.SetComponent(&fm);
    // Set the normal vector of the viewing plane (xz plane).
    fieldView.SetPlane(0, -1, 0, 0, 0, 0);
    fieldView.SetArea(-2 * pitch, 0.0, 2 * pitch, 0.17);

    // Set the plot limits in the current viewing plane.
    // fieldView.SetArea(-0.0375, -0.012, 0.0375, 0.012);
    // fieldView.SetVoltageRange(-0.0, 1500.);
    TCanvas *cf = new TCanvas("cf", "", 800, 600);
    cf->SetLeftMargin(0.16);
    fieldView.SetCanvas(cf);
    fieldView.Plot("p", "COLZ");
  }

  // ViewFEMesh meshView;
  // constexpr bool plotMesh = true;
  // if (plotMesh) {
  //   meshView.SetComponent(&fm);
  //   meshView.SetPlane(0, -1, 0, 0, 0, 0);
  //   meshView.SetFillMesh(false);
  //   TCanvas *cm = new TCanvas("cm", "", 1000, 1000);
  //   cm->SetLeftMargin(0.16);
  //   meshView.SetCanvas(cm);
  // }

  // Setup the gas.
  MediumMagboltz gas;
  gas.SetComposition("he", 96.5, "co2", 3.5);
  gas.SetTemperature(293.15);
  gas.SetPressure(760.);
  gas.Initialise(true);
  // Set the Penning transfer efficiency.
  constexpr double rPenning = 0.51;
  constexpr double lambdaPenning = 0.;
  // gas.EnablePenningTransfer(rPenning, lambdaPenning, "he");
  //  Load the ion mobilities.
  const std::string path = std::getenv("GARFIELD_INSTALL");
  gas.LoadIonMobility(path + "/share/Garfield/Data/IonMobility_He+_He.txt");
  // Associate the gas with the corresponding field map material.
  const unsigned int nMaterials = fm.GetNumberOfMaterials();
  for (unsigned int i = 0; i < nMaterials; ++i) {
    const double eps = fm.GetPermittivity(i);
    if (eps == 1.)
      fm.SetMedium(i, &gas);
  }
  fm.PrintMaterials();

  // Create the sensor.
  Sensor sensor;
  sensor.AddComponent(&fm);
  sensor.AddElectrode(&fm, label);
  // sensor.AddWhiteNoise(2);
  sensor.SetTransferFunction(transfer);
  sensor.SetTimeWindow(0., 0.1, 4000);
  //  ~40 mesh cells
  sensor.SetArea(-20 * pitch, -20 * pitch, 0, 20 * pitch, 20 * pitch, 0.17);

  ViewField wfieldView;
  constexpr bool plotWeightingField = true;
  if (plotWeightingField) {
    wfieldView.SetComponent(&fm);
    wfieldView.SetSensor(&sensor);
    fieldView.SetVoltageRange(0.0, 1.0);
    wfieldView.SetPlane(0, -1, 0, 0, 0, 0);
    wfieldView.SetArea(-2 * pitch, 0.0, 2 * pitch, 0.17);
    TCanvas *cw = new TCanvas("cw", "", 600, 600);
    cw->SetLeftMargin(0.16);
    wfieldView.SetCanvas(cw);
    wfieldView.PlotWeightingField(label, "p", "COLZ");
  }

  // // Create a track (120 GeV e-)
  // TrackHeed track;
  // track.SetParticle("pi");
  // track.SetKineticEnergy(120e9);
  // track.SetSensor(&sensor);

  // AvalancheMicroscopic aval;
  // aval.SetSensor(&sensor);
  // aval.UseWeightingPotential(true);
  // aval.EnableWeightingFieldIntegration(true);
  // aval.EnableSignalCalculation();

  // AvalancheMC drift;
  // drift.SetSensor(&sensor);
  // drift.UseWeightingPotential(true);
  // drift.EnableSignalCalculation();
  // drift.SetDistanceSteps(2.e-4);

  // ViewDrift driftView;
  // constexpr bool plotDrift = true;
  // if (plotDrift) {
  //   aval.EnablePlotting(&driftView);
  //   drift.EnablePlotting(&driftView);
  //   track.EnablePlotting(&driftView);
  // }

  // auto plotSignal = true;
  // ViewSignal *signalView;
  // TCanvas *cSignal;
  // if (plotSignal) {
  //   cSignal = new TCanvas("cSignal", "", 600, 600);
  //   signalView = new ViewSignal();
  //   signalView->SetCanvas(cSignal);
  //   signalView->SetSensor(&sensor);
  // }

  // // Number of tracks
  // constexpr unsigned int nEvents = 50;
  // for (unsigned int i = 0; i < nEvents; ++i) {
  //   sensor.NewSignal();
  //   std::cout << i << "/" << nEvents << "\n";
  //   // Randomize the initial position.
  //   //double xtrack0 = -5 * pitch + RndmUniform() * 10 * pitch;

  //   double x0, y0, z0, t0, e0;
  //   int nc;
  //   double extra;

  //   track.NewTrack(-0.01, 0.0, 0.16, 0, 1, 0, -1);
  //   while (track.GetCluster(x0, y0, z0, t0, nc, e0, extra)) {
  //     for (int k = 0; k < nc; ++k) {
  //       double xe = 0., ye = 0., ze = 0., te = 0., ee = 0.;
  //       double dx = 0., dy = 0., dz = 0.;
  //       track.GetElectron(k, xe, ye, ze, te, ee, dx, dy, dz);
  //       // drift.DriftElectron(xe, ye, ze, te);
  //       aval.AvalancheElectron(xe, ye, ze, te, ee, dx, dy, dz);
  //       const unsigned int np = aval.GetNumberOfElectronEndpoints();
  //       double xe1, ye1, ze1, te1, e1;
  //       double xe2, ye2, ze2, te2, e2;
  //       double xi1, yi1, zi1, ti1;
  //       double xi2, yi2, zi2, ti2;
  //       int status;
  //       for (unsigned int j = 0; j < np; ++j) {
  //         aval.GetElectronEndpoint(j, xe1, ye1, ze1, te1, e1, xe2, ye2, ze2,
  //                                  te2, e2, status);
  //         drift.DriftIon(xe1, ye1, ze1, te1);
  //         drift.GetIonEndpoint(0, xi1, yi1, zi1, ti1, xi2, yi2, zi2, ti2,
  //                              status);
  //       }
  //     }
  //   }
  // }

  // // sensor.IntegrateSignals();
  // if (plotSignal) {
  //   // Plot signals
  //   sensor.ConvoluteSignals();
  //   signalView->PlotSignal(label, true, true, true);
  // }

  // if (plotDrift) {
  //   TCanvas *cd = new TCanvas("cd", "", 600, 600);
  //   constexpr bool plotMesh = false;
  //   if (plotMesh) {
  //     ViewFEMesh *meshView = new ViewFEMesh();
  //     meshView->SetCanvas(cd);
  //     meshView->SetComponent(&fm);
  //     // x-z projection.
  //     meshView->SetPlane(0, -1, 0, 0, 0, 0);
  //     meshView->SetFillMesh(true);
  //     // Set the color of the kapton and the metal.
  //     meshView->SetColor(1, kYellow + 3);
  //     meshView->SetColor(0, kGray);
  //     meshView->SetArea(-20 * pitch, -20 * pitch, 0, 20 * pitch, 20 * pitch,
  //                       0.17);
  //     meshView->EnableAxes();
  //     meshView->SetViewDrift(&driftView);
  //     meshView->Plot();
  //   } else {
  //     driftView.SetPlane(0, -1, 0, 0, 0, 0);
  //     driftView.SetArea(-20 * pitch, 0.0, 20 * pitch, 0.2);
  //     driftView.SetCanvas(cd);
  //     constexpr bool twod = true;
  //     driftView.Plot(twod);
  //   }
  // }

  app.Run(kTRUE);
}
