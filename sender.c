/*

  yay - fast and simple yuv viewer

  (c) 2005-2010 by Matthias Wientapper
  (m.wientapper@gmx.de)

  Support of multiple formats added by Cuero Bugot.
 
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
//#include "regex.h"
//#include <inttypes.h>
#include <stdint.h>

//from Sender.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
//end of from Sender.c

/*
#include "SDL.h"
//#include <SDL/SDL.h>

SDL_Surface     *screen;
SDL_Event       event;
SDL_Rect        video_rect;
SDL_Overlay     *my_overlay;
const SDL_VideoInfo* info = NULL;
*/
uint32_t width = 0;
uint32_t height = 0;
char *vfilename; 
FILE *fpointer;
FILE *output;
uint8_t *y_data, *cr_data, *cb_data, *tmp_data;
uint8_t *y_data_abrupt_cmp, *cr_data_abrupt_cmp, *cb_data_abrupt_cmp;
uint8_t *y_data_gradual_cmp, *cr_data_gradual_cmp, *cb_data_gradual_cmp;
uint16_t zoom = 1;
uint16_t min_zoom = 1;
uint16_t frame = 0;
uint16_t scene_chane_frame_number = 1;
uint16_t quit = 0;
uint8_t grid = 0;
uint8_t bpp = 0;
int cfidc = 1;//format; decided by optarg's sscanf()
int sp = 0;//if semi-planar or not


static const uint8_t SubWidthC[4] =
  {
    0, 2, 2, 1
  };
static const uint8_t SubHeightC[4] =
  {
    0, 2, 1, 1
  };
static const uint8_t SubSizeC[4] =
  {
    0, 4, 2, 1
  };
static const uint8_t MbWidthC[4] =
  {
    0, 8, 8, 16
  };
static const uint8_t MbHeightC[4] =
  {
    0, 8, 16, 16
  };
static const uint8_t FrameSize2C[4] =
  {
    2, 3, 4, 6
  };

#define PORT "7000"
#define MAXDATASIZE 25344+5
  //Declare variables for socket setup, used for both TCP & UDP connection
  const char *connection_option = "DGRAM";
  const char *dest_IP = "127.0.0.1";
  int sockfd;
  struct addrinfo hints, *receiver_info;
  int rv;
  char s[INET_ADDRSTRLEN];

  //Declare variables for file transfer
  unsigned char buffer[MAXDATASIZE];
  unsigned char *buffer_beg_ptr, *buffer_data_ptr;

  //Declare variables used in UDP file transfer only
  int packet_counter;
  uint32_t HEADER = 0xaaaaaaaa;
  uint32_t MAGICNUM = 0xdeadbeef;
  int last_packet;


