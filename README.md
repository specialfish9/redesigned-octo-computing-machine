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
In questa sezione sono spiegate brevemente le funzionalità dei moduli più
importanti. Per ulteriori informazioni sulle funzioni menzionate si rimanda alla 
documentazione generata del codice.

* FASE 1
     * __asl:__ funzioni per la gestione delle Active Semaphore Lists.
     * __pcb:__ funzioni per la gestione di code e alberi dei PCB.
* FASE 2
     * __kernel:__ inizializzatore e gestore delle eccezioni del sistema operativo.
     * __scheduler:__ gestore dei processi attivi.
     * __syscalls:__ chiamate di sistema.
     * __interrupts:__ interrupts associati ai dispositivi connessi e al clock di sistema.
* FASE 3
     * __support:__ handler per le eccezioni e funzioni di utility per il livello supporto.
     * __sys_support:__ systemcall di livello supporto.
     * __pager:__ gestore della memoria virtuale a livello supporto.
     * __init_proc:__ inizializzatore processi utente di test di fase 3.
* ALTRO
     * __utils:__ funzioni di utility.
     * __klog:__ funzioni per il logging su un buffer in memoria.
     * __pandos_types:__ tipi predefiniti di PandOS.
     * __pandos_const:__ costanti predefinite di PandOS. 

### Kernel
Nel modulo kernel si trovano due funzioni molto importanti: ```main``` e
```exception_handler```. La prima si occupa dell'inizializzazione del kernel. 
In particolare inizializza tutte le strutture dati necessarie, crea il processo 
init e infine lascia il controllo allo scheduler. La seconda gestisce interrupt, 
syscall e trap.

### Scheduler
Lo scheduler contiene due code di processi _ready_ (una per priorità). Gli altri 
moduli si interfacciano con queste tramite le funzioni ```enqueue_proc``` e
```dequeue_proc``` che si occupano di aggiungere e rimuovere processi. Lo scheduler 
mette inoltre a disposizione le funzioni ```mk_proc``` e ```kill_proc``` per 
creare ed uccidere processi. Infine il _core_ di questo modulo è la funzione 
```scheduler_next``` che decide quale sarà il prossimo processo da caricare e 
lo carica. 

### Syscalls Negative
In questo modulo è contenuto il gestore per le chiamate di sistema: 
```handle_syscall```. Questo si basa sul valore del registro A0 (ed eventualmente 
anche dei registri A1, A2 e A3) e restituisce l'azione che l'exception handler 
deve svolgere una volta gestita la syscall. 
Sono presenti anche le implementazioni delle system call con codice da -1 a -10,
tra cui la ```passeren``` (NSYS3) e la```verhogen```  (NSYS4) che svolgono le 
operazioni di P  e V sul semaforo binario specificato. Infine è presente la 
funzione ```passup_or_die``` che si occupa delle eccezioni non gestite dagli 
appositi handler: decide se passare l'eccezione al livello di supporto o uccidere 
il processo; prende in input il tipo e restituisce TRUE se il processo attivo 
deve essere rimesso in stato di ready, FALSE altrimenti.

### Interrupts
Nel modulo interrupts è presente la funzione ```handle_interrupts```; questa
gestisce l'interrupt della linea specificata. Prende come parametro l'intero 
corrispondente alla linea di interrupt attiva e restituisce l'azione che
l'exception handler deve svolgere una volta gestito l'interrupt. Per la gestione 
delle linee associate ai vari device è presente la funzione 
```generic_interrupt_handler```.

### Support e Syscall Positive
Il modulo ```support``` racchiude un gestore generico (```support_exec_handler```), 
richiamato dal livello sottostante tramite Pass Up or Die) con lo scopo
di scindere le syscall del livello supporto dalle trap. 
A seguito della divisione, sono invocati altri due handler: 
```support_syscall_handler``` e ```support_trap_handler```.
Il primo gestore è implementato nel modulo sys_support ed ha lo scopo di 
determinare quale syscall è stata invocata, eseguire le azioni corrispondenti
e, se l'esecuzione ha successo, inserire il valore di ritorno in ```reg_v0```.
Alcuni dei servizi forniti sono la scrittura su stampante (SYS3) e la 
lettura/scrittura su terminale (SYS4 e SYS5).
Il gestore delle trap invece termina il processo corrente tramite la SYS2 
(```terminate```), avendo cura di segnalarlo al pager.

### TLB e memoria virtuale
Le eccezioni del TLB sono gestiti da due handler: ```uTLB_refill_handler``` 
per le ecceioni di tipo _TBL refill_, e ```tlb_exec_handler``` per tutte le 
altre. Il primo si trova nel modulo ```kernel``` e si occupa semplicemente
di andare ad inserire nel TLB la pagina richiesta, andando a recuperarla dalla
tabella delle pagine privata del processo. Il secondo invece si trova nel 
modulo ```pager``` ed è il cuore della gestione della memoria virtuale.
Si occupa di andare a caricare la pagina richiesta all'interno della ```swap pool```,
interagendo con essa e con i dispositivi flash che fungono da backing store
per i processi utente di test di fase 3. Gestisce anche, tramite un apposito 
algoritmo di rimpiazzamento, l'eventualità in cui la swap pool sia occupata
per intero.

## Scelte implementative

