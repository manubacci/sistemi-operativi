#include "ns.h"

struct nsd_t NS_PID_nsd [MAXPROC];
/* Si utilizza un array diverso per ogni tipo di namespace. In questo caso, è sufficiente
un singolo array di NSD di tipo NS_PID con dimensione massima MAX_PROC. */

struct list_head NS_PID_nsFree_h;
// Lista degli NSD di tipo NS_PID liberi o inutilizzati.

struct list_head NS_PID_nsList_h;
// Lista dei namespace di tipo NS_PID attivi.

/* Inizializza tutte le liste dei namespace liberi. Questo metodo viene invocato una volta sola durante 
l’inizializzazione della struttura dati. */
void initNamespaces (){
    INIT_LIST_HEAD (&NS_PID_nsList_h);
    INIT_LIST_HEAD (&NS_PID_nsFree_h);
    /* Inizializza gli elementi sentinella delle liste degli NSD liberi e attivi di tipo NS_PID: 
    NS_PID_nsList_h e NS_PID_nsFree_h, in modo che puntino a due liste vuote. Per implementare altri tipi
    di NSD occorre definire e inizializzare anche le rispettive liste. */
    for (int i = 0; i < MAXPROC; i++)
        list_add (&NS_PID_nsd[i].n_link, &NS_PID_nsFree_h);
        /* Per ciascuno dei MAXPROC elementi di NS_PID_nsd, inserisce la sentinella n_link dell'elemento 
        corrispondente all'i-esimo NSD di NS_PID_nsd in coda alla lista puntata dall'elemento sentinella 
        NS_PID_nsFree_h, ossia alla lista dei namespace liberi. */
}

// Ritorna il namespace di tipo type associato al processo p (o NULL).
nsd_t* getNamespace (pcb_t* p, int type){
    if (p == NULL)
        return NULL;
    // In caso di errore ritorna NULL.
    struct nsd_t* search = NULL;
    for (int i = 0; i < NS_TYPE_MAX; i++){
        if (p -> namespaces[i] != NULL && p -> namespaces[i] -> n_type == type)
            search = p -> namespaces[i];
        /* Per ciascuno dei NS_TYPE_MAX possibili tipi di NSD, verifica se il tipo associato al campo n_type 
        dell'i-esimo elemento del campo namespace associato al PCB puntato da p corrisponde al tipo specificato. 
        In caso affermativo, assegna al NSD cercato questo elemento. */
    }
    return search;
    /* Restituisce il NSD cercato. Nel caso in cui al PCB puntato da p non sia associato alcun namespace di tipo 
    type, restitituisce il valore a cui è stato inizializzato, ossia NULL. */
}

// Associa al processo p e a tutti i suoi figli il namespace ns. Ritorna FALSE in caso di errore, TRUE altrimenti.
int addNamespace (pcb_t* p, nsd_t* ns){
    if (p == NULL || ns == NULL)
        return FALSE;
    // Ritorna FALSE in caso di errore.
    struct list_head* search;
    bool found = FALSE;
    list_for_each (search, &NS_PID_nsList_h){
    // Scorre gli elementi della lista degli NSD attivi puntata dall'elemento sentinella NS_PID_nsList_h.
        if (container_of (search, nsd_t, n_link) == ns)
            found = TRUE;
    }
    if (found == FALSE)
        ns = allocNamespace (ns -> n_type);
    /* Se a nessuno dei puntatori alle strutture dati di tipo nsd_t che contengono gli elementi considerati, le cui 
    sentinelle corrispondono a n_link, è associato il namespace ns, ossia se esso non è presente all'interno della 
    lista degli NSD attivi, allora deve essere allocato. */
    struct list_head* item;
    list_for_each (item, &p -> p_child) {
    /* Scorre gli elementi della lista dei figli del PCB puntato da p, puntata dall'elemento sentinella del campo 
    p_child del PCB. */
        pcb_t* child = container_of (item, pcb_t, p_sib);
        /* Per ciascuno degli elementi considerati esamina il puntatore alla struttura dati di tipo pcb_t che lo 
        contiene, la cui sentinella corrisponde p_sib, ossia all'n-esimo PCB della coda dei processi fratelli del
        figlio del PCB puntato da p, puntata da p_sib. */
        for (int i = 0; i < NS_TYPE_MAX; i++){
            if (child -> namespaces[i] == NULL || child -> namespaces[i] -> n_type == ns -> n_type)
                child -> namespaces[i] = ns;
        }
        /* Per ciascuno dei NS_TYPE_MAX possibili tipi di NSD, verifica se all'i-esimo elemento del campo 
        namespace associato al PCB esaminato è assegnato il valore NULL, oppure se il suo tipo associato al campo n_type 
        corrisponde al tipo n_type del namespace ns. In caso affermativo assegna a questo elemento il NSD ns. */
    }
    for (int j = 0; j < NS_TYPE_MAX; j++){
        if (p -> namespaces[j] == NULL || p -> namespaces[j] -> n_type == ns -> n_type)
            p -> namespaces[j] = ns;
    // Ripete la stessa operazione sugli elementi del campo namespace associato al PCB puntato da p.
    }
    return TRUE;
    // Ritorna TRUE se non si sono verificati errori.
}

