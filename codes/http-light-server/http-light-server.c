#include "contiki.h"
#include "contiki-net.h"
#include "httpd-simple.h"
#include "dev/light-sensor.h"

#include <stdio.h>
#include <string.h>

/* Buffer pour les réponses HTTP */
static char buf[128];
static int blen;

/*---------------------------------------------------------------------------*/
/* Processus principal */
PROCESS(http_light_sensor_process, "HTTP Light Sensor Process");
AUTOSTART_PROCESSES(&http_light_sensor_process);

/*---------------------------------------------------------------------------*/
/* Configuration du capteur de lumière */
static void configure_light_sensor() {
  printf("[HTTP] Configuration du capteur de lumière...\n");

  light_sensor.configure(LIGHT_SENSOR_SOURCE, ISL29020_LIGHT__AMBIENT);
  light_sensor.configure(LIGHT_SENSOR_RESOLUTION, ISL29020_RESOLUTION__16bit);
  light_sensor.configure(LIGHT_SENSOR_RANGE, ISL29020_RANGE__1000lux);
  SENSORS_ACTIVATE(light_sensor);

  printf("[HTTP] Capteur de lumière configuré et activé.\n");
}

/*---------------------------------------------------------------------------*/
/* Génération de la réponse HTTP */
static
PT_THREAD(generate_routes(struct httpd_state *s)) {
  PSOCK_BEGIN(&s->sout);

  printf("[HTTP] Requête HTTP reçue sur /sensors/light\n");

  // Lecture du capteur de lumière
  int light_val = light_sensor.value(0);
  float light = ((float)light_val) / LIGHT_SENSOR_VALUE_SCALE;

  printf("[HTTP] Valeur de luminosité lue : %.2f lux\n", light);

  // Construction de la réponse JSON
  blen = snprintf(buf, sizeof(buf), "{\"light\": %.2f}\n", light);

  printf("[HTTP] Réponse générée : %s", buf);

  // Envoi de la réponse au client HTTP
  SEND_STRING(&s->sout, buf);
  printf("[HTTP] Réponse envoyée au client.\n");

  PSOCK_END(&s->sout);
}

/*---------------------------------------------------------------------------*/
/* Script HTTP */
httpd_simple_script_t httpd_simple_get_script(const char *name) {
  printf("[HTTP] Script demandé pour la ressource : %s\n", name);
  return generate_routes;
}

/*---------------------------------------------------------------------------*/
/* Processus principal */
PROCESS_THREAD(http_light_sensor_process, ev, data) {
  PROCESS_BEGIN();

  printf("[HTTP] Démarrage du serveur HTTP Light Sensor.\n");

  // Initialiser le capteur de lumière
  configure_light_sensor();

  // Démarrer le serveur HTTP
  httpd_init();
  printf("[HTTP] Serveur HTTP initialisé et prêt à recevoir des requêtes.\n");

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    printf("[HTTP] Événement TCP/IP reçu, traitement de la requête en cours...\n");
    httpd_appcall(data);
  }

  PROCESS_END();
}

