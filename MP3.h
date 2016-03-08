#ifndef __MP3_H_
#define __MP3_H_

#define MUSIC_FOLDER "songs"

typedef enum            //播放器状态
{
    PLAY    = 0,
    PAUSE   = 1,
    PREV    = 2,
    NEXT    = 3,
    STOP    = 4,
    FASTFW  = 5,
    FREWIND = 6,
    EXIT    = 7,
}PLAY_STAT;

/*歌曲链表（双向链表）*/
typedef struct song
{
     char   sname[50];
     struct song *prev;
     struct song *next;	
}*SONG;


typedef struct player
{
    char song_name[50]; //当前播放歌曲名
    int  cur_num;       //当前播放到第几首
    int  total_num;     //歌曲总数
    int  voice;         //音量
    int  cur_time;      //当前播放时间
    int  total_time;    //总的播放时间
    PLAY_STAT stat;     //播放器的当前状态
}*PLAYER;



void mp3_init(void);
void sigint_handler(int sig);
void *player(void *arg);
void mp3_play(void);
void mp3_pause(void);
void mp3_stop(void);
void mp3_prev(void);
void mp3_next(void);
void mp3_rewind(void);
void mp3_fastforward(void);
void mp3_ui(void);
void mp3_voice_up(void);
void mp3_voice_down(void);
void get_total_plytime(char *song_name,int *ply_time);
void update_cur_plytime(void);

#endif