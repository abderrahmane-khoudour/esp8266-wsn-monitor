# TP WSN — Deux ESP8266 : nœud capteur, sink et interface Web

## Objectif

Réaliser un réseau de capteurs sans fil minimal : un nœud mesure la température et l'humidité, les transmet par radio à un sink, et le sink les affiche sur une page Web consultable depuis un téléphone.

## Architecture

```
ESP8266 n° 1 + DHT11  ── ESP-NOW ──>  ESP8266 n° 2 (sink)  ── Wi-Fi / HTTP ──>  navigateur
   nœud capteur                        point d'accès + serveur Web
```

Le sink crée son propre réseau Wi-Fi : aucun routeur ni accès Internet n'est nécessaire.

## Matériel et câblage

- 2 cartes NodeMCU ESP8266, 1 capteur DHT11, 2 câbles USB.
- Le DHT11 est branché uniquement sur le nœud capteur :

| DHT11 | ESP8266 n° 1 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| DATA | D2 (GPIO4) |

## Environnement logiciel

- `arduino-cli` avec le cœur de carte ESP8266 (3.1.2).
- Bibliothèque « DHT sensor library » (Adafruit).
- Pilotes USB des cartes (CH340 et CP2102).

## Travail réalisé

### 1. Sink (`sink/sink.ino`)

- Mode `WIFI_AP_STA` : point d'accès `WSN-SINK` (mot de passe `WSN12345`, canal 1).
- Réception des paquets ESP-NOW (rôle `SLAVE`) et mémorisation de la dernière mesure.
- Serveur Web sur `http://192.168.4.1` :
  - `/` : page HTML de supervision ;
  - `/data` : dernière mesure au format JSON (température, humidité, compteur, âge).

### 2. Nœud capteur (`capteur/capteur.ino`)

- Lecture du DHT11 toutes les 5 secondes.
- Envoi par ESP-NOW (rôle `CONTROLLER`, canal 1) d'une structure de 16 octets : identifiant du nœud, température, humidité, compteur de paquets.
- Destination : l'adresse MAC station du sink, relevée dans son moniteur série (`DC:4F:22:3B:16:4A`).

### 3. Mise en service

1. Téléverser le sink et relever sa MAC dans le moniteur série (115200 bauds).
2. Inscrire cette MAC dans `sinkAddress` du programme capteur, puis le téléverser.
3. Connecter le téléphone au Wi-Fi `WSN-SINK` et ouvrir `http://192.168.4.1`.

## Résultats attendus

| Test | Manipulation | Résultat |
|---|---|---|
| Réception | Démarrer les deux cartes | Température et humidité affichées, état « Connecté » |
| Actualisation | Attendre plusieurs cycles | Page rafraîchie toutes les 2 s, compteur incrémenté toutes les 5 s |
| Arrêt du capteur | Éteindre le nœud capteur | État « Hors ligne » après 15 s |
| Erreur DHT11 | Débrancher le capteur | `Erreur lecture DHT11` dans le moniteur série, aucun envoi |

Côté moniteur série, le capteur affiche `T=... C, H=... %` puis `Envoi : SUCCES`, et le sink affiche `Noeud 1 | T=... C | H=... % | N=...` à chaque paquet reçu.

## Points clés à retenir

- **Nœud capteur / sink** : le premier produit les mesures, le second les collecte et les rend accessibles.
- **`WIFI_AP_STA`** : le sink doit à la fois servir de point d'accès pour le téléphone et garder son interface station, dont la MAC est la destination des paquets ESP-NOW.
- **ESP-NOW / HTTP** : ESP-NOW transporte les mesures entre les cartes (sans connexion, adressage par MAC, très léger) ; HTTP présente les données à l'utilisateur (requête/réponse au-dessus de TCP/IP).
- **Canal radio** : les deux cartes doivent être sur le même canal (1).
- **Compteur** : un saut dans la numérotation révèle des paquets perdus ; un retour à 1 révèle un redémarrage du capteur.

Les réponses détaillées aux questions pédagogiques sont dans `REPONSES.md`.