// Alloca un namespace di tipo type dalla lista corretta.
nsd_t* allocNamespace (int type){
    if (type == NS_PID){
        /* Se il tipo type corrisponde a NS_PID, allora occorre lavorare sulle liste di NSD dello stesso tipo: 
        NS_PID_nsFree_h e NS_PID_nsList_h. Per operare con altri tipi di namespace si devono definire le liste
        corrispondenti. */
        if (list_empty (&NS_PID_nsFree_h))
            return NULL;
        /* Se la lista puntata da NS_PID_nsFree_h, ossia la lista degli NSD di tipo NS_PID liberi, è vuota, e non è 
        quindi possibile allocare alcun namespace, ritorna NULL. */
        struct nsd_t* namespace = container_of (NS_PID_nsFree_h.next, nsd_t, n_link);
        list_del (NS_PID_nsFree_h.next);
        /* Ottiene un puntatore alla struttura dati di tipo nsd_t che contiene NS_PID_nsFree_h.next, la cui sentinella 
        corrisponde a n_link. NS_PID_nsFree_h.next è il NSD in testa alla lista dei namespace di tipo NS_PID liberi, 
        puntata da NS_PID_nsFree_h. Elimina poi questo elemento dalla lista. */
        list_add_tail (&namespace -> n_link, &NS_PID_nsList_h);
        /* Inserisce la sentinella del campo n_link del namespace considerato nella coda della lista degli NSD 
        di tipo NS_PID attivi, puntata da NS_PID_nsFree_h. */
        return namespace;
    } else return NULL;
    // Ritorna il namespace allocato o NULL in caso di errore.
}

// Libera il namespace ns ri-inserendolo nella lista di namespace corretta.
void freeNamespace (nsd_t* ns){
    if (ns == NULL)
        return;
    // In caso di errore ritorna NULL.
    if (ns -> n_type == NS_PID){
    /* Se il campo n_type che indica il tipo del namespace corrisponde a NS_PID, allora occorre lavorare sulle liste
    NS_PID_nsFree_h e NS_PID_nsList_h. */
        struct list_head* search;
        list_for_each (search, &NS_PID_nsList_h){
        // Scorre gli elementi della lista degli NSD di tipo NS_PID attivi puntata dall'elemento sentinella NS_PID_nsList_h.
            if (container_of (search, nsd_t, n_link) == ns)
                list_del (search);
            /* Per ciascuno degli elementi della lista esamina il puntatore alla struttura dati di tipo nsd_t che lo 
            contiene, la cui sentinella corrisponde a n_link, ossia all'n-esimo NSD della lista dei namespace attivi.
            Se questo elemento corrisponde al namespace cercato, lo cancella dalla lista. */
        }
        list_add_tail (&ns -> n_link, &NS_PID_nsFree_h);
        /* Inserisce la sentinella del campo n_link del namespace ns nella coda della lista degli NSD di tipo NS_PID 
        liberi, puntata da NS_PID_nsFree_h.*/
    }
    // Per operare con altri tipi di namespace si devono prima definire le liste corrispondenti. 
}