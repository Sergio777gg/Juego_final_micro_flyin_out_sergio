#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
// Include Sprites
#include "Sprite.h"

// Pines del TFT
#define TFT_DC 7
#define TFT_CS 6
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST 10
#define TFT_MISO 12

// Configuración de la pantalla TFT
Adafruit_ILI9341 screen = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

#define SPRITE_HEIGHT 16
#define SPRITE_WIDTH 16
#define SALTAR_BTN 18  // Botón para saltar
#define SCORE_MARGIN 30 // Espacio reservado para el puntaje

// Variables del juego
int game_state = 0; // 0: Pantalla de inicio, 1: En juego, 2: Game Over
int score = 0;
int high_score = 0;
int bird_x = 60;
int bird_y;
int momentum = 0;
int wall_x[2];
int wall_y[2];
int cloud_x[2]; // Posiciones horizontales de las nubes
int cloud_y[2]; // Posiciones verticales de las nubes
int wall_gap = 120; // Tamaño del hueco entre paredes
int wall_width = 20;
const int XMAX = 240;
const int YMAX = 320;

// Declaración de funciones
void drawSprite(int x, int y, const unsigned char* sprite);
void drawBoldTextCentered(int y, String txt);
void resetGame();

void setup() {
  Serial.begin(9600);
  screen.begin();
  pinMode(SALTAR_BTN, INPUT);

  randomSeed(analogRead(0));
  screen.fillScreen(ILI9341_BLUE);

  // Pantalla de inicio
  drawBoldTextCentered(YMAX / 2 - 20, "Flyin' out");
  drawBoldTextCentered(YMAX / 2 + 20, "Presione el boton");
  drawBoldTextCentered(YMAX / 2 + 40, "para iniciar");
  
}

