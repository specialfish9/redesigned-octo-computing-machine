
/*
unsigned int retValue = SYSCALL (GETTOD, 0, 0, 0);
GETTOD=1
*/
unsigned int get_TOD(){
    //l'handler dovrà prendere il valore restituito da questa funzione e piazzarlo nel registro v0 di U-proc
    unsigned int ret;
    STCK(ret);
    return ret;

    /*
    Access to the TOD clock value can be accomplished either of the following
    ways:
        • Direct access to the Bus Register memory location: 0x1000.001C
        • Appendix C contains a listing of the µMPS3 System-wide constants file
        contst.h. Included in this file is a pre-defined macro STCK(T) which takes
        an unsigned integer as its input parameter and populates it with the value of
        the low-order word of the TOD clock divided by the Time Scale. [Section
        4.1.1]
    */
}

/*
SYSCALL (TERMINATE, 0, 0, 0);
TERMINATE=2
*/
void terminate(){
    //spara al processo utente che l'ha chiamato
    pcb_t *proc = act_proc;  //NON SO SE SIA GIUSTO, DEVO CAPIRE SE U-PROC E IL PROCESSO ATTIVO SIANO DUE COSE EQUIVALENTI

    kill_parent_and_progeny(proc);
    //return is_alive(act_proc) ? RENQUEUE : NOTHING;
}

/*
int retValue = SYSCALL (WRITEPRINTER, char *virtAddr, int len, 0);
WRITEPRINTER=3
*/
int write_to_printer(){
    //sospende il processo chiamante fino alla fine della trasmissione al printer associato al processo
    //PARAMETRI: indirizzo virtuale del primo carattere della stringa da trasmettere + lunghezza della stringa
    //RETURN: restituisce il numero di caratteri trasmessi (se ha avuto successo), altrimenti (status diverso da 1, device ready) return dello status del device con segno cambiato
    //l'handler dovrà prendere il valore restituito da questa funzione e piazzarlo nel registro v0 di U-proc
    //ERRORI: se è chiamato da un indirizzo fuori dallo spazio logico di indirizzi, o con una lunghezza > 128, o con una lunghezza < 0: ammazza il processo (SYS2)

}

/*
int retValue = SYSCALL (WRITETERMINAL, char *virtAddr, int len, 0);
WRITETERMINAL=4
*/
void write_to_terminal(){
    //sospende il processo chiamante fino alla fine della trasmissione al terminale associato al processo
    //PARAMETRI: indirizzo virtuale del primo carattere della stringa da trasmettere + lunghezza della stringa
    //RETURN: restituisce il numero di caratteri trasmessi (se ha avuto successo), altrimenti (status diverso da 5, character transmitted) return dello status del device con segno cambiato
    //l'handler dovrà prendere il valore restituito da questa funzione e piazzarlo nel registro v0 di U-proc
    //ERRORI: se è chiamato da un indirizzo fuori dallo spazio logico di indirizzi, o con una lunghezza > 128, o con una lunghezza < 0: ammazza il processo (SYS2)

}

/*
int retValue = SYSCALL (READTERMINAL, char *virtAddr, 0, 0);
READTERMINAL=5
*/
void read_from_terminal(){
    //sospende il processo chiamante fino a che una linea di input (stringa) è stata trasmessa dal terminale associato al processo
    //PARAMETRI: indirizzo virtuale di un buffer stringa dove devono essere inseriti i caratteri ricevuti
    //RETURN: restituisce il numero di caratteri trasmessi (se ha avuto successo), altrimenti (status diverso da 5, chatacter received) return dello status del device con segno cambiato
    //l'handler dovrà prendere il valore restituito da questa funzione e piazzarlo nel registro v0 di U-proc
    //NB: i caratteri ricevuti vanno inseriti nel buffer a partire dall'indirizzo ricevuto come parametro in a1
    //ERRORI: se l'indirizzo è fuori dallo spazio logico degli indirizzi del processo: ammazza il processo (SYS2)
}