#include <cstdlib>
#include <ctime>
#include <sstream>
#include <string>
#include <iostream>
#include <windows.h>
#include <iomanip>
#include "miniwin.h"

using namespace std;
using namespace miniwin;

const int TAM = 25; 
const int FILAS = 20;
const int COLUMNAS = 10;

typedef int Tablero[COLUMNAS][FILAS]; //Para saber dónde hay o no bloques

struct Coord { int x, y; }; //Creación de una dupla, para usar coordenadas x y y

struct Pieza {
   Coord orig;     // Rectángulo central 
   Coord perif[3]; // Bloques periféricos
   int color;
   Coord posicion(int n) const; // Dada una pieza entre 0 y 3 (0 = central, 1 a 3 = perif.)
};

Coord Pieza::posicion(int n) const {  //Devolver las coordenadas	
   Coord ret = { orig.x, orig.y };
   if (n != 0) {
      ret.x += perif[n-1].x;
      ret.y += perif[n-1].y;
   }
   return ret;
}

Coord rota_derecha(const Coord& c) { //Rotación de coordenadas
   Coord ret = { -c.y, c.x };
   return ret;
}

const Coord perifs[7][3] = {     // Coordenadas de las piezas
   { { 1,0 }, { 0,1 }, { 1,1 } }, // Cuadrado
   { { 1,0 }, {-1,1 }, { 0,1 } }, // S
   { { 0,1 }, { 1,1 }, {-1,0 } }, // Dos
   { { 0,1 }, { 0,-1}, { 1,1 } }, // L
   { { 0,1 }, { 0,-1}, {-1,1 } }, // L al revés
   { {-1,0 }, { 1,0 }, { 0,1 } }, // T
   { { 0,1 }, { 0,-1}, { 0,2 } }, // Palo
};

string a_string(int puntos) {
   stringstream sout;
   sout << puntos;
   return sout.str();
}

const int puntos_limite[5] = { //Niveles
   100,200,300,400,500
};

const int tics_nivel[5] = { //Escala de velocidades con respecto al nivel
   33, 20, 12, 8, 2
};

//Funciones por prototipo 
void cuadrado(int x, int y);
void rota_derecha(Pieza& P);
void pinta_pieza(const Pieza& P);
void pieza_nueva(Pieza& P);
void tablero_vacia(Tablero& T);
void tablero_incrusta_pieza(Tablero& T, const Pieza& P);
void tablero_pinta(const Tablero& T);
int tablero_colapsa(Tablero& T, int fila);
int tablero_cuenta_lineas(Tablero& T, int &puntos);
bool tablero_colision(const Tablero& T, const Pieza& P);
bool tablero_fila_llena(const Tablero& T, int fila);
void repinta(const Tablero& T, const Pieza& p, const Pieza& sig,
             int puntos, int nivel, int vidas);
void winner();
void game_over();

