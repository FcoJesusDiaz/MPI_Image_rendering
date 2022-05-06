/* Pract2  RAP 09/10    Javier Ayllon*/
//#include <openmpi/mpi.h>
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h> 
#include <assert.h>   
#include <unistd.h>  
#include <time.h> 

#include "definitions.h"

/*Variables Globales */

XColor colorX;
Colormap mapacolor;
char cadenaColor[]="#000000";
Display *dpy;
Window w;
GC gc;

/*Funciones auxiliares */

void initX() {

      dpy = XOpenDisplay(NIL);
      assert(dpy);

      int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
      int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

      w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0,
                                    400, 400, 0, blackColor, blackColor);
      XSelectInput(dpy, w, StructureNotifyMask);
      XMapWindow(dpy, w);
      gc = XCreateGC(dpy, w, 0, NIL);
      XSetForeground(dpy, gc, whiteColor);
      for(;;) {
            XEvent e;
            XNextEvent(dpy, &e);
            if (e.type == MapNotify)
                  break;
      }


      mapacolor = DefaultColormap(dpy, 0);

}

void dibujaPunto(int x,int y, int r, int g, int b) {

      sprintf(cadenaColor,"#%.2X%.2X%.2X",r,g,b);
      XParseColor(dpy, mapacolor, cadenaColor, &colorX);
      XAllocColor(dpy, mapacolor, &colorX);
      XSetForeground(dpy, gc, colorX.pixel);
      XDrawPoint(dpy, w, gc,x,y);
      XFlush(dpy);
}



/* Programa principal */

int main (int argc, char *argv[]) {

      int rank,size;
      MPI_Comm commPadre;
      MPI_Status status;
      int punto[5];


      MPI_Init(&argc, &argv);
      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
      MPI_Comm_size(MPI_COMM_WORLD, &size);
      MPI_Comm_get_parent(&commPadre);

      if ((commPadre == MPI_COMM_NULL) && (rank == 0)){
            initX();
            /* Codigo del maestro */
            MPI_Comm_spawn("exec/pract2", MPI_ARGV_NULL, N_HIJOS, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &commPadre, NULL);
            
            //MPI_Request request; /*For non-blocking call*/
            double begin = MPI_Wtime();

            for(int i = 0; i <  TOTAL_FILAS * TOTAL_COLUMNAS; i++){
                  //NON BLOCKING CALL
                  /*MPI_Irecv(punto, 5, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, commPadre, &request);
                  if(MPI_Wait(&request, MPI_STATUS_IGNORE) != MPI_SUCCESS){
                        printf("RANK 0: Some error ocurred on the send or receive operation\n");
                  }*/

                  //BLOCKING CALL
                  MPI_Recv(punto, 5, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, commPadre, &status);
                  
                  printf("Puntos recibidos %d --> x = %d, y = %d, r = %d, g = %d, b = %d\n", (i+1),
                              punto[0], punto[1], punto[2], punto[3], punto[4]);

                  dibujaPunto(punto[0], punto[1], punto[2], punto[3], punto[4]);
            }
            printf("Imagen completada. Tiempo de computo: %.3f\n", MPI_Wtime()-begin);
            sleep(2);
      }     
      else {
            /* Codigo de todos los trabajadores */
            /* El archivo sobre el que debemos trabajar es foto.dat */
            int n_filas = TOTAL_FILAS/N_HIJOS;
            int bloque = n_filas * TOTAL_COLUMNAS * 3 * sizeof(unsigned char);
            int disp = rank * bloque;
            int fila_inicial = n_filas * rank;
            int fila_final =(rank == (N_HIJOS - 1)) ? TOTAL_FILAS : fila_inicial + n_filas;

            printf("HIjo con rango %d creado, disp: %d, n_filas: %d, fila_final: %d\n", rank, disp, n_filas, fila_final);
            MPI_File manejador;
            MPI_File_open(MPI_COMM_WORLD, FILENAME, MPI_MODE_RDONLY, MPI_INFO_NULL, &manejador);
            MPI_File_set_view(manejador, disp, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, "native", MPI_INFO_NULL);


            unsigned char color[3];
            //MPI_Request request; /*For non-blocking call*/
            for(int i = fila_inicial; i < fila_final; i++){
                  for(int j = 0; j < TOTAL_COLUMNAS; j++){
                        punto[0] = j;
                        punto[1] = i;
                        MPI_File_read(manejador, color, 3, MPI_UNSIGNED_CHAR, &status);
                        punto[2] = (int)color[0];
                        punto[3] = (int)color[1];
                        punto[4] = (int)color[2];
                        
                        //NON BLOCKING CALL
                        /*MPI_Isend(punto, 5, MPI_INT, 0, 1, commPadre, &request);

                        if(MPI_Wait(&request, MPI_STATUS_IGNORE) != MPI_SUCCESS){
                        printf("RANK %d: Some error ocurred on the send or receive operation\n", rank);
                        }*/

                        /*BLOCKING CALL*/
                        MPI_Send(punto, 5, MPI_INT, 0, 1, commPadre);

                  }     
            }

            MPI_File_close(&manejador);
      }

      MPI_Finalize();
      return EXIT_SUCCESS;
}

