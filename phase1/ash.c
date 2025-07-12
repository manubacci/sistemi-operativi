#include "ash.h"

struct semd_t semd_table [MAXPROC];
// Array di SEMD con dimensione massima di MAXPROC.

struct list_head semdFree_h;
// Lista dei SEMD liberi o inutilizzati.

DEFINE_HASHTABLE (semd_h, 5); // 5 o 20?
// Hash dei semafori attivi.
/* La dimensione dell'hashtable semd_h, ossia 5, è espressa dal numero di bit della chiave, che corrisponde al numero 
minimo di bit che occorrono per indicizzare tutti i MAXPROC processi: 2^5 = 32 > 20. Per la chiave si utilizza la codifica 
u32. */ 

/* Viene inserito il PCB puntato da p nella coda dei processi bloccati associata al SEMD con chiave semAdd. Se il 
semaforo corrispondente non è presente nella ASH, alloca un nuovo SEMD dalla lista di quelli liberi (semdFree) e 
lo inserisce nella ASH, settando i campi in maniera opportuna (i.e. key e s_procQ). Se non è possibile allocare un 
nuovo SEMD perché la lista di quelli liberi è vuota, restituisce TRUE. In tutti gli altri casi, restituisce FALSE. */
int insertBlocked (int* semAdd, pcb_t* p){
    if (semAdd == NULL || p == NULL)
        return FALSE;
    // In caso di errore restituisce FALSE.
    semd_t* search = NULL;
    semd_t* item;
    u32 index;
    hash_for_each (semd_h, index, item, s_link){
    /* Scorre gli elementi della tabella semd_h, salvando un indice in codifica u32 temporaneo in index e assegnando ad 
    ogni iterazione il puntatore all'elemento corrente a item. */
        if (item -> s_key == semAdd)
            search = item;
        /* Ad ogni iterazione, verifica se la chiave associata al campo s_key del semaforo item corrisponde alla chiave 
        semAdd cercata. In caso affermativo, associa item al SEMD cercato. */
    }
    if (search != NULL){
        insertProcQ (&search -> s_procq, p);
        /* Se il semaforo corrispondente alla chiave semAdd è presente nella ASH, inserisce il PCB puntato da p nella coda
        dei processi associata al campo s_procq del SEMD cercato. */
    } else {
        /* Se il semaforo corrispondente alla chiave semAdd non è presente nella ASH, al SEMD cercato rimane associato il
        valore con cui è inizializzato, ossia NULL. */
        if (list_empty (&semdFree_h))
            return TRUE;
        /* Se la sentinella semdFree_h punta ad una lista vuota, ossia se alla lista dei SEMD liberi non appartiene alcun
        elemento e non è quindi possibile allocare un nuovo semaforo, ritorna TRUE. */
        semd_t* allocate = container_of (semdFree_h.next, semd_t, s_freelink);
        list_del (semdFree_h.next);
        hash_add (semd_h, &allocate -> s_link, *semAdd);
        /* Ottiene un puntatore alla struttura dati di tipo semd_t che contiene semdFree_h.next, la cui sentinella 
        corrisponde a s_freelink. semdFree_h.next è il SEMD in testa alla lista dei semafori liberi puntata da semdFree_h.
        Rimuove poi questo elemento dalla lista e aggiuonge alla hashtable semd_h l'oggetto corrispondente al semaforo da 
        allocare con il descrittore contenuto nel campo s_link, associandolo alla chiave semAdd. */
        allocate -> s_key = semAdd;
        INIT_LIST_HEAD (&allocate -> s_procq);
        list_add_tail (&p -> p_list, &allocate -> s_procq);
        /* Assegna la chiave semAdd al campo s_key, destinato alla chiave del semaforo. Inizializza la sentinella del campo 
        s_procq, relativo alla coda dei processi bloccati nel semaforo, in modo che punti ad una lista vuota, nella cui coda 
        viene inseritla sentinella del campo p_list del PCB puntato da p, che punta alla coda dei processi. */
    }
    p -> p_semAdd = semAdd; 
    /* Assegna la chiave semAdd al campo p_semAdd del PCB puntato da p, che puntata al semaforo su cui il processo è
    attualmente bloccato. */
    return FALSE;
    // In tutti i casi ad eccezione di quello sopra analizzato, ritorna FALSE.
}

/* Ritorna il primo PCB dalla coda dei processi bloccati (s_procq) associata al SEMD della ASH con chiave semAdd. Se 
tale descrittore non esiste nella ASH, restituisce NULL. Altrimenti, restituisce l’elemento rimosso. Se la coda dei 
processi bloccati per il semaforo diventa vuota, rimuove il descrittore corrispondente dalla ASH e lo inserisce nella
coda dei descrittori liberi (semdFree_h). */
pcb_t* removeBlocked (int* semAdd){
    semd_t* search = NULL;
    if (semAdd == NULL)
        return NULL;
    semd_t* item;
    u32 index;
    hash_for_each (semd_h, index, item, s_link){
        if (item -> s_key == semAdd)
            search = item;
    }
    if (search == NULL)
        return NULL;
    /* Verifica se il semaforo al cui campo s_key è associato alla chiave semAdd è presente all'interno dell'ASH. In caso 
    affermativo, lo assegna al semaforo cercato. */
    struct pcb_t* first = removeProcQ (&(search -> s_procq));
    // Rimuove il PCB in testa alla coda dei processi bloccati puntata dalla sentinella del campo s_procq del semaforo cercato.
    if (emptyProcQ (&(search -> s_procq))){
        hash_del (&search -> s_link);
        list_add_tail (&search -> s_freelink, &semdFree_h);
    }
    /* Verifica se la coda dei processi bloccati puntata dalla sentinella del campo s_procq del semaforo con chiave semAdd è 
    diventata vuota. In caso affermativo, rimuove dalla hashtable l'oggetto corrispondentente al semaforo cercato il cui 
    descrittore è contenuto nel campo s_link, e aggiunge la sentinella del campo s_freelink in coda alla lista dei descrittori 
    liberi puntata da semdFree_h. */
    return first;
    // Restituisce il puntatore al PCB rimosso dalla testa della coda dei processi bloccati.
}

