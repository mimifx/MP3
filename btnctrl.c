#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "btnctl.h"
#include "MP3.h"

extern PLAYER mp3_player;
int run = 1;

int ctrl_loop(void)
{
    int fd;
    int key_num = 0;
    
    fd = open(CTRL_BTN_DEV,O_RDONLY);
    if(fd==-1)
    {
    	perror("open file fail");
    	return -1;
    }
      
    while(run)
    {
    	read(fd,&key_num,4);
    
	    switch(key_num)
	    {
	    	case BTN_REWIND:   //快退
	    	break;
	    	
	    	case BTN_PREV:     //前一首
	    	{
	    	    mp3_prev();
	    	    mp3_ui();
	    	}
	    	break;
	    	
	    	case BTN_PLAY_PAUSE: //播放/暂停
	    	{
	    	    if( PLAY == mp3_player->stat )
	    	    {
	    	    	mp3_pause();
	    	    }
	    	    else
	    	    {
	    	    	mp3_play();
	    	    }
	    	    mp3_ui();
	    	}
	    	break;
	    	
	    	case BTN_NEXT:  //下一首
	    	{
	    	    mp3_next();
	    	    mp3_ui();
	    	}
	    	break;
	    	
	    	case BTN_FASTFW: //快进
	    	break;
	    	 
	    	case BTN_STOP:  //停止
	    	{
	    	    mp3_stop();
	    	    while(mp3_player->stat != EXIT )
	    	    usleep(100);
	    	    run = 0;
	    	}
	    	break;
	    	
	    	case VOICE_UP:
	    	{
	    	    mp3_voice_up();	
	    	   // mp3_fastforward();
	    	}
	    	break;
	    	
	    	case VOICE_DOWN:
	    	{
	    	    mp3_voice_down();
	    	   // mp3_rewind();
	    	}
	    	break;
	    	
	    	default:
	    	break;
	    }
    } 
    
    close(fd);
    return 0;
}