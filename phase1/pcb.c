#include "pcb.h"

HIDDEN struct list_head pcbFree_h; 
// Lista dei PCB che sono liberi o inutilizzati.

HIDDEN struct pcb_t pcbFree_table [MAXPROC];
// Array di PCB con dimensione massima di MAX_PROC.

/* Inizializza la lista pcbFree in modo da contenere tutti gli elementi della pcbFree_table. Questo metodo deve 
essere chiamato una volta sola in fase di inizializzazione della struttura dati. */
void initPcbs (){
    INIT_LIST_HEAD (&pcbFree_h);
    /* Inizializza l'elemento sentinella di pcbFree_h in modo che i campi prev e next puntino alla 
    sentinella stessa, così da creare una lista vuota. */
    for (int i = 0; i < MAXPROC; i++)
        list_add_tail (&pcbFree_table[i].p_list, &pcbFree_h);
        /* Per ciascuno dei MAXPROC elementi di pcbFree_table, inserisce la sentinella p_list dell'elemento 
        corrispondente all'i-esimo PCB di pcbFree_table in coda alla lista puntata da pcbFree_h. L'elemento viene
        aggiunto nella lista prima dell'elemento sentinella. p_list punta alla coda dei processi a cui appartiene il PCB. */
}

// Inserisce il PCB puntato da p nella lista dei PCB liberi (pcbFree_h).
void freePcb (pcb_t* p){
    if (p == NULL) return;
    list_add_tail (&p -> p_list, &pcbFree_h);
    /* Aggiunge la sentinella del campo p_list del PCB puntato da p in coda alla lista dei PCB liberi, puntata da 
    pcbFree_h. */
}

/* Restituisce NULL se la pcbFree_h è vuota. Altrimenti rimuove un elemento dalla pcbFree, inizializza tutti i 
campi (NULL/0) e restituisce l’elemento rimosso. */
pcb_t* allocPcb (){
    if (list_empty (&pcbFree_h)) 
        return NULL;
    /* Se la lista puntata da pcbFree_h è vuota, ossia se i campi dell'elemento sentinella puntano alla sentinella
    stessa, restituisce NULL. */
    else {
        struct pcb_t* iteration = container_of (pcbFree_h.next, pcb_t, p_list);
        list_del (pcbFree_h.next);
        /* Ottiene un puntatore alla struttura dati di tipo pcb_t che contiene pcbFree_h.next, la cui sentinella 
        corrisponde a p_list. pcbFree_h.next è l'elemento successivo a cui punta la sentinella pcbFree_h, ossia
        il PCB in testa alla coda dei processi liberi. Questo elemento viene poi rimosso dalla lista. */
        INIT_LIST_HEAD (&iteration -> p_list);
        /* Inizializza l'elemento sentinella del campo p_list del PCB rimosso dalla lista puntato dal puntatore
        generato, in modo che punti ad una lista vuota. La sentinella p_list punta alla coda dei processi a cui 
        appartiene il PCB. */
        iteration -> p_parent = NULL;
        INIT_LIST_HEAD (&iteration -> p_child);
        INIT_LIST_HEAD (&iteration -> p_sib);
        /* Inizializza i campi relativi all'albero dei processi del puntatore generato, ossia inizializza le
        sentinelle alle liste dei PCB figli e fratelli in modo che puntino a liste vuote. */
        iteration -> p_s.cause = 0;
        iteration -> p_s.entry_hi = 0;
        for (int i = 0; i < STATE_GPR_LEN; i++){
            iteration -> p_s.gpr[i] = 0;
        }
        iteration -> p_s.hi = 0;
        iteration -> p_s.lo = 0;
        iteration -> p_s.pc_epc = 0;
        iteration -> p_s.status = 0;
        // Inizializza a 0 tutti i sotto-campi del campo che indica lo stato del processore. 
        iteration -> p_time = 0;
        // Inizializza a 0 il campo che indica il tempo della CPU impiegato.
        iteration -> p_semAdd = NULL;
        // Inizializza a NULL il puntatore al semaforo su cui il processo è attualmente bloccato.
        for (int i = 0; i < NS_TYPE_MAX; i++){
            iteration -> namespaces[i] = NULL;
        }
        // Inizializza a NULL ogni NSD appartenente alla lista dei namespace.
        return iteration;
        // Restituisce il puntatore al PCB rimosso dalla lista i cui campi sono stati correttamente inizializzati.
    }
}