int main() {
   vredimensiona(TAM * COLUMNAS + 220, TAM * FILAS + 100); //Dimensión de la pantalla
   srand(time(0)); 
   const int ancho = TAM * COLUMNAS;
   const int alto  = TAM * FILAS;
   int tic = 0, puntos = 0, nivel = 0, vidas=0;
   bool pausa=false;
   Tablero T;
   tablero_vacia(T);
   Pieza c, sig;
   pieza_nueva(c);
   pieza_nueva(sig);
   c.orig.x = 5;
   repinta(T, c, sig, puntos, nivel, vidas);

   int t = tecla();
   while (t != ESCAPE) { //Se inicia el juego, con ESCAPE sales 
      // 0. Copiar la pieza actual
      Pieza copia = c;

      if (t == NINGUNA && tic > tics_nivel[nivel]) { //Movimiento automático de las piezas 
		 tic = 0;
         if(pausa==false){
         	t=ABAJO;
		 }
      }else if(t==ESPACIO){ //Pausa con tecla ESPACIO
      	 if(pausa==true){
      	 	pausa=false;
	   	 }else{
			pausa=true;
			vidas++;
		 } 
      	 if (t == ARRIBA){    //Rota en pausa
         	rota_derecha(c);
		}
	  }
	  
      //Movimiento de las teclas con teclado
      if (t == ABAJO && pausa==false) {            							//Mover hacia abajo
		 c.orig.y++;
      } else if (t == ARRIBA && vidas<4 || t==ARRIBA && pausa==false)  {    //Rotar hacia la derecha
         rota_derecha(c);
      } else if (t == DERECHA && vidas<4 || t==DERECHA && pausa==false) {   //Mover hacia la derecha
         c.orig.x++;
      } else if (t == IZQUIERDA && vidas<4 || t==IZQUIERDA && pausa==false) { //Mover hacia la izquierda
         c.orig.x--;
  	  } 
	  
      //Detectar las colisiones
      if (tablero_colision(T, c)) {
         c = copia; 
         if (t == ABAJO) {
            tablero_incrusta_pieza(T, c);
            int cont = tablero_cuenta_lineas(T, puntos); //Si una línea se borra, se suman los puntos
			
		//Si la última pieza en incrustarse es roja, se borra la pantalla y se suman los puntos de esa línea
			if (cont>0 && c.color==1){
	   			tablero_vacia(T);
			}
			
            if (puntos >= puntos_limite[nivel]) { //Si se tiene la cantidad de puntos requeridos o más, se sube de nivel
               nivel++;
            }
            c = sig;
            pieza_nueva(sig);
            c.orig.x = 5;
            if (tablero_colision(T, c)) { //Si alguna pieza llega al límite superior de tablero, el juego acaba
               game_over();
            }
         }
      }
	if(nivel==4){ //Al llegar al nivel 5, ganas (nivel empieza en 0)
		repinta(T, c, sig, puntos, nivel, vidas);
        winner(); 
	}
      //Repinta
      if (t != NINGUNA) { 
         repinta(T, c, sig, puntos, nivel, vidas);
      }
      espera(30);
      tic++;
      t = tecla();
   }
   
   vcierra(); //Cerrar ventana
   return 0;
}

//Definición de funciones
void cuadrado(int x, int y) {            //CREACIÓN DE LOS RECTANGULOS
   rectangulo_lleno(20 + 1 + x * TAM,    //LE SUMAMOS 20 A LA IZQUIERDA PARA APARENTAR UN BORDE EN EL TABLRERO
                    20 + 1 + y * TAM,    //LE SUMAMOS 1 PARA AÑADIR UN MINI BORDE ENTRE CADA RECTÁNGULO
                    20 + x * TAM + TAM,
                    20 + y * TAM + TAM);
}

void rota_derecha(Pieza& P) {    //ROTAR COORDENADAS DE LOS PERIFÉRICOS 
   for (int i = 0; i < 3; i++) {
      P.perif[i] = rota_derecha(P.perif[i]);
   }
}

void pinta_pieza(const Pieza& P) { //PINTAR PIEZAS
   color(P.color);
   for (int i = 0; i < 4; i++) {
      Coord c = P.posicion(i);
      cuadrado(c.x, c.y);
   }
}

void pieza_nueva(Pieza& P) { //GENERAR PIEZAS NUEVAS AL AZAR Y ASIGNA COLORES
  P.orig.x = 12;
  P.orig.y = 2;
  P.color = 1 + rand() % 7;
  int r = rand() % 7;
  for (int i = 0; i < 3; i++) {
     P.perif[i] = perifs[r][i];
  }
}

void tablero_vacia(Tablero& T) {     //ASIGNAR EL COLOR NEGRO
   for (int i = 0; i < COLUMNAS; i++) {
      for (int j = 0; j < FILAS; j++) {
         T[i][j] = NEGRO;               //SIGNIFICA QUE LA CASILLA ESTÁ VACÍA
      }
   }
}
 void tablero_incrusta_pieza(Tablero& T, const Pieza& P) { //INCRUSTRAMOS LAS PIEZAS 
   for (int i = 0; i < 4; i++) {
      Coord c = P.posicion(i);
      T[c.x][c.y] = P.color;
   }
}

