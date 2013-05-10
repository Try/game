#ifdef __ANDROID__

#include <jni.h>
#include <cstring>

extern int main(int, char**);
extern "C"
JNIEXPORT void JNICALL Java_com_android_game_Tempest_androidMain(JNIEnv * env, jobject obj){
  int ac = 1;
  char* av[2] = {};

  av[0] = strdup("TempestApp");
  main(ac, av);
  }

#endif
