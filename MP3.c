#include "MP3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

SONG SL;
SONG sp; 
PLAYER mp3_player;

extern pthread_t thread;
extern pthread_mutex_t player_mutex;
extern pthread_cond_t player_ready;
pid_t player_pid;
FILE *madplay_tmp_fp;

void mp3_init(void)
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    
    SL = (SONG)malloc(sizeof(struct song)); //创建链表
    SL->prev = NULL;
    SL->next = NULL;
    
    mp3_player = (PLAYER)malloc(sizeof(struct player)); //创建一个播放器
    mp3_player->cur_num = 0;
    mp3_player->total_num = 0;
    mp3_player->stat = STOP;
    mp3_player->voice = 50;
    mp3_player->cur_time = 0;
    mp3_player->total_time = 0;
    
    sp = SL;
           
    system("ls songs > songs_list");
    
    fp = fopen("songs_list", "r");
    
    if(NULL == fp)
    {
    	perror("open file songs_list fail");
    	return;
    }
    
    madplay_tmp_fp = fopen("/tmp/madplay_tmp_file", "w+"); //打开记录歌曲播放时间的文件
     
     
    printf("\n-------------MP3 Player------------\n");
    while ((read = getline(&line, &len, fp)) != -1) 
    {
    	 SONG s = (SONG)malloc(sizeof(struct song));    //插入
    	 if(NULL==s)continue;
    	 s->next = NULL;
    	 
    	 strcpy(s->sname,line); //保存歌曲名
    	 s->sname[ strlen(s->sname)-1 ]='\0';
    	 
    	 s->prev = sp;	 
    	 sp->next = s;
    	 sp = s;
    	 
    	 mp3_player->total_num++;
    	 
         printf("*  %s", line);
    }
    
    mp3_player->cur_num = 1;
    
    sp = SL->next;
    sprintf(mp3_player->song_name,"%s/%s",MUSIC_FOLDER,sp->sname);
    
    printf("\n-----BY GDUFSCS XCG 2016/2/27------"); 
    printf("\nS2:REWIND  S3:PREV  S4:PLAY/STOP  S5:NEXT  S6:STOP\n");
     
    if (line)  free(line);
    fclose(fp);
            
}

void sigint_handler(int sig)
{
    if (SIGINT == sig) {
        if (player_pid)
        { 
            kill(SIGKILL, player_pid);
            printf("kill player!\n");
        }
        exit(0);
    }
}

/*播放器主线程*/
void *player(void *arg)
{ 
    signal(SIGINT, sigint_handler);
    while(1)
    {
    	if(mp3_player->stat == PLAY || mp3_player->stat == PAUSE)
    	{
    	    
    	    player_pid = fork();
    	    if(-1 == player_pid) //ERR_PRINT_EXT(EXIT_FAILURE);
    	    return;
    	    if(0 == player_pid)
    	    {
    	    	char t[20];
    	    	sprintf(t, "%02d:%02d", (mp3_player->cur_time / 1000) / 60,(mp3_player->cur_time / 1000) % 60);
    	    	
    	    	if(execl("/sbin/madplay", "madplay","-s",t,mp3_player->song_name,NULL))
    	    	{
    	    	    //ERR_PRINT_EXT(EXIT_FAILURE);
    	    	    return;
    	    	}    	     
                            
    	    }
    	    else
    	    {
    	    	waitpid(player_pid,NULL, 0);
    	    	printf("play end!\n");
    	    }
    	    
    	}
    	else
    	{
    	    pthread_mutex_lock(&player_mutex);
    	    
    	    while(mp3_player->stat == STOP || mp3_player->stat == FASTFW || mp3_player->stat == FREWIND )
    	        pthread_cond_wait(&player_ready,&player_mutex);
    	        
    	    pthread_mutex_unlock(&player_mutex);
    	}
    }
    
    return NULL;
}




/*播放*/
void mp3_play(void)
{
    char cmd[100];
    
    if( PAUSE == mp3_player->stat)
    {
        sprintf(cmd,"kill -SIGCONT %d",player_pid);
        system(cmd);
    }
    else
    {
    	get_total_plytime(mp3_player->song_name,&(mp3_player->total_time));//获取播放时间
    	mp3_player->cur_time = 0;
    	
        pthread_mutex_lock(&player_mutex);
    	pthread_cond_signal(&player_ready);
    	pthread_mutex_unlock(&player_mutex);
    	
    }
    mp3_player->stat = PLAY;
}
/*暂停*/
void mp3_pause(void)
{
    char cmd[100];
    if( PLAY == mp3_player->stat)
    {
    	sprintf(cmd,"kill -SIGSTOP %d",player_pid);
        system(cmd);
        mp3_player->stat = PAUSE;
    }
}

/*停止*/
void mp3_stop(void)
{
    char cmd[100];
    if( PLAY == mp3_player->stat || PAUSE == mp3_player->stat )
    {
    	sprintf(cmd,"kill -SIGKILL %d",player_pid);
        system(cmd);
        mp3_player->stat = EXIT;
        fclose(madplay_tmp_fp);
    }
}


/*上一首*/
void mp3_prev(void)
{
    if( mp3_player->cur_num <= 1 ) 
    {
    	printf("Not preview one!\n");
    }
    else
    {
        mp3_stop();
    
        sp = sp->prev;                //指向当前播放的音乐
        //获取上一首音乐
        sprintf(mp3_player->song_name,"%s/%s",MUSIC_FOLDER,sp->sname);
        
        mp3_player->cur_num --;
        mp3_player->stat = NEXT;
    
        mp3_play();
    }
}
/*下一首*/
void mp3_next(void)
{
    if( mp3_player->total_num == mp3_player->cur_num ) 
    {
    	printf("Not next one!\n");
    }
    else
    {
	mp3_stop();
	
	sp = sp->next;         //指向当前播放的音乐   
	 //获取下一首音乐
	sprintf(mp3_player->song_name,"%s/%s",MUSIC_FOLDER,sp->sname);
	
	mp3_player->cur_num ++;
	mp3_player->stat = PREV;
	    
	mp3_play();
    }
    
}
/*快退*/
void mp3_rewind(void)
{
    if( mp3_player->stat == PLAY )
    {
    	mp3_pause();
    }
    
    update_cur_plytime();
    
    if( mp3_player->cur_time <= 0 )
    {
    	perror("Has reached the end of the song!");
    	return;
    }
    
    mp3_player->cur_time -= 500;
    
    fwrite(&(mp3_player->cur_time),4,1,madplay_tmp_fp);
    
    mp3_play();
    
}

/*快进*/
void mp3_fastforward(void)
{
    if( mp3_player->stat == PLAY )
    {
    	mp3_pause();
    }
    
    update_cur_plytime();
    
    if( mp3_player->cur_time >=mp3_player->total_time )
    {
    	perror("Has reached the end of the song!");
    	return;
    }
    
    mp3_player->cur_time += 1000;
    
    fwrite(&(mp3_player->cur_time),4,1,madplay_tmp_fp);
    
    mp3_play();
}


/*音量+*/
void mp3_voice_up(void)
{
    char cmd[100];
    if(mp3_player->voice<100)
    {
    	mp3_player->voice = mp3_player->voice + 5;
    	
    	sprintf(cmd, "amixer cset name='PCM Playback Volume' %d%% > /dev/null ",mp3_player->voice);
    	system(cmd);
    }
    else
    {
    	printf("current voice is 100%,can't bigger anymore!\n");
    }
}

/*音量-*/
void mp3_voice_down(void)
{
     char cmd[100];
    if(mp3_player->voice>0)
    {
    	mp3_player->voice = mp3_player->voice - 5 ;
    	sprintf(cmd, "amixer cset name='PCM Playback Volume' %d%% > /dev/null ",mp3_player->voice);
    	system(cmd);
    }
    else
    {
    	printf("current voice is 0%,can't smaller anymore!\n");
    }
}


   
/*获取播放时间*/
void get_total_plytime(char *song_name,int *ply_time)
{
    char buf[30];
    pid_t pid = fork();
    
    if( pid == -1 ) 
    {
    	perror("fork");
    	return;
    }
    
    if( pid == 0 )
    {
    	execl("/sbin/madplay", "madplay","-QI",song_name,NULL);
    	
    }
    else
    {
    	waitpid(pid,NULL,0);
    	fgets(buf, 30,madplay_tmp_fp);
    	*ply_time = atoi(buf) * 1000;
    	
    } 
}


/*更新播放时间*/
void update_cur_plytime(void)
{
    char buf[30];
    fflush(madplay_tmp_fp);
    fseek(madplay_tmp_fp, 0, SEEK_SET);
    if (NULL==fgets(buf, 30, madplay_tmp_fp))return;
 
    mp3_player->cur_time = atoi(buf);
}

void mp3_ui(void) //播放器界面
{
    system("clear"); //清屏
    printf("\n-------------MP3 Player------------\n");
    SONG ui;
    ui = SL->next;
    while( NULL != ui )
    {
    	printf("*  %s\n", ui->sname);
    	ui = ui->next;
    }
    printf("*  \n");
    printf("current playing:  %s\n", mp3_player->song_name);
    printf("-----BY GDUFSCS XCG 2016/2/27------"); 
    printf("\nS2:REWIND  S3:PREV  S4:PLAY/STOP  S5:NEXT  S6:STOP\n");
}
