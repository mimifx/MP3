#include <stdio.h>
#include <pthread.h>
#include "MP3.h"
#include "btnctl.h"

pthread_t thread;
pthread_mutex_t player_mutex;
pthread_cond_t player_ready;

int main()
{
    mp3_init();
    
    pthread_create(&thread,NULL,player,0);
    pthread_mutex_init(&player_mutex,NULL);
    pthread_cond_init(&player_ready,NULL);
    
    ctrl_loop();//¼àÌý°´¼ü
    
    pthread_mutex_destroy(&player_mutex);
    pthread_cond_destroy(&player_ready);
    
    return 0;
}