int load_frame(){

  uint32_t cnt;  
  /* Fill in video data */
  cnt = fread(buffer_data_ptr, 1, width*height, fpointer);//printf("Y-len=%d ",strlen(y_data));// int i;printf("y_data\n");for(i=0; i<width*height;i++)printf("%x",y_data[i]);
  //fprintf(stderr,"read %d y bytes\n", cnt);
  if(cnt < width*height){
    return 0;
  }
  else if (cfidc>0)
    {
      uint32_t HEADER_netorder = htonl(HEADER);
      memcpy(&buffer[0], (void *)&HEADER_netorder, (size_t)sizeof HEADER);
      sendto(sockfd, buffer_beg_ptr, cnt+4, 0, receiver_info->ai_addr, receiver_info->ai_addrlen);
      if(!sp) {
        cnt = fread(buffer_data_ptr, 1, height * width / SubSizeC[cfidc], fpointer);//printf("U-len=%d ",strlen(cb_data));//printf("cb_data\n");for(i=0; i<height * width / SubSizeC[cfidc];i++)printf("%x",cb_data[i]);
          // fprintf(stderr,"read %d cb bytes\n", cnt);
        if(cnt < width * height / 4){
        	return 0;
        } else {

            uint32_t HEADER_netorder = htonl(HEADER);
            memcpy(&buffer[0], (void *)&HEADER_netorder, (size_t)sizeof HEADER);
            sendto(sockfd, buffer_beg_ptr, cnt+4, 0, receiver_info->ai_addr, receiver_info->ai_addrlen);

            cnt = fread(buffer_data_ptr, 1, height * width / SubSizeC[cfidc], fpointer);//printf("V-len=%d\n",strlen(cr_data));//printf("cr_data\n");for(i=0; i<height * width / SubSizeC[cfidc];i++)printf("%x",cr_data[i]);
            // fprintf(stderr,"read %d cr bytes\n", cnt);
            if(cnt < width * height / 4){
                return 0;
            } else {
              uint32_t HEADER_netorder = htonl(HEADER);
              memcpy(&buffer[0], (void *)&HEADER_netorder, (size_t)sizeof HEADER);
              sendto(sockfd, buffer_beg_ptr, cnt+4, 0, receiver_info->ai_addr, receiver_info->ai_addrlen);
            }
        }
      } else {
        int sz = height * width / SubSizeC[cfidc];
        int tmp_sz = sz * 2;
        int i;
        cnt = fread(tmp_data, 1, tmp_sz, fpointer);
        if(cnt < tmp_sz) {
            return 0;
        }

        for(i = 0; i < sz; i++) {
            cb_data[i] = tmp_data[i * 2];
            cr_data[i] = tmp_data[i * 2 + 1];
        }
      }
    }
  return 1;
}
void copy_frame_for_abrupt_scp()
{
  memcpy ( y_data_abrupt_cmp, y_data, strlen(y_data) );
  memcpy ( cr_data_abrupt_cmp, cr_data, strlen(cr_data) );
  memcpy ( cb_data_abrupt_cmp, cb_data, strlen(cb_data) );
}
void copy_frame_for_gradual_scp()
{
  memcpy ( y_data_gradual_cmp, y_data, strlen(y_data) );
  memcpy ( cr_data_gradual_cmp, cr_data, strlen(cr_data) );
  memcpy ( cb_data_gradual_cmp, cb_data, strlen(cb_data) );
}
void blocks_calc(uint8_t *y_data_blocks, uint8_t *cr_data_blocks, uint8_t *cb_data_blocks, uint8_t *y_data_src, uint8_t *cr_data_src, uint8_t *cb_data_src)
{
  int i,j,ii,jj;

  //Y-block
  for (i = 0; i < height/4; ++i)
  {
    for (j = 0; j < width/4; ++j)
    {
      y_data_blocks[ i*(width/4)+j ] = 0;
      for (ii = 0; ii < 4; ++ii)
      {
        for (jj = 0; jj < 4; ++jj)
        {
          y_data_blocks[ i*(width/4)+j ] += y_data_src[ ((4*i+ii)*width)+(4*j+jj) ];
        }
      }
      y_data_blocks[ i*(width/4)+j ] = y_data_blocks[ i*(width/4)+j ]/16;
    }
  }
  //U-block and V-block
  for (i = 0; i < height/8; ++i)
  {
    for (j = 0; j < width/8; ++j)
    {
      cr_data_blocks[ i*(width/8)+j ] = 0;
      cb_data_blocks[ i*(width/8)+j ] = 0;
      for (ii = 0; ii < 4; ++ii)
      {
        for (jj = 0; jj < 4; ++jj)
        {
          cr_data_blocks[ i*(width/8)+j ] += cr_data_src[ ((4*i+ii)*width/2)+(4*j+jj) ];
          cb_data_blocks[ i*(width/8)+j ] += cb_data_src[ ((4*i+ii)*width/2)+(4*j+jj) ];
        }
      }
      cr_data_blocks[ i*(width/8)+j ] = cr_data_blocks[ i*(width/8)+j ]/16;
      cb_data_blocks[ i*(width/8)+j ] = cb_data_blocks[ i*(width/8)+j ]/16;
    }
  }
}
#define THRESHHOLD_ABRUPT  1.5
#define THRESHHOLD_GRADUAL 2.1
#define ABRUPT  0
#define GRADUAL 1
int grater_than_threshhold(int mode, float a, float b, float c)
{
  int count=0;
  float threshold;
  switch (mode)
  {
    case ABRUPT:
      threshold = THRESHHOLD_ABRUPT;
      break;
    case GRADUAL:
      threshold = THRESHHOLD_GRADUAL;
      break;
  }
  if (a > threshold) count++;
  if (b > threshold) count++;
  if (c > threshold) count++;
  if (mode==ABRUPT && count >1)
    return 1;
  else if (mode==GRADUAL && count >0)
    return 1;
  else
    return 0;
}

