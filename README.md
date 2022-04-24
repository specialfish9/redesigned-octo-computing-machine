# Really Optimistic Computing Machine

## Compilazione ed esecuzione

### Dipendenze
1. [__umps3__](https://github.com/virtualsquare/umps3) per eseguire il progetto
2. [__cmake-format__](https://cmake-format.readthedocs.io/en/latest/) per la formattazione del codice
3. [__doxygen__ ](https://www.doxygen.nl) per generare la documentazione

### Utilizzo

Compilazione del kernel, creazione dell'immagine del disco e della documentazione con:
```
make all
```
o più semplicemente:
```
make
```
_Nota:_ Gli output generati sono salvati nella directory __output__.

_Nota:_ La documentazione generata (in formato pdf e html) è salvata nella directory __doc__.

Sarà inoltre necessario creare una macchina all'interno dell'emulatore umps3, utilizzando i file generati come configurazione.

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
In questa sezione sono spiegate brevemente le funzionalità dei moduli più importanti. Per ulteriori informazioni sulle
funzioni menzionate si rimanda alla documentazione generata del codice.

* FASE 1
     * __asl:__ funzioni per la gestione delle Active Semaphore Lists.
     * __pcb:__ funzioni per la gestione di code e alberi dei PCB.
* FASE 2
     * __kernel:__ inizializzatore e gestore delle eccezioni del sistema operativo.
     * __scheduler:__ gestore dei processi attivi.
     * __syscalls:__ chiamate di sistema.
     * __interrupts:__ interrupts associati ai dispositivi connessi e al clock di sistema.
* ALTRO
     * __utils:__ funzioni di utility.
     * __klog:__ funzioni per il logging su un buffer in memoria.
     * __pandos_types:__ tipi predefiniti di PandOS.
     * __pandos_const:__ costanti predefinite di PandOS. 

### Kernel
Nel modulo kernel si trovano due funzioni molto importanti: ```main``` e ```exception_handler```.
La prima si occupa dell'inizializzazione del kernel. In particolare inizializza tutte le strutture dati necessarie,
crea il processo init e infine lascia
il controllo allo scheduler. La seconda invece funge da gestore di interrupts, syscalls e traps.

### Scheduler
Lo scheduler contiene due code di processi _ready_ (una per priorità). Gli altri moduli si interfacciano con queste
tramite le funzioni ```enqueue_proc``` e ```dequeue_proc``` che si occupano di aggiungere e rimuovere processi. Lo
scheduler mette inoltre a disposizione le funzioni ```mk_proc``` e ```kill_proc``` per creare e uccidere processi.
Infine il _core_ di questo modulo è la funzione ```scheduler_next``` che decide quale sarà il prossimo processo da 
caricare e lo carica. Se si vogliono più informazioni su queste funzioni leggere la documentazione generata del codice.

### Syscalls
In questo modulo è contenuto il gestore per le chiamate di sistema: ```handle_syscall```. Questo si basa sul valore del 
registro A0 (ed eventualmente anche dei registri A1-A2-A3) e restituisce l'azione che l'exception handler deve svolgere
una volta gestita la syscall. 
Sono presenti anche le implementazioni delle system call con codice da -1 a -10, tra cui la ```passeren``` (NSYS3) e la
```verhogen```  (NSYS4) che svolgono le operazioni di P  e V sul semaforo binario specificato. 
Infine è presente la funzione ```passup_or_die```: questa si occupa delle eccezioni non gestite dagli appositi handler:
decide se passare l'eccezione al livello di supporto o uccidere il processo; prende in input il tipo e restituisce TRUE
se il processo attivo deve essere rimesso in stato di ready, FALSE altrimenti.

### Interrupts
Nel modulo interrupts è presente la funzione ```handle_interrupts```; questa gestisce l'interrupt della linea
specificata. Prende come parametro l'intero corrispondente alla linea di interrupt attiva e restituisce l'azione che
l'exception handler deve svolgere una volta gestito l'interrupt. Per la gestione delle linee associate ai vari device è
presente la funzione ```generic_interrupt_handler```.

## Scelte implementative

### Gestione PID
Una delle scelta implementative che sono state affrontate è quella del come assegnare i _process id_ ai processi. Data
una struttura del tipo ```pcb_t```
è stato deciso di assegnare l'indirizzo di memoria della struttura stessa. In questo modo sapendo il pid si può
accedere al pcb in tempo costante. Questa
però è un'operazione pericolosa perché si rischia di accedere ad indirizzi erronei se non si presta attenzione, per
questo motivo è stata creata la funzione ```pcb_t find_by_pid(const unisgned int pid) ``` definita in ```pcb.h``` che
effettua opportuni controlli e ritorna il pcb in tempo costante.
Bisogna fare attenzione al fatto che se si lavora con un'architettura a 64 bit è necesserio passare da ```unsigned
int``` a ```unsigned long```.

### Tempo di CPU di un processo
Per tenere conto del totale dei millisecondi in cui un processo è stato in esecuzione bisogna interrogare la struttura
```pcb_t``` rappresentante il processo, in particolare il tempo totale in millisecondi è contenuto dentro al campo
```cpu_t p_time```. Per renderne più agevole l'aggiornamento è stato ritenuto opportuno modificare la struttura
```pcb_t``` aggiungendo il campo ```cpu_t p_tm_updt``` che rappresenta il valore del TOD nel momento dell'ultima
modifica di ```p_time```. 
In caso di un'eccezione di tipo syscall, il tempo impiegato per la gestione di questa viene assegnato al processo che
l'ha generata, mentre nel caso di un interrupt non se ne terrà conto.

### NSYS3 e NSYS4 (passeren e verhogen)
Le funzioni passeren e verhogen hanno un funzionamento quasi speculare: nel caso della NSYS3 (P) se il valore del
semaforo è 1 il processo viene bloccato mentre se è 0 viene sbloccato; accade l'opposto per la NSYS4 (V). 
Per il blocco di un processo si procede rimuovendolo dalla lista dei processi attivi e inserendolo nella lista dei
bloccati assegnati al semaforo passato come parametro (diventa soft blocked); per quanto riguarda lo sblocco invece si
rimuove dalla lista dei processi soft blocked sul semaforo e lo si riassegna alla lista di processi attivi nella coda
di priorità corretta. 
