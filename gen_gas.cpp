#include <cstdlib>
#include <fstream>
#include <iostream>

#include "Garfield/AvalancheMC.hh"
#include "Garfield/AvalancheMicroscopic.hh"
#include "Garfield/ComponentComsol.hh"
#include "Garfield/MediumMagboltz.hh"
#include "Garfield/Random.hh"
#include "Garfield/Sensor.hh"
#include "Garfield/TrackHeed.hh"
#include "Garfield/ViewFEMesh.hh"
#include "Garfield/ViewMedium.hh"
#include "Garfield/ViewSignal.hh"

int main(int argc, char *argv[]) {
  using namespace Garfield;
  MediumMagboltz gas;
  const size_t nE = 100;
  const double emin = 100.;
  const double emax = 100000.;
  // Flag to request logarithmic spacing.
  constexpr bool useLog = true;
  gas.SetFieldGrid(emin, emax, nE, useLog);

  gas.SetComposition("He", 96.5, "C2H6", 3.5);
  // Set the temperature [K].
  gas.SetTemperature(293.15);
  // Set the pressure [Torr].
  gas.SetPressure(1.3 * 760.);

  const int ncoll = 4;
  // Run Magboltz to generate the gas table.
  gas.GenerateGasTable(ncoll);

  gas.WriteGasFile("data/He_965_c2h6_035.gas");

  ViewMedium view;
  view.SetMedium(&gas);
  view.PlotElectronVelocity('e');
}
