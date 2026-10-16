// BPlayer - player state via SQLite database.
//
// Previously used Player.state INI file with FileHelpers. Now uses
// Database::loadPlayerState/savePlayerState and
// Database::loadSceneProgress/saveSceneProgress.
#include <sstream>

using namespace std;

#include "BGame.h"
#include "BPlayer.h"
#include "Database.h"

BPlayer::BPlayer() {
  m_dCash = 50.0;
  m_dFuel = 25.0;
  m_dKerosine = 25.0;
  m_sValidVehicles = ">Bogian<";
  m_sSceneInfo = "";
}

void BPlayer::SaveStateFile() {
  if (!Database::isReady()) return;

  Database::PlayerState state;
  state.cash = m_dCash;
  state.fuel = m_dFuel;
  state.kerosine = m_dKerosine;
  state.validVehicles = m_sValidVehicles;
  state.checksum = BGame::GetScrambleChecksum();
  Database::savePlayerState(state);
}

void BPlayer::LoadStateFile() {
  if (!Database::isReady()) return;

  Database::PlayerState state;
  if (!Database::loadPlayerState(state)) {
    // No player state in DB yet - use defaults.
    return;
  }

  m_dCash = state.cash;
  m_dFuel = state.fuel;
  m_dKerosine = state.kerosine;
  m_sValidVehicles = state.validVehicles;

  // Checksum validation (same as legacy Player.state).
  if (state.checksum.compare(BGame::GetScrambleChecksum()) != 0) {
    BGame::MyAfxMessageBox("Player state checksum doesn't match!");
    BGame::MyAfxMessageBox("Resetting the player state to default values.");
    m_dCash = 1000.0;
    m_dFuel = 25.0;
    m_dKerosine = 25.0;
    m_sValidVehicles = ">Bogian<";
    SaveStateFile();
  }
}

void BPlayer::SaveCurrentSceneInfo() {
  if (!Database::isReady()) return;

  BScene *pScene = BGame::GetSimulation()->GetScene();
  BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();

  Database::SceneProgress progress;
  progress.randomDelivery = pScene->m_bRandomDeliveryInUse;
  progress.deliveryEntry = pScene->m_nCurrentDeliveryEntry;
  progress.vehicleX = pVehicle->m_vLocation.m_dX;
  progress.vehicleY = pVehicle->m_vLocation.m_dY;
  progress.vehicleZ = pVehicle->m_vLocation.m_dZ;
  progress.hasVehicleLocation = true;

  Database::saveSceneProgress(pScene->m_sName, progress);
}

bool BPlayer::LoadCurrentSceneInfo(BVector &rvVehicleLoc) {
  if (!Database::isReady()) return false;

  BScene *pScene = BGame::GetSimulation()->GetScene();

  Database::SceneProgress progress;
  if (!Database::loadSceneProgress(pScene->m_sName, progress)) {
    // No saved progress for this scene.
    return false;
  }

  // Apply delivery state.
  pScene->m_bRandomDeliveryInUse = progress.randomDelivery;
  if (!progress.randomDelivery) {
    int nClientCounter = progress.deliveryEntry;
    if ((nClientCounter < 0) ||
        (nClientCounter > (pScene->m_nDeliveryOrderEntries - 1))) {
      pScene->m_bRandomDeliveryInUse = true;
    } else {
      pScene->m_nCurrentDeliveryEntry = nClientCounter;
    }
  }

  // Apply vehicle location.
  if (progress.hasVehicleLocation) {
    rvVehicleLoc = BVector(progress.vehicleX, progress.vehicleY,
                           progress.vehicleZ);
    return true;
  }

  return false;
}
