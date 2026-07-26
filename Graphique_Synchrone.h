/*
  Graphique.h - Bibliotheque modulaire header-only pour graphiques Google Charts (ESP8266/ESP32/Arduino)
  Auteur    : Olivier FOURNET
  License   : GPL-3.0
  GitHub    : https://github.com/Fo170/Graphique_Synchrone
*/

#ifndef GRAPHIQUE_H
#define GRAPHIQUE_H

#include <Arduino.h>

// ============================================================
// CLASSE GRAPHIQUE
// ============================================================
class Graphique {
  public:
    static const int TYPE_TIME   = 10;
    static const int COURBE_1    = 0;
    static const int COURBE_2    = 1;
    static const int COURBE_3    = 2;
    static const int COURBE_4    = 3;

    enum ModeTemps { TEMPS_RELATIF, TEMPS_HHMM };
    enum AxeY      { AXE_GAUCHE = 0, AXE_DROITE = 1 };

    Graphique(int nb_courbes = 1, int nb_points = 30);
    ~Graphique();

    void begin(int nb_courbes, int nb_points);
    void setNbCourbes(int n);
    void setNbPoints(int n);

    void setTitre(const String& titre);
    void setSubtitle(const String& subtitle);

    void setModeTemps(ModeTemps mode);
    ModeTemps getModeTemps() const;

    void setTime(float temps);
    void setTimeHMS(int heure, int minute, float seconde = 0.0f);

    void setCouleur(int courbe, const String& couleurHex);
    void setLegende(int courbe, const String& legende);

    void setAxeY(int courbe, AxeY axe);
    void setTitreAxeY(AxeY axe, const String& titre);
    void setFormatAxeY(AxeY axe, const String& format);

    // --- Nouveaux setters pour les dimensions du graphique ---
    void setWidth(int width);
    void setHeight(int height);
    void setDimensions(int width, int height);
    int getWidth() const;
    int getHeight() const;

    static String CouleurNoir()    { return "#000000"; }
    static String CouleurRouge()   { return "#FF0000"; }
    static String CouleurJaune()   { return "#FFFF00"; }
    static String CouleurOrange()  { return "#FFA000"; }
    static String CouleurVert()    { return "#00FF00"; }
    static String CouleurCyan()    { return "#00FFFF"; }
    static String CouleurBleu()    { return "#0000FF"; }
    static String CouleurMagenta() { return "#FF00FF"; }
    static String CouleurBlanc()   { return "#FFFFFF"; }

    void reset();
    void decaler();
    void addValue(int type, float valeur);
    void incrementSample();

    void calculerMinMax();

    float getValeur(int courbe, int index) const;
    float getTemps(int index) const;
    float getXmin() const;
    float getXmax() const;
    float getYmin() const;
    float getYmax() const;
    float getYminAxe(AxeY axe) const;
    float getYmaxAxe(AxeY axe) const;
    int   getNbPoints() const;
    int   getNbCourbes() const;
    unsigned long getSample() const;
    float getSeconds() const;

    String toCSV() const;
    String toJSON() const;

    // --- Ancienne API (String massive) ---
    String getPageWeb();

    // --- NOUVELLE API : streaming zero-copy ---
    void streamPageWeb(Print& out);

  private:
    int _nb_courbes;
    int _nb_points;
    unsigned long _sample;
    float _seconds;

    float*  _time;
    float** _data;
    String* _legendes;
    String* _couleurs;
    String  _titre;
    String  _subtitle;

    ModeTemps _modeTemps;

    int*    _axeY;
    String  _titreAxeY[2];
    String  _formatAxeY[2];
    float   _y_min_axis[2];
    float   _y_max_axis[2];

    float _x_min, _x_max, _y_min, _y_max;

    // --- Dimensions du graphique ---
    int _width;
    int _height;

    void allouerMemoire();
    void libererMemoire();
    String escapeJSON(const String& s) const;
};

// ============================================================
// IMPLEMENTATIONS
// ============================================================

Graphique::Graphique(int nb_courbes, int nb_points) {
    _time = nullptr; _data = nullptr; _legendes = nullptr;
    _couleurs = nullptr; _axeY = nullptr;
    begin(nb_courbes, nb_points);
}

