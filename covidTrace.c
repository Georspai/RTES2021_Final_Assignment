//RTES 2021
//SPAIAS GEORGIOS
//AEM: 8910
//Potential solutions for time drift at lines 89,96,97(First way) and line 99(Second Way)
#include "CovidTrace.h"

#define FIN_TIME 25920 // in seconds
#define TEST_TIME 144 // in seconds
#define BT_EXPIRE_TIME 12
#define CLC_EXPIRE_TIME 12096
#define MIN_CLC_TIME 2.4



pthread_mutex_t  *timer_lock, *timer_lock_2 , *q_lock;
pthread_cond_t   *timer_cond_bt ,*timer_cond_covT ;

bool isTimerFinished;

FILE *f1;
FILE *f2;

int main(){

    queue * BT_queue=queueInit();
    if (BT_queue==NULL)
    {   fprintf(stderr, "main: BT_Queue Init failed.\n");
        exit(1);
    }

    queue * CLC_queue=queueInit();
    if (CLC_queue==NULL)
    {   fprintf(stderr, "main: CLC_Queue Init failed.\n");
        exit(1);
    }

    f1 = fopen("BTnearMe.bin", "wb");
    if (f1 == NULL) {
    printf("Error opening file1!\n");
    exit(1);
    }
    f2 = fopen("CLC_upload.txt", "w");
    if (f2 == NULL) {
    printf("Error opening file2!\n");
    exit(1);
    }
    fprintf(f2,"Close Contact Upload file \n");

    struct pthread_data pth_data;

    pth_data.BT_queue= BT_queue;
    pth_data.CLC_queue= CLC_queue;

    pth_data.BT_exp_time= BT_EXPIRE_TIME;
    pth_data.CLC_exp_time=CLC_EXPIRE_TIME;

    pthread_t tid_1, tid_2;

    timer_lock=(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(timer_lock, NULL);

    timer_lock_2=(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(timer_lock_2, NULL);

    q_lock=(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(q_lock, NULL);

    timer_cond_bt=(pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    pthread_cond_init(timer_cond_bt, NULL );

    timer_cond_covT=(pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    pthread_cond_init(timer_cond_covT, NULL );

    if(pthread_create(&tid_1,NULL,bt_thr_routine, &pth_data)!=0){
        perror("Error: could not create thread 1");
    }
    if(pthread_create(&tid_2,NULL,covT_thr_routine, &pth_data)!=0){
        perror("Error: could not create thread 2");
    }
    
    struct timeval block_start;
    struct timeval block_finish;
    *
    u_int count=0;
    isTimerFinished=false;
    Start=tic();
    while (count<(FIN_TIME*10)) 		//for ease of use we let the program run just for the time needed and not forever
    {   
      //gettimeofday(&block_start,NULL); 	//First way to reduce Time drift 
	pthread_cond_signal(timer_cond_bt);
        if (count%(TEST_TIME*10)==0)
        {
           pthread_cond_signal(timer_cond_covT);
        }
        count++ ;
      //gettimeofday(&block_finish,NULL);
      //usleep(100000-(block_finish.tv_sec-block_start.tv_sec)*1000000-(block_finish.tv_usec-block_start.tv_usec));    
        usleep(100000);
      //usleep(99800); 			  	//Second way to reduce time drift by substracting average delay (200us)
    }
    isTimerFinished=true;
    sleep(2);
    pthread_cond_signal(timer_cond_covT);
    sleep(2);
    pthread_cond_signal(timer_cond_bt);
    //printf("\n Timer Finished\n");


    if(pthread_join(tid_1,NULL)!=0)
    {
        perror("failed to join thread 1");
    }
     if(pthread_join(tid_2,NULL)!=0)
    {
        perror("failed to join thread 2");
    }

    pthread_mutex_destroy(timer_lock);
    pthread_mutex_destroy(timer_lock_2);
    pthread_mutex_destroy(q_lock);
    pthread_cond_destroy(timer_cond_bt);
    pthread_cond_destroy(timer_cond_covT);

    fclose(f1);
    fclose(f2);

    free(BT_queue);
    free(CLC_queue);
    return 0;
}

struct timeval tic()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv;
}

double toc(struct timeval begin)
{
  struct timeval end;
  gettimeofday(&end, NULL);
  double stime=((double)end.tv_sec-(double)begin.tv_sec)*1000 +
                ((double)end.tv_usec-(double)begin.tv_usec)/1000 ;
  stime= stime/1000;
  size_t fw=fwrite(&stime,sizeof(stime),1,f1);
  return stime;
}

void * bt_thr_routine(void * args)
{   struct pthread_data *pth=(struct pthread_data *)args;
    queue * BT_queue=pth->BT_queue;
    queue * CLC_queue=pth->CLC_queue;
    int bt_exp_t=pth->BT_exp_time;
    int clc_exp_t=pth->CLC_exp_time;
    u_int16_t *freq_t=pth->freq_mat;

    int count=0;
    macaddress mac_in;
    double time_in=0;

    while(1){
        pthread_cond_wait(timer_cond_bt,timer_lock);
        if (isTimerFinished)
        {
            break;
        }
        macaddress mac_in=BTnearMe();
        time_in=toc(Start);
        queueAdd(BT_queue,mac_in,time_in);
        queueUpdate(BT_queue,bt_exp_t);

        pthread_mutex_lock(q_lock);
        closeContactsAdd(BT_queue,CLC_queue);
        clc_freq(CLC_queue,freq_t);
        removeDuplicates(CLC_queue,freq_t);
        queueUpdate(CLC_queue,clc_exp_t);
        pthread_mutex_unlock(q_lock);

        count++;
       // printf("%d mac:0:0:0:0:%X:%X time: %f \n", count,BT_queue->mac_buffer[BT_queue->tail-1].bytes[1],BT_queue->mac_buffer[BT_queue->tail-1].bytes[0], time_in);
    }
}

void * covT_thr_routine(void * args)
{   struct pthread_data *pth=(struct pthread_data *)args;
    queue * clc_queue= pth->CLC_queue ;
    u_int16_t *freq_t=pth->freq_mat;
    while (1)
    {
        pthread_cond_wait(timer_cond_covT,timer_lock_2);
        if (isTimerFinished)
        {
            break;
        }
        if (testCOVID()==1)
        { pthread_mutex_lock(q_lock);
          uploadContacts(clc_queue->mac_buffer , clc_queue->entries);
	  pthread_mutex_unlock(q_lock);
        }
    }
    return (NULL);
}


macaddress BTnearMe()
{
    macaddress mac ;
    mac.bytes[0]=(unsigned char) (rand()%10);
    mac.bytes[1]=(unsigned char) (rand()%10);
    for (size_t i = 2; i < 6; i++)
    {
      mac.bytes[i]=0;
    }
    return mac;
}

bool testCOVID()
{
    u_int8_t neg_prob=rand()%5; // probability for positive test 1/5
    if (neg_prob>0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void uploadContacts(macaddress* mac_in ,int num)
{
    macaddress * mac;
    //mac=(macaddress *)malloc(num*sizeof(macaddress));
    mac=mac_in;

    fprintf(f2,"\n%d :",num);
    for (int i = 0; i < num; i++)
    {
    fprintf(f2," ");
    for (size_t k = 0; k <6; k++)
    {
       fprintf(f2,"%X",mac[i].bytes[5-k]);
    }
  }
}


queue * queueInit()
{
    queue *(q);
    q=(queue *)malloc(sizeof(queue));
    if (q==NULL)
    {
        return NULL ;
    }
    q->head=0;
    q->tail=0;
    q->entries=0;
    q->added=0;

    return q;
}

void queueAdd(queue * q, macaddress mac_in, double time_in)
{
    for (size_t i = 0; i < 6; i++)
    {
        *(q->mac_buffer[q->tail].bytes+i)=mac_in.bytes[i];
    }
    q->time_buffer[q->tail]=time_in;
    q->tail++;
    q->entries++;
    q->added++;
    if (q->tail==QUEUESIZE)
    {
        q->tail=0;
    }
    if (q->entries==QUEUESIZE)
    {
        printf("Warning: Queue is full!\n");
    }
}

void queueUpdate(queue *q,int exp_time)
{   int16_t last;
    if (q->tail==0){ last=QUEUESIZE-1; }
    else{ last=q->tail-1; }

    if (q->time_buffer[last]-q->time_buffer[q->head]>=exp_time)
    {   //printf("REMOVING: %d \n",q->head);
        q->head++ ;
        q->entries-- ;
        if(q->head==QUEUESIZE)
        {
            q->head=0;
        }
        if (q->entries==0)
        {
            printf("Warning: Queue is empty!\n");
        }
    }
}

void  closeContactsAdd(queue * q, queue * clc_q)
{
     while (q->added>0)
    {
      int i=q->head;
      while (i!=q->tail)
      {
        if (memcmp(q->mac_buffer[q->tail-1].bytes,q->mac_buffer[i].bytes,6)==0)
        {
          if(q->time_buffer[q->tail-1]- q->time_buffer[i]>MIN_CLC_TIME){
            queueAdd(clc_q,q->mac_buffer[q->tail-1],q->time_buffer[q->tail-1]);
            // printf("NEW CLOSE CONTACT %d:\n",clc_q->entries);
            // printf("mac:0:0:0:0:0:%X time: %f   %d\n",q->mac_buffer[q->tail-1].bytes[0],q->time_buffer[q->tail-1],(q->tail-1));
            // printf("mac:0:0:0:0:0:%X time: %f   %d\n",q->mac_buffer[i].bytes[0],q->time_buffer[i],i);
            break;
          }
        }
        i++;
        if (i==QUEUESIZE) i=0;
      }
      q->added--;
    }
}

void clc_freq(queue * q , u_int16_t *freq )
{
    int last;
    if (q->tail!=0) { last=q->tail-1;}
    else{ last=QUEUESIZE -1; }
    freq[last]=0;
    int i=q->head;
    while (i!=q->tail)
    {
        if (memcmp(q->mac_buffer[last].bytes,q->mac_buffer[i].bytes,6)==0) { freq[i]++; }
        i++;
        if (i==QUEUESIZE) {  i=0;}
    }
}

void removeDuplicates(queue * q,u_int16_t *freq)
{
    int i=q->head;
    int j=i;
    while(i!=q->tail)
    {
        if (freq[i]==1)
        {
            memcpy(q->mac_buffer[j].bytes , q->mac_buffer[i].bytes,6);
            q->time_buffer[j]=q->time_buffer[i];
            freq[j]=freq[i] ;
            j++;
            if (j==QUEUESIZE){ j=0 ;}
        }else
        {
            freq[i]=0;
            q->entries--;
        }
        i++;
        if (i==QUEUESIZE){ i=0 ;}
    }
    q->tail=j;
    //printf("ENTRIES LEFT %d \n",j);
}
