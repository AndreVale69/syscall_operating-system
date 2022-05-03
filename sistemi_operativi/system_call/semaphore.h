/// @file semaphore.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione dei SEMAFORI.

#pragma once

#ifndef SEMUN_H
#define SEMUN_H
#include <sys/sem.h>

// definition of the union semun
union semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

// Definition of a semaphore operation whitout using a buffer
void semop_usr (int semid, unsigned short sem_num, short sem_op);

#endif