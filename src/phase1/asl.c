#include "asl.h"

struct semd_t semd_table[MAXPROC]; //array di SEMD con dimensione massima MAXPROC
struct list_head *semdFree_h;       //head lista dei SEMD liberi o inutilizzati
struct list_head semdFree;          //lista SEMD liberi
struct list_head *semd_h;           //hrad lista ASL
struct list_head semd;              //lista dei semafori attivi (ASL)

semd_t* getSemd(int *s_key){    //per ottenere il semaforo di appartenenza a partire dalla key
    struct list_head *iter;
    list_for_each(iter, semd_h){
        struct semd_t *s = container_of(iter,semd_t,s_link);
        if(s->s_key == s_key)
            return s;
    }
    return NULL;    //restituisce NULL se non esiste un semaforo con quella key
}



//DESCRIZIONI SOMMARIE DELLE FUNZIONI, PER QUELLE COMPLETE VEDERE FILE asl.h

int insertBlocked(int *semAdd,pcb_t *p){    //p punta al PCB dalla coda dei processi bloccati, semAdd è la chiave per il SEMD associato a quella coda. Return TRUE se non è possibile allocare un SEMD perchè la lista dei liberi è vuota, FALSE altrimenti
    struct semd_t *s = getSemd(p->p_semAdd);   //ottengo il semaforo su cui è bloccato p

    if(s == NULL){  //se il semaforo corrispondente a semAdd non esiste nella ASL
        if(list_empty(semdFree_h))
            return TRUE;    //restituisco TRUE se non è possibile allocare un nuovo semaforo perchè la lista di quelli liberi è vuota
        else{
            struct semd_t *tmp = container_of(semdFree_h->next,semd_t,s_link);      //ottengo un nuovo SEMD dalla lista dei liberi
            list_del(semdFree_h->next);             //elimino il SEMD dalla lista dei liberi
            list_add(&(tmp->s_link),semd_h);        //lo inserisco nella lista dei SEMD

            insertProcQ(&(tmp->s_procq),p);         //associo il processo al SEMD
            tmp->s_key = semAdd;
        }
    }else       //se il semaforo esiste già inserisco il processo in coda
        insertProcQ(&(s->s_procq),p);
    
    p->p_semAdd = semAdd;

    return FALSE; //restituisco FALSE come caso base
}

pcb_t* removeBlocked(int *semAdd){  //rimuovere il primo PCB nella lista dei bloccati associati al SEMD tramite semAdd. Return del PCB, altrimenti null se non esiste
    struct semd_t *s = getSemd(semAdd);   //ottengo il semaforo associato a semAdd
//FORSE QUESTO IF E' SUPERFLUO
    if(s == NULL)
        return NULL;

    struct pcb_t *pcb = removeProcQ(&(s->s_procq));  //rimuovo il primo elemento bloccato su quel SEMD
    
    if(list_empty(&(s->s_procq))){              //Se la coda dei processi bloccati per il semaforo diventa vuota
        list_del(&(s->s_link));                 //rimuove il descrittore corrispondente dalla ASL
        list_add(&(s->s_link),semdFree_h);      //e lo inserisce nella coda dei descrittori liberi
    }
    return pcb;       //return del PCB se compare
}

pcb_t* outBlocked(pcb_t *p){    //rimuovere PCB puntato da p->p_semAdd dalla coda del semaforo su cui è bloccato. Return null se il PCB non compare, p altrimenti
    struct semd_t *s = getSemd(p->p_semAdd);   //ottengo il semaforo su cui è bloccato p
//FORSE QUESTO IF E' SUPERFLUO
    if(s == NULL)       
        return NULL;    //return NULL se il PCB non compare
    
    struct pcb_t *pcb = outProcQ(&(s->s_procq), p);   //elimino il PCB p dalla lista
    
    if(list_empty(&(s->s_procq))){              //Se la coda dei processi bloccati per il semaforo diventa vuota
        list_del(&(s->s_link));                 //rimuove il descrittore corrispondente dalla ASL
        list_add(&(s->s_link),semdFree_h);      //e lo inserisce nella coda dei descrittori liberi
    }
    return pcb;       //return del PCB se compare

}

pcb_t* headBlocked(int *semAdd){    //restituire senza rimuovere il puntatore del PCB in testa alla coda dei processi del SEMD identificato da semAdd, null se il SEMD non compare o se compare ma la sua lista dei processi è vuota
    struct semd_t *s = getSemd(semAdd);   //ottengo il semaforo associato a semAdd

    if(s == NULL || list_empty(&(s->s_procq)))  //se SEMD non compare o se la sua lista dei processi è vuota return NULL
        return NULL;
    
    return container_of(&(s->s_procq),pcb_t,p_list);        //restituisco il puntatore del PCB in testa alla coda

}

void initASL(){ //inizializzare lista di semdFree per contenere tutti gli elementi di semd_table
    INIT_LIST_HEAD(&semdFree);  //inizializzo una nuova lista di SEMD liberi
    semdFree_h = &semdFree;     //testa della lista SEMD liberi
    INIT_LIST_HEAD(&semd);      //inizializzo una nuova lista di semafori
    semd_h = &semd;             //testa della lista semafori

    for(int i=0; i<MAXPROC; i++){       //inizializzo MAXPROC semafori liberi e li aggiungo a semdFree
        struct semd_t *tmp = &(semd_table[i]);
        INIT_LIST_HEAD(&(tmp->s_procq));    //lista vuota di PCB bloccati
        INIT_LIST_HEAD(&(tmp->s_link));     //lista semafori
        list_add(&(tmp->s_link),semdFree_h);    //aggiunta alla lista dei semafori liberi
    }
}