void tablero_pinta(const Tablero& T) { //PINTAR TODAS LAS CASILLAS DE NEGRO
   for (int i = 0; i < COLUMNAS; i++) {
      for (int j = 0; j < FILAS; j++) {
         color(T[i][j]);
         cuadrado(i, j);
      }
   }
}

int tablero_colapsa(Tablero& T, int fila) { //BAJA FILAS CUANDO ALGUNA COLAPSE
	char soundfile[] = "C:\Fila.wav";       //SONIDO CUANDO UNA FILA COLAPSA
    cout<<PlaySound((LPCSTR)soundfile,NULL,SND_FILENAME | SND_ASYNC);
    int puntos=0;
    for (int i = 0; i < COLUMNAS; i++) {
     puntos+= T[i][fila];
   }
    
   // COPIAR LO DE ARRIBA HACIA ABAJO
   for (int j = fila; j > 0; j--) {
      for (int i = 0; i < COLUMNAS; i++) {
         T[i][j] = T[i][j-1];
      }
   }
   
   // VACIAR LA DE ARRIBA
   for (int i = 0; i < COLUMNAS; i++) {
      T[i][0] = NEGRO;
   }
   return puntos+1;
}

int tablero_cuenta_lineas(Tablero& T, int &puntos) {
   int fila = FILAS - 1, cont = 0;
   while (fila >= 0) {
      if (tablero_fila_llena(T, fila)) {
         puntos+=tablero_colapsa(T, fila);
         cont++; 
      } else {
         fila--;
      }
   }
   return cont;
}

bool tablero_colision(const Tablero& T, const Pieza& P) {
   for (int i = 0; i < 4; i++) {
      Coord c = P.posicion(i);
      // COMPROBAR LOS LÍMITES
      if (c.x < 0 || c.x >= COLUMNAS) { //SI NO ENTRA EN NINGUN IF, NO HAY COLISIÓN
         return true;
      }
      if (c.y < 0 || c.y >= FILAS) {
         return true;
      }
      
      if (T[c.x][c.y] != NEGRO) {
         return true;
      }
   }
   return false;
}
bool tablero_fila_llena(const Tablero& T, int fila) { //SABER SI UNA FILA ESTÁ LLENA
   for (int i = 0; i < COLUMNAS; i++) {
      if (T[i][fila] == NEGRO) return false;
   }
   return true;
}

void repinta(const Tablero& T, const Pieza& p, const Pieza& sig, //PERSONALIZA EL TABLERO
             int puntos, int nivel, int vidas)
{
   const int ancho = TAM * COLUMNAS;
   const int alto  = TAM * FILAS;
   borra();
   tablero_pinta(T);
   color_rgb(128, 128, 128);
   linea(20, 20, 20, 20 + alto);
   linea(20, 20 + alto,
         20 + ancho, 20 + alto);
   linea(20 + ancho, 20 + alto,
         20 + ancho, 20);
    color_rgb(0,255,0);
   texto(40 + ancho, 20, "Pieza siguiente");
   texto(40 + ancho, 150, "Nivel");
   texto(40 + ancho, 250, "Puntos");
   texto(40 + ancho, 350, "Pausas");
   color(BLANCO);
   texto(40 + ancho, 270, a_string(puntos));
   texto(40 + ancho, 170, a_string(nivel + 1));
   texto(40 + ancho, 370, a_string(vidas));
   pinta_pieza(p);
   pinta_pieza(sig);
   refresca();
}

void winner() {       //SE GANA EL JUEGO Y EMITE SONIDO
	char soundfile[] = "C:\Bomba.wav";
    cout<<PlaySound((LPCSTR)soundfile,NULL,SND_FILENAME | SND_ASYNC); 
   color(BLANCO);
   texto(140, 240, "GANASTE!");
   refresca();
   espera(10000);
}

void game_over() {     //FINALIZAR EL JUEGO AL PERDER Y PONER UN SONIDO
	char soundfile[] = "C:\Over.wav"; 
    cout<<PlaySound((LPCSTR)soundfile,NULL,SND_FILENAME | SND_ASYNC);
   color(BLANCO);
   texto(140, 240, "GAME OVER!");
   refresca();
   espera(10000);
}