void ratio_calc()
{
  //initialize the variables
  uint8_t abrupt_scene_change_detected=0;
  int i,j;
  int Y_SASD=0, U_SASD=0,V_SASD=0;
  int Y_SATD=0, U_SATD=0, V_SATD=0;  
  uint8_t *y_data_blocks_cmp, *cr_data_blocks_cmp, *cb_data_blocks_cmp;
  y_data_blocks_cmp  = malloc( (width * height * sizeof(uint8_t))/16 );
  if (cfidc > 0)
    {
      cr_data_blocks_cmp = malloc( (width * height * sizeof(uint8_t) / SubSizeC[cfidc])/16 );
      cb_data_blocks_cmp = malloc( (width * height * sizeof(uint8_t) / SubSizeC[cfidc])/16 );
    }
  uint8_t *y_data_blocks, *cr_data_blocks, *cb_data_blocks;
  y_data_blocks  = malloc( (width * height * sizeof(uint8_t))/16 );
  if (cfidc > 0)
    {
      cr_data_blocks = malloc( (width * height * sizeof(uint8_t) / SubSizeC[cfidc])/16 );
      cb_data_blocks = malloc( (width * height * sizeof(uint8_t) / SubSizeC[cfidc])/16 );
    }

  // calculate the blocks of "n-th frame"
  blocks_calc(y_data_blocks, cr_data_blocks, cb_data_blocks, y_data, cr_data, cb_data);
  //Y-SASD
  for (i = 1; i < (height/4)-1; ++i)
  {
    for (j = 1; j < (width/4)-1; ++j)
    {
      Y_SASD += ( abs(y_data_blocks[ i*(width/4)+j ] - y_data_blocks[ (i+1)*(width/4)+j ]) + abs(y_data_blocks[ i*(width/4)+j ] - y_data_blocks[ (i-1)*(width/4)+j ]) + abs(y_data_blocks[ i*(width/4)+j ] - y_data_blocks[ i*(width/4)+(j+1) ]) + abs(y_data_blocks[ i*(width/4)+j ] - y_data_blocks[ i*(width/4)+(j-1) ]) + abs(y_data_blocks[ i*(width/4)+j ] - y_data_blocks[ (i-1)*(width/4)+(j-1) ]) + abs(y_data_blocks[ i*(width/4)+j ] - y_data_blocks[ (i+1)*(width/4)+(j-1) ]) + abs(y_data_blocks[ i*(width/4)+j ] - y_data_blocks[ (i+1)*(width/4)+(j+1) ]) + abs(y_data_blocks[ i*(width/4)+j ] - y_data_blocks[ (i-1)*(width/4)+(j-1) ]) )/8;

    }
  }
  //U-SASD and V-SASD
  for (i = 1; i < (height/8)-1; ++i)
  {
    for (j = 1; j < (width/8)-1; ++j)
    {
      U_SASD += ( abs(cr_data_blocks[ i*(width/8)+j ] - cr_data_blocks[ (i+1)*(width/8)+j ]) + abs(cr_data_blocks[ i*(width/8)+j ] - cr_data_blocks[ (i-1)*(width/8)+j ]) + abs(cr_data_blocks[ i*(width/8)+j ] - cr_data_blocks[ i*(width/8)+(j+1) ]) + abs(cr_data_blocks[ i*(width/8)+j ] - cr_data_blocks[ i*(width/8)+(j-1) ]) + abs(cr_data_blocks[ i*(width/8)+j ] - cr_data_blocks[ (i-1)*(width/8)+(j-1) ]) + abs(cr_data_blocks[ i*(width/8)+j ] - cr_data_blocks[ (i+1)*(width/8)+(j-1) ]) + abs(cr_data_blocks[ i*(width/8)+j ] - cr_data_blocks[ (i+1)*(width/8)+(j+1) ]) + abs(cr_data_blocks[ i*(width/8)+j ] - cr_data_blocks[ (i-1)*(width/8)+(j-1) ]) )/8;
      V_SASD += ( abs(cb_data_blocks[ i*(width/8)+j ] - cb_data_blocks[ (i+1)*(width/8)+j ]) + abs(cb_data_blocks[ i*(width/8)+j ] - cb_data_blocks[ (i-1)*(width/8)+j ]) + abs(cb_data_blocks[ i*(width/8)+j ] - cb_data_blocks[ i*(width/8)+(j+1) ]) + abs(cb_data_blocks[ i*(width/8)+j ] - cb_data_blocks[ i*(width/8)+(j-1) ]) + abs(cb_data_blocks[ i*(width/8)+j ] - cb_data_blocks[ (i-1)*(width/8)+(j-1) ]) + abs(cb_data_blocks[ i*(width/8)+j ] - cb_data_blocks[ (i+1)*(width/8)+(j-1) ]) + abs(cb_data_blocks[ i*(width/8)+j ] - cb_data_blocks[ (i+1)*(width/8)+(j+1) ]) + abs(cb_data_blocks[ i*(width/8)+j ] - cb_data_blocks[ (i-1)*(width/8)+(j-1) ]) )/8;
    }
  }
  printf("Y-SASD = %d, U-SASD = %d, V-SASD = %d\n",Y_SASD, U_SASD, V_SASD);


/********************************************
*        detect abrupt scene change         *
********************************************/

  // calculate the blocks of "(n-1)-th frame"
  blocks_calc(y_data_blocks_cmp, cr_data_blocks_cmp, cb_data_blocks_cmp, y_data_abrupt_cmp, cr_data_abrupt_cmp, cb_data_abrupt_cmp);
  //Y-SATD
  for (i = 1; i < (height/4)-1; ++i)
  {
    for (j = 1; j < (width/4)-1; ++j)
    {
      Y_SATD += abs( y_data_blocks[ i*(width/4)+j ] - y_data_blocks_cmp[ i*(width/4)+j ] );
    }
  }
  //U-SATD and V-SATD
  for (i = 1; i < (height/8)-1; ++i)
  {
    for (j = 1; j < (width/8)-1; ++j)
    {
      U_SATD += abs( cr_data_blocks[ i*(width/8)+j ] - cr_data_blocks_cmp[ i*(width/8)+j ] );
      V_SATD += abs( cb_data_blocks[ i*(width/8)+j ] - cb_data_blocks_cmp[ i*(width/8)+j ] );
    }
  }
  printf("abrupt scene change detection: Y-SATD = %d, U-SATD = %d, V-SATD = %d\n",Y_SATD, U_SATD, V_SATD);
  printf("  the ratio: SATD / SASD is %f, %f, %f \n", (float)Y_SATD/Y_SASD, (float)U_SATD/U_SASD, (float)V_SATD/V_SASD);

  if ( grater_than_threshhold(ABRUPT, (float)Y_SATD/Y_SASD, (float)U_SATD/U_SASD, (float)V_SATD/V_SASD ) )
  {
    scene_chane_frame_number = frame;
    output = fopen("output.txt", "a");
    fprintf(output, "%d Abrupt\n", frame);
    fclose(output);
    abrupt_scene_change_detected=1;
    printf("      ****************  abrupt scene change detected  ****************\n");
  }



/********************************************
*        detect gradul scene change         *
********************************************/
  if (!abrupt_scene_change_detected)
  {
    // calculate the blocks of "the first frame of this scene"
    blocks_calc(y_data_blocks_cmp, cr_data_blocks_cmp, cb_data_blocks_cmp, y_data_gradual_cmp, cr_data_gradual_cmp, cb_data_gradual_cmp);
    Y_SATD=0, U_SATD=0, V_SATD=0;
    //Y-SATD
    for (i = 1; i < (height/4)-1; ++i)
    {
      for (j = 1; j < (width/4)-1; ++j)
      {
        Y_SATD += abs( y_data_blocks[ i*(width/4)+j ] - y_data_blocks_cmp[ i*(width/4)+j ] );
      }
    }
    //U-SATD and V-SATD
    for (i = 1; i < (height/8)-1; ++i)
    {
      for (j = 1; j < (width/8)-1; ++j)
      {
        U_SATD += abs( cr_data_blocks[ i*(width/8)+j ] - cr_data_blocks_cmp[ i*(width/8)+j ] );
        V_SATD += abs( cb_data_blocks[ i*(width/8)+j ] - cb_data_blocks_cmp[ i*(width/8)+j ] );
      }
    }
    printf("gradul scene change detection: Y-SATD = %d, U-SATD = %d, V-SATD = %d\n",Y_SATD, U_SATD, V_SATD);
    printf("  the ratio: SATD / SASD is %f, %f, %f \n", (float)Y_SATD/Y_SASD, (float)U_SATD/U_SASD, (float)V_SATD/V_SASD);

    if ( grater_than_threshhold(GRADUAL, (float)Y_SATD/Y_SASD, (float)U_SATD/U_SASD, (float)V_SATD/V_SASD ) )
    {
      scene_chane_frame_number = frame;
      output = fopen("output.txt", "a");
      fprintf(output, "%d Gradual\n", frame);
      fclose(output);
      printf("      ****************  gradul scene change detected  ****************\n");
    }
  }
}

