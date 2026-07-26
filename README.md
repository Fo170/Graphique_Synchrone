# Graphique_Synchrone

Bibliothèque **header-only** pour Arduino (ESP8266 / ESP32) qui génère du HTML/JS Google Charts afin d'afficher des graphiques temps réel sur une page web embarquée.

Auteur : Olivier FOURNET &nbsp;|&nbsp; Licence : GPL-3.0

---

## Fonctionnalités

- **jusqu'à 4 courbes** simultanées, nombre d'échantillons configurable
- **double axe Y** : chaque courbe peut être affectée à l'axe gauche ou droit
- **deux APIs de rendu** : `getPageWeb()` (renvoie une `String`, consomme beaucoup de RAM) ou `streamPageWeb(Print&)` (zéro-copie, recommandée sur les petits microcontrôleurs)
- **export JSON et CSV** : `toJSON()`, `toCSV()`
- **deux modes de temps** : `TEMPS_RELATIF` (secondes) ou `TEMPS_HHMM` (horloge HH:MM)
- **dimensions dynamiques** : largeur/hauteur du graphique modifiables à chaud
- **couleurs, légendes, titres d'axes** entièrement paramétrables
- **aucune dépendance externe** : utilise uniquement le core Arduino / ESP8266 / ESP32

---

## Installation

### PlatformIO

Ajoutez cette ligne dans la section `[env:xxx]` de votre `platformio.ini` :

```ini
lib_deps = https://github.com/Fo170/Graphique_Synchrone.git@^1.0.0
```

### Arduino IDE

Téléchargez le fichier `Graphique_Synchrone.h` et placez-le dans le dossier de votre sketch (à côté du `.ino`), ou dans le dossier `libraries/` de votre installation Arduino.

---

## Utilisation rapide

```cpp
#include <Arduino.h>
#include <Graphique_Synchrone.h>

Graphique g(2, 60);   // 2 courbes, 60 points d'historique

void setup() {
    Serial.begin(115200);

    g.setDimensions(1200, 600);
    g.setTitre("Ma Station");
    g.setModeTemps(Graphique::TEMPS_RELATIF);

    g.setLegende(Graphique::COURBE_1, "Temperature (°C)");
    g.setCouleur(Graphique::COURBE_1, Graphique::CouleurRouge());
    g.setAxeY(Graphique::COURBE_1, Graphique::AXE_GAUCHE);

    g.setLegende(Graphique::COURBE_2, "Humidite (%)");
    g.setCouleur(Graphique::COURBE_2, Graphique::CouleurBleu());
    g.setAxeY(Graphique::COURBE_2, Graphique::AXE_DROITE);
}

void loop() {
    // Lecture capteurs...
    float temperature = 22.5;
    float humidite    = 60.0;

    // Mise à jour du graphique
    g.decaler();                              // décale l'historique
    g.setTime(millis() / 1000.0);             // timestamp en secondes
    g.addValue(Graphique::COURBE_1, temperature);
    g.addValue(Graphique::COURBE_2, humidite);
    g.incrementSample();

    // Génération de la page web
    // Méthode 1 : String (consomme beaucoup de RAM)
    String page = g.getPageWeb();

    // Méthode 2 : streaming (zéro-copie, recommandé)
    // client.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"));
    // g.streamPageWeb(client);

    delay(1000);
}
```

---

## Référence de l'API

### Constructeur et configuration générale

| Méthode | Description |
|---------|-------------|
| `Graphique(nb_courbes, nb_points)` | Construit un graphique (max 4 courbes, défaut 30 pts) |
| `begin(nb_courbes, nb_points)` | Réinitialise et redimensionne |
| `setTitre(titre)` | Titre du graphique |
| `setSubtitle(subtitle)` | Sous-titre |
| `setDimensions(largeur, hauteur)` | Taille en pixels |
| `setWidth(w)` / `setHeight(h)` | Largeur ou hauteur individuellement |
| `getWidth()` / `getHeight()` | Dimensions actuelles |

### Courbes