Graphique::~Graphique() { libererMemoire(); }

void Graphique::allouerMemoire() {
    _time = new float[_nb_points];
    _data = new float*[_nb_courbes];
    for (int c = 0; c < _nb_courbes; c++) _data[c] = new float[_nb_points];
    _legendes = new String[_nb_courbes];
    _couleurs = new String[_nb_courbes];
    _axeY     = new int[_nb_courbes];
}

void Graphique::libererMemoire() {
    if (_time)     { delete[] _time;     _time = nullptr; }
    if (_data) {
        for (int c = 0; c < _nb_courbes; c++) if (_data[c]) delete[] _data[c];
        delete[] _data; _data = nullptr;
    }
    if (_legendes) { delete[] _legendes; _legendes = nullptr; }
    if (_couleurs) { delete[] _couleurs; _couleurs = nullptr; }
    if (_axeY)     { delete[] _axeY;     _axeY = nullptr; }
}

void Graphique::begin(int nb_courbes, int nb_points) {
    libererMemoire();
    _nb_courbes = constrain(nb_courbes, 1, 4);
    _nb_points  = (nb_points > 2) ? nb_points : 30;
    allouerMemoire();

    _modeTemps = TEMPS_RELATIF;
    _titre     = "Average values";
    _subtitle  = "";
    _seconds   = 0.0f;
    _sample    = 0;
    _x_min = _x_max = _y_min = _y_max = 0.0f;
    _y_min_axis[0] = _y_min_axis[1] = 0.0f;
    _y_max_axis[0] = _y_max_axis[1] = 0.0f;
    _titreAxeY[0]  = "";
    _titreAxeY[1]  = "";
    _formatAxeY[0] = "#.######";
    _formatAxeY[1] = "#.######";

    // Valeurs par defaut des dimensions (compatibilite ascendante)
    _width  = 900;
    _height = 500;

    String defauts[4] = { CouleurRouge(), CouleurVert(), CouleurBleu(), CouleurOrange() };
    for (int c = 0; c < _nb_courbes; c++) {
        _couleurs[c] = defauts[c];
        _legendes[c] = "Courbe " + String(c + 1);
        _axeY[c]     = 0;
    }
    reset();
}

void Graphique::setNbCourbes(int n) { begin(constrain(n, 1, 4), _nb_points); }
void Graphique::setNbPoints(int n)  { begin(_nb_courbes, (n > 2) ? n : 30); }
void Graphique::setTitre(const String& titre)     { _titre = titre; }
void Graphique::setSubtitle(const String& subtitle){ _subtitle = subtitle; }
void Graphique::setModeTemps(ModeTemps mode)      { _modeTemps = mode; }
Graphique::ModeTemps Graphique::getModeTemps() const { return _modeTemps; }

// --- Nouveaux setters/getters dimensions ---
void Graphique::setWidth(int width)   { _width = (width > 0) ? width : 900; }
void Graphique::setHeight(int height) { _height = (height > 0) ? height : 500; }
void Graphique::setDimensions(int width, int height) {
    setWidth(width);
    setHeight(height);
}
int Graphique::getWidth() const  { return _width; }
int Graphique::getHeight() const { return _height; }

void Graphique::setTime(float temps) {
    _time[_nb_points - 1] = temps;
    _seconds = temps;
}

void Graphique::setTimeHMS(int heure, int minute, float seconde) {
    float minutes = (float)heure * 60.0f + (float)minute + seconde / 60.0f;
    _time[_nb_points - 1] = minutes;
    _seconds = minutes;
}

void Graphique::setCouleur(int courbe, const String& couleurHex) {
    if (courbe >= 0 && courbe < _nb_courbes) _couleurs[courbe] = couleurHex;
}
void Graphique::setLegende(int courbe, const String& legende) {
    if (courbe >= 0 && courbe < _nb_courbes) _legendes[courbe] = legende;
}
void Graphique::setAxeY(int courbe, AxeY axe) {
    if (courbe >= 0 && courbe < _nb_courbes) _axeY[courbe] = (int)axe;
}
void Graphique::setTitreAxeY(AxeY axe, const String& titre) {
    _titreAxeY[(int)axe] = titre;
}
void Graphique::setFormatAxeY(AxeY axe, const String& format) {
    _formatAxeY[(int)axe] = format;
}

