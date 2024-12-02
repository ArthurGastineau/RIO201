import asyncio
import time
import random
import requests
from aiocoap import *

# Définir les seuils pour les capteurs
LIGHT_THRESHOLD = 200
TEMPERATURE_THRESHOLD_HIGH = 30  # Température nécessitant une ventilation
TEMPERATURE_THRESHOLD_LOW = 15  # Température nécessitant un chauffage

# URIs des capteurs
COAP_LIGHT_SENSOR_URI = "coap://[2001:660:5307:3142::9567]:5683/sensors/light"
HTTP_LIGHT_SENSOR_URI = "http://[2001:660:5307:3142::9175]/sensors/light"
TEMPERATURE_SENSOR_URI = "coap://[2001:660:5307:3142::b369]:5683/sensors/temperature"

# URIs des actionneurs
LIGHT_ACTUATOR_URI = "coap://[2001:660:5307:3142::9871]/actuator/led"
TEMP_ACTUATOR_URI = "coap://[2001:660:5307:3142::b582]/actuator/led"

# État de l'alarme incendie
alarm_state = False

async def get_value_coap(sensor_uri):
    """Récupérer une valeur de capteur via CoAP."""
    context = await Context.create_client_context()
    request = Message(code=GET, uri=sensor_uri)
    start_time = time.time()
    try:
        response = await context.request(request).response
        duration = time.time() - start_time
        print(f"[CoAP] Réponse reçue en {duration:.4f} secondes : {response.payload.decode('utf-8')}")
        value = parse_sensor_response(response.payload.decode('utf-8'))
        return value, duration
    except Exception as e:
        print(f"[CoAP] Erreur lors de la requête CoAP : {e}")
        return None, None

def get_value_http(sensor_uri):
    """Récupérer une valeur de capteur via HTTP."""
    start_time = time.time()
    try:
        response = requests.get(sensor_uri, timeout=5)
        duration = time.time() - start_time
        if response.status_code == 200:
            print(f"[HTTP] Réponse reçue en {duration:.4f} secondes : {response.text.strip()}")
            value = parse_sensor_response(response.text)
            return value, duration
        else:
            print(f"[HTTP] Erreur HTTP : Code {response.status_code}")
            return None, None
    except requests.exceptions.RequestException as e:
        print(f"[HTTP] Erreur lors de la requête HTTP : {e}")
        return None, None

def parse_sensor_response(payload):
    """Extraire la valeur d'un capteur à partir d'une réponse JSON."""
    try:
        payload = payload.strip("{} \n")  # Nettoyer les espaces et les sauts de ligne
        value = float(payload.split(":")[1])
        return value
    except Exception as e:
        print(f"Erreur de parsing : {e}")
        return None

async def control_light(value, actuator_uri, led_type):
    """Contrôler une LED via CoAP en fonction d'une valeur."""
    context = await Context.create_client_context()
    if value is not None:
        if value < LIGHT_THRESHOLD:
            payload = f"{led_type}_on"
            print(f"[Light Actuator] Activation de la LED {led_type}.")
        else:
            payload = f"{led_type}_off"
            print(f"[Light Actuator] Désactivation de la LED {led_type}.")

        request = Message(code=POST, uri=actuator_uri, payload=payload.encode('utf-8'))
        try:
            response = await context.request(request).response
            print(f"[Light Actuator] Réponse POST : {response.payload.decode('utf-8') if response.payload else '[Aucun contenu]'}")
        except Exception as e:
            print(f"[Light Actuator] Erreur lors de la commande POST : {e}")

async def control_temperature_leds(value, actuator_uri):
    """Contrôler les LEDs de température via CoAP."""
    context = await Context.create_client_context()
    if value is not None:
        # Ventilation (LED verte)
        if value > TEMPERATURE_THRESHOLD_HIGH:
            payload = "ventilation_on"
            print("[Temperature Actuator] Activation de la ventilation (LED verte).")
        else:
            payload = "ventilation_off"
            print("[Temperature Actuator] Désactivation de la ventilation (LED verte).")
        request = Message(code=POST, uri=actuator_uri, payload=payload.encode('utf-8'))
        try:
            await context.request(request).response
        except Exception as e:
            print(f"[Temperature Actuator] Erreur lors de la commande POST : {e}")

        # Chauffage (LED rouge)
        if value < TEMPERATURE_THRESHOLD_LOW:
            payload = "heating_on"
            print("[Temperature Actuator] Activation du chauffage (LED rouge).")
        else:
            payload = "heating_off"
            print("[Temperature Actuator] Désactivation du chauffage (LED rouge).")
        request = Message(code=POST, uri=actuator_uri, payload=payload.encode('utf-8'))
        try:
            await context.request(request).response
        except Exception as e:
            print(f"[Temperature Actuator] Erreur lors de la commande POST : {e}")

async def control_alarm(alarm_active, actuator_uri):
    """Contrôler la LED d'alarme incendie via CoAP."""
    context = await Context.create_client_context()
    payload = "alarm_on" if alarm_active else "alarm_off"
    action = "Activation" if alarm_active else "Désactivation"
    print(f"[Temperature Actuator] {action} de l'alarme incendie (LED orange).")
    request = Message(code=POST, uri=actuator_uri, payload=payload.encode('utf-8'))
    try:
        response = await context.request(request).response
        print(f"[Temperature Actuator] Réponse POST : {response.payload.decode('utf-8') if response.payload else '[Aucun contenu]'}")
    except Exception as e:
        print(f"[Temperature Actuator] Erreur lors de la commande POST : {e}")

async def main():
    global alarm_state

    while True:
        print("\n[IoT-Server] Démarrage d'un nouveau cycle de collecte...")
        
        # Pièce 1 : Mesurer la lumière via CoAP
        coap_light_value, coap_duration = await get_value_coap(COAP_LIGHT_SENSOR_URI)
        print(f"[Metrics] Durée CoAP : {coap_duration:.4f} secondes, Valeur : {coap_light_value}")

        # Pièce 2 : Mesurer la lumière via HTTP
        http_light_value, http_duration = get_value_http(HTTP_LIGHT_SENSOR_URI)
        print(f"[Metrics] Durée HTTP : {http_duration:.4f} secondes, Valeur : {http_light_value}")

        # Température : Mesurer via CoAP
        temperature_value, temp_duration = await get_value_coap(TEMPERATURE_SENSOR_URI)
        print(f"[Metrics] Température : {temp_duration:.4f} secondes, Valeur : {temperature_value}")

        # Pièce 1 : Contrôler l'actionneur (LED verte)
        await control_light(coap_light_value, LIGHT_ACTUATOR_URI, "green")

        # Pièce 2 : Contrôler l'actionneur (LED orange)
        await control_light(http_light_value, LIGHT_ACTUATOR_URI, "yellow")

        # Contrôler les LEDs pour la température
        await control_temperature_leds(temperature_value, TEMP_ACTUATOR_URI)

        # Alarme incendie : Probabilité de 10 % d'activation
        alarm_state = random.random() < 0.1
        await control_alarm(alarm_state, TEMP_ACTUATOR_URI)

        # Pause avant le prochain cycle
        await asyncio.sleep(10)

if __name__ == "__main__":
    asyncio.run(main())