// Crea una lista di PCB, inizializzandola come lista vuota.
void mkEmptyProcQ (struct list_head* head){
    if (head == NULL) 
        return;
    // In caso di errore ritorna NULL.
    INIT_LIST_HEAD (head);
    // Inizializza l'elemento sentinella head in modo che punti a se stesso, creando una lista vuota.
}

// Restituisce TRUE se la lista puntata da head è vuota, FALSE altrimenti.
int emptyProcQ (struct list_head* head){
    if (head == NULL) 
        return FALSE;
    // In caso di errore ritorna NULL.
    return list_empty (head);
    /* Se l'elemento sentinella head punta ad una lista vuota, ossia se tutti i suoi campi puntano alla 
    sentinella stessa, restituisce TRUE. In caso contrario restituisce FALSE. */
}

// Inserisce l’elemento puntato da p nella coda dei processi puntata da head.
void insertProcQ (struct list_head* head, pcb_t* p){
    if (p == NULL) 
        return;
    // In caso di errore ritorna NULL.
    list_add_tail (&p -> p_list, head);
    /* Inserisce la sentinella del campo p_list del PCB puntato dal puntatore p in coda alla coda dei processi 
    puntata dalla sentinella head, posizionandola prima di questa. */
}

/* Restituisce l’elemento di testa della coda dei processi da head, SENZA RIMUOVERLO. Ritorna NULL se la coda
non ha elementi. */
pcb_t* headProcQ (struct list_head* head){
    if (head == NULL || emptyProcQ (head))
        return NULL;
    /* In caso di errore, oppure se la lista a cui punta la sentinella head è vuota, ossia se alla corrispondente 
    coda di processi non appartiene nessun elemento, restituisce NULL. */
    else return container_of (head -> next, pcb_t, p_list);
    /* Restituisce un puntatore alla struttura dati di tipo pcb_t che contiene head -> next, la cui sentinella 
    corrisponde a p_list. head -> next è il PCB in testa alla coda dei processi puntata da head. */
}

/* Rimuove il primo elemento dalla coda dei processi puntata da head. Ritorna NULL se la coda è vuota. Altrimenti 
ritorna il puntatore all’elemento rimosso dalla lista. */
pcb_t* removeProcQ (struct list_head* head){
    if (head == NULL || emptyProcQ (head))
        return NULL;
    /* In caso di errore oppure se alla coda di processi puntata da head non appartiene nessun elemento, restituisce 
    NULL. */
    struct pcb_t* temporary = container_of (head -> next, pcb_t, p_list);
    list_del (head -> next);
    return temporary;
    /* Ottiene un puntatore alla struttura dati di tipo pcb_t che contiene head -> next, la cui sentinella 
    corrisponde a p_list, ossia alla struttura che contiene il PCB in testa alla coda dei processi puntata da head. 
    Elimina poi questo elemento dalla lista e ritorna il puntatora al PCB rimosso. */
}

