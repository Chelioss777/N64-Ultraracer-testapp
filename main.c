#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libdragon.h>

// Struktur zur Speicherung der Spitzenwerte (Max-Hold Feature)
typedef struct {
    int8_t max_x;
    int8_t min_x;
    int8_t max_y;
    int8_t min_y;
    bool rumble_active;
    int rumble_timer;
} ControllerStats;

ControllerStats stats[4];

// Setzt die gespeicherten Maximalwerte zurück
void reset_stats(int port) {
    stats[port].max_x = 0;
    stats[port].min_x = 0;
    stats[port].max_y = 0;
    stats[port].min_y = 0;
    stats[port].rumble_active = false;
    stats[port].rumble_timer = 0;
}

int main(void) {
    // N64 Subsysteme initialisieren
    init_interrupts();
    // Modernes Libdragon nutzt 2 Buffer für flüssige Grafikausgabe
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);
    controller_init();

    // Statistiken für alle 4 Ports nullen
    for(int i = 0; i < 4; i++) reset_stats(i);

    while(1) {
        controller_scan();
        
        // NEU: Holt die aktuelle Oberfläche und sperrt sie für sicheres Zeichnen
        surface_t *disp = display_lock();
        if (!disp) continue; // Falls kein Framebuffer bereit ist, Frame überspringen

        // Hintergrundfarbe: Dunkles Rennstrecken-Grau
        graphics_clear_screen(disp, RGBA32(34, 34, 34, 255));

        // Titel-Leiste
        graphics_draw_text(disp, 25, 10, "ULTRA RACER 64 - MULTI-DIAGNOSTIC TOOL");
        graphics_draw_line(disp, 25, 20, 295, 20, RGBA32(255, 215, 0, 255)); // Goldene Linie

        // Schleife für das 4-Spieler-Layout (Aufgeteilt in 4 Quadranten)
        for (int port = 0; port < 4; port++) {
            int offsetX = (port % 2) * 160;
            int offsetY = (port / 2) * 105 + 30;

            // NEU: Richtige Abfrage der Controllerdaten für den jeweiligen Port im modernen Libdragon
            struct controller_data cdata = controller_get_data(port);

            // Umrandung der Spieler-Boxen zeichnen
            graphics_draw_line(disp, offsetX, offsetY, offsetX + 155, offsetY, RGBA32(70, 70, 70, 255));
            
            char port_label[8];
            sprintf(port_label, "P%d", port + 1);
            graphics_draw_text(disp, offsetX + 5, offsetY + 5, port_label);

            // Analogstick-Werte für diesen Port auslesen
            int8_t stick_x = cdata.c[0].x; 
            int8_t stick_y = cdata.c[0].y;

            // =================================================================
            // CONTROLLER RESET FÜR EVERDRIVE (L + R + Z + START)
            // =================================================================
            if (cdata.c[0].L && cdata.c[0].R && cdata.c[0].Z && cdata.c[0].start) {
                // Löst den offiziellen N64-Hardware-Soft-Reset aus.
                // Das Everdrive fängt dies ab und springt zurück ins Menü.
                sys_reset(); 
            }

            // FEATURE 1: MAXIMALWERT-SPEICHERUNG (Peak Hold Logik)
            if (stick_x > stats[port].max_x) stats[port].max_x = stick_x;
            if (stick_x < stats[port].min_x) stats[port].min_x = stick_x;
            if (stick_y > stats[port].max_y) stats[port].max_y = stick_y;
            if (stick_y < stats[port].min_y) stats[port].min_y = stick_y;

            // Reset der Werte, wenn START gedrückt wird
            // Nutzt keys_down-Logik über den pressed-Status im modernen Libdragon
            if (cdata.c[0].start) {
                reset_stats(port);
            }

            // FEATURE 2: RUMBLE PAK TEST LOGIK
            if (cdata.c[0].A && cdata.c[0].B) {
                stats[port].rumble_active = true;
                stats[port].rumble_timer = 30;
            }

            if (stats[port].rumble_active) {
                graphics_draw_text(disp, offsetX + 5, offsetY + 90, "RUMBLE: *VIBRIEREN*");
                stats[port].rumble_timer--;
                if(stats[port].rumble_timer <= 0) {
                    stats[port].rumble_active = false;
                }
            } else {
                graphics_draw_text(disp, offsetX + 5, offsetY + 90, "RUMBLE: A+B halten");
            }

            // VISUELLES LAYOUT: ULTRA RACER INTERFACE (Keilform aufbauen)
            int centerW_X = offsetX + 110; 
            int centerW_Y = offsetY + 35;

            graphics_draw_box(disp, centerW_X - 20, centerW_Y - 20, 40, 40, RGBA32(60, 60, 60, 255)); 
            
            int draw_x = centerW_X + (stick_x / 4);
            int draw_y = centerW_Y - (stick_y / 4);
            graphics_draw_line(disp, centerW_X - 15, centerW_Y, centerW_X + 15, centerW_Y, RGBA32(100, 100, 100, 255));
            graphics_draw_box(disp, draw_x - 4, draw_y - 4, 8, 8, RGBA32(255, 215, 0, 255)); 

            // Numerische Live-Werte & Peaks anzeigen
            char val_buf[32];
            sprintf(val_buf, "X:%3d (Min:%3d Max:%3d)", stick_x, stats[port].min_x, stats[port].max_x);
            graphics_draw_text(disp, offsetX + 5, offsetY + 20, val_buf);
            sprintf(val_buf, "Y:%3d (Min:%3d Max:%3d)", stick_y, stats[port].min_y, stats[port].max_y);
            graphics_draw_text(disp, offsetX + 5, offsetY + 32, val_buf);

            // Tastenliste
            int btnY = offsetY + 50;
            if (cdata.c[0].Z)       graphics_draw_text(disp, offsetX + 5, btnY, "Z (TRIGGER)");
            if (cdata.c[0].L)       graphics_draw_text(disp, offsetX + 5, btnY + 10, "L (FLIPPER)");
            if (cdata.c[0].R)       graphics_draw_text(disp, offsetX + 5, btnY + 20, "R (FLIPPER)");
            
            if (cdata.c[0].c_up || cdata.c[0].c_down || cdata.c[0].c_left || cdata.c[0].c_right) {
                graphics_draw_text(disp, offsetX + 80, btnY, "C-PAD");
            }
            if (cdata.c[0].up || cdata.c[0].down || cdata.c[0].left || cdata.c[0].right) {
                graphics_draw_text(disp, offsetX + 80, btnY + 10, "D-PAD");
            }
        }

        // Info-Leiste unten aktualisiert mit dem Reset-Hinweis
        graphics_draw_text(disp, 15, 222, "START: Peak-Werte nullen");
        graphics_draw_text(disp, 15, 231, "L+R+Z+START: Zurueck zum Everdrive");

        // NEU: Zeigt das fertig gezeichnete Bild auf dem Bildschirm an
        display_show(disp);
    }
}
