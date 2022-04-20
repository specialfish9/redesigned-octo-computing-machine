# Really Optimistic Computing Machine

## Compilazione ed esecuzione

### Dipendenze:
1. [umps3](https://github.com/virtualsquare/umps3)
2. __cmake-format__ per la formattazione del codice
3. __doxygen__ per generare la documentazione

### Utilizzo:

Compilazione del kernel, creazione dell'immagine del disco e della documentazione con:
```
make all
```
o più semplicemente:
```
make
```
_Nota:_ Gli output generati sono salvati nella directory __output__.

_Nota:_ La documentazione generata è salvata nella directory __doc__.

Sarà inoltre necessario creare una macchina all'interno dell'emulatore umps3, utilizzando i files generati come configurazione.

### Altri comandi:

Compilazione del kernel:
```
make kernel
```

Pulizia del progetto:
```
make clean
```

Formattazione del codice:
```
make format
```

Generazione della documentazione:
```
make docs 
```

## Struttura di ROCM

### Sorgenti
1. FASE 1
      * asl (funzioni per la gestione delle Active Semaphore Lists)
      * pcb (funzioni per la gestione di code e alberi dei PCB)
2. FASE 2
      * kernel (inizializzazione e nucleo del sistema operativo)
      * scheduler (gestore dei processi attivi e "soft blocked")
      * syscalls (chiamate di sistema invocate dal kernel)
      * interrupts (interrupts associati ai dispositivi connessi e al clock di sistema)

### Funzioni principali (fase 2)
* ```main()``` in **kernel** &#8594; DESCRIZIONE DESCRIZIONE DESCRIZIONE
* ```exception_handler()``` in **kernel** &#8594; DESCRIZIONE DESCRIZIONE DESCRIZIONE
* ```init_scheduler()``` in **scheduler** &#8594; DESCRIZIONE DESCRIZIONE DESCRIZIONE
* ```scheduler_next()``` in **scheduler** &#8594; DESCRIZIONE DESCRIZIONE DESCRIZIONE
* ```handle_syscall()``` in **syscalls** &#8594; DESCRIZIONE DESCRIZIONE DESCRIZIONE
* ```passup_or_die(size_tt kind)``` in **syscalls** &#8594; DESCRIZIONE DESCRIZIONE DESCRIZIONE
* ```handle_interrupts(const int line)``` in **interrupts** &#8594; DESCRIZIONE DESCRIZIONE DESCRIZIONE
* ```generic_interrupt_handler(int line, int *semaphores)``` in **interrupts** &#8594; DESCRIZIONE DESCRIZIONE DESCRIZIONE

## Scelte implementative