/* Rimuove il PCB puntato da p dalla coda del semaforo su cui è bloccato (indicato da p->p_semAdd). Se il PCB non compare in 
tale coda, allora restituisce NULL (condizione di errore). Altrimenti, restituisce p. Se la coda dei processi bloccati per il
semaforo diventa vuota, rimuove il descrittore corrispondente dalla ASH e lo inserisce nella coda dei descrittori liberi. */
pcb_t* outBlocked (pcb_t* p){
    semd_t* search = NULL;
    if (p == NULL)
        return NULL;
    semd_t* item;
    u32 index;
    hash_for_each (semd_h, index, item, s_link){
        if (item -> s_key == p -> p_semAdd)
            search = item;
    }
    if (search == NULL) 
        return NULL; 
    /* Verifica se è presente all'interno dell'ASH il semaforo il cui campo s_key corrispondente al campo p_semAdd del PCB
    puntato da p, ossia alla chiave del semaforo su cui il processo è bloccato. In caso affermativo, assegna il SEMD al 
    semaforo cercato, altrimenti ritorna NULL. */                   
    p = outProcQ (&search -> s_procq, p);
    /* Rimuove il PCB puntato da p dalla coda dei processi bloccati puntata dalla sentinella del campo s_procq del semaforo 
    cercato, ossia del semaforo su cui è bloccato. */
    p -> p_semAdd = NULL;
    /* Assegna il valore NULL al campo p_semAdd del PCB puntato da p, per indicare che il processo non è più bloccato da alcun 
    semforo. */
    if (emptyProcQ (&search -> s_procq)){
        hash_del (&search -> s_link);
        list_add_tail (&search -> s_freelink, &semdFree_h);
    }
    /* Verifica se la coda dei processi bloccati del semaforo cercato è diventata vuota. In caso affermativo, rimuove dalla 
    hashtable l'oggetto corrispondentente al semaforo cercato, il cui descrittore è contenuto nel campo s_link, e aggiunge la 
    sentinella del campo s_freelink in coda alla lista dei descrittori liberi puntata da semdFree_h. */
    return p;
    // Restituisce il puntatore p al PCB rimosso.
}

/* Restituisce (senza rimuovere) il puntatore al PCB che si trova in testa alla coda dei processi associata al SEMD con chiave 
semAdd. Ritorna NULL se il SEMD non compare nella ASH oppure se compare ma la sua coda dei processi è vuota. */
pcb_t* headBlocked (int* semAdd) {
    semd_t* search = NULL;
    if (semAdd == NULL)
        return NULL;
    semd_t* item;
    u32 index;
    hash_for_each (semd_h, index, item, s_link){
        if (item -> s_key == semAdd)
            search = item;
    }
    /* Verifica se il semaforo al cui campo s_key è associato alla chiave semAdd è presente all'interno dell'ASH. In caso 
    affermativo, lo assegna al semaforo cercato. */
    if (search == NULL || emptyProcQ (&search -> s_procq))
        return NULL;
    /* Se il semaforo cercato non è presente nella ASH o se la sua coda dei processi bloccati puntata dalla sentinella del 
    campo s_procq del semaforo cercato è vuota, ritorna NULL. */
    pcb_t* head = headProcQ (&(search -> s_procq));
    return head;
    /* Restituisce l’elemento in testa alla coda dei processi puntata dal campo s_procq del semaforo associato alla chiave 
    semAdd, senza rimuoverlo. */
}

/* Inizializza la lista dei semdFree in modo da contenere tutti gli elementi della semdTable. Questo metodo
viene invocato una volta sola durante l’inizializzazione della struttura dati. */
void initASH (){
    INIT_LIST_HEAD (&semdFree_h);
    /* Inizializza l'elemento sentinella di semdFree_h in modo che i campi prev e next puntino alla sentinella stessa, così 
    da creare una lista vuota. */
    for (int i = 0; i < MAXPROC; i++){
        list_add_tail (&semd_table[i].s_freelink, &semdFree_h);
        /* Per ciascuno dei MAXPROC elementi di semd_table, inserisce la sentinella s_freelink dell'elemento corrispondente 
        all'i-esimo SEMD di semd_table in coda alla lista puntata dall'elemento sentinella semdFree_h. s_freelink punta alla 
        lista dei semafori liberi a cui appartiene il SEMD. */
    }
}