/* Rimuove il PCB puntato da p dalla coda dei processi puntata da head. Se p non è presente nella coda, restituisce
NULL. (NOTA: p può trovarsi in una posizione arbitraria della coda). */
pcb_t* outProcQ (struct list_head* head, pcb_t* p){
    if (p == NULL || emptyProcQ (head))
        return NULL;
    /* In caso di errore oppure se alla coda di processi puntata da head non appartiene nessun elemento, restituisce 
    NULL. */
    struct pcb_t* item = NULL;
    list_for_each_entry (item, head, p_list){
    /* Scorre gli elementi della coda dei processi puntata dall'elemento sentinella head, assegnando ad ogni 
    iterazione il puntatore all'elemento corrente a search. */
        if (item == p){
            list_del (&p -> p_list);
            return p;
        }
        /* Ad ogni iterazione, verifica se l'elemento considerato corrisponde al PCB puntato da p. In caso affermativo,
        elimina la sentinella del campo p_list del PCB dalla coda dei processi e restituisce il puntatore al PCB rimosso. */
    }
    return NULL;
    /* Se scorrendo completamente la lista nessun elemento corrisponde al PCB puntato da p, allora esso non è presente 
    nella coda dei processi puntata da head, perciò restituisce NULL. */
}

// Restituisce TRUE se il PCB puntato da p non ha figli, FALSE altrimenti.
int emptyChild (pcb_t* p){
    if (p == NULL)
        return FALSE;
    // In caso di errore ritorna FALSE.
    return emptyProcQ (&p -> p_child);
    /* Se l'elemento sentinella del campo p_child del PCB puntato da p punta ad una lista vuota, restituisce TRUE, 
    altrimenti FALSE. */
}

// Inserisce il PCB puntato da p come figlio del PCB puntato da prnt.
void insertChild (pcb_t* prnt, pcb_t* p){
    if (prnt == NULL || p == NULL)
        return;
    // In caso di errore interrompe l'esecuzione.
    p -> p_parent = prnt;
    // Assegna il PCB puntato da prnt al campo p_parent del PCB puntato da p.
    list_add_tail (&p -> p_sib, &prnt -> p_child);
    /* Inserisce la sentinella del campo p_sib del PCB puntato da p in coda alla lista puntata dal campo p_child del PCB
    puntato da prnt. La sentinella p_sib punta alla coda di processi contenente il PCB e tutti i suoi fratelli .*/
}

// Rimuove il primo figlio del PCB puntato da p. Se p non ha figli, restituisce NULL.
pcb_t* removeChild (pcb_t* p){
    if (p == NULL || emptyProcQ (&p -> p_child))
        return NULL;
    /* Se nella lista puntata dalla sentinella del campo p_child del PCB puntato da p non sono presenti elementi, 
    ossia se p non ha figli, restituisce NULL. */
    struct pcb_t* first = container_of (&p -> p_child.next, pcb_t, p_sib);
    list_del (p -> p_child.next);
    return first;
    /* Ottiene un puntatore alla struttura dati di tipo pcb_t che contiene p -> p_child.next, la cui sentinella 
    corrisponde a p_sib. p -> p_child.next è il PCB in testa alla coda dei processi fratelli a cui appartengono 
    i figli del PCB puntato da p. Elimina poi questo elemento dalla lista e restituisce il puntatora al PCB rimosso. */
}

/* Rimuove il PCB puntato da p dalla lista dei figli del padre. Se il PCB puntato da p non ha un padre, restituisce 
NULL, altrimenti restituisce l’elemento rimosso (cioè p). A differenza della removeChild, p può trovarsi in una 
posizione arbitraria (ossia non è necessariamente il primo figlio del padre). */
pcb_t* outChild (pcb_t* p){
    if (p == NULL || p -> p_parent == NULL || emptyProcQ (&p -> p_sib))
        return NULL;
    /* In caso di errore, se il PCB puntato da p non ha un padre o se la lista puntata dalla sentinella p_sib del 
    PCB è vuota, ossia se p non ha fratelli, restituisce NULL. */
    struct pcb_t* sibling = container_of (&p -> p_sib.next, pcb_t, p_sib);
    list_del (p -> p_sib.next);
    return sibling;
    /* Ottiene un puntatore alla struttura dati di tipo pcb_t che contiene p -> p_sib.next, la cui sentinella 
    corrisponde a p_sib. p -> p_sib.next è il PCB successivo al PCB puntato da p nella coda dei processi fratelli 
    a cui appartengono. Elimina poi questo elemento dalla lista e restituisce il puntatora al PCB rimosso. */
}