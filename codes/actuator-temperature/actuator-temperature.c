#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "rest-engine.h"
#include "dev/leds.h"

/* États des LEDs */
static uint8_t led_green_state = 0;  // Ventilation
static uint8_t led_red_state = 0;    // Chauffage
static uint8_t led_yellow_state = 0; // Alarme incendie

/*---------------------------------------------------------------------------*/
/* Gestionnaire pour les requêtes GET (état des LEDs) */
static void res_get_led_handler(void *request, void *response, uint8_t *buffer,
                                uint16_t preferred_size, int32_t *offset) {
  snprintf((char *)buffer, REST_MAX_CHUNK_SIZE,
           "{\"ventilation\": %d, \"heating\": %d, \"alarm\": %d}",
           led_green_state, led_red_state, led_yellow_state);

  REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
  REST.set_response_payload(response, buffer, strlen((char *)buffer));

  printf("[CoAP] État des LEDs demandé : Ventilation=%d, Chauffage=%d, Alarme=%d\n",
         led_green_state, led_red_state, led_yellow_state);
}

/*---------------------------------------------------------------------------*/
/* Gestionnaire pour les requêtes POST (contrôle des LEDs) */
static void res_post_led_handler(void *request, void *response, uint8_t *buffer,
                                 uint16_t preferred_size, int32_t *offset) {
  const char *payload = NULL;
  REST.get_request_payload(request, (const uint8_t **)&payload);

  if (payload == NULL) {
    printf("[CoAP] Aucun payload dans la requête POST.\n");
    REST.set_response_status(response, REST.status.BAD_REQUEST);
    return;
  }

  printf("[CoAP] Commande reçue : %s\n", payload);

  if (strncmp(payload, "ventilation_on", strlen("ventilation_on")) == 0) {
    leds_on(LEDS_GREEN);
    led_green_state = 1;
    printf("[CoAP] Ventilation activée.\n");
  } else if (strncmp(payload, "ventilation_off", strlen("ventilation_off")) == 0) {
    leds_off(LEDS_GREEN);
    led_green_state = 0;
    printf("[CoAP] Ventilation désactivée.\n");
  } else if (strncmp(payload, "heating_on", strlen("heating_on")) == 0) {
    leds_on(LEDS_RED);
    led_red_state = 1;
    printf("[CoAP] Chauffage activé.\n");
  } else if (strncmp(payload, "heating_off", strlen("heating_off")) == 0) {
    leds_off(LEDS_RED);
    led_red_state = 0;
    printf("[CoAP] Chauffage désactivé.\n");
  } else if (strncmp(payload, "alarm_on", strlen("alarm_on")) == 0) {
    leds_on(LEDS_YELLOW);
    led_yellow_state = 1;
    printf("[CoAP] Alarme incendie activée.\n");
  } else if (strncmp(payload, "alarm_off", strlen("alarm_off")) == 0) {
    leds_off(LEDS_YELLOW);
    led_yellow_state = 0;
    printf("[CoAP] Alarme incendie désactivée.\n");
  } else {
    printf("[CoAP] Commande inconnue : %s\n", payload);
    REST.set_response_status(response, REST.status.BAD_REQUEST);
    return;
  }

  REST.set_response_status(response, REST.status.CHANGED);
}

/*---------------------------------------------------------------------------*/
/* Ressource REST pour les LEDs */
RESOURCE(res_led,
         "title=\"Temperature Actuator\";rt=\"Control\"",
         res_get_led_handler,
         res_post_led_handler,
         NULL,
         NULL);

/*---------------------------------------------------------------------------*/
/* Processus principal */
PROCESS(coap_temperature_actuator_process, "CoAP Temperature Actuator");
AUTOSTART_PROCESSES(&coap_temperature_actuator_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coap_temperature_actuator_process, ev, data) {
  PROCESS_BEGIN();

  printf("[CoAP] Démarrage du serveur CoAP Temperature Actuator\n");

  // Initialisation du moteur REST
  rest_init_engine();

  // Activation de la ressource REST
  rest_activate_resource(&res_led, "actuator/temperature");

  while (1) {
    PROCESS_WAIT_EVENT();
    // Aucun événement spécifique géré dans ce processus principal
  }

  PROCESS_END();
}