### Gestione PID
Una delle scelta implementative che sono state affrontate è quella del come 
assegnare i _process id_ ai processi. Data una struttura del tipo ```pcb_t```
è stato deciso di assegnare come relativo PID, l'indirizzo di memoria della 
struttura stessa. In questo modo sapendo il pid si può accedere al pcb in tempo 
costante. Questa però è un'operazione pericolosa perché si rischia di accedere 
ad indirizzi erronei se non si presta attenzione, per questo motivo è stata creata 
la funzione ```pcb_t find_by_pid(const unisgned int pid) ``` definita in ```pcb.h``` 
che effettua opportuni controlli e ritorna il pcb cercato in tempo costante.
Bisogna prestaer attenzione al fatto che se si lavora con un'architettura a 64 bit 
è necesserio trattare i PID come ```unsigned long``` e non come ```unsigned int```.

### Tempo di CPU di un processo
Per tenere conto del totale dei millisecondi in cui un processo è stato in 
esecuzione bisogna interrogare la struttura ```pcb_t```, rappresentante il 
processo. In particolare il tempo totale in millisecondi è contenuto dentro al 
campo ```cpu_t p_time```. Per renderne più agevole l'aggiornamento è stato 
ritenuto opportuno modificare la struttura ```pcb_t``` aggiungendo il campo 
```cpu_t p_tm_updt``` che rappresenta il valore del TOD nel momento dell'ultima
modifica di ```p_time```. In caso di un'eccezione il tempo impiegato per la 
gestione di questa viene assegnato al processo che l'ha generata, se esiste.

### NSYS3 e NSYS4 (passeren e verhogen)
Le funzioni passeren e verhogen hanno un funzionamento quasi speculare: nel caso 
della NSYS3 (P) se il valore del semaforo è 1 il processo viene bloccato mentre 
se è 0 viene sbloccato; accade l'opposto per la NSYS4 (V). Per il blocco di un
processo si procede rimuovendolo dalla lista dei processi attivi e inserendolo 
nella lista dei bloccati assegnati al semaforo passato come parametro (diventa 
soft blocked); per quanto riguarda lo sblocco invece si rimuove dalla lista dei 
processi soft blocked sul semaforo e lo si riassegna alla lista di processi attivi 
nella coda di priorità corretta. 

### L'algoritmo di rimpiazzamento
Per scegliere su quale frame della swap pool andare a caricare la pagina è stata 
sviluppata una variante del semplice algoritmo di rimpiazzamento suggerito
dal manuale di PandOS+. Questo, partendo dall'ultimo frame sostituito, prevede 
di andare a scorrere in modo circolare tutti i frame della swap pool finchè non
se ne trova uno libero da ritornare. Nel caso in cui tutti i frame dovessero
risultare occupati, si fa affidamento all'algoritmo suggerito dal manuale, e si
ritorna il frame succesivo all'ultimo rimpiazzato. Anche se di complessità maggiore
(```O(n)``` invece che ``` O(1)```) questo algoritmo migliora le performance del 
sistema. Infatti limita di parecchio i casi in cui si va a rimpiazzare un frame 
occupato, limitando drasticamente le operazioni di IO.

### Ottimizzazioni sulla Swap Pool
Per evitare inutili scritture su device flash è stata introdotta la variabile 
```sp_asids``` all'interno del modulo ```pager```. Ogni volta che viene inserito
un frame di un processo con asid ```i``` all'interno della swap pool, il bit alla
posizione ```i-1```-esima viene acceso, mentre viene spento quando il processo
termina. In questo modo, quando l'algoritmo di rimpiazzamento sceglie il frame 
da andare a riempire, oltre a controllare se questo è occupato oppure no, è possibile
sapere anche se il processo proprietario è ancora in vita. Se non lo dovesse essere
significa che è possibile andare a sovrascrivere il frame senza dover salvarne il 
contenuto. Oltre a far risparmiare tempo, questo previene anche la scrittura su 
device non validi in quanto appartenuti a processi non più in esecuzione.

### Accesso ai device in mutua esclusione 
Anche se non strettamente necessario per far funzionare gli 8 processi di test di
fase 3, è stato ritenuto opportuno mediare gli accessi a ciascun device mediante
semafori, per garantirne la mutua esclusione. In particolare nel modulo ```support```
è presente una matrice ```dev_sems``` grande ```Numero tipi device + 1 x Numero device```.
Il tipo aggiuntivo è per gestire in modo separato l'input e l'output dei terminali.

## Problemi

### Master semaphore
Come suggerito dal manuale è stato implementato il meccanismo del ```master semaphore```.
Purtroppo il gruppo non è riuscito in tempo a renderlo pienamente funzionante. In particolare
dopo aver creato gli 8 processi utente, il processo di test esegue correttamente una ```P``` 
sul master semaphore. Alla terminazione del primo uproc questo esegue una ```V``` e _sveglia_
il padre che viene correttamente inserito nelle code dello scheduler. Quando questo gli cede 
il controllo, il processo test solleva un kernel panic senza riuscire a fare la seconda ```P```.
Il motivo di ciò è oscuro al gruppo. I sospetti cadono sul meccanismo con cui ```passeren``` e 
```verhogen``` gesticono la sospensione e la ripresa dei processi, anche se sembrano funzionare
correttamente nel resto del progetto. Purtroppo il gruppo non è stato in grado di indagare più
a fondo per mancanza di tempo. Per far terminare gli 8 processi la riga in cui viene eseguita la
```V``` sul master semaphore è stata commentata.