void Graphique::reset() {
    for (int i = 0; i < _nb_points; i++) {
        _time[i] = 0.0f;
        for (int c = 0; c < _nb_courbes; c++) _data[c][i] = 0.0f;
    }
    _sample = 0;
    _x_min = _x_max = _y_min = _y_max = 0.0f;
    _y_min_axis[0] = _y_min_axis[1] = 0.0f;
    _y_max_axis[0] = _y_max_axis[1] = 0.0f;
}

void Graphique::decaler() {
    for (int i = 1; i < _nb_points; i++) {
        _time[i - 1] = _time[i];
        for (int c = 0; c < _nb_courbes; c++) _data[c][i - 1] = _data[c][i];
    }
}

void Graphique::addValue(int type, float valeur) {
    if (type == TYPE_TIME) {
        _time[_nb_points - 1] = valeur;
        _seconds = valeur;
    } else if (type >= 0 && type < _nb_courbes) {
        _data[type][_nb_points - 1] = valeur;
    }
}

void Graphique::incrementSample() { _sample++; }

void Graphique::calculerMinMax() {
    _x_min = _x_max = _seconds;
    _y_min = _y_max = 0.0f;
    _y_min_axis[0] = _y_min_axis[1] =  999999.0f;
    _y_max_axis[0] = _y_max_axis[1] = -999999.0f;

    for (int i = 0; i < _nb_points; i++) {
        if (_time[i] < _x_min) _x_min = _time[i];
        if (_time[i] > _x_max) _x_max = _time[i];
        for (int c = 0; c < _nb_courbes; c++) {
            float v = _data[c][i];
            int axe = _axeY[c];
            if (v < _y_min) _y_min = v;
            if (v > _y_max) _y_max = v;
            if (v < _y_min_axis[axe]) _y_min_axis[axe] = v;
            if (v > _y_max_axis[axe]) _y_max_axis[axe] = v;
        }
    }
    for (int a = 0; a < 2; a++) {
        if (_y_min_axis[a] ==  999999.0f) _y_min_axis[a] = 0.0f;
        if (_y_max_axis[a] == -999999.0f) _y_max_axis[a] = 0.0f;
    }
}

float Graphique::getValeur(int courbe, int index) const {
    if (courbe >= 0 && courbe < _nb_courbes && index >= 0 && index < _nb_points)
        return _data[courbe][index];
    return 0.0f;
}
float Graphique::getTemps(int index) const {
    if (index >= 0 && index < _nb_points) return _time[index];
    return 0.0f;
}
float Graphique::getXmin() const { return _x_min; }
float Graphique::getXmax() const { return _x_max; }
float Graphique::getYmin() const { return _y_min; }
float Graphique::getYmax() const { return _y_max; }
float Graphique::getYminAxe(AxeY axe) const { return _y_min_axis[(int)axe]; }
float Graphique::getYmaxAxe(AxeY axe) const { return _y_max_axis[(int)axe]; }
int   Graphique::getNbPoints() const { return _nb_points; }
int   Graphique::getNbCourbes() const { return _nb_courbes; }
unsigned long Graphique::getSample() const { return _sample; }
float Graphique::getSeconds() const { return _seconds; }

String Graphique::toCSV() const {
    String csv = "Index";
    csv += (_modeTemps == TEMPS_HHMM) ? ",Heure" : ",Seconds";
    for (int c = 0; c < _nb_courbes; c++) csv += "," + _legendes[c];
    csv += "\r\n";
    for (int i = 0; i < _nb_points; i++) {
        csv += String(i) + "," + String(_time[i], 3);
        for (int c = 0; c < _nb_courbes; c++) csv += "," + String(_data[c][i], 6);
        csv += "\r\n";
    }
    return csv;
}

String Graphique::escapeJSON(const String& s) const {
    String out;
    for (unsigned int i = 0; i < s.length(); i++) {
        char c = s[i];
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else out += c;
    }
    return out;
}

