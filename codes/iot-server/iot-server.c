#include "contiki.h"
#include "coap-engine.h"
#include "http-socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COAP_LIGHT_URI "coap://[CAPTEUR_LUMIERE_1_IPV6]/sensor/light"
#define HTTP_LIGHT_URI "http://[CAPTEUR_LUMIERE_2_IPV6]/sensor/light"
#define LED_CONTROL_URI "coap://[ACTIONNEUR_ECLAIRAGE_IPV6]/actuator/led"

// Définir les seuils de luminosité
#define LIGHT_THRESHOLD 200

PROCESS(iot_server_process, "IoT Server Process");
AUTOSTART_PROCESSES(&iot_server_process);

static int parse_light_response(const char *payload) {
  int light_value = 0;
  sscanf(payload, "{\"light\": %d}", &light_value);
  return light_value;
}

PROCESS_THREAD(iot_server_process, ev, data) {
  static coap_endpoint_t coap_light_ep;
  static coap_message_t coap_light_req[1];
  static struct http_socket http_light_socket;
  static char http_buffer[128];
  static int coap_light_value = 0;
  static int http_light_value = 0;
  static struct etimer timer;

  PROCESS_BEGIN();

  etimer_set(&timer, CLOCK_SECOND * 10); // Collecter toutes les 10 secondes

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);

    printf("Collecting light data...\n");

    // Récupération via CoAP (Pièce 1)
    coap_endpoint_parse(COAP_LIGHT_URI, strlen(COAP_LIGHT_URI), &coap_light_ep);
    coap_init_message(coap_light_req, COAP_TYPE_CON, COAP_GET, 0);
    coap_set_header_uri_path(coap_light_req, "/sensor/light");

    char coap_buffer[64];
    COAP_BLOCKING_REQUEST(&coap_light_ep, coap_light_req, {
      memcpy(coap_buffer, (char *)response->payload, response->payload_len);
      coap_buffer[response->payload_len] = '\0';
      coap_light_value = parse_light_response(coap_buffer);
      printf("CoAP light value: %d\n", coap_light_value);
    });

    // Récupération via HTTP (Pièce 2)
    memset(http_buffer, 0, sizeof(http_buffer));
    http_socket_get(&http_light_socket, HTTP_LIGHT_URI, 0, 0, NULL, NULL);
    PROCESS_WAIT_EVENT_UNTIL(ev == http_socket_event);
    strncpy(http_buffer, (char *)http_light_socket.output_data, http_light_socket.output_datalen);
    http_light_value = parse_light_response(http_buffer);
    printf("HTTP light value: %d\n", http_light_value);

    // Contrôle des LEDs
    printf("Sending LED control commands...\n");

    // Contrôle de la LED verte (Pièce 1)
    if(coap_light_value < LIGHT_THRESHOLD) {
      coap_endpoint_parse(LED_CONTROL_URI, strlen(LED_CONTROL_URI), &coap_light_ep);
      coap_init_message(coap_light_req, COAP_TYPE_CON, COAP_POST, 0);
      coap_set_header_uri_path(coap_light_req, "/actuator/led");
      const char *payload_green = "{\"color\": \"green\", \"state\": \"on\"}";
      coap_set_payload(coap_light_req, payload_green, strlen(payload_green));
      COAP_BLOCKING_REQUEST(&coap_light_ep, coap_light_req, NULL);
    } else {
      const char *payload_off = "{\"color\": \"green\", \"state\": \"off\"}";
      coap_set_payload(coap_light_req, payload_off, strlen(payload_off));
      COAP_BLOCKING_REQUEST(&coap_light_ep, coap_light_req, NULL);
    }

    // Contrôle de la LED orange (Pièce 2)
    if(http_light_value < LIGHT_THRESHOLD) {
      coap_endpoint_parse(LED_CONTROL_URI, strlen(LED_CONTROL_URI), &coap_light_ep);
      coap_init_message(coap_light_req, COAP_TYPE_CON, COAP_POST, 0);
      coap_set_header_uri_path(coap_light_req, "/actuator/led");
      const char *payload_orange = "{\"color\": \"orange\", \"state\": \"on\"}";
      coap_set_payload(coap_light_req, payload_orange, strlen(payload_orange));
      COAP_BLOCKING_REQUEST(&coap_light_ep, coap_light_req, NULL);
    } else {
      const char *payload_off = "{\"color\": \"orange\", \"state\": \"off\"}";
      coap_set_payload(coap_light_req, payload_off, strlen(payload_off));
      COAP_BLOCKING_REQUEST(&coap_light_ep, coap_light_req, NULL);
    }
  }

  PROCESS_END();
}
