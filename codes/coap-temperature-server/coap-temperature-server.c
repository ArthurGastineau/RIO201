#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "rest-engine.h"
#include "dev/temperature-sensor.h"

/*---------------------------------------------------------------------------*/
/* Ressource REST : capteur de température */
static void res_get_temperature_handler(void *request, void *response, uint8_t *buffer,
                                        uint16_t preferred_size, int32_t *offset) {
  int raw_temp = temperature_sensor.value(0); // Lecture brute du capteur
  float temperature = ((float)raw_temp) / TEMPERATURE_SENSOR_SCALE; // Conversion en Celsius
  snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{\"temperature\": %.2f}", temperature);

  printf("Requête reçue, température : %.2f °C\n", temperature);

  REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
  REST.set_response_payload(response, buffer, strlen((char *)buffer));
}

RESOURCE(res_temperature, "title=\"Temperature Sensor\";rt=\"Sensor\"", res_get_temperature_handler, NULL, NULL, NULL);

/*---------------------------------------------------------------------------*/
/* Processus principal */
PROCESS(coap_temperature_server_process, "CoAP Temperature Server");
AUTOSTART_PROCESSES(&coap_temperature_server_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coap_temperature_server_process, ev, data) {
  PROCESS_BEGIN();

  printf("Démarrage du serveur CoAP Temperature\n");

  // Activation du capteur de température
  SENSORS_ACTIVATE(temperature_sensor);

  // Initialisation du moteur REST
  rest_init_engine();

  // Activation de la ressource REST
  rest_activate_resource(&res_temperature, "sensors/temperature");

  while (1) {
    PROCESS_WAIT_EVENT();
    // Aucun événement spécifique géré dans ce processus principal
  }

  PROCESS_END();
}