/*
void convert_chroma_to_420()
{
  int i, j;
  //printf("%dx%d\n",width, height);
  if (cfidc >0)
    {
      for(j=0; j<height/2; j++)
	for(i=0; i<width/2; i++)
	  {
	    my_overlay->pixels[1][j*my_overlay->pitches[1]+i] = cr_data[i*MbWidthC[cfidc]/8+j*(width/SubWidthC[cfidc])*MbHeightC[cfidc]/8];
	    my_overlay->pixels[2][j*my_overlay->pitches[2]+i] = cb_data[i*MbWidthC[cfidc]/8+j*(width/SubWidthC[cfidc])*MbHeightC[cfidc]/8];
	  }
    }
  else
    {
      for (i = 0; i < height/2; i++)
	{
	  memset(my_overlay->pixels[1]+i*my_overlay->pitches[1], 128, width/2); 
	  memset(my_overlay->pixels[2]+i*my_overlay->pitches[2], 128, width/2); 
	}

    }
}
*//*
void draw_frame(){ 
  Sint16 x, y;
  Uint16 i;
  
  // Fill in pixel data - the pitches array contains the length of a line in each plane
  SDL_LockYUVOverlay(my_overlay);
  
  // we cannot be sure, that buffers are contiguous in memory
  if (width != my_overlay->pitches[0]) {
    for (i = 0; i < height; i++) {
      memcpy(my_overlay->pixels[0]+i*my_overlay->pitches[0], y_data+i*width, width); 
    }
  } else {
    memcpy(my_overlay->pixels[0], y_data, width*height);
  }

  if (cfidc == 1)
    {
      if (width != my_overlay->pitches[1]) {
	for (i = 0; i < height/2; i++) {
	  memcpy(my_overlay->pixels[1]+i*my_overlay->pitches[1], cr_data+i*width/2, width/2); 
	}
      } else {
	memcpy(my_overlay->pixels[1], cr_data, width*height/4);
      }

      if (width != my_overlay->pitches[2]) {
	for (i = 0; i < height/2; i++) {
	  memcpy(my_overlay->pixels[2]+i*my_overlay->pitches[2], cb_data+i*width/2, width/2); 
	}
      } else {
	memcpy(my_overlay->pixels[2], cb_data, width*height/4);
      }
    }
  convert_chroma_to_420();

  if(grid){
   
    // horizontal grid lines
    for(y=0; y<height; y=y+16){
      for(x=0; x<width; x+=8){
	*(my_overlay->pixels[0] + y   * my_overlay->pitches[0] + x  ) = 0xF0;
	*(my_overlay->pixels[0] + y   * my_overlay->pitches[0] + x+4  ) = 0x20;
      }
    }
    // vertical grid lines
    for(x=0; x<width; x=x+16){
      for(y=0; y<height; y+=8){
	*(my_overlay->pixels[0] + y   * my_overlay->pitches[0] + x  ) = 0xF0;
	*(my_overlay->pixels[0] + (y+4)   * my_overlay->pitches[0] + x  ) = 0x20;
      }
    }
  }

  SDL_UnlockYUVOverlay(my_overlay);

  video_rect.x = 0;
  video_rect.y = 0;
  video_rect.w = width*zoom;
  video_rect.h = height*zoom;

  SDL_DisplayYUVOverlay(my_overlay, &video_rect);
}
*/

