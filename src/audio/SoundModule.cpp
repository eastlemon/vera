// SoundModule - SDL_mixer implementation.
//
// All gameplay code calls the same SoundModule::Start* / Stop* / Set*Volume
// API as before; this file just swaps the guts from a hand-rolled mixer
// (SDL_OpenAudio + libvorbisfile + manual sample mixing) to SDL_mixer.
//
// SDL_mixer provides:
//   - OGG/MP3/WAV/FLAC decoding (Mix_LoadWAV / Mix_LoadMUS)
//   - Multi-channel mixing with priority and reserved channels
//   - Per-channel and per-chunk volume control
//   - Streaming music channel (Mix_Music) for long background tracks
// so we get to delete ~300 lines of glue code.

#include <cstdlib>
#include <iostream>

#include "BGame.h"
#include "SoundModule.h"

using namespace std;

bool SoundModule::m_bRunning = false;

Mix_Music *SoundModule::m_pMenuMusic     = nullptr;
Mix_Chunk *SoundModule::m_pEngineSound   = nullptr;
Mix_Chunk *SoundModule::m_pHeliSound     = nullptr;
Mix_Chunk *SoundModule::m_pJetSound      = nullptr;
Mix_Chunk *SoundModule::m_pMessageSound  = nullptr;
Mix_Chunk *SoundModule::m_pCrashSound    = nullptr;

int SoundModule::m_nMenuMusicVolume     = MIX_MAX_VOLUME;
int SoundModule::m_nVehicleSoundsVolume = MIX_MAX_VOLUME;
int SoundModule::m_nSkidSoundVolume     = 0;
int SoundModule::m_nEngineSoundVolume   = 160;
int SoundModule::m_nMessageSoundVolume  = MIX_MAX_VOLUME;
int SoundModule::m_nHeliSoundVolume     = MIX_MAX_VOLUME;
int SoundModule::m_nJetSoundVolume      = MIX_MAX_VOLUME;
int SoundModule::m_nSpace               = 1;

SoundModule::SoundModule() {}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void SoundModule::Initialize() {
  // SDL_Init(SDL_INIT_AUDIO) is a no-op if already initialised for video.
  if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
    cout << "SoundModule: SDL_InitSubSystem(AUDIO) failed: " << SDL_GetError()
         << endl;
    return;
  }
  // 44.1 kHz, 16-bit, stereo, 2048-byte sample buffer (latency ~46ms).
  if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048) < 0) {
    cout << "SoundModule: Mix_OpenAudio failed: " << Mix_GetError() << endl;
    return;
  }
  // Reserve the first N channels for looping sounds so Mix_PlayChannel(-1,..)
  // for one-shot effects never collides with them.
  Mix_ReserveChannels(CH_RESERVED_COUNT);
  // Pre-allocate 16 virtual channels for one-shot sounds.
  Mix_AllocateChannels(16);

  m_bRunning = true;
}

void SoundModule::Close() {
  if (!m_bRunning) {
    return;
  }
  // Free all loaded chunks/music.
  if (m_pMenuMusic)    { Mix_FreeMusic(m_pMenuMusic);    m_pMenuMusic   = nullptr; }
  if (m_pEngineSound)  { Mix_FreeChunk(m_pEngineSound);  m_pEngineSound = nullptr; }
  if (m_pHeliSound)    { Mix_FreeChunk(m_pHeliSound);    m_pHeliSound   = nullptr; }
  if (m_pJetSound)     { Mix_FreeChunk(m_pJetSound);     m_pJetSound    = nullptr; }
  if (m_pMessageSound) { Mix_FreeChunk(m_pMessageSound); m_pMessageSound= nullptr; }
  if (m_pCrashSound)   { Mix_FreeChunk(m_pCrashSound);   m_pCrashSound  = nullptr; }

  Mix_CloseAudio();
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
  m_bRunning = false;
}

// ---------------------------------------------------------------------------
// Volume helpers
// ---------------------------------------------------------------------------

// Mix_Volume's range is 0..MIX_MAX_VOLUME (128). The game's UI uses 0..255.
// We translate on every Set* call.
static inline int scaleToMixer(int v) {
  if (v < 0)   v = 0;
  if (v > 255) v = 255;
  return (v * MIX_MAX_VOLUME) / 255;
}

void SoundModule::SetMenuMusicVolume(int nVol) {
  m_nMenuMusicVolume = nVol;
  // Mix_Music has its own volume API, separate from chunk volume.
  Mix_VolumeMusic(scaleToMixer(nVol));
}

