#ifndef MESSAGE_SOURCES
#define MESSAGE_SOURCES
#define SOURCE_FOREGROUND 0
#define SOURCE_BACKGROUND 1
#endif

#ifndef LOGGING_TAG
#define LOGGING_TAG 0xFEDE1983
#endif

#define NUM_SAMPLES 10

// Session logging
//static DataLoggingSessionRef s_session_ref;
//static bool paused = false;

static void worker_message_handler(uint16_t type, AppWorkerMessage *message);
static void toggle_pause();
static void send_init_message();
static void send_deinit_message();
static void send_pause_message();
static void send_resume_message();
static void send_resume_message();
static void accel_data_handler(AccelData *data, uint32_t num_samples);
static void prv_init();
static void prv_deinit();
