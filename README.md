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
    * interrupts (interrupts associati ai dispositivi connessi e al clock di sistema)
    * syscalls (chiamate di sistema invocate dal kernel)
    * scheduler (gestore dei processi attivi e "soft blocked")
    * kernel (inizializzazione e nucleo del sistema operativo)


## Scelte implementative






