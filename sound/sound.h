#ifndef SOUND_H
#define SOUND_H

#include <string>

class Sound {
  public:
    Sound();
    Sound( const Sound& other ) = delete;
    ~Sound();

    Sound& operator = ( const Sound& other ) = delete;

    void load( const std::string &f );

    void play();
  private:
    unsigned long channel;
  };

class SoundDevice{
  public:
    SoundDevice(void *win);
    SoundDevice( const SoundDevice& other ) = delete;
    ~SoundDevice();

    SoundDevice& operator = ( const Sound& other ) = delete;
  private:

  };

#endif // SOUND_H