String Graphique::toJSON() const {
    String json = "{\r\n";
    json += "  \"mode_temps\": " + String((_modeTemps == TEMPS_HHMM) ? "\"HHMM\"" : "\"relatif\"") + ",\r\n";
    json += "  \"nb_points\": " + String(_nb_points) + ",\r\n";
    json += "  \"nb_courbes\": " + String(_nb_courbes) + ",\r\n";
    json += "  \"sample\": " + String(_sample) + ",\r\n";
    json += "  \"width\": " + String(_width) + ",\r\n";
    json += "  \"height\": " + String(_height) + ",\r\n";
    json += "  \"time\": [";
    for (int i = 0; i < _nb_points; i++) {
        json += String(_time[i], 3);
        if (i < _nb_points - 1) json += ", ";
    }
    json += "],\r\n";
    json += "  \"series\": [\r\n";
    for (int c = 0; c < _nb_courbes; c++) {
        json += "    {\r\n";
        json += "      \"legende\": \"" + escapeJSON(_legendes[c]) + "\",\r\n";
        json += "      \"couleur\": \"" + _couleurs[c] + "\",\r\n";
        json += "      \"axeY\": " + String(_axeY[c]) + ",\r\n";
        json += "      \"data\": [";
        for (int i = 0; i < _nb_points; i++) {
            json += String(_data[c][i], 6);
            if (i < _nb_points - 1) json += ", ";
        }
        json += "]\r\n";
        json += "    }";
        if (c < _nb_courbes - 1) json += ",";
        json += "\r\n";
    }
    json += "  ]\r\n";
    json += "}";
    return json;
}

// ============================================================
// ANCIENNE API : String massive (conservee pour compatibilite)
// ============================================================
String Graphique::getPageWeb() {
    calculerMinMax();
    String page = "";

    page += "<script type='text/javascript'>\r\n";
    page += "var Seconds=" + String(_seconds, 3) + "; ";
    page += "var x_min="    + String(_x_min, 3) + ", ";
    page += "x_max="      + String(_x_max, 3) + ", ";
    page += "y_min="      + String(_y_min, 6) + ", ";
    page += "y_max="      + String(_y_max, 6) + "; ";
    page += "</script>\r\n";

    page += "<p>Seconds = <script>document.write(Seconds)</script> / ";
    page += "Sample : " + String(_sample) + " / NB_SAMPLES : " + String(_nb_points) + "<br>\r\n";
    page += "x_min = <script>document.write(x_min)</script> / ";
    page += "x_max = <script>document.write(x_max)</script> / ";
    page += "y_min = <script>document.write(y_min)</script> / ";
    page += "y_max = <script>document.write(y_max)</script>\r\n";
    page += "</p><hr>\r\n";

    page += "<script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script>\r\n";
    page += "<script type='text/javascript'>\r\n";
    page += "google.charts.load('current', {'packages':['line']});\r\n";
    page += "google.charts.setOnLoadCallback(drawChart);\r\n";
    page += "function drawChart() {\r\n";
    page += "var data = new google.visualization.DataTable();\r\n";

    if (_modeTemps == TEMPS_HHMM) {
        page += "data.addColumn('timeofday', 'Heure');\r\n";
    } else {
        page += "data.addColumn('number', 'Seconds');\r\n";
    }

    for (int c = 0; c < _nb_courbes; c++) {
        page += "data.addColumn('number', '" + escapeJSON(_legendes[c]) + "');\r\n";
    }

    page += "data.addRows([\r\n";
    for (int i = 0; i < _nb_points; i++) {
        page += "[";
        if (_modeTemps == TEMPS_HHMM) {
            int totalMin = (int)_time[i];
            int h = totalMin / 60;
            int m = totalMin % 60;
            int sec = (int)((_time[i] - (float)totalMin) * 60.0f);
            page += "[" + String(h) + "," + String(m) + "," + String(sec) + "]";
        } else {
            page += String(_time[i], 3);
        }
        for (int c = 0; c < _nb_courbes; c++) {
            page += ", " + String(_data[c][i], 6);
        }
        page += "]";
        if (i < _nb_points - 1) page += ",\r\n";
        else page += "\r\n";
    }
    page += "]);\r\n";

    page += "var options = {\r\n";

    bool hasDualAxis = false;
    for (int c = 0; c < _nb_courbes; c++) {
        if (_axeY[c] == 1) { hasDualAxis = true; break; }
    }
    if (hasDualAxis) {
        page += "  series: {\r\n";
        for (int c = 0; c < _nb_courbes; c++) {
            page += "    " + String(c) + ": {targetAxisIndex: " + String(_axeY[c]) + "}";
            if (c < _nb_courbes - 1) page += ",";
            page += "\r\n";
        }
        page += "  },\r\n";
    }

    page += "  colors: [";
    for (int c = 0; c < _nb_courbes; c++) {
        page += "'" + _couleurs[c] + "'";
        if (c < _nb_courbes - 1) page += ", ";
    }
    page += "],\r\n";

    page += "  chart: { title: '" + escapeJSON(_titre) + "', subtitle: '" + escapeJSON(_subtitle) + "' },\r\n";

    page += "  vAxes: {\r\n";
    for (int a = 0; a < 2; a++) {
        page += "    " + String(a) + ": { ";
        if (_titreAxeY[a].length() > 0) {
            page += "title: '" + escapeJSON(_titreAxeY[a]) + "', ";
        }
        page += "format: '" + _formatAxeY[a] + "' ";
        page += "}";
        if (a < 1) page += ",";
        page += "\r\n";
    }
    page += "  },\r\n";

    if (_modeTemps == TEMPS_HHMM) {
        page += "  hAxis: {format:'HH:mm', gridlines: { count: 10 }},\r\n";
    } else {
        page += "  hAxis: {format:'#.###', gridlines: { count: 10 }},\r\n";
    }

    // --- Dimensions dynamiques ---
    page += "  width: " + String(_width) + ", height: " + String(_height) + "\r\n";
    page += "};\r\n";

    page += "var chart = new google.charts.Line(document.getElementById('curve_chart'));\r\n";
    page += "chart.draw(data, google.charts.Line.convertOptions(options));\r\n";
    page += "}\r\n";
    page += "</script>\r\n";

    // --- Div avec dimensions dynamiques ---
    page += "<div id='curve_chart' style='width: " + String(_width) + "px; height: " + String(_height) + "px'></div>\r\n";
    page += "<p><hr></p>\r\n";

    return page;
}

