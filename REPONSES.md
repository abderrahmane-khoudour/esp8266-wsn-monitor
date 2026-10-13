# TP WSN — Réponses aux questions pédagogiques

## 1. Quelle est la différence entre le nœud capteur et le sink ?

- **Nœud capteur (ESP8266 n° 1)** : il est à la source des données. Il lit le DHT11 toutes les 5 secondes, range la mesure dans une structure `SensorData` (identifiant du nœud, température, humidité, compteur) et l'émet par radio. Il ne stocke rien et n'offre aucune interface utilisateur. Dans ESP-NOW il joue le rôle `CONTROLLER` (émetteur).
- **Sink (ESP8266 n° 2)** : c'est le point de collecte du réseau, aussi appelé puits ou station de base. Il n'a pas de capteur : il reçoit les paquets (rôle `SLAVE`), garde la dernière mesure en mémoire et sert de passerelle vers l'utilisateur grâce à son point d'accès Wi-Fi et son serveur Web.

En résumé : le nœud capteur **produit** les données, le sink les **collecte et les rend accessibles**. Dans un vrai WSN il y a beaucoup de nœuds capteurs (souvent sur batterie) pour un seul sink (souvent alimenté en continu).

## 2. Pourquoi le sink utilise-t-il le mode WIFI_AP_STA ?

Le sink doit faire deux choses en même temps :

- **AP (point d'accès)** : créer le réseau `WSN-SINK` pour que le téléphone ou l'ordinateur s'y connecte et ouvre `http://192.168.4.1`.
- **STA (station)** : garder active l'interface station, dont l'adresse MAC est celle que le capteur utilise comme destination (`sinkAddress`). C'est `WiFi.macAddress()` qui l'affiche.

L'ESP8266 a deux adresses MAC différentes, une par interface. En mode `WIFI_AP` seul, l'interface station serait éteinte et les paquets ESP-NOW envoyés à la MAC station ne seraient pas reçus. Le mode `WIFI_AP_STA` active les deux interfaces.

Comme il n'y a qu'une seule radio, les deux interfaces partagent le même canal : c'est pourquoi le point d'accès est créé sur le canal 1 et que le capteur est lui aussi réglé sur le canal 1.

## 3. Quelle est la différence entre ESP-NOW et HTTP dans ce système ?

| | ESP-NOW | HTTP |
|---|---|---|
| Liaison | capteur → sink | navigateur ↔ sink |
| Niveau | couche liaison (trames Wi-Fi, adressage par MAC) | couche application, au-dessus de TCP/IP |
| Connexion | sans connexion, pas d'association, pas d'adresse IP | connexion au point d'accès, adresse IP par DHCP, session TCP |
| Mode | le capteur **pousse** un paquet quand il a une mesure | le navigateur **demande** (`GET /data`) toutes les 2 s, le sink répond |
| Format | structure binaire de 16 octets | texte : page HTML et JSON |
| Taille maximale | 250 octets par paquet | pas de limite pratique |
| Coût | très faible : rapide, peu d'énergie | plus lourd, mais compris par n'importe quel navigateur |

ESP-NOW sert au **transport des mesures à l'intérieur du réseau de capteurs** (léger et économe), HTTP sert à la **présentation des données à l'utilisateur** (standard et universel). Le sink fait la passerelle entre les deux.

## 4. Comment le compteur peut-il aider à détecter des pertes de paquets ?

Le capteur incrémente `counter` de 1 à chaque envoi. Le sink doit donc recevoir une suite continue : 1, 2, 3, 4…

- Si le sink reçoit le paquet 8 juste après le paquet 5, les paquets 6 et 7 ont été perdus. Le nombre de paquets perdus est `reçu − précédent − 1`.
- En cumulant ces écarts, on calcule un taux de perte : `perdus / (dernier compteur reçu)`, ce qui mesure la qualité de la liaison radio (distance, obstacles, interférences).
- Si le compteur **repart à 1**, le capteur a redémarré (coupure d'alimentation, reset).
- Si le même compteur arrive deux fois, c'est un doublon.

À noter : dans le programme du TP, le compteur n'augmente que lorsque la lecture du DHT11 réussit. Une erreur de lecture ne crée donc pas de trou dans la numérotation ; elle se voit par l'âge de la dernière réception qui augmente.

Exemple de code à ajouter dans `onDataRecv` du sink :

```cpp
static uint32_t lastCounter = 0;
static uint32_t lostPackets = 0;
if (lastCounter != 0 && msg.counter > lastCounter + 1) {
  lostPackets += msg.counter - lastCounter - 1;
}
lastCounter = msg.counter;
```

## 5. Comment ajouter un graphique historique ou des seuils d'alerte ?

### Graphique historique

- **Côté sink** : garder les N dernières mesures dans un tableau circulaire (par exemple 60 valeurs, soit 5 minutes à une mesure toutes les 5 s) et les publier sur une nouvelle route `/history` au format JSON.
- **Côté page Web** : tracer la courbe dans un élément `<canvas>` en JavaScript pur. Il ne faut pas compter sur une bibliothèque en ligne comme Chart.js, car le téléphone connecté à `WSN-SINK` n'a pas accès à Internet (ou alors il faut stocker la bibliothèque dans la mémoire flash du sink avec LittleFS).
- Variante plus simple : laisser le navigateur accumuler lui-même les valeurs reçues de `/data` dans un tableau JavaScript. L'historique est alors perdu à chaque rechargement de la page.
- L'historique en RAM est perdu au redémarrage du sink. Pour le conserver, il faut l'écrire en flash (LittleFS) ou sur une carte SD, et ajouter une horloge (NTP ou module RTC) pour dater les mesures.

### Seuils d'alerte

- Définir des seuils, par exemple `TEMP_MAX = 30.0` et `HUM_MAX = 70.0`.
- **Sur le sink** : comparer chaque mesure reçue aux seuils, ajouter un champ `"alert"` dans le JSON et, si besoin, commander une LED ou un buzzer sur une broche GPIO pour une alerte locale.
- **Sur la page** : afficher la valeur en rouge et un message d'avertissement lorsque le seuil est dépassé.
- Pour rendre les seuils réglables, ajouter un formulaire dans la page et une route (par exemple `/config?tmax=30`) qui met à jour les seuils, enregistrés en EEPROM ou LittleFS.
- Prévoir une hystérésis (par exemple alerte à 30 °C, fin d'alerte à 29 °C) pour éviter que l'alerte clignote quand la mesure oscille autour du seuil.
