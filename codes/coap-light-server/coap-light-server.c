#include "contiki.h"
#include "coap-engine.h"
#include "dev/light-sensor.h"

static void get_light_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
  int light_value = light_sensor.value(0); // Lecture du capteur
  sprintf((char *)buffer, "{\"light\": %d}", light_value);
  coap_set_payload(response, buffer, strlen((char *)buffer));
}

RESOURCE(light_sensor_resource, "title=\"Light Sensor\";rt=\"light\"", get_light_handler, NULL, NULL, NULL);

PROCESS(coap_light_server_process, "CoAP Light Server Process");
AUTOSTART_PROCESSES(&coap_light_server_process);

PROCESS_THREAD(coap_light_server_process, ev, data) {
  PROCESS_BEGIN();
  rest_init_engine();
  rest_activate_resource(&light_sensor_resource, "sensor/light");
  PROCESS_END();
}