// ============================================================
// NOUVELLE API : streaming direct sur un Print&
// ============================================================
void Graphique::streamPageWeb(Print& out) {
    calculerMinMax();

    // --- Debug JS ---
    out.print(F("<script type='text/javascript'>\r\n"));
    out.print(F("var Seconds=")); out.print(_seconds, 3); out.print(F("; "));
    out.print(F("var x_min="));    out.print(_x_min, 3);  out.print(F(", "));
    out.print(F("x_max="));        out.print(_x_max, 3);  out.print(F(", "));
    out.print(F("y_min="));        out.print(_y_min, 6);  out.print(F(", "));
    out.print(F("y_max="));        out.print(_y_max, 6);  out.print(F("; "));
    out.print(F("</script>\r\n"));

    // --- Infos debug ---
    out.print(F("<p>Seconds = <script>document.write(Seconds)</script> / "));
    out.print(F("Sample : ")); out.print(_sample); out.print(F(" / NB_SAMPLES : ")); out.print(_nb_points); out.print(F("<br>\r\n"));
    out.print(F("x_min = <script>document.write(x_min)</script> / "));
    out.print(F("x_max = <script>document.write(x_max)</script> / "));
    out.print(F("y_min = <script>document.write(y_min)</script> / "));
    out.print(F("y_max = <script>document.write(y_max)</script>\r\n"));
    out.print(F("</p><hr>\r\n"));

    // --- Google Charts ---
    out.print(F("<script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script>\r\n"));
    out.print(F("<script type='text/javascript'>\r\n"));
    out.print(F("google.charts.load('current', {'packages':['line']});\r\n"));
    out.print(F("google.charts.setOnLoadCallback(drawChart);\r\n"));
    out.print(F("function drawChart() {\r\n"));
    out.print(F("var data = new google.visualization.DataTable();\r\n"));

    if (_modeTemps == TEMPS_HHMM) {
        out.print(F("data.addColumn('timeofday', 'Heure');\r\n"));
    } else {
        out.print(F("data.addColumn('number', 'Seconds');\r\n"));
    }

    for (int c = 0; c < _nb_courbes; c++) {
        out.print(F("data.addColumn('number', '"));
        out.print(escapeJSON(_legendes[c]));
        out.print(F("');\r\n"));
    }

    out.print(F("data.addRows([\r\n"));
    for (int i = 0; i < _nb_points; i++) {
        out.print(F("["));
        if (_modeTemps == TEMPS_HHMM) {
            int totalMin = (int)_time[i];
            int h = totalMin / 60;
            int m = totalMin % 60;
            int sec = (int)((_time[i] - (float)totalMin) * 60.0f);
            out.print(F("[")); out.print(h); out.print(F(",")); out.print(m); out.print(F(",")); out.print(sec); out.print(F("]"));
        } else {
            out.print(_time[i], 3);
        }
        for (int c = 0; c < _nb_courbes; c++) {
            out.print(F(", ")); out.print(_data[c][i], 6);
        }
        out.print(F("]"));
        if (i < _nb_points - 1) out.print(F(",\r\n"));
        else out.print(F("\r\n"));
    }
    out.print(F("]);\r\n"));

    // --- Options ---
    out.print(F("var options = {\r\n"));

    bool hasDualAxis = false;
    for (int c = 0; c < _nb_courbes; c++) {
        if (_axeY[c] == 1) { hasDualAxis = true; break; }
    }
    if (hasDualAxis) {
        out.print(F("  series: {\r\n"));
        for (int c = 0; c < _nb_courbes; c++) {
            out.print(F("    ")); out.print(c); out.print(F(": {targetAxisIndex: ")); out.print(_axeY[c]); out.print(F("}"));
            if (c < _nb_courbes - 1) out.print(F(","));
            out.print(F("\r\n"));
        }
        out.print(F("  },\r\n"));
    }

    out.print(F("  colors: ["));
    for (int c = 0; c < _nb_courbes; c++) {
        out.print(F("'")); out.print(_couleurs[c]); out.print(F("'"));
        if (c < _nb_courbes - 1) out.print(F(", "));
    }
    out.print(F("],\r\n"));

    out.print(F("  chart: { title: '")); out.print(escapeJSON(_titre)); out.print(F("', subtitle: '")); out.print(escapeJSON(_subtitle)); out.print(F("' },\r\n"));

    out.print(F("  vAxes: {\r\n"));
    for (int a = 0; a < 2; a++) {
        out.print(F("    ")); out.print(a); out.print(F(": { "));
        if (_titreAxeY[a].length() > 0) {
            out.print(F("title: '")); out.print(escapeJSON(_titreAxeY[a])); out.print(F("', "));
        }
        out.print(F("format: '")); out.print(_formatAxeY[a]); out.print(F("' "));
        out.print(F("}"));
        if (a < 1) out.print(F(","));
        out.print(F("\r\n"));
    }
    out.print(F("  },\r\n"));

    if (_modeTemps == TEMPS_HHMM) {
        out.print(F("  hAxis: {format:'HH:mm', gridlines: { count: 10 }},\r\n"));
    } else {
        out.print(F("  hAxis: {format:'#.###', gridlines: { count: 10 }},\r\n"));
    }

    // --- Dimensions dynamiques ---
    out.print(F("  width: ")); out.print(_width); out.print(F(", height: ")); out.print(_height); out.print(F("\r\n"));
    out.print(F("};\r\n"));

    out.print(F("var chart = new google.charts.Line(document.getElementById('curve_chart'));\r\n"));
    out.print(F("chart.draw(data, google.charts.Line.convertOptions(options));\r\n"));
    out.print(F("}\r\n"));
    out.print(F("</script>\r\n"));

    // --- Div avec dimensions dynamiques ---
    out.print(F("<div id='curve_chart' style='width: ")); out.print(_width); out.print(F("px; height: ")); out.print(_height); out.print(F("px'></div>\r\n"));
    out.print(F("<p><hr></p>\r\n"));
}

#endif // GRAPHIQUE_H
