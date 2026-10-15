// Full-access sound module for all users of the game

#include <cstdlib>

#include "BGame.h"
#include "SoundModule.h"

bool SoundModule::m_bRunning = false;

int SoundModule::m_nMenuMusicVolume = 128;
int SoundModule::m_nVehicleSoundsVolume = 128;
int SoundModule::m_nSkidSoundVolume = 0;
int SoundModule::m_nEngineSoundVolume = 160;
int SoundModule::m_nMessageSoundVolume = 128;
int SoundModule::m_nHeliSoundVolume = 255;
int SoundModule::m_nJetSoundVolume = 255;

Sound SoundModule::sMenuMusic;
Sound SoundModule::sVehicleSounds;
Sound SoundModule::sEngineSound1;
Sound SoundModule::sEngineSound2;
Sound SoundModule::sMessageSound;
Sound SoundModule::sSkidSound;
Sound SoundModule::sCrashSound;
Sound SoundModule::sHeliSound;
Sound SoundModule::sJetSound;
Sound SoundModule::s43AASound;

vector<Sound *> SoundModule::playPool;
vector<Sound *> SoundModule::sounds;
SDL_mutex *SoundModule::mix_mutex;

int SoundModule::m_nSpace = 1;

SoundModule::SoundModule() {}

void SoundModule::Initialize() {
  SDL_AudioSpec *spec = new SDL_AudioSpec;
  spec->freq = 44100;
  spec->format = AUDIO_S16;
  spec->samples = 4096;
  spec->callback = mix;
  spec->userdata = NULL;
  spec->channels = 2;
  if (SDL_OpenAudio(spec, NULL) < 0) {
    cout << "AudioMixer, Unable to open audio: " << SDL_GetError() << endl;
  } else {
    m_bRunning = true;
    SDL_PauseAudio(0);
  }

  sounds.push_back(&sMenuMusic);
  sounds.push_back(&sVehicleSounds);
  sounds.push_back(&sEngineSound1);
  sounds.push_back(&sEngineSound2);
  sounds.push_back(&sMessageSound);
  sounds.push_back(&sSkidSound);
  sounds.push_back(&sCrashSound);
  sounds.push_back(&sHeliSound);
  sounds.push_back(&sJetSound);
  sounds.push_back(&s43AASound);

  mix_mutex = SDL_CreateMutex();
}

void SoundModule::Close() {
  m_bRunning = false;
  SDL_DestroyMutex(mix_mutex);
}

void SoundModule::SetMenuMusicVolume(int nVol) {
  m_nMenuMusicVolume = nVol;
  sMenuMusic.setVolume(m_nMenuMusicVolume);
}

void SoundModule::SetVehicleSoundsVolume(int nVol) {
  m_nVehicleSoundsVolume = nVol;
  sVehicleSounds.setVolume(m_nVehicleSoundsVolume);
}

void SoundModule::SetSkidSoundVolume(int nVol) {
  m_nSkidSoundVolume = nVol;
  sSkidSound.setVolume(
      int(double(m_nSkidSoundVolume) * double(m_nVehicleSoundsVolume) / 255.0));
}

void SoundModule::SetEngineSoundVolume(int nVol) {
  m_nEngineSoundVolume = nVol;
  sEngineSound1.setVolume(int(double(m_nEngineSoundVolume) *
                              double(m_nVehicleSoundsVolume) / 255.0));
}

void SoundModule::SetCrashSoundVolume(int nVol) {
  int nNewVol = int(double(nVol) * double(m_nVehicleSoundsVolume) / 255.0);
  if (nNewVol > 255) {
    nNewVol = 255;
  }
  sCrashSound.setVolume(nNewVol);
}

void SoundModule::SetHeliSoundVolume(int nVol) {
  m_nHeliSoundVolume = nVol;

  int nNewVol =
      int(double(m_nHeliSoundVolume) * double(m_nVehicleSoundsVolume) / 255.0);
  if (nNewVol > 255) {
    nNewVol = 255;
  }
  sHeliSound.setVolume(nNewVol);
}

