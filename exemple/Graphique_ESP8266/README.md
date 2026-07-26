# 📊 Graphique.h - Exemple ESP8266 (PlatformIO)

Exemple complet d'utilisation de la bibliothèque **Graphique.h** sur un ESP8266 avec PlatformIO sous VS Code.

## 🎯 Fonctionnalités démontrées

- **Graphique temps réel** avec 2 courbes (température + humidité)
- **Double axe Y** : température à gauche, humidité à droite
- **Dimensions dynamiques** : largeur et hauteur modifiables via l'interface web
- **Export de données** : JSON (`/json`) et CSV (`/csv`)
- **Design responsive** : page web stylisée en dark mode
- **Zero dépendance externe** : utilise uniquement `ESP8266WebServer` (inclus dans le core ESP8266)

## 📁 Structure du projet

```
Graphique_Example_ESP8266/
├── platformio.ini          # Configuration PlatformIO
├── src/
│   ├── main.cpp            # Sketch Arduino principal
│   └── Graphique.h         # Bibliothèque header-only
└── README.md               # Ce fichier
```

## 🛠️ Prérequis

- [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) (extension VS Code)
- Carte ESP8266 (Wemos D1 Mini, NodeMCU, etc.)
- Connexion WiFi

## ⚙️ Configuration

1. **Ouvrez le projet** dans VS Code (dossier `Graphique_Example_ESP8266/`)
2. **Modifiez les identifiants WiFi** dans `src/main.cpp` :
   ```cpp
   const char* WIFI_SSID     = "VOTRE_SSID";
   const char* WIFI_PASSWORD = "VOTRE_PASSWORD";
   ```
3. **(Optionnel)** Ajustez les dimensions par défaut :
   ```cpp
   monGraph.setDimensions(1200, 600);   // Largeur x Hauteur en pixels
   ```

## 🚀 Compilation et flash

Depuis VS Code :
- **Compiler** : `Ctrl+Alt+B` (ou icône ✅ en bas)
- **Uploader** : `Ctrl+Alt+U` (ou icône ➡️ en bas)
- **Moniteur série** : `Ctrl+Alt+S` (ou icône 🔌 en bas)

En ligne de commande (depuis le dossier projet) :
```bash
pio run              # Compiler
pio run --target upload   # Flasher
pio device monitor   # Moniteur série
```

## 🌐 Utilisation

1. Ouvrez le **Moniteur Série** (115200 bauds)
2. Attendez la connexion WiFi — l'IP s'affiche :
   ```
   [WIFI] Connecte ! IP : 192.168.1.42
   [WIFI] URL : http://192.168.1.42
   ```
3. Ouvrez l'URL dans votre navigateur

### Routes disponibles

| Route | Description |
|-------|-------------|
| `/` | Page principale avec le graphique |
| `/json` | Données brutes au format JSON |
| `/csv` | Export CSV téléchargeable |
| `/setsize?w=800&h=400` | Change la taille du graphique (redirection vers `/`) |

### Modifier la taille du graphique

Depuis la page web, entrez les dimensions souhaitées et cliquez sur **🔄 Appliquer**. Le graphique est régénéré avec les nouvelles valeurs.

## 🧪 Adapter avec de vrais capteurs

Remplacez la section `updateSensors()` dans `main.cpp` par la lecture de vos capteurs :

```cpp
// Exemple avec DHT22
#include <DHT.h>
#define DHT_PIN  D4
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

void updateSensors() {
    temperature = dht.readTemperature();
    humidite    = dht.readHumidity();

    if (isnan(temperature) || isnan(humidite)) return;

    monGraph.decaler();
    monGraph.setTime(millis() / 1000.0);
    monGraph.addValue(Graphique::COURBE_1, temperature);
    monGraph.addValue(Graphique::COURBE_2, humidite);
    monGraph.incrementSample();
}
```

## 📐 API de la bibliothèque (rappel)

```cpp
Graphique g(2, 60);                    // 2 courbes, 60 points

g.setDimensions(1200, 600);            // Taille personnalisée
g.setWidth(800);                       // Uniquement la largeur
g.setHeight(400);                      // Uniquement la hauteur

int w = g.getWidth();                  // Lecture largeur
int h = g.getHeight();                 // Lecture hauteur

g.setTitre("Mon Graphique");
g.setLegende(Graphique::COURBE_1, "Serie A");
g.setCouleur(Graphique::COURBE_1, Graphique::CouleurRouge());
g.setAxeY(Graphique::COURBE_1, Graphique::AXE_GAUCHE);

g.decaler();                           // Fait défiler l'historique
g.addValue(Graphique::COURBE_1, 23.5); // Ajoute une valeur
g.incrementSample();

String html = g.getPageWeb();          // Génère le code HTML/JS
String json = g.toJSON();              // Export JSON
String csv  = g.toCSV();               // Export CSV
```

## 📝 License

GPL-3.0 — Olivier FOURNET