void SoundModule::SetVehicleSoundsVolume(int nVol) {
  m_nVehicleSoundsVolume = nVol;
  // Engine/heli/jet volumes are derived from (per-sound * vehicle-sounds).
  SetEngineSoundVolume(m_nEngineSoundVolume);
  SetHeliSoundVolume(m_nHeliSoundVolume);
  SetJetSoundVolume(m_nJetSoundVolume);
}

void SoundModule::SetSkidSoundVolume(int nVol) {
  m_nSkidSoundVolume = nVol;
  // Skid sample is not shipped (WhiteNoise.ogg missing) - no-op.
}

void SoundModule::SetEngineSoundVolume(int nVol) {
  m_nEngineSoundVolume = nVol;
  int scaled = scaleToMixer(int(double(nVol) *
                                 double(m_nVehicleSoundsVolume) / 255.0));
  if (m_pEngineSound) {
    Mix_VolumeChunk(m_pEngineSound, scaled);
  }
}

void SoundModule::SetEngineSoundRPM(int /*nRPM*/) {
  // SDL_mixer does not support per-chunk playback frequency modulation.
  // The previous implementation used this to pitch-shift the engine loop
  // based on RPM. Future work: use Mix_SetPanning or SDL2_mixer's effects
  // API to approximate this. For now it's a no-op.
}

void SoundModule::SetMessageSoundVolume(int nVol) {
  m_nMessageSoundVolume = nVol;
  if (m_pMessageSound) {
    Mix_VolumeChunk(m_pMessageSound, scaleToMixer(nVol));
  }
}

void SoundModule::SetSoundSpace(int nSpace) { m_nSpace = nSpace; }

void SoundModule::SetOutput(int /*nOutput*/) {}
int  SoundModule::GetOutput() { return 0; }
void SoundModule::SetDriver(int /*nDriver*/) {}
int  SoundModule::GetNumDrivers() { return 0; }
char *SoundModule::GetDriverName(int /*nDriver*/) {
  // Legacy caller (Settings menu) expects a non-null return.
  static char empty[] = "";
  return empty;
}

// ---------------------------------------------------------------------------
// Looping sounds
// ---------------------------------------------------------------------------

void SoundModule::StartMenuMusic() {
  if (!m_bRunning) return;
  if (!m_pMenuMusic) {
    m_pMenuMusic = Mix_LoadMUS("Sounds/MixDown3.ogg");
    if (!m_pMenuMusic) {
      cout << "SoundModule: failed to load Sounds/MixDown3.ogg: "
           << Mix_GetError() << endl;
      return;
    }
  }
  Mix_VolumeMusic(scaleToMixer(m_nMenuMusicVolume));
  if (Mix_PlayingMusic() == 0) {
    Mix_PlayMusic(m_pMenuMusic, -1);  // loop forever
  } else if (Mix_PausedMusic() == 1) {
    Mix_ResumeMusic();
  }
}

void SoundModule::StopMenuMusic() {
  if (!m_bRunning) return;
  Mix_HaltMusic();
}

void SoundModule::StartSkidSound() { /* no skid asset shipped */ }
void SoundModule::StopSkidSound()  { /* no skid asset shipped */ }

void SoundModule::SetCrashSoundVolume(int nVol) {
  int nNewVol = int(double(nVol) * double(m_nVehicleSoundsVolume) / 255.0);
  if (nNewVol > 255) nNewVol = 255;
  if (m_pCrashSound) {
    Mix_VolumeChunk(m_pCrashSound, scaleToMixer(nNewVol));
  }
}

void SoundModule::StartEngineSound() {
  if (!m_bRunning) return;
  if (!m_pEngineSound) {
    m_pEngineSound = Mix_LoadWAV("Sounds/EngineJuliet2.ogg");
    if (!m_pEngineSound) {
      cout << "SoundModule: failed to load Sounds/EngineJuliet2.ogg: "
           << Mix_GetError() << endl;
      return;
    }
  }
  SetEngineSoundVolume(m_nEngineSoundVolume);
  if (Mix_Playing(CH_ENGINE) == 0) {
    Mix_PlayChannel(CH_ENGINE, m_pEngineSound, -1);  // loop
  }
}

void SoundModule::StopEngineSound() {
  if (!m_bRunning) return;
  Mix_HaltChannel(CH_ENGINE);
}

void SoundModule::StartMessageSound() {
  if (!m_bRunning) return;
  if (!m_pMessageSound) {
    m_pMessageSound = Mix_LoadWAV("Sounds/IncomingMessage.ogg");
    if (!m_pMessageSound) {
      // Asset not shipped - silently skip.
      return;
    }
  }
  SetMessageSoundVolume(m_nMessageSoundVolume);
  if (Mix_Playing(CH_MESSAGE) == 0) {
    Mix_PlayChannel(CH_MESSAGE, m_pMessageSound, -1);
  }
}