void SoundModule::SetJetSoundVolume(int nVol) {
  m_nJetSoundVolume = nVol;

  int nNewVol =
      int(double(m_nJetSoundVolume) * double(m_nVehicleSoundsVolume) / 255.0);
  if (nNewVol > 255) {
    nNewVol = 255;
  }

  sJetSound.setVolume(nNewVol);
}

void SoundModule::SetMessageSoundVolume(int nVol) {
  m_nMessageSoundVolume = nVol;
  sMessageSound.setVolume(m_nMessageSoundVolume);
}

void SoundModule::SetSoundSpace(int nSpace) { m_nSpace = nSpace; }

void SoundModule::SetOutput(int nOutput) {}

int SoundModule::GetOutput() { return 0; }

void SoundModule::SetDriver(int nDriver) {}

int SoundModule::GetNumDrivers() { return 0; }

char *SoundModule::GetDriverName(int nDriver) {
  char *moi = new char[200];
  return moi;
}

void SoundModule::StartMenuMusic() {
  if (m_bRunning) {
    if (!sMenuMusic.isLoaded()) {
      sMenuMusic.loadSound("Sounds/MixDown3.ogg");
    }
    if (sMenuMusic.isLoaded()) {
      sMenuMusic.play();
      sMenuMusic.setVolume(m_nMenuMusicVolume);
    }
  }
}

void SoundModule::StopMenuMusic() {
  if (m_bRunning) {
    sMenuMusic.stop();
  }
}

void SoundModule::StartSkidSound() {
  return;
  if (m_bRunning) {
    if (!sSkidSound.isLoaded()) {
      sSkidSound.loadSound("Sounds/WhiteNoise.ogg");
    }
    if (sSkidSound.isLoaded()) {
      sSkidSound.play();
      sSkidSound.setVolume(m_nSkidSoundVolume);
    }
  }
}

void SoundModule::StopSkidSound() {
  return;
  if (m_bRunning) {
    sSkidSound.stop();
  }
}

void SoundModule::StartEngineSound() {
  if (m_bRunning) {
    if (!sEngineSound1.isLoaded()) {
      sEngineSound1.loadSound("Sounds/EngineJuliet2.ogg");
    }
    if (sEngineSound1.isLoaded()) {
      sEngineSound1.play();
      SetEngineSoundVolume(255);
    }
  }
}

void SoundModule::StopEngineSound() {
  if (m_bRunning) {
    sEngineSound1.stop();
  }
}

void SoundModule::SetEngineSoundRPM(int nRPM) {
  static int nFluctuation = 0;
  nFluctuation += rand() % 200 - 99;
  if (abs(nFluctuation) > 999) {
    nFluctuation = 999 * nFluctuation / 1000;
  }
  sEngineSound1.setFreq(44100 + int(double(nRPM) / 1.5) + nFluctuation / 2);
}

void SoundModule::StartMessageSound() {
  if (m_bRunning) {
    if (!sMessageSound.isLoaded()) {
      sMessageSound.loadSound("Sounds/IncomingMessage.ogg");
    }
    if (sMessageSound.isLoaded()) {
      sMessageSound.play();
      SetMessageSoundVolume(128);
    }
  }
}

void SoundModule::StopMessageSound() {
  if (m_bRunning) {
    sMessageSound.stop();
  }
}

void SoundModule::PreCache43AASound() {}

void SoundModule::Start43AASound() {
  if (m_bRunning) {
    if (!s43AASound.isLoaded()) {
      s43AASound.loadSound("Sounds/43aa.ogg");
    }
    if (s43AASound.isLoaded()) {
      s43AASound.play();
      s43AASound.setVolume(255);
    }
  }
}

void SoundModule::Stop43AASound() {
  if (m_bRunning) {
    s43AASound.stop();
    s43AASound.unloadSound();
  }
}

