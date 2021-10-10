#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>
#include <string.h>

#define QUEUESIZE 1000

/*Struct: macaddress
* Struct to store the 6 bytes (48bits) of  a macaddresses
*/
typedef struct macaddress {
    u_char bytes[6];
} macaddress;

/*Struct: queue
* Struct to implement queues
*/
typedef struct queue{
    macaddress mac_buffer[QUEUESIZE];
    double     time_buffer[QUEUESIZE];
    u_int16_t head,tail,entries,added;
} queue;



/*Struct: pthread_data
* Struct to store the data that are passed into the thread routines
*/
struct pthread_data
{
    queue * BT_queue;
    queue * CLC_queue;
    u_int16_t freq_mat[QUEUESIZE];
    int BT_exp_time;
    int CLC_exp_time;
} pthread_data;

struct timeval Start;

/*Function: BTnearMe
*   Returns a  macaddress 0:0:0:0:X:Y where X,Y are random ints between 0 and 9.
*   (Substitute for function that returns the macadress of a bluetooth device in close proximity)
*/
macaddress BTnearMe();

/*Function: testCovid
*   Returns TRUE when rand%X==0 and FALSE when rand%X>0 where X is a number
*   we chose to give certain probability to simulate the results of a covid test.
*   (Substitute for function that returns the TRUE if the user's covid Test is positive)
*/
bool testCOVID();

/*Function: uploadContacts
*   Writes the mac adresses of the user's close contactts and the number of close contacts in
*   a .txt file (CLC_upload.txt)
*   (Substitute for function that uploads the close contacts' mac addresses to some corresponding service)
*
*   Inputs: mac address array of close contacts ( macaddress* mac)
*           the number of the user's close contacts
*/
void uploadContacts(macaddress* mac ,int num);

/*Function: clc_freq
*   Computes and updates the frequency that its value exists in the clc_queue
*
*   Inputs: queue which we wanna compute the frequency ( queue * q)
*           Table of the frequencies corresponding to the input queue (uint_16t *freq)
*/
void clc_freq(queue * q , u_int16_t *freq );

/*Function: removeDuplicates
*   Deletes duplicate eentries of the input queue based on their frequency
*
*   Inputs: queue which we examine ( queue * q)
*           Table of the frequencies corresponding to the input queue (uint_16t *freq)
*/
void removeDuplicates(queue * q,u_int16_t *freq);

struct timeval tic();

double toc(struct timeval begin);

/*Function: bt_thr_routine
*   Routine of the first thread (tid_1)
*/
void* bt_thr_routine(void* args);

/*Function: covT_thr_routine
*   Routine of the first thread (tid_2)
*/
void* covT_thr_routine(void* args);


/*Function: queueInit
*  Initializes the queue
*/
queue * queueInit();

/*Function: queueAdd
*   Adds a new entry in the tail of the queue
*
*   Inputs: the queue in which we add the new entry (queue * q)
*           macaddress data of new entry            (macaddress * mac_in)
*           time data of new entry                  ( double time_in)
*/
void queueAdd(queue * q,macaddress mac_in, double time_in);

/*Function: queueUpdate
*   removes the head entry of the queue if it has expired
*
*   Inputs: the queue we examine                     (queue * q)
*           expiration time specific to the queue    (int exp_time)
*/
void queueUpdate(queue *q,int exp_time);

/*Function: closeContactsAdd
*   checks if the tail entry of q qualifies as a cloes contact and if so
*   adds it to the clc_queue
*   Inputs: the queue we examine                     (queue * q)
*           the queue we add to                      (queue * clc_q)
*/
void closeContactsAdd(queue * q,queue *clc_q);