void loop() {
  if (game_state == 0) {
    // Pantalla de inicio, espera a que se presione el botón para comenzar
    if (digitalRead(SALTAR_BTN) == HIGH) {
      resetGame();
      game_state = 1;
      delay(50); // Pausa para evitar rebote
    }

  } else if (game_state == 1) {
    // Borrar el área del pájaro antes de redibujarlo
    
    screen.fillRect(bird_x, bird_y, SPRITE_WIDTH, SPRITE_HEIGHT, ILI9341_BLUE);

    // Verifica si el botón de salto está presionado
    if (digitalRead(SALTAR_BTN) == HIGH) {
      momentum = -10; // El pájaro salta hacia arriba
    }

    momentum += 4; // Gravedad
    bird_y += momentum;

    // Restricciones en la posición vertical del pájaro
    if (bird_y < SCORE_MARGIN) bird_y = SCORE_MARGIN; // Evita que el pájaro suba al área de puntaje
    if (bird_y > YMAX - SPRITE_HEIGHT) {
      bird_y = YMAX - SPRITE_HEIGHT;
      momentum = -8;
    }

    // Dibujar el pájaro
    if (momentum < 0) {
      if (random(2) == 0) {
        drawSprite(bird_x, bird_y, wing_down_bmp);
      } else {
        drawSprite(bird_x, bird_y, wing_up_bmp);
      }
    } else {
      drawSprite(bird_x, bird_y, wing_up_bmp);
    }

    // Dibujar paredes y verificar colisiones
    for (int i = 0; i < 2; i++) {
      // Dibujar la pared superior (evitando el área de puntaje)
      screen.fillRect(0, SCORE_MARGIN, 20, 320, ILI9341_BLUE);
      screen.fillRect(wall_x[i], SCORE_MARGIN, wall_width, wall_y[i] - SCORE_MARGIN, ILI9341_GREEN);
      screen.fillRect(wall_x[i] + 20, SCORE_MARGIN, wall_width, wall_y[i] - SCORE_MARGIN, ILI9341_BLUE);

       // Dibujar la nube en medio del hueco
      //int nube_x = wall_x[i] + 48; // Centrar la nube en la tubería
      //int nube_y = wall_y[i] + wall_gap / 2 - 8; // Ubicar la nube en el centro del hueco
      //screen.fillRect(nube_x,nube_y - 60, 32, 16, ILI9341_WHITE);
      //screen.fillRect(nube_x + 38 ,nube_y -60, 32, 16, ILI9341_BLUE);
      // Dibujar la pared inferior
      screen.fillRect(wall_x[i], wall_y[i] + wall_gap, wall_width, YMAX - wall_y[i] - wall_gap, ILI9341_GREEN);
      screen.fillRect(wall_x[i]+20, wall_y[i] + wall_gap, wall_width, YMAX - wall_y[i] - wall_gap, ILI9341_BLUE);
      
      if (wall_x[i] < 0) {
        wall_y[i] = random(SCORE_MARGIN + 10, YMAX - wall_gap);  // Nueva altura aleatoria para el hueco
        wall_x[i] = XMAX;  // Reinicia la posición de la pared
      }

      // Puntaje cuando el pájaro pasa la pared
      if (wall_x[i] == bird_x) {
        score++;
        high_score = max(score, high_score);
      }

      // Detección de colisión
      if ((bird_x + SPRITE_WIDTH > wall_x[i] && bird_x < wall_x[i] + wall_width) &&
          (bird_y < wall_y[i] || bird_y + SPRITE_HEIGHT > wall_y[i] + wall_gap)) {
        delay(500);  // Pequeña pausa antes del Game Over
        game_state = 2; // Cambia al estado de Game Over
      }

      wall_x[i] -= 12;  // Movimiento de las paredes hacia la izquierda

      for (int i = 0; i < 2; i++) {
      cloud_x[i] -= 6; // La velocidad puede ajustarse según sea necesario
  
      // Reinicia la nube si sale de la pantalla
      if (cloud_x[i] < -32) { // Tamaño aproximado de la nube
       cloud_x[i] = XMAX; // Vuelve al extremo derecho de la pantalla
       cloud_y[i] = random(SCORE_MARGIN + 10, YMAX - 60); // Nueva posición aleatoria vertical
       }

      // Dibujar la nube
      screen.fillRect(cloud_x[i], cloud_y[i], 32, 16, ILI9341_WHITE);
      screen.fillRect(cloud_x[i]+30, cloud_y[i], 32, 16, ILI9341_BLUE);
}


    }

    // Mostrar puntaje en pantalla
    screen.fillRect(0, 0, XMAX, SCORE_MARGIN, ILI9341_BLACK); // Limpia el área de puntaje
    drawBoldTextCentered(5, "Score: " + String(score));
    delay(10);

  } else if (game_state == 2) {
    // Pantalla de Game Over
    screen.fillScreen(ILI9341_BLACK);
    drawBoldTextCentered(YMAX / 2, "GAME OVER");
    drawBoldTextCentered(YMAX / 2 + 20, "Score: " + String(score));
    drawBoldTextCentered(YMAX / 2 + 40, "High Score: " + String(high_score));
    drawBoldTextCentered(YMAX / 2 + 60, "Presione para ");
    drawBoldTextCentered(YMAX / 2 + 80, " reiniciar ");
    

    // Reinicia el juego cuando se suelta el botón
    if (digitalRead(SALTAR_BTN) == LOW) {
      delay(200); // Pausa para evitar rebote
      game_state = 0;
    }
  }
}

// Función para reiniciar el juego
void resetGame() {
  bird_y = YMAX / 2;
  momentum = -4;
  wall_x[0] = XMAX;
  wall_y[0] = YMAX / 2 - wall_gap / 2;
  wall_x[1] = XMAX + XMAX / 2;
  wall_y[1] = YMAX / 2 - wall_gap / 2;
  cloud_x[0] = wall_x[0] + 48; // Inicialmente en el centro de la primera tubería
  cloud_y[0] = wall_y[0] + wall_gap / 2 - 8;

  cloud_x[1] = wall_x[1] + 48; // Inicialmente en el centro de la segunda tubería
  cloud_y[1] = wall_y[1] + wall_gap / 2 - 8;

  score = 0;
  screen.fillScreen(ILI9341_BLUE); // Limpia la pantalla para el nuevo juego
}

// Función para dibujar el sprite
void drawSprite(int x, int y, const unsigned char* sprite) {
  screen.drawBitmap(x, y, sprite, SPRITE_WIDTH, SPRITE_HEIGHT, ILI9341_YELLOW);
}

// Función para centrar el texto en la pantalla
void drawBoldTextCentered(int y, String txt) {
  int x = XMAX / 2 - txt.length() * 6;
  screen.setCursor(x, y);
  screen.setTextColor(ILI9341_WHITE);
  screen.setTextSize(2);
  screen.print(txt);
}




