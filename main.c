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
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    controller_init();

    // Statistiken für alle 4 Ports nullen
    for(int i = 0; i < 4; i++) reset_stats(i);

    while(1) {
        controller_scan();
        
        surface_t *disp = display_get();
        // Hintergrundfarbe: Dunkles Rennstrecken-Grau
        graphics_clear_screen(disp, RGBA32(34, 34, 34, 255));

        // Titel-Leiste
        graphics_draw_text(disp, 25, 10, "ULTRA RACER 64 - MULTI-DIAGNOSTIC TOOL");
        graphics_draw_line(disp, 25, 20, 295, 20, RGBA32(255, 215, 0, 255)); // Goldene Linie

        // Schleife für das 4-Spieler-Layout (Aufgeteilt in 4 Quadranten)
        for (int port = 0; port < 4; port++) {
            int offsetX = (port % 2) * 160;
            int offsetY = (port / 2) * 105 + 30;

            struct controller_data keys_down = get_keys_down();
            struct controller_data held = get_keys_held();

            // Umrandung der Spieler-Boxen zeichnen
            graphics_draw_line(disp, offsetX, offsetY, offsetX + 155, offsetY, RGBA32(70, 70, 70, 255));
            
            char port_label[8];
            sprintf(port_label, "P%d", port + 1);
            graphics_draw_text(disp, offsetX + 5, offsetY + 5, port_label);

            int8_t stick_x = (port == 0) ? held.c.x : 0; 
            int8_t stick_y = (port == 0) ? held.c.y : 0;

            // =================================================================
            // NEUES FEATURE: CONTROLLER RESET FÜR EVERDRIVE (L + R + Z + START)
            // =================================================================
            if (held.c.L && held.c.R && held.c.Z && held.c.start) {
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
            if ((port == 0 && keys_down.c.start)) {
                reset_stats(port);
            }

            // FEATURE 2: RUMBLE PAK TEST LOGIK
            if ((port == 0) && held.c.A && held.c.B) {
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
            if (port == 0) {
                if (held.c.Z)       graphics_draw_text(disp, offsetX + 5, btnY, "Z (TRIGGER)");
                if (held.c.L)       graphics_draw_text(disp, offsetX + 5, btnY + 10, "L (FLIPPER)");
                if (held.c.R)       graphics_draw_text(disp, offsetX + 5, btnY + 20, "R (FLIPPER)");
                
                if (held.c.c_up || held.c.c_down || held.c.c_left || held.c.c_right) {
                    graphics_draw_text(disp, offsetX + 80, btnY, "C-PAD");
                }
                if (held.c.up || held.c.down || held.c.left || held.c.right) {
                    graphics_draw_text(disp, offsetX + 80, btnY + 10, "D-PAD");
                }
            }
        }

        // Info-Leiste unten aktualisiert mit dem Reset-Hinweis
        graphics_draw_text(disp, 15, 222, "START: Peak-Werte nullen");
        graphics_draw_text(disp, 15, 231, "L+R+Z+START: Zurueck zum Everdrive");

        display_show(disp);
    }
}
