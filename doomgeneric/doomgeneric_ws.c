#include "doomgeneric.h"
#include "websockets.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

uint32_t screen_buffer[DOOMGENERIC_RESX*DOOMGENERIC_RESY];

void DG_Init()
{
  DWS_Init();
}

void DG_DrawFrame()
{
  uint32_t i, pixel;

  memset(screen_buffer, 0, DOOMGENERIC_RESX*DOOMGENERIC_RESY);
  // For some reason DG_ScreenBuffer data frame is
  // A R G B
  // Since we are using RGB888 and we need alpha transparency to be 255,
  // we will just swap Red with Blue and set alpha to 255
  for (i = 0; i < DOOMGENERIC_RESX*DOOMGENERIC_RESY; ++i) {
    pixel = DG_ScreenBuffer[i];
    screen_buffer[i] = (
      (pixel & 0xff     ) << 16 |
      (pixel & 0xff00   )       |
      (pixel & 0xff0000 ) >> 16 |
      0xff000000
    );
  }
  DWS_write_frame(screen_buffer);
}

void DG_SleepMs(uint32_t ms)
{
  usleep (ms * 1000);
}

uint32_t DG_GetTicksMs()
{
  struct timeval  tp;
  struct timezone tzp;

  gettimeofday(&tp, &tzp);

  return (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
  return 0;
}

void DG_SetWindowTitle(const char * title)
{
}