//Get the socket address, IPv4 or IPv6 (taken from Beej's guide)
/*If the sa_family field is AF_INET (IPv4), return the IPv4 address. Otherwise return the IPv6 address.*/
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int socket_setting(){

  //Loading struct addrinfo hints
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;

  hints.ai_socktype = SOCK_DGRAM;
  printf("Establish a UDP connection\n");

  //Get your target's (the receiver's) address information
  if ((rv = getaddrinfo(dest_IP, PORT, &hints,
                        &receiver_info)) != 0) {
      perror("Unable to get receiver's address info\n");
      return 2;
  }
    //Take fields from first record in receiver_info, and create socket from it. (Beej's guide loops through all nodes to find an available one, I just take the first one)
  if ((sockfd = socket(receiver_info->ai_family,receiver_info->ai_socktype,receiver_info->ai_protocol)) == -1) {
      perror("Unable to create socket for the Receiver\n");
      return 3;
  }

  //Print out the IP address of the receiver that the sender is connecting to
  inet_ntop(receiver_info->ai_family, get_in_addr((struct sockaddr*)receiver_info->ai_addr), s, sizeof s);
  printf("Sender: sending data to %s\n", s);

  /*Each packet that is sent out will have a 4-byte hex header, and a 1020-byte payload (the real file data). buffer_beg_ptr points to the beginning of the buffer where the hex header will go, and buffer_data_ptr points to the index where the rest of the payload will begin.*/

  buffer_beg_ptr = &buffer[0]; //pointer to position of 4-byte packet header
  buffer_data_ptr = &buffer[4]; //pointer to position of payload
        


}

