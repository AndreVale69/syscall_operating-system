/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "defines.h"
#include "err_exit.h"
#include "fifo.h"
#include "semaphore.h"
#include "shared_memory.h"

// set of signals
sigset_t original_set_signals;
sigset_t new_set_signals;

// manipulation of a signal & mod signal mask
void sigHandler(int sig);
void set_original_mask();
void create_signal_mask();
int semid;
pid_t pid;

int main(int argc, char * argv[]) {
    /**************
     * CHECK ARGS *
     **************/

    if (argc != 2)
        errExit("Intended usage: ./client_0 <HOME>/myDir/");

    // pointer to the inserted path
    char *path_to_dir = argv[1];


    /**********************
     * CREATE SIGNAL MASK *
     **********************/

    create_signal_mask();

    // attend a signal...
    pause();

    // blocking all blockable signals
    sigfillset(&new_set_signals);
    if(sigprocmask(SIG_SETMASK, &new_set_signals, NULL) == -1)
        errExit("sigprocmask(original_set) failed");


    /*****************
     * OBTAINING IDs *
     *****************/

    // try to obtain set id of semaphores
    // 
    do {
        printf("Looking for the semaphore...\n\n");
        semid = semget(ftok("client_0", 'a'), 0, S_IRUSR | S_IWUSR);
        if (semid == -1)
            sleep(2);
    } while(semid == -1);

    // waiting for IPCs to be created
    semop_usr(semid, 0, -1);
    
    // opening of all the IPC's
    int fifo1_fd = open_fifo("FIFO1", O_WRONLY);
    int fifo2_fd = open_fifo("FIFO2", O_WRONLY);
    int queue_id = msgget(ftok("client_0", 'a'), S_IRUSR | S_IWUSR);
    int shmem_id = alloc_shared_memory(ftok("client_0", 'a'), sizeof(struct queue_msg) * 50, S_IRUSR | S_IWUSR);
    struct queue_msg *shmpointer = (struct  queue_msg *) attach_shared_memory(shmem_id, 0);


    /****************
     * FILE READING *
     ****************/

    // change process working directory
    if (chdir(path_to_dir) == -1)
        errExit("Error while changing directory");

    // alloc 150 character
    char buf[PATH];
    
    // get current wotking directory
    getcwd(buf, MAX_LENGTH_PATH);

    printf("Ciao %s, ora inizio l’invio dei file contenuti in %s\n\n", getenv("USER"), buf);

    // file found counter
    int count = 0;
    // Creating an array to store the file paths
    char to_send[MAX_FILES][PATH];

    // search files into directory
    count = search_dir (buf, to_send, count);


    /*******************************
     * CLIENT-SERVER COMMUNICATION *
     *******************************/

    // Writing n_files on FIFO1
    write_fifo(fifo1_fd, &count, sizeof(count));

    // Unlocking semaphore 1 (allow server to read from FIFO1)
    semop_usr(semid, 1, 1);

    semop_usr(semid, 4, -1);

    if(strcmp(shmpointer[0].fragment, "READY") != 0){
        errExit("Corrupted start message");
    }

    for(int i = 1; i <= count; i++){
        pid = fork();


    }


    /*
    // Printing all file routes
    printf("\nn = %d\n\n", count);
    for (int i = 0 ; i < count ; i++)
        printf("to_send[%d] = %s\n", i, to_send[i]);
    
    */

    /**************
     * CLOSE IPCs *
     **************/ 

    close(fifo1_fd);
    close(fifo2_fd);
    free_shared_memory(shmpointer);

    return 0;
}




// gimmy attento che qui finisce la main!!!




    /******************************
    * BEGIN FUNCTIONS DEFINITIONS *
    *******************************/

// Personalised signal handler for SIGUSR1 and SIGINT
void sigHandler (int sig) {
    // if signal is SIGUSR1, set original mask and kill process
    if(sig == SIGUSR1) {
        set_original_mask();
        exit(0);
    }

    // if signal is SIGINT, happy(end) continue
    if(sig == SIGINT)
        printf("\n\nI'm awake!\n\n");
}

// function used to reset the default mask of the process
void set_original_mask() {
    // reset the signal mask of the process = restore original mask
    if(sigprocmask(SIG_SETMASK, &original_set_signals, NULL) == -1)
        errExit("sigprocmask(original_set) failed");
}

void create_signal_mask() {
    // initialize new_set_signals to contain all signals of OS
    if(sigfillset(&new_set_signals) == -1)
        errExit("sigfillset failed!");

    // remove SIGINT from new_set_signals
    if(sigdelset(&new_set_signals, SIGINT) == -1)
        errExit("sigdelset(SIGINT) failed!");
    // remove SIGUSR1 from new_set_signals
    if(sigdelset(&new_set_signals, SIGUSR1) == -1)
        errExit("sigdelset(SIGUSR1) failed!");

    // blocking all signals except:
    // SIGKILL, SIGSTOP (default)
    // SIGINT, SIGUSR1
    if(sigprocmask(SIG_SETMASK, &new_set_signals, &original_set_signals) == -1)
        errExit("sigprocmask(new_set) failed!");

    // definition manipulate SIGINT
    if (signal(SIGINT, sigHandler) == SIG_ERR)
        errExit("change signal handler (SIGINT) failed!");

    // definition manipulate SIGUSR1
    if (signal(SIGUSR1, sigHandler) == SIG_ERR)
        errExit("change signal handler (SIGUSR1) failed!");
}