# RIO201 - Arthur Gastineau

## Introduction
L’objectif de ce projet est de concevoir une application IoT qui exploite les capacités de la plateforme FIT IoT-LAB pour fournir une automatisation intelligente dans un environnement de travail. L’application s’appuie sur des capteurs pour surveiller des paramètres environnementaux clés, comme la luminosité et la température, et utilise des actionneurs pour répondre automatiquement à des changements d’état. De plus, une alarme incendie est intégrée pour renforcer la sécurité des utilisateurs.

Le projet met également en avant une comparaison pratique entre deux protocoles IoT : CoAP (Constrained Application Protocol), conçu pour des environnements contraints, et HTTP, un protocole largement utilisé mais plus gourmand en ressources. Ces deux protocoles sont testés sur des fonctions similaires pour évaluer leurs performances.

## Architecture

L’application IoT est déployée dans un environnement de travail simulé, intégrant des capteurs pour collecter les données, un serveur IoT pour les décisions logiques (jouant le rôle de **client CoAP/HTTP**), et deux actionneurs séparés pour gérer les LEDs d’éclairage, de régulation de température et d’alarme incendie.

### 1. Capteurs

Les capteurs IoT jouent le rôle de **serveurs**, exposant leurs ressources via CoAP ou HTTP pour fournir des données au serveur IoT (client). Ils incluent :

1. **Capteurs de lumière (2 unités) :**
   - Chaque pièce est équipée d’un capteur ISL29020 pour mesurer la luminosité ambiante.
   - **Pièce 1 :** Expose une ressource CoAP accessible via `/sensor/light`.
   - **Pièce 2 :** Expose une ressource HTTP accessible via `/sensor/light`.

2. **Capteur de température (1 unité) :**
   - Un capteur LPS331AP surveille la température pour les deux pièces.
   - Expose une ressource CoAP accessible via `/sensor/temperature`.

### 2. Border Router

Un capteur IoT-LAB M3 est configuré comme **Border Router**, jouant le rôle de passerelle entre les capteurs (réseau 6LoWPAN) et le serveur IoT. Ses fonctions incluent :
- Relier le réseau IoT (6LoWPAN) et le réseau IPv6 principal.
- Établir un tunnel IPv6 (`tunslip6`) vers le serveur IoT.

### 3. Client IoT

Le serveur central (IoT-Server) agit comme un **client CoAP/HTTP**, responsable de :
- **Réception des données :**
  - Envoie des requêtes `GET` aux capteurs pour collecter les données de lumière et de température.
- **Prise de décisions :**
  - Compare les données reçues avec des seuils prédéfinis pour contrôler les LEDs des actionneurs.
- **Gestion des commandes :**
  - Envoie des requêtes `POST` aux actionneurs pour activer/désactiver les LEDs.
  - Reçoit les commandes SSH pour déclencher l’alarme incendie.

### 4. Actionneurs

#### **Actionneur 1 : Éclairage**
- **LED verte :** Éclairage pour la **pièce 1**.
- **LED orange :** Éclairage pour la **pièce 2**.

#### **Actionneur 2 : Régulation de température et alarme incendie**
- **LED rouge :** Chauffage pour les deux pièces (température basse).
- **LED verte :** Ventilation pour les deux pièces (température élevée).
- **LED orange :** Alarme incendie activée via SSH.

### Résumé de l'Architecture

| **Rôle**            | **Nombre** | **Description**                                                                 |
|----------------------|------------|---------------------------------------------------------------------------------|
| **Capteurs**         | 3          | 2 capteurs de lumière (1 CoAP et 1 HTTP), 1 capteur de température.             |
| **Border Router**     | 1          | Configure un tunnel IPv6 pour connecter les capteurs au serveur central.        |
| **Serveur IoT**       | 1          | Collecte les données des capteurs (client) et contrôle les deux actionneurs.    |
| **Actionneurs**       | 2          | 1 pour l’éclairage, 1 pour la température et l’alarme incendie.                 |

![Description de l'image](images/architecture.png)