void send_end_signal()
{
    //Once we've reached the end (bytes_left = 0), we send out a last packet with the hex header oxDEADBEEF to let the receiver know that we are done.
    uint32_t MAGICNUM_netorder = htonl(MAGICNUM);
    memcpy(&buffer[0], (void *)&MAGICNUM_netorder, (size_t)sizeof MAGICNUM);
    last_packet = sendto(sockfd, buffer_beg_ptr, MAXDATASIZE, 0, receiver_info->ai_addr, receiver_info->ai_addrlen);
    close(sockfd);
    return;
}
void print_usage(){
  fprintf(stdout, "Usage: scd [-s <widht>x<heigh>] [-f format] [-p] filename.yuv\n\t format can be: 0-Y only, 1-YUV420, 2-YUV422, 3-YUV444\n\t specify '-p' to enable semi-planar mode\n");
}


int main(int argc, char *argv[])
{
  int     opt;
  //char    caption[32];
  regex_t reg;
  regmatch_t pm;
  int     result;
  char    picsize[32]="";
  int     used_s_opt = 0;
  //int     play_yuv = 0;
  //unsigned int start_ticks = 0;
  //uint32_t vflags;

  if (argc == 1) {
    print_usage();
    return 1;
  } else {
    while((opt = getopt(argc, argv, "f:s:p")) != -1)
      switch(opt){
      case 's':
        if (sscanf(optarg, "%dx%d", &width, &height) != 2) {
          fprintf(stdout, "No geometry information provided by -s parameter.\n");
          return 1;
	}
	used_s_opt = 1;
	break;
      case 'f':
	if (sscanf(optarg, "%d", &cfidc) != 1 || (cfidc<0 && cfidc>3)) {
	  fprintf(stdout, "Invalid format provided by -f parameter.\n");
	  return 1;
	}
	break;
      case 'p':
        // Enable semi-planar mode
        sp = 1;
        break;
      default:
	print_usage();
	return 1;
	break;
      }
  }
  argv += optind;
  argc -= optind;
  
  vfilename = argv[0];
  
  if(!used_s_opt) {
    // try to find picture size from filename or path
    if (regcomp(&reg, "_[0-9]+x[0-9]+", REG_EXTENDED) != 0) return -1;
    result = regexec(&reg, vfilename, 1, &pm, REG_NOTBOL);
    if(result == 0){
      strncpy(picsize, (vfilename + pm.rm_so + 1), (pm.rm_eo - pm.rm_so -1 ));
      strcat(picsize, "\0");
    }
    if (sscanf(picsize, "%dx%d", &width, &height) != 2) {
      fprintf(stdout, "No geometry information found in path/filename.\nPlease use -s <width>x<height> paramter.\n");
      return 1;
    }
  }
  // some WM can't handle small windows...
  if (width < 100){
    zoom = 2;
    min_zoom = 2;
  }
  //printf("using x=%d y=%d\n", width, height);
/*  
  // SDL init
  if(SDL_Init(SDL_INIT_VIDEO) < 0){ 
    fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
    exit(1);
  }
  atexit(SDL_Quit); 
  
  info = SDL_GetVideoInfo();
  if( !info ) 
    {      fprintf(stderr, "SDL ERROR Video query failed: %s\n", SDL_GetError() );
      SDL_Quit(); exit(0); 
    }
  
  bpp = info->vfmt->BitsPerPixel;
  if(info->hw_available)
    vflags = SDL_HWSURFACE;
  else	
    vflags = SDL_SWSURFACE;
  
  if( (screen = SDL_SetVideoMode(width*zoom, height*zoom, bpp,
				 vflags)) == 0 ) 
    {       
      fprintf(stderr, "SDL ERROR Video mode set failed: %s\n", SDL_GetError() );
      SDL_Quit(); exit(0); 
    }
 */ 
  
  // DEBUG output
  // printf("SDL Video mode set successfully. \nbbp: %d\nHW: %d\nWM: %d\n", 
  // 	info->vfmt->BitsPerPixel, info->hw_available, info->wm_available);
  
/*
  SDL_EnableKeyRepeat(500, 10);

  my_overlay = SDL_CreateYUVOverlay(width, height, SDL_YV12_OVERLAY, screen);
  if(!my_overlay){ //Couldn't create overlay?
    fprintf(stderr, "Couldn't create overlay\n"); //Output to stderr and quit
    exit(1);
  }
*/
  /* should allocate memory for y_data, cr_data, cb_data here */
  y_data  = malloc(width * height * sizeof(uint8_t));
  if (cfidc > 0)
    {
      cb_data = malloc(width * height * sizeof(uint8_t) / SubSizeC[cfidc]);
      cr_data = malloc(width * height * sizeof(uint8_t) / SubSizeC[cfidc]);
    }

  y_data_abrupt_cmp  = malloc(width * height * sizeof(uint8_t));
  if (cfidc > 0)
    {
      cb_data_abrupt_cmp = malloc(width * height * sizeof(uint8_t) / SubSizeC[cfidc]);
      cr_data_abrupt_cmp = malloc(width * height * sizeof(uint8_t) / SubSizeC[cfidc]);
    }

  y_data_gradual_cmp  = malloc(width * height * sizeof(uint8_t));
  if (cfidc > 0)
    {
      cb_data_gradual_cmp = malloc(width * height * sizeof(uint8_t) / SubSizeC[cfidc]);
      cr_data_gradual_cmp = malloc(width * height * sizeof(uint8_t) / SubSizeC[cfidc]);
    }

  fpointer = fopen(vfilename, "rb"); 
  if (fpointer == NULL){
    fprintf(stderr, "Error opening %s\n", vfilename);
    return 1;
  }
  // send event to display first frame
/*  event.type = SDL_KEYDOWN;
  event.key.keysym.sym = SDLK_RIGHT;
  SDL_PushEvent(&event);
*//*
  // main loop
  while (!quit){
    sprintf(caption, "frame %d, zoom=%d", frame, zoom);
    SDL_WM_SetCaption( caption, NULL );

    // wait for SDL event
    SDL_WaitEvent(&event);

    switch(event.type)
      {
      case SDL_KEYDOWN: 
	switch(event.key.keysym.sym)
	  {
	  case SDLK_SPACE:
	    {
	      play_yuv = 1; // play it, sam!
	      while(play_yuv){
		start_ticks = SDL_GetTicks();
		sprintf(caption, "frame %d, zoom=%d", frame, zoom);
		SDL_WM_SetCaption( caption, NULL );

		// check for next frame existing
		if(load_frame()){
		  draw_frame();
		  //insert delay for real time viewing
		  if(SDL_GetTicks() - start_ticks < 40)
		    SDL_Delay(40 - (SDL_GetTicks() - start_ticks));
		  frame++;
		} else {
		  play_yuv = 0;
		}
		// check for any key event
		if(SDL_PollEvent(&event)){
		  if(event.type == SDL_KEYDOWN){
		    // stop playing
		    play_yuv = 0;
		  }
		}
	      }
	      break;
	    }
	  case SDLK_RIGHT: 
	    {
	      // check for next frame existing
	      if(load_frame()){
		draw_frame();
		frame++;
	      }

	      break;
	    } 
	  case SDLK_BACKSPACE:
	  case SDLK_LEFT:
	    { 
	      if(frame>1){
		frame--;
		fseek(fpointer, ((frame-1)*height*width*FrameSize2C[cfidc])/2 , SEEK_SET);
		//if(draw_frame())
		load_frame();
		draw_frame();
	      }
	      break;
	    }
	  case SDLK_UP:
	    {
	      zoom++;
	      screen = SDL_SetVideoMode(width*zoom, height*zoom, bpp,
					vflags); 
	      video_rect.w = width*zoom;
	      video_rect.h = height*zoom;
	      SDL_DisplayYUVOverlay(my_overlay, &video_rect);
	      break;
	    }
	  case SDLK_DOWN:
	    {
	      if(zoom>min_zoom){
		zoom--;
		screen = SDL_SetVideoMode(width*zoom, height*zoom, bpp,
					  vflags); 
		video_rect.w = width*zoom;
		video_rect.h = height*zoom;
		SDL_DisplayYUVOverlay(my_overlay, &video_rect);
	      }
	      break;
	    }
	  case SDLK_r:
	    { 
	      if(frame>1){
		frame=1;
		fseek(fpointer, 0, SEEK_SET);
		//if(draw_frame())
		load_frame();
		draw_frame();
	      }
	      break;
	    }
	  case SDLK_g:
	    grid = ~grid;
	    draw_frame();
	    break;
	  case SDLK_q:
	    quit = 1;
	    break;
	  case SDLK_f:
	    SDL_WM_ToggleFullScreen(screen);
	    break;
	  default:
	    break;
	  } // switch key
	break;
      case SDL_QUIT:
	quit = 1;
	break;
      case SDL_VIDEOEXPOSE:
	SDL_DisplayYUVOverlay(my_overlay, &video_rect);
	break;
      default:
	break;

      } // switch event type

  } // while
  // clean up
  SDL_FreeYUVOverlay(my_overlay); */
  socket_setting();
  while(load_frame()){
      printf("  ===== frame %d =====\n", ++frame);
      //if (frame==2)break;
      /*if (frame >1)
      {
        ratio_calc();
      }
      copy_frame_for_abrupt_scp();
      if (frame == scene_chane_frame_number)
        copy_frame_for_gradual_scp();*/
  }
  send_end_signal();

  free(y_data);printf("  ===== ff1 \n");
  free(cb_data);printf("  ===== ff2 \n");
  free(cr_data);printf("  ===== ff3 \n");
  free(y_data_abrupt_cmp);printf("  ===== ff4 \n");
  free(cb_data_abrupt_cmp);printf("  ===== ff5 \n");
  free(cr_data_abrupt_cmp);printf("  ===== ff6 \n");
  free(y_data_gradual_cmp);printf("  ===== ff7 \n");
  free(cb_data_gradual_cmp);printf("  ===== ff8 \n");
  /*free(cr_data_gradual_cmp);*/printf("  ===== ff9\n");
  if (sp)
    free(tmp_data);
  fclose(fpointer);
  if (!used_s_opt)
    regfree(&reg);

  return 0;
}

// vim: tabstop=4 expandtab
