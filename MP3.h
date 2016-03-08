#ifndef __MP3_H_
#define __MP3_H_

#define MUSIC_FOLDER "songs"

typedef enum            //������״̬
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

/*��������˫������*/
typedef struct song
{
     char   sname[50];
     struct song *prev;
     struct song *next;	
}*SONG;


typedef struct player
{
    char song_name[50]; //��ǰ���Ÿ�����
    int  cur_num;       //��ǰ���ŵ��ڼ���
    int  total_num;     //��������
    int  voice;         //����
    int  cur_time;      //��ǰ����ʱ��
    int  total_time;    //�ܵĲ���ʱ��
    PLAY_STAT stat;     //�������ĵ�ǰ״̬
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