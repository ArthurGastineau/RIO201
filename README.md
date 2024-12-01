# RIO201 - Arthur Gastineau

## Introduction
L’objectif de ce projet est de concevoir une application IoT qui exploite les capacités de la plateforme FIT IoT-LAB pour fournir une automatisation intelligente dans un environnement de travail. L’application s’appuie sur des capteurs pour surveiller des paramètres environnementaux clés, comme la luminosité et la température, et utilise des actionneurs pour répondre automatiquement à des changements d’état. De plus, une alarme incendie est intégrée pour renforcer la sécurité des utilisateurs.

Le projet met également en avant une comparaison pratique entre deux protocoles IoT : CoAP (Constrained Application Protocol), conçu pour des environnements contraints, et HTTP, un protocole largement utilisé mais plus gourmand en ressources. Ces deux protocoles sont testés sur des fonctions similaires pour évaluer leurs performances.

## Architecture

L’application IoT est déployée dans un environnement de travail simulé, intégrant des capteurs pour collecter les données, un serveur IoT pour les décisions logiques, et deux actionneurs séparés pour gérer les LEDs d’éclairage, de régulation de température et d’alarme incendie.

### 1. Clients (Capteurs)

Les capteurs IoT jouent le rôle de **clients**, collectant des données environnementales et envoyant les informations au serveur IoT pour traitement. Ils incluent :

1. **Capteurs de lumière (2 unités) :**
   - Chaque pièce est équipée d’un capteur ISL29020 pour mesurer la luminosité ambiante.
   - **Pièce 1 :** Données envoyées au serveur via **CoAP**.
   - **Pièce 2 :** Données envoyées au serveur via **HTTP**.

2. **Capteur de température (1 unité) :**
   - Un capteur LPS331AP surveille la température pour les deux pièces.
   - Données transmises au serveur central via **CoAP**.

### 2. Border Router

Un capteur IoT-LAB M3 est configuré comme **Border Router**, jouant le rôle de passerelle entre les capteurs et le serveur IoT. Ses fonctions incluent :
- Relier le réseau IoT (6LoWPAN) et le réseau IPv6 principal.
- Établir un tunnel IPv6 (`tunslip6`) vers le serveur IoT.

### 3. Serveur IoT

Le serveur central est responsable de :
- **Réception des données :**
  - Collecte des données des capteurs de lumière et de température.
- **Prise de décisions :**
  - Compare les données reçues avec des seuils prédéfinis pour contrôler les LEDs.
- **Gestion des commandes :**
  - Reçoit les commandes SSH pour activer l’alarme incendie et contrôle les LEDs en conséquence.

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
| **Clients (Capteurs)**| 3          | 2 capteurs de lumière, 1 capteur de température.                                |
| **Border Router**     | 1          | Configure un tunnel IPv6 pour connecter les capteurs au serveur central.        |
| **Serveur IoT**       | 1          | Collecte les données des capteurs et contrôle les deux actionneurs.             |
| **Actionneurs**       | 2          | 1 pour l’éclairage, 1 pour la température et l’alarme incendie.                 |
