#include "randomgen.h"

RandomGen::RandomGen()
  : m_z(Z0), m_w(W0) {
  }

unsigned RandomGen::operator ()() {
  m_z = ZM * (m_z & 0xFFFF) + (m_z >> 16);
  m_w = WM * (m_w & 0xFFFF) + (m_w >> 16);
  return (m_z << 16) + m_w;
  }

void RandomGen::reset() {
  m_z = Z0;
  m_w = W0;
  }
