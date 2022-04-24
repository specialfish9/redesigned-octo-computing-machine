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
* FASE 1
     * asl (funzioni per la gestione delle Active Semaphore Lists)
     * pcb (funzioni per la gestione di code e alberi dei PCB)
* FASE 2
     * kernel (inizializzazione e nucleo del sistema operativo)
     * scheduler (gestore dei processi attivi e "soft blocked")
     * syscalls (chiamate di sistema invocate dal kernel)
     * interrupts (interrupts associati ai dispositivi connessi e al clock di sistema)

### Funzioni principali (fase 2)
* ```main()``` in **kernel** &#8594; inizializzazione ed entry point del kernel
* ```exception_handler()``` in **kernel** &#8594; gestore di interrupts, syscalls e traps    
* ```init_scheduler()``` in **scheduler** &#8594; inizializzazione di variabili e liste utilizzate all'interno dello scheduler
* ```scheduler_next()``` in **scheduler** &#8594; funzione per la scelta del processo successivo da eseguire, estraendolo da una delle due code (alta/bassa priorità)
* ```handle_syscall()``` in **syscalls** &#8594; gestore per le chiamate di sistema basato sul valore nel registro A0 (ed eventualmente anche nei registri A1-A2-A3); restituisce l'azione che l'exception handler deve svolgere una volta gestita la syscall
* ```passup_or_die(size_tt kind)``` in **syscalls** &#8594; gestione per le eccezioni non gestite dagli appositi handler: decide se passare l'eccezione al livello di supporto o uccidere il processo; prende in input il tipo e restituisce TRUE se il processo attivo deve essere rimesso in stato di ready, FALSE altrimenti
* ```passeren(int *semaddr)``` in **syscalls** &#8594; operazione P sul semaforo binario (NSYS3); prende in input il semaforo su cui svolgere l'operazione e restituisce l'azione che l'exception handler deve svolgere una volta gestita la syscall
* ```verhogen(int *semaddr)``` in **syscalls** &#8594; operazione V sul semaforo binario (NSYS4); prende in input il semaforo su cui svolgere l'operazione e restituisce un puntatore al pcb del processo in questione
* ```handle_interrupts(const int line)``` in **interrupts** &#8594; gestisce l'interrupt dalla linea specificata; prende come parametro l'intero corrispondente alla linea di interrupt attiva e restituisce l'azione che l'exception handler deve svolgere una volta gestito l'interrupt
* ```generic_interrupt_handler(int line, int *semaphores)``` in **interrupts** &#8594; funzione chiamata da handle_interrupts ALTRA DESCRIZIONE CHE NON SO

## Scelte implementative

### Gestione PID
SCRIVI SCRIVI SCRIVI Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.

### Temporizzazione CPU
SCRIVI SCRIVI SCRIVI Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.

### SYSCALL 3 e 4 (passeren e verhogen)
Le funzioni passeren e verhogen hanno un funzionamento quasi speculare: nel caso della NSYS3 (P) se il valore del semaforo è 1 il processo viene bloccato mentre se è 0 viene sbloccato; accade l'opposto per la NSYS4 (V). 
Per il blocco di un processo si procede rimuovendolo dalla lista dei processi attivi e inserendolo nella lista dei bloccati assegnati al semaforo passato come parametro (diventa soft blocked); per quanto riguarda lo sblocco invece si rimuove dalla lista dei processi soft blocked sul semaforo e lo si riassegna alla lista di processi attivi nella coda di priorità corretta. 





