#ifndef RANDOMGEN_H
#define RANDOMGEN_H

class RandomGen {
    enum
    {
        Z0 = 362436069, W0 = 521288629,
        ZM = 36969, WM = 18000
    };

    unsigned long m_z;
    unsigned long m_w;

  public:
    RandomGen();

    unsigned operator()();
    void reset();
  };

#endif // RANDOMGEN_H
