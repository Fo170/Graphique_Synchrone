/*
  main.cpp - Exemple d'utilisation de Graphique.h sur ESP8266 (PlatformIO)
  Auteur    : Olivier FOURNET
  License   : GPL-3.0

  Ce sketch simule une station météo (température + humidité) et affiche
  un graphique Google Charts via un serveur web embarqué.

  Fonctionnalités démontrées :
  - Graphique avec 2 courbes et double axe Y
  - Dimensions personnalisables (width / height)
  - Export JSON et CSV
  - Mise à jour toutes les 2 secondes

  Matériel testé : Wemos D1 Mini, NodeMCU 1.0
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Graphique_Synchrone.h>

// ============================================================
// CONFIGURATION WIFI
// ============================================================
const char* WIFI_SSID     = "VOTRE_SSID";      // <-- MODIFIEZ ICI
const char* WIFI_PASSWORD = "VOTRE_PASSWORD";  // <-- MODIFIEZ ICI

// ============================================================
// OBJETS GLOBAUX
// ============================================================
ESP8266WebServer server(80);
Graphique monGraph(2, 60);   // 2 courbes, 60 points d'historique

unsigned long lastUpdate = 0;
const unsigned long PERIODE_MS = 2000;  // 2 secondes entre chaque échantillon

// Valeurs simulées (remplacez par vos vrais capteurs : DHT22, BME280, etc.)
float temperature = 22.0;
float humidite    = 55.0;

// ============================================================
// PROTOTYPES
// ============================================================
void handleRoot();
void handleJSON();
void handleCSV();
void handleSetSize();
void handleNotFound();
void updateSensors();
String buildHtmlPage();

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n============================================");
  Serial.println("  Graphique.h - Demo ESP8266");
  Serial.println("============================================");

  // ----------------------------------------------------------
  // 1) Configuration du graphique
  // ----------------------------------------------------------
  monGraph.setDimensions(1200, 600);   // Largeur x Hauteur (pixels)
  monGraph.setTitre("Station Meteo - ESP8266");
  monGraph.setSubtitle("Temperature & Humidite en temps reel");
  monGraph.setModeTemps(Graphique::TEMPS_RELATIF);

  // Courbe 0 : Temperature -> Axe GAUCHE (rouge)
  monGraph.setLegende(Graphique::COURBE_1, "Temperature (°C)");
  monGraph.setCouleur(Graphique::COURBE_1, Graphique::CouleurRouge());
  monGraph.setAxeY(Graphique::COURBE_1, Graphique::AXE_GAUCHE);
  monGraph.setTitreAxeY(Graphique::AXE_GAUCHE, "Temperature °C");
  monGraph.setFormatAxeY(Graphique::AXE_GAUCHE, "#.0");

  // Courbe 1 : Humidite -> Axe DROIT (bleu)
  monGraph.setLegende(Graphique::COURBE_2, "Humidite (%)");
  monGraph.setCouleur(Graphique::COURBE_2, Graphique::CouleurBleu());
  monGraph.setAxeY(Graphique::COURBE_2, Graphique::AXE_DROITE);
  monGraph.setTitreAxeY(Graphique::AXE_DROITE, "Humidite %");
  monGraph.setFormatAxeY(Graphique::AXE_DROITE, "#.0");

  Serial.println("[GRAPH] Configuration OK");
  Serial.print("[GRAPH] Dimensions : ");
  Serial.print(monGraph.getWidth());
  Serial.print(" x ");
  Serial.println(monGraph.getHeight());

  // ----------------------------------------------------------
  // 2) Connexion WiFi
  // ----------------------------------------------------------
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("[WIFI] Connexion en cours");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("[WIFI] Connecte ! IP : ");
  Serial.println(WiFi.localIP());
  Serial.print("[WIFI] URL : http://");
  Serial.println(WiFi.localIP());

  // ----------------------------------------------------------
  // 3) Routes HTTP
  // ----------------------------------------------------------
  server.on("/",         HTTP_GET, handleRoot);      // Page principale
  server.on("/json",     HTTP_GET, handleJSON);      // API JSON
  server.on("/csv",      HTTP_GET, handleCSV);       // Export CSV
  server.on("/setsize",  HTTP_GET, handleSetSize);   // ?w=800&h=400
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("[HTTP] Serveur demarre sur le port 80");
  Serial.println("============================================\n");
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  server.handleClient();

  unsigned long now = millis();
  if (now - lastUpdate >= PERIODE_MS) {
    lastUpdate = now;
    updateSensors();
  }
}

// ============================================================
// MISE A JOUR DES CAPTEURS (simulation)
// ============================================================
void updateSensors() {
  // Simulation sinusoidale + bruit aleatoire
  // Remplacez ces lignes par la lecture de vos vrais capteurs :
  //   temperature = dht.readTemperature();
  //   humidite    = dht.readHumidity();

  float t = now() / 10000.0;
  temperature = 20.0 + 8.0 * sin(t) + random(-5, 6) / 10.0;
  humidite    = 50.0 + 15.0 * cos(t * 0.7) + random(-5, 6) / 10.0;

  // Contraintes realistes
  temperature = constrain(temperature, -10.0, 50.0);
  humidite    = constrain(humidite, 0.0, 100.0);

  // --- Mise a jour du graphique ---
  monGraph.decaler();                           // Fait defiler l'historique
  monGraph.setTime(millis() / 1000.0);          // Temps en secondes
  monGraph.addValue(Graphique::COURBE_1, temperature);
  monGraph.addValue(Graphique::COURBE_2, humidite);
  monGraph.incrementSample();

  // Debug serie
  Serial.print("[DATA] Sample #");
  Serial.print(monGraph.getSample());
  Serial.print(" | T=");
  Serial.print(temperature, 1);
  Serial.print("°C  H=");
  Serial.print(humidite, 1);
  Serial.println("%");
}

// ============================================================
// HANDLER : PAGE PRINCIPALE
// ============================================================
void handleRoot() {
  server.send(200, "text/html", buildHtmlPage());
}

// ============================================================
// HANDLER : API JSON
// ============================================================
void handleJSON() {
  server.send(200, "application/json", monGraph.toJSON());
}

// ============================================================
// HANDLER : EXPORT CSV
// ============================================================
void handleCSV() {
  String csv = monGraph.toCSV();
  server.sendHeader("Content-Disposition", "attachment; filename=\"data.csv\"");
  server.send(200, "text/csv", csv);
}

// ============================================================
// HANDLER : CHANGEMENT DE TAILLE
// URL : /setsize?w=800&h=400
// ============================================================
void handleSetSize() {
  bool changed = false;

  if (server.hasArg("w")) {
    int w = server.arg("w").toInt();
    if (w > 100) {
      monGraph.setWidth(w);
      changed = true;
    }
  }
  if (server.hasArg("h")) {
    int h = server.arg("h").toInt();
    if (h > 100) {
      monGraph.setHeight(h);
      changed = true;
    }
  }

  if (changed) {
    Serial.print("[HTTP] Nouvelles dimensions : ");
    Serial.print(monGraph.getWidth());
    Serial.print(" x ");
    Serial.println(monGraph.getHeight());
  }

  // Redirection vers la page principale
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting...");
}

// ============================================================
// HANDLER : 404
// ============================================================
void handleNotFound() {
  String msg = "404 - Page non trouvee\n\n";
  msg += "Routes disponibles :\n";
  msg += "  /         -> Page principale (graphique)\n";
  msg += "  /json     -> Donnees au format JSON\n";
  msg += "  /csv      -> Export CSV\n";
  msg += "  /setsize  -> ?w=800&h=400 (changer taille)\n";
  server.send(404, "text/plain", msg);
}

// ============================================================
// CONSTRUCTION DE LA PAGE HTML
// ============================================================
String buildHtmlPage() {
  String page;
  page.reserve(8000);  // Optimisation memoire : pre-allocation

  page += "<!DOCTYPE html><html lang='fr'><head>";
  page += "<meta charset='UTF-8'>";
  page += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  page += "<title>Graphique ESP8266</title>";
  page += "<style>";
  page += "body{font-family:'Segoe UI',Arial,sans-serif;background:#0f172a;color:#e2e8f0;";
  page += "margin:0;padding:20px;text-align:center;}";
  page += "h1{color:#38bdf8;margin-bottom:5px;font-size:1.8em;}";
  page += ".subtitle{color:#94a3b8;font-size:0.95em;margin-bottom:25px;}";
  page += ".panel{background:#1e293b;border-radius:12px;padding:20px;";
  page += "max-width:" + String(monGraph.getWidth() + 40) + "px;margin:0 auto 20px auto;box-shadow:0 4px 15px rgba(0,0,0,0.3);}";
  page += ".controls{display:flex;justify-content:center;align-items:center;gap:15px;";
  page += "flex-wrap:wrap;margin-bottom:20px;}";
  page += ".controls label{color:#cbd5e1;font-size:0.9em;}";
  page += ".controls input{width:90px;padding:8px;border-radius:6px;border:1px solid #334155;";
  page += "background:#0f172a;color:#e2e8f0;text-align:center;font-size:1em;}";
  page += ".controls button{background:#38bdf8;color:#0f172a;border:none;padding:10px 24px;";
  page += "border-radius:6px;cursor:pointer;font-weight:bold;font-size:0.95em;transition:background 0.2s;}";
  page += ".controls button:hover{background:#0ea5e9;}";
  page += ".graph-box{background:#0f172a;border-radius:8px;padding:10px;display:inline-block;}";
  page += ".links{margin-top:20px;}";
  page += ".links a{color:#38bdf8;text-decoration:none;margin:0 15px;font-size:0.9em;}";
  page += ".links a:hover{text-decoration:underline;}";
  page += ".footer{color:#64748b;font-size:0.8em;margin-top:25px;}";
  page += "</style></head><body>";

  // Titre
  page += "<h1>📊 Station Meteo ESP8266</h1>";
  page += "<div class='subtitle'>Demo Graphique.h - Double axe Y avec dimensions dynamiques</div>";

  // Panneau de controle
  page += "<div class='panel'>";
  page += "<div class='controls'>";
  page += "<label>Largeur (px) : <input type='number' id='w' value='" + String(monGraph.getWidth()) + "' min='200' max='2000'></label>";
  page += "<label>Hauteur (px) : <input type='number' id='h' value='" + String(monGraph.getHeight()) + "' min='150' max='1200'></label>";
  page += "<button onclick=\"window.location.href='/setsize?w='+document.getElementById('w').value+'&h='+document.getElementById('h').value\">🔄 Appliquer</button>";
  page += "</div>";

  // Graphique (injection du code JS/Google Charts)
  page += "<div class='graph-box'>";
  page += monGraph.getPageWeb();
  page += "</div>";

  page += "</div>";  // fin panel

  // Liens
  page += "<div class='links'>";
  page += "<a href='/json' target='_blank'>📄 Voir JSON</a>";
  page += "<a href='/csv' target='_blank'>📥 Telecharger CSV</a>";
  page += "<a href='/' onclick='location.reload()'>🔄 Rafraichir</a>";
  page += "</div>";

  // Footer
  page += "<div class='footer'>";
  page += "IP : " + WiFi.localIP().toString();
  page += " | Sample #" + String(monGraph.getSample());
  page += " | Graph : " + String(monGraph.getWidth()) + "x" + String(monGraph.getHeight()) + "px";
  page += "</div>";

  page += "</body></html>";
  return page;
}
