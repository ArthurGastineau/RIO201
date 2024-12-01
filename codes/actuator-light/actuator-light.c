#include "contiki.h"
#include "coap-engine.h"
#include "dev/leds.h"

static void post_led_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  const char *state = NULL;
  size_t len = coap_get_payload(request, (const uint8_t **)&state);
  if(len > 0 && strncmp(state, "{\"state\": \"on\"}", len) == 0) {
    leds_on(LEDS_GREEN);
  } else {
    leds_off(LEDS_GREEN);
  }
}

RESOURCE(led_actuator_resource, "title=\"LED Actuator\";rt=\"Control\"", NULL, post_led_handler, NULL, NULL);

PROCESS(led_actuator_process, "LED Actuator Process");
AUTOSTART_PROCESSES(&led_actuator_process);

PROCESS_THREAD(led_actuator_process, ev, data) {
  PROCESS_BEGIN();
  rest_init_engine();
  rest_activate_resource(&led_actuator_resource, "actuator/led");
  PROCESS_END();
}
