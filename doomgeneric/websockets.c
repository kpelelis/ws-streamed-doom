#include "doomgeneric.h"
#include "websockets.h"

#include <libwebsockets.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define FRAME_BUFFER_SIZE (DOOMGENERIC_RESY*DOOMGENERIC_RESX*4)

static unsigned char *ws_fb = NULL;

static int destroy_flag = 0;
static int connected_flag = 0;

struct lws_context *cx = NULL;
struct lws *ws = NULL;

static void log(char *msg) {
  time_t ltime;
  struct tm result;
  char stime[32];

  ltime = time(NULL);
  localtime_r(&ltime, &result);
  strftime(stime, sizeof(stime), "%d/%m/%Y %H:%M:%S", &result);
  printf("[%s][Grafana-Doom] %s\n", stime, msg);
}

// noop
static int
callback_http(
  struct lws *wsi,
  enum lws_callback_reasons reason,
  void *user,
  void *in,
  size_t len
) {
  switch(reason)
  {
    case LWS_CALLBACK_HTTP:
      break;
    default:
      break;
  }

  return 0;
}

struct payload
{
  unsigned char data[LWS_SEND_BUFFER_PRE_PADDING + 10 + LWS_SEND_BUFFER_POST_PADDING];
  size_t len;
} received_payload;

static int
gd_ws_callback(
  struct lws *wsi,
  enum lws_callback_reasons reason,
  void *user,
  void *in,
  size_t len
) {
  switch(reason)
  {
    case LWS_CALLBACK_RECEIVE:
      memcpy(&received_payload.data[LWS_SEND_BUFFER_PRE_PADDING], in, len);
      received_payload.len = len;
      lws_callback_on_writable_all_protocol(lws_get_context(wsi), lws_get_protocol(wsi));
      break;

    case LWS_CALLBACK_ESTABLISHED:
      ws = wsi;
      connected_flag = 1;
      break;

    case LWS_CALLBACK_CLOSED:
      ws = NULL;
      connected_flag = 0;
    default:
      break;
  }

  return 0;
}

enum protocols
{
  PROTOCOL_HTTP = 0,
  PROTOCOL_GRAFANA_DOOM,
  PROTOCOL_COUNT
};

static struct lws_protocols protocols[] =
{
  {
    "http-only",   /* name */
    callback_http, /* callback */
    0,             /* No per session data. */
    0,             /* max frame size / rx buffer */
  },
  {
    "grafana-doom",
    gd_ws_callback,
    0,
    FRAME_BUFFER_SIZE,
  }
};

static void *DWS_pthread_routine(void * data)
{
  while(!destroy_flag)
  {
    lws_service(cx, 5000);
  }

  lws_context_destroy(cx);
  return NULL;
}

int DWS_Init(void)
{
  log("Setting up web socket client");
  log("Allocating frame buffer");
  ws_fb = (unsigned char*) malloc(sizeof(unsigned char)*(LWS_SEND_BUFFER_PRE_PADDING + FRAME_BUFFER_SIZE + LWS_SEND_BUFFER_POST_PADDING));
  if (ws_fb == NULL) {
    log("Allocating frame buffer failed");
    exit(1);
  } 
  struct lws_context_creation_info info;
  memset(&info, 0, sizeof(info));

  info.port = 5000;
  info.protocols = protocols;
  info.gid = -1;
  info.uid = -1;

  log("Creating WebSocket context");
  cx = lws_create_context(&info);
  if (!cx) {
    log("Creating WebSocket context failed");
  }

  log("Creating WebSocket loop thread");
  pthread_t pid;
  pthread_create(&pid, NULL, DWS_pthread_routine, NULL);
  pthread_detach(pid);
  return 0;
}

int DWS_write_frame(uint32_t *frame) {
  if (frame == NULL || ws == NULL || !connected_flag) return -1;

  memcpy (ws_fb, frame, FRAME_BUFFER_SIZE);
  return lws_write(ws, ws_fb + LWS_PRE, FRAME_BUFFER_SIZE, LWS_WRITE_BINARY);
}
