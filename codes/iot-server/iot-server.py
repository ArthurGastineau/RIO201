import asyncio
import time
import requests
from aiocoap import *

# Définir les seuils de luminosité
LIGHT_THRESHOLD = 200

# URIs des capteurs et de l'actionneur
COAP_LIGHT_SENSOR_URI = "coap://[2001:660:5307:3142::9567]:5683/sensors/light"
HTTP_LIGHT_SENSOR_URI = "http://[2001:660:5307:3142::9175]/sensors/light"
ACTUATOR_URI = "coap://[2001:660:5307:3142::9871]/actuator/led"
LED_STATE_URI = "coap://[2001:660:5307:3142::9871]/actuator/led"

async def get_light_value_coap(sensor_uri):
    """
    Récupérer la valeur de lumière via CoAP.
    """
    context = await Context.create_client_context()
    request = Message(code=GET, uri=sensor_uri)
    start_time = time.time()
    try:
        response = await context.request(request).response
        duration = time.time() - start_time
        print(f"[CoAP] Réponse reçue en {duration:.4f} secondes : {response.payload.decode('utf-8')}")
        light_value = parse_light_response(response.payload.decode('utf-8'))
        return light_value, duration
    except Exception as e:
        print(f"[CoAP] Erreur lors de la requête CoAP : {e}")
        return None, None

def get_light_value_http(sensor_uri):
    """
    Récupérer la valeur de lumière via HTTP.
    """
    start_time = time.time()
    try:
        print(f"[HTTP] Envoi de la requête GET à {sensor_uri}...")
        response = requests.get(sensor_uri, timeout=5)
        duration = time.time() - start_time
        print(f"[HTTP] Statut HTTP : {response.status_code}")
        print(f"[HTTP] Contenu brut de la réponse : {response.text.strip()}")
        if response.status_code == 200:
            light_value = parse_light_response(response.text)
            print(f"[HTTP] Réponse reçue en {duration:.4f} secondes : {response.text.strip()}")
            return light_value, duration
        else:
            print(f"[HTTP] Erreur HTTP : Code {response.status_code}")
            return None, None
    except requests.exceptions.RequestException as e:
        print(f"[HTTP] Erreur lors de la requête HTTP : {e}")
        return None, None

def parse_light_response(payload):
    """
    Extraire la valeur de lumière à partir du payload JSON.
    """
    try:
        payload = payload.strip("{} \n")  # Nettoyer les espaces et les sauts de ligne
        light_value = float(payload.split(":")[1])
        return light_value
    except Exception as e:
        print(f"Erreur de parsing : {e}")
        return None

async def control_light(light_value, actuator_uri, led_type):
    """
    Contrôler la LED en fonction de la valeur mesurée.
    """
    context = await Context.create_client_context()
    if light_value is not None:
        if light_value < LIGHT_THRESHOLD:
            payload = f"{led_type}_on"
            print(f"[Actuator] Activation de la LED {led_type}.")
        else:
            payload = f"{led_type}_off"
            print(f"[Actuator] Désactivation de la LED {led_type}.")

        request = Message(code=POST, uri=actuator_uri, payload=payload.encode('utf-8'))
        try:
            response = await context.request(request).response
            print(f"[Actuator] Réponse POST : {response.payload.decode('utf-8') if response.payload else '[Aucun contenu]'}")
        except Exception as e:
            print(f"[Actuator] Erreur lors de la commande POST : {e}")
        
        # Vérifier l'état des LEDs après l'envoi de la commande
        await get_led_state(LED_STATE_URI)

async def get_led_state(state_uri):
    """
    Récupérer l'état actuel des LEDs.
    """
    context = await Context.create_client_context()
    request = Message(code=GET, uri=state_uri)
    try:
        response = await context.request(request).response
        print(f"[Actuator] État actuel des LEDs : {response.payload.decode('utf-8')}")
    except Exception as e:
        print(f"[Actuator] Erreur lors de la requête GET pour l'état des LEDs : {e}")

async def main():
    """
    Boucle principale du serveur IoT.
    """
    while True:
        print("\n[IoT-Server] Démarrage d'un nouveau cycle de collecte...")
        
        # Pièce 1 : Mesurer la lumière via CoAP
        coap_light_value, coap_duration = await get_light_value_coap(COAP_LIGHT_SENSOR_URI)
        print(f"[Metrics] Durée CoAP : {coap_duration:.4f} secondes, Valeur : {coap_light_value}")

        # Pièce 2 : Mesurer la lumière via HTTP
        http_light_value, http_duration = get_light_value_http(HTTP_LIGHT_SENSOR_URI)
        print(f"[Metrics] Durée HTTP : {http_duration:.4f} secondes, Valeur : {http_light_value}")

        # Pièce 1 : Contrôler l'actionneur (LED verte)
        await control_light(coap_light_value, ACTUATOR_URI, "green")

        # Pièce 2 : Contrôler l'actionneur (LED orange)
        await control_light(http_light_value, ACTUATOR_URI, "yellow")

        # Pause avant le prochain cycle
        await asyncio.sleep(10)

if __name__ == "__main__":
    asyncio.run(main())

