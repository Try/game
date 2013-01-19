#include "sound.h"

#include <bass.h>

Sound::Sound() {
  channel = 0;
  }

Sound::~Sound() {
  if( channel ){
    BASS_SampleFree( channel );
    BASS_MusicFree( channel );
    BASS_StreamFree(channel);
    }
  }

void Sound::load( const std::string& f ) {
  unsigned long newchan = BASS_MusicLoad( FALSE, f.data(),
                                          0,0,
                                          BASS_MUSIC_RAMP|BASS_SAMPLE_3D,1);

  if( newchan==0 )
    newchan=BASS_SampleLoad( FALSE, f.data(),
                             0,0,1,
                             BASS_SAMPLE_3D|BASS_SAMPLE_MONO);


  if( newchan ){
    channel = newchan;

    BASS_SampleGetChannel(channel,FALSE);
    }

  if( newchan==0 ){
    newchan = BASS_StreamCreateFile(FALSE, f.data(), 0, 0, 0);
    channel = newchan;
    }
  }

void Sound::play() {
  BASS_ChannelPlay( channel, false );
  }


SoundDevice::SoundDevice( void* win ) {
  BASS_Init( -1,44100,BASS_DEVICE_3D, (HWND)win, NULL );
  BASS_Set3DFactors(1,1,1);

  }

SoundDevice::~SoundDevice() {
  BASS_Free();
  }