void SoundModule::StartHeliSound() {
  if (m_bRunning) {
    if (!sHeliSound.isLoaded()) {
      sHeliSound.loadSound("Sounds/huey.ogg");
    }
    if (sHeliSound.isLoaded()) {
      sHeliSound.play();
      SetHeliSoundVolume(m_nHeliSoundVolume);
    }
  }
}

void SoundModule::StopHeliSound() {
  if (m_bRunning) {
    sHeliSound.stop();
  }
}

void SoundModule::SetHeliSoundPhase(double dPhase, double dBladePower) {
  SetHeliSoundVolume(int(dPhase * 255.0 * dBladePower));
  sHeliSound.setFreq(int(44100.0 * dPhase));
}

void SoundModule::StartJetSound() {
  if (m_bRunning) {
    if (!sJetSound.isLoaded()) {
      sJetSound.loadSound("Sounds/JetMono.ogg");
    }
    if (sJetSound.isLoaded()) {
      sJetSound.play();
      SetJetSoundVolume(255);
    }
  }
}

void SoundModule::StopJetSound() {
  if (m_bRunning) {
    sJetSound.stop();
  }
}

void SoundModule::SetJetSoundPhase(double dPhase) {
  sJetSound.setFreq(44100 + int(20000.0 * dPhase));
}

void SoundModule::PlayCrashSound(double dVolume) {
  static unsigned clockPrev = SDL_GetTicks();
  unsigned clockNow = SDL_GetTicks();
  if (m_bRunning && ((double(clockNow - clockPrev) / 1000.0) > 0.02) &&
      (dVolume > 0.1)) {
    if (dVolume > 1.0) {
      dVolume = 1.0;
    }
    clockPrev = clockNow;
    if (!sCrashSound.isLoaded()) {
      sCrashSound.loadSound("Sounds/CRASH.ogg");
      sCrashSound.setLoop(false);
    }
    if (sCrashSound.isLoaded()) {
      sCrashSound.setFreq(22050 + rand() % 10000);
      SetCrashSoundVolume(55 + int(dVolume * 200.0));
      playOnSoundPool(sCrashSound);
    }
  }
}

void SoundModule::Update3DSounds(BVector &rvSouLoc, BVector &rvSouVel,
                                 BVector &rvLisLoc, BOrientation &roLis,
                                 BVector &rvLisVel) {}

void SoundModule::mix(void *userdata, Uint8 *stream, int len) {
  memset(stream, 0, len);

  for (int y = 0; y < sounds.size(); y++) {
    mixOne(sounds[y], stream, len);
  }

  SDL_LockMutex(mix_mutex);
  for (int y = 0; y < playPool.size(); y++) {
    mixOne(playPool[y], stream, len);
  }
  SDL_UnlockMutex(mix_mutex);
}

void SoundModule::mixOne(Sound *sound, Uint8 *stream, int len) {
  char *samples = new char[len];
  sound->getSamples(samples, len);
  Sint16 *p_stream = (Sint16 *)stream;
  Sint16 *p_samples = (Sint16 *)samples;
  for (int x = 0; x < len; x += 2) {
    Sint32 val = (Sint32)*p_stream + *p_samples;
    if (val > 32767) {
      val = 32767;
    } else if (val < -32766) {
      val = -32766;
    }
    *p_stream = val;
    p_stream++;
    p_samples++;
  }
  delete[] samples;
}

void SoundModule::playOnSoundPool(Sound &sound) {
  SDL_LockMutex(mix_mutex);
  for (int y = 0; y < playPool.size(); y++) {
    if (!playPool[y]->isPlaying()) {
      delete playPool[y];
      playPool.erase(playPool.begin() + y);
      y--;
    }
  }
  Sound *new_sound = new Sound(sound);
  new_sound->play();
  playPool.push_back(new_sound);
  SDL_UnlockMutex(mix_mutex);
}
