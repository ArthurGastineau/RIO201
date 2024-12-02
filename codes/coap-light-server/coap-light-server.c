#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "rest-engine.h"
#include "dev/light-sensor.h"

/*---------------------------------------------------------------------------*/
/* Ressource REST : capteur de lumière */
static void res_get_light_handler(void *request, void *response, uint8_t *buffer,
                                  uint16_t preferred_size, int32_t *offset) {
  int light_val = light_sensor.value(0); // Lecture brute du capteur
  float light = ((float)light_val) / LIGHT_SENSOR_VALUE_SCALE; // Conversion en lux
  snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{\"light\": %.2f}", light);

  printf("Requête reçue, luminosité : %.2f lux\n", light);

  REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
  REST.set_response_payload(response, buffer, strlen((char *)buffer));
}

RESOURCE(res_light_local, "title=\"Light Sensor\";rt=\"Sensor\"", res_get_light_handler, NULL, NULL, NULL);

/*---------------------------------------------------------------------------*/
/* Processus principal */
PROCESS(coap_light_server_process, "CoAP Light Server");
AUTOSTART_PROCESSES(&coap_light_server_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coap_light_server_process, ev, data) {
  PROCESS_BEGIN();

  printf("Démarrage du serveur CoAP Light\n");

  // Activation du capteur de lumière
  light_sensor.configure(LIGHT_SENSOR_SOURCE, ISL29020_LIGHT__AMBIENT);
  light_sensor.configure(LIGHT_SENSOR_RESOLUTION, ISL29020_RESOLUTION__16bit);
  light_sensor.configure(LIGHT_SENSOR_RANGE, ISL29020_RANGE__1000lux);
  SENSORS_ACTIVATE(light_sensor);

  // Initialisation du moteur REST
  rest_init_engine();

  // Activation de la ressource REST
  rest_activate_resource(&res_light_local, "sensors/light");

  while (1) {
    PROCESS_WAIT_EVENT();
    // Aucun événement spécifique géré dans ce processus principal
  }

  PROCESS_END();
}

