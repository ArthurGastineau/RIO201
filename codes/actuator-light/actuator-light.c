#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"
#include "dev/leds.h"
#include <stdio.h>
#include <string.h>

/* États des LEDs */
static uint8_t led_green_state = 0;  // LED pour la pièce 1
static uint8_t led_yellow_state = 0; // LED pour la pièce 2

/*---------------------------------------------------------------------------*/
/* Gestionnaire pour les requêtes GET */
static void res_get_led_handler(void *request, void *response, uint8_t *buffer,
                                uint16_t preferred_size, int32_t *offset) {
  printf("[CoAP] Requête GET reçue pour les états des LEDs.\n");

  snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{\"led_green\": %d, \"led_yellow\": %d}",
           led_green_state, led_yellow_state);

  REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
  REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));

  printf("[CoAP] Réponse : {\"led_green\": %d, \"led_yellow\": %d}\n",
         led_green_state, led_yellow_state);
}

/*---------------------------------------------------------------------------*/
/* Gestionnaire pour les requêtes POST */
static void res_post_led_handler(void *request, void *response, uint8_t *buffer,
                                 uint16_t preferred_size, int32_t *offset) {
  const char *payload = NULL;

  REST.get_request_payload(request, (const uint8_t **)&payload);

  if (payload == NULL) {
    printf("[CoAP] Aucun payload dans la requête POST.\n");
    REST.set_response_status(response, REST.status.BAD_REQUEST);
    return;
  }

  printf("[CoAP] Requête POST reçue : %s\n", payload);

  if (strncmp(payload, "green_on", strlen("green_on")) == 0) {
    leds_on(LEDS_GREEN);
    led_green_state = 1;
    printf("[CoAP] LED verte activée (pièce 1).\n");
  } else if (strncmp(payload, "green_off", strlen("green_off")) == 0) {
    leds_off(LEDS_GREEN);
    led_green_state = 0;
    printf("[CoAP] LED verte désactivée (pièce 1).\n");
  } else if (strncmp(payload, "yellow_on", strlen("yellow_on")) == 0) {
    leds_on(LEDS_YELLOW);
    led_yellow_state = 1;
    printf("[CoAP] LED orange activée (pièce 2).\n");
  } else if (strncmp(payload, "yellow_off", strlen("yellow_off")) == 0) {
    leds_off(LEDS_YELLOW);
    led_yellow_state = 0;
    printf("[CoAP] LED orange désactivée (pièce 2).\n");
  } else {
    printf("[CoAP] Commande inconnue : %s\n", payload);
    REST.set_response_status(response, REST.status.BAD_REQUEST);
    return;
  }

  REST.set_response_status(response, REST.status.CHANGED);
}

/*---------------------------------------------------------------------------*/
/* Ressource CoAP pour les LEDs */
RESOURCE(res_led,
         "title=\"LED Actuator\";rt=\"Control\"",
         res_get_led_handler,
         res_post_led_handler, // Activation du gestionnaire POST
         NULL,
         NULL);

/*---------------------------------------------------------------------------*/
/* Processus principal */
PROCESS(actuator_light_process, "Actuator Light Process");
AUTOSTART_PROCESSES(&actuator_light_process);

PROCESS_THREAD(actuator_light_process, ev, data) {
  PROCESS_BEGIN();

  printf("[CoAP] Démarrage du serveur CoAP Actuator Light.\n");

  /* Initialisation du moteur REST */
  rest_init_engine();

  /* Activation de la ressource LED */
  rest_activate_resource(&res_led, "actuator/led");
  printf("[CoAP] Ressource LED activée à /actuator/led.\n");

  while (1) {
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}