void SoundModule::StopMessageSound() {
  if (!m_bRunning) return;
  Mix_HaltChannel(CH_MESSAGE);
}

void SoundModule::PreCache43AASound() { /* legacy no-op */ }
void SoundModule::Start43AASound()    { /* asset not shipped */ }
void SoundModule::Stop43AASound()     { /* asset not shipped */ }

void SoundModule::StartHeliSound() {
  if (!m_bRunning) return;
  if (!m_pHeliSound) {
    m_pHeliSound = Mix_LoadWAV("Sounds/huey.ogg");
    if (!m_pHeliSound) {
      // Asset not shipped - silently skip.
      return;
    }
  }
  SetHeliSoundVolume(m_nHeliSoundVolume);
  if (Mix_Playing(CH_HELICOPTER) == 0) {
    Mix_PlayChannel(CH_HELICOPTER, m_pHeliSound, -1);
  }
}

void SoundModule::SetHeliSoundVolume(int nVol) {
  m_nHeliSoundVolume = nVol;
  int scaled = scaleToMixer(int(double(nVol) *
                                 double(m_nVehicleSoundsVolume) / 255.0));
  if (m_pHeliSound) {
    Mix_VolumeChunk(m_pHeliSound, scaled);
  }
}

void SoundModule::SetHeliSoundPhase(double dPhase, double dBladePower) {
  // Without pitch modulation we can only scale volume to suggest spool-up.
  SetHeliSoundVolume(int(dPhase * 255.0 * dBladePower));
}

void SoundModule::StopHeliSound() {
  if (!m_bRunning) return;
  Mix_HaltChannel(CH_HELICOPTER);
}

void SoundModule::StartJetSound() {
  if (!m_bRunning) return;
  if (!m_pJetSound) {
    m_pJetSound = Mix_LoadWAV("Sounds/JetMono.ogg");
    if (!m_pJetSound) {
      // Asset not shipped - silently skip.
      return;
    }
  }
  SetJetSoundVolume(m_nJetSoundVolume);
  if (Mix_Playing(CH_JET) == 0) {
    Mix_PlayChannel(CH_JET, m_pJetSound, -1);
  }
}

void SoundModule::SetJetSoundVolume(int nVol) {
  m_nJetSoundVolume = nVol;
  int scaled = scaleToMixer(int(double(nVol) *
                                 double(m_nVehicleSoundsVolume) / 255.0));
  if (m_pJetSound) {
    Mix_VolumeChunk(m_pJetSound, scaled);
  }
}

void SoundModule::SetJetSoundPhase(double /*dPhase*/) {
  // Without pitch modulation there's nothing meaningful to do here.
  // The jet sound simply plays at its current volume while the jet is on.
}

void SoundModule::StopJetSound() {
  if (!m_bRunning) return;
  Mix_HaltChannel(CH_JET);
}

// ---------------------------------------------------------------------------
// One-shot sounds
// ---------------------------------------------------------------------------

void SoundModule::PlayCrashSound(double dVolume) {
  if (!m_bRunning) return;
  // Throttle: same 20ms minimum gap as the original code.
  static unsigned clockPrev = 0;
  unsigned clockNow = SDL_GetTicks();
  if ((double(clockNow - clockPrev) / 1000.0) <= 0.02) return;
  if (dVolume <= 0.1) return;
  if (dVolume > 1.0) dVolume = 1.0;
  clockPrev = clockNow;

  if (!m_pCrashSound) {
    m_pCrashSound = Mix_LoadWAV("Sounds/CRASH.ogg");
    if (!m_pCrashSound) {
      cout << "SoundModule: failed to load Sounds/CRASH.ogg: "
           << Mix_GetError() << endl;
      return;
    }
  }
  SetCrashSoundVolume(55 + int(dVolume * 200.0));
  // Mix_PlayChannel with -1 picks any free non-reserved channel.
  Mix_PlayChannel(-1, m_pCrashSound, 0);
}

// ---------------------------------------------------------------------------
// Legacy 3D audio API - was already a no-op in the previous implementation.
// ---------------------------------------------------------------------------

void SoundModule::Update3DSounds(BVector &/*rvSouLoc*/, BVector &/*rvSouVel*/,
                                 BVector &/*rvLisLoc*/, BOrientation &/*roLis*/,
                                 BVector &/*rvLisVel*/) {
  // SDL_mixer has no built-in 3D positional audio. If needed, the
  // SDL_mixer effects API (Mix_RegisterEffect) could be used to apply
  // pan/attenuation based on relative position. The original Vera
  // implementation also left this stubbed out.
}
