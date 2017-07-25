#include <pebble_worker.h>

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
static DataLoggingSessionRef s_session_ref;
static bool paused = false;

static void accel_data_handler(AccelData *data, uint32_t num_samples) {
  uint32_t i;
	for (i=0; i < num_samples; i++) {
    //uint8_t *data_to_log = (uint8_t *) &data[i];
    if (data_logging_log(s_session_ref, (uint8_t *) &data[i], 1) != DATA_LOGGING_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "DATA LOGGING ERROR.");
    }
	}
}

static void send_init_message() {
  AppWorkerMessage message = {
    .data0 = 0xFFFF,
    .data1 = 0xFFFF,
    .data2 = 0xFFFF
  };
  app_worker_send_message(SOURCE_BACKGROUND, &message);
}

static void send_deinit_message() {
  AppWorkerMessage message = {
    .data0 = 0x0000,
    .data1 = 0x0000,
    .data2 = 0x0000
  };
  app_worker_send_message(SOURCE_BACKGROUND, &message);
}

static void send_pause_message() {
  AppWorkerMessage message = {
    .data0 = 0x0000,
    .data1 = 0x0000,
    .data2 = 0x0001
  };
  app_worker_send_message(SOURCE_BACKGROUND, &message);
}

static void send_resume_message() {
  AppWorkerMessage message = {
    .data0 = 0x0000,
    .data1 = 0x0000,
    .data2 = 0x0002
  };
  app_worker_send_message(SOURCE_BACKGROUND, &message);
}

static void toggle_pause() {
  if (paused == true) {
    accel_service_set_sampling_rate(ACCEL_SAMPLING_50HZ);
    accel_data_service_subscribe(NUM_SAMPLES, accel_data_handler);
    paused = false;
    send_resume_message();
  } else {
    accel_data_service_unsubscribe();
    paused = true;
    send_pause_message();
  }
}

static void worker_message_handler(uint16_t type, AppWorkerMessage *message) {
  if (type == SOURCE_FOREGROUND) {
    if (message->data0 == 0x0000 && message->data1 == 0x0000 && message->data2 == 0x0001) {
      // Message for Toggle pause
      toggle_pause();
    }
    if (message->data0 == 0x0000 && message->data1 == 0x0000 && message->data2 == 0x003F) {
      // Status request
      if (paused == true) {
        send_pause_message();
      } else {
        send_resume_message();
      }
    }
  }
}

static void prv_init() {
  // Initialize the worker here
  app_worker_message_subscribe(worker_message_handler);
  send_init_message();

  // Data logging
  // s_session_ref = data_logging_create(LOGGING_TAG, DATA_LOGGING_INT, sizeof(int16_t), true);
	s_session_ref = data_logging_create(LOGGING_TAG, DATA_LOGGING_BYTE_ARRAY, sizeof(AccelData), false);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_50HZ);
	// Subscribe to batched data events
	accel_data_service_subscribe(NUM_SAMPLES, accel_data_handler);
}

static void prv_deinit() {
  // Deinitialize the worker here
  send_deinit_message();
  app_worker_message_unsubscribe();

  // Unsuscribe accel data service
	accel_data_service_unsubscribe();

	// Finish data loggin
	data_logging_finish(s_session_ref);
}

int main(void) {
  prv_init();
  worker_event_loop();
  prv_deinit();
}