| Méthode | Description |
|---------|-------------|
| `setLegende(courbe, legende)` | Nom de la courbe (COURBE_1 à COURBE_4) |
| `setCouleur(courbe, couleurHex)` | Couleur hexadécimale (ex. `"#FF0000"`) |
| `setAxeY(courbe, axe)` | AXE_GAUCHE (0) ou AXE_DROITE (1) |
| `setTitreAxeY(axe, titre)` | Titre d'un axe Y |
| `setFormatAxeY(axe, format)` | Format Google Charts (ex. `"#.0"`, `"#.#####"`) |

### Temps

| Méthode | Description |
|---------|-------------|
| `setModeTemps(mode)` | TEMPS_RELATIF ou TEMPS_HHMM |
| `setTime(secondes)` | Timestamp en secondes |
| `setTimeHMS(heure, minute, seconde)` | Heure au format horloge |

### Alimentation des données

| Méthode | Description |
|---------|-------------|
| `decaler()` | Décale l'historique d'un cran (appeler en premier) |
| `addValue(type, valeur)` | `TYPE_TIME` pour le temps, ou `COURBE_X` pour une valeur |
| `incrementSample()` | Incrémente le compteur d'échantillons |
| `reset()` | Remet tout à zéro |
| `calculerMinMax()` | Recalcule les bornes (appelé automatiquement par `getPageWeb`/`streamPageWeb`) |

### Getters

| Méthode | Retour |
|---------|--------|
| `getValeur(courbe, index)` | Valeur d'une courbe à un index |
| `getTemps(index)` | Temps à un index |
| `getXmin()` / `getXmax()` | Bornes X |
| `getYmin()` / `getYmax()` | Bornes Y globales |
| `getYminAxe(axe)` / `getYmaxAxe(axe)` | Bornes Y par axe |
| `getNbCourbes()` / `getNbPoints()` | Dimensions |
| `getSample()` | Compteur d'échantillons |
| `getSeconds()` | Dernier timestamp |

### Export et rendu

| Méthode | Description |
|---------|-------------|
| `toJSON()` | Retourne une String au format JSON |
| `toCSV()` | Retourne une String au format CSV |
| `getPageWeb()` | Retourne le HTML/JS complet (String) |
| `streamPageWeb(Print&)` | Écrit le HTML/JS sur un flux (zéro-copie) |

### Couleurs prédéfinies (static)

`CouleurNoir()`, `CouleurRouge()`, `CouleurJaune()`, `CouleurOrange()`, `CouleurVert()`, `CouleurCyan()`, `CouleurBleu()`, `CouleurMagenta()`, `CouleurBlanc()`

---

## Exemple complet

Un exemple fonctionnel pour ESP8266 (Wemos D1 Mini) se trouve dans le dossier `exemple/Graphique_ESP8266/` :

```bash
cd exemple/Graphique_ESP8266
pio run                    # compiler
pio run --target upload    # flasher
pio device monitor         # moniteur série (115200 baud)
```

Routes disponibles une fois l'ESP8266 connecté au WiFi :

| Route | Description |
|-------|-------------|
| `/` | Page principale avec le graphique |
| `/json` | Données brutes au format JSON |
| `/csv` | Export CSV téléchargeable |
| `/setsize?w=800&h=400` | Redimensionne le graphique |

> **Attention :** les identifiants WiFi dans `main.cpp` (`WIFI_SSID` / `WIFI_PASSWORD`) sont des placeholders — modifiez-les avant de flasher.

---

## Remarques

- La bibliothèque dépend d'`Arduino.h` (core ESP8266/ESP32/Arduino). Aucune autre dépendance externe.
- Google Charts est chargé depuis le CDN Google (`www.gstatic.com`). Sans connexion internet, le graphique ne s'affichera pas.
- Sur les cartes avec peu de RAM (ESP8266 ~80 ko libres), préférez `streamPageWeb()` à `getPageWeb()` pour éviter les fragmentations mémoire.
- Les commentaires et la documentation sont en français ; les noms d'API et le code sont en anglais.

---

## Licence

Ce projet est distribué sous licence **GPL-3.0**. Voir le fichier `LICENSE` ou https://www.gnu.org/licenses/gpl-3.0.html.
