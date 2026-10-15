//
// SoundModule - audio playback via SDL_mixer.
//
// Replaces the previous hand-rolled mixer (SDL_OpenAudio callback + libvorbisfile
// decoder + custom sample-mixing code) with SDL_mixer, which handles all of
// that internally. The public API is preserved 1:1 so no gameplay code needs
// to change.
//
// Sounds are loaded lazily on first Start* call; Mix_LoadWAV supports OGG,
// MP3, WAV and FLAC out of the box. Looping sounds (engine, music, heli, jet)
// use dedicated channels so they can be stopped independently. One-shot
// sounds (crash) go through the shared channel pool.
//

#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>

using namespace std;

#include "BaseClasses.h"

class SoundModule {
  // Dedicated channels for looping sounds. SDL_mixer reserves them with
  // Mix_ReserveChannels so they won't be picked up by Mix_PlayChannel.
  enum Channel {
    CH_MENU_MUSIC = 0,
    CH_ENGINE,
    CH_HELICOPTER,
    CH_JET,
    CH_MESSAGE,
    CH_RESERVED_COUNT
  };

  static bool m_bRunning;

  // Looping sound chunks. Loaded lazily; stay resident for the rest of
  // the session.
  static Mix_Music *m_pMenuMusic;        // streamed via Mix_Music
  static Mix_Chunk *m_pEngineSound;      // RAM-loaded via Mix_Chunk
  static Mix_Chunk *m_pHeliSound;
  static Mix_Chunk *m_pJetSound;
  static Mix_Chunk *m_pMessageSound;
  static Mix_Chunk *m_pCrashSound;

  // Per-sound volume (0..MIX_MAX_VOLUME). The "vehicle sounds volume"
  // acts as a master multiplier for engine/heli/jet/skid/crash.
  static int m_nMenuMusicVolume;
  static int m_nVehicleSoundsVolume;
  static int m_nSkidSoundVolume;
  static int m_nEngineSoundVolume;
  static int m_nMessageSoundVolume;
  static int m_nHeliSoundVolume;
  static int m_nJetSoundVolume;
  static int m_nSpace;

public:
  SoundModule();

  // ----- Volume getters / setters (preserved API) -----
  static int  GetMenuMusicVolume()      { return m_nMenuMusicVolume; }
  static void SetMenuMusicVolume(int nVol);
  static int  GetVehicleSoundsVolume()  { return m_nVehicleSoundsVolume; }
  static void SetVehicleSoundsVolume(int nVol);
  static int  GetSkidSoundVolume()      { return m_nSkidSoundVolume; }
  static void SetSkidSoundVolume(int nVol);
  static int  GetEngineSoundVolume()    { return m_nEngineSoundVolume; }
  static void SetEngineSoundVolume(int nVol);
  static void SetEngineSoundRPM(int nRPM);  // no-op with SDL_mixer (kept for API)
  static int  GetMessageSoundVolume()   { return m_nMessageSoundVolume; }
  static void SetMessageSoundVolume(int nVol);
  static int  GetSoundSpace()           { return m_nSpace; }
  static void SetSoundSpace(int nSpace);

  // ----- Legacy stubs (preserved API, no longer used) -----
  static void SetOutput(int nOutput);
  static int  GetOutput();
  static void SetDriver(int nDriver);
  static int  GetNumDrivers();
  static char *GetDriverName(int nDriver);

  // ----- Looping sounds -----
  static void StartMenuMusic();
  static void StopMenuMusic();

  static void StartSkidSound();   // no skid asset in Sounds/ - kept as no-op
  static void StopSkidSound();
  static void SetCrashSoundVolume(int nVol);

  static void StartEngineSound();
  static void StopEngineSound();

  static void StartMessageSound();
  static void StopMessageSound();

  static void PreCache43AASound();      // legacy no-op
  static void Start43AASound();          // legacy no-op (asset not shipped)
  static void Stop43AASound();

  static void StartHeliSound();
  static void SetHeliSoundVolume(int nVol);
  static void SetHeliSoundPhase(double dPhase, double dBladePower);
  static void StopHeliSound();

  static void StartJetSound();
  static void SetJetSoundVolume(int nVol);
  static void SetJetSoundPhase(double dPhase);
  static void StopJetSound();

  // ----- One-shot sounds -----
  static void PlayCrashSound(double dVolume);

  // ----- Lifecycle -----
  static void Initialize();
  static void Close();

  // Legacy 3D audio API - was already a no-op in the previous implementation.
  static void Update3DSounds(BVector &rvSouLoc, BVector &rvSouVel,
                             BVector &rvLisLoc, BOrientation &roLis,
                             BVector &rvLisVel);
};
