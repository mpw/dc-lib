/* ============================================================================
 *  rm_alloc.c
 * ============================================================================

 *  Author:         (c) 2010 the rm team & Emilio Coppa <ercoppa@gmail.com>
 *  License:        See the end of this file for license information
 *  Created:        July 21, 2010
 *  Note:           reactive memory allocator

 *  Last changed:   $Date: 2011/01/31 12:21:05 $
 *  Changed by:     $Author: alex $
 *  Revision:       $Revision: 1.11 $
*/

#include "_rm_private.h"


/* private function prototypes */
static void* _rm_malloc     (size_t size);
static void* _rm_calloc     (size_t num, size_t size);
static void* _rm_realloc    (void* ptr, size_t size);
static void  _rm_free       (void* ptr);


/* ============================================================================
 *  wrappers
 * ============================================================================
*/

/* ----------------------------------------------------------------------------
 *  rm_malloc
 * ----------------------------------------------------------------------------
*/
void* rm_malloc(size_t size) {
    void* p = _rm_malloc(size);
    if(p == NULL) {
        return NULL;
    }
    return (p + rm_OFFSET);
}


/* ----------------------------------------------------------------------------
 *  rm_calloc
 * ----------------------------------------------------------------------------
*/
void* rm_calloc(size_t num, size_t size) {
    void* p = _rm_calloc(num, size);
    if(p == NULL) {
        return NULL;
    }
    return (p + rm_OFFSET);
}


/* ----------------------------------------------------------------------------
 *  rm_realloc
 * ----------------------------------------------------------------------------
*/
void* rm_realloc(void* ptr, size_t size) {
    ptr = rm_get_inactive_ptr(ptr);
    void* p = _rm_realloc(ptr, size);
    if(p == NULL) {
        return NULL;
    }
    return (p + rm_OFFSET);
}


/* ----------------------------------------------------------------------------
 *  rm_free
 * ----------------------------------------------------------------------------
*/
void rm_free(void* ptr) {
    ptr = rm_get_inactive_ptr(ptr);
    _rm_free(ptr);
}


/* ============================================================================
 *  allocator code
 * ============================================================================
*/

/*
 * -----------------------------------------
 * | BSA - Bad segregated (fits) allocator |
 * -----------------------------------------
 *
 * Informazioni:
 * 
 * Creato da Emilio Coppa <ercoppa@gmail.com> per la tesina del corso
 * Ingegneria degli Algoritmi tenuto dal docente Camil Demetrescu
 * presso l'universita' La Sapienza di Roma facolta' di Ingegneria
 * Informatica 2009-2010.
 *
 * Introduzione:
 * 
 * BSA e' un allocatore della famiglia segregated fits, esso presenta
 * in testa all'heap una lookup table con puntatori alle liste di
 * blocchi liberi.
 *
 * La lookup table e' composta da due "livelli":
 *
 * - il primo livello e' composta da liste che contengono blocchi
 *   di dimensione esatta in base al loro indice (esempio: lista
 *   i-esima conterra' blocchi di (FIXED_SIZE*i) byte). La prima lista
 *   necessariamente conterra' blocchi di dimensione MIN_BLOCK_SIZE, al
 *   contrario l'ultima lista per questo livello conterra' blocchi
 *   di dimensione pari a MAX_FIXED. Fra una lista e l'altra, i blocchi
 *   avranno una differenza in dimensione di FIXED_BLOCK_SIZE.
 *
 * - il secondo livello e' composto da liste che contengono blocchi
 *   di dimensione compresa fra (EXP_BASE^(i-1))+1 e EXP_BASE^i, dove i
 *   e' l'indice relativo alla lista. L'indice minimo per le liste di
 *   secondo livello e' calcolato in base all'ultimo indice delle liste
 *   di primo livello (maggiorato di uno) oppure, se quest'ultime non
 *   sono presenti, in base a MIN_BLOCK_SIZE. L'indice massimo e'
 *   pari a MAX_EXP. Ogni blocco di dimensione superiore a qualunque
 *   lista e' inserito nell'ultima.
 *
 * Data una dimensione (di una richiesta di allocazione), la scelta
 * del blocco dentro una lista e' effettuata con le seguenti policy:
 * 
 * - FIRSTFIT: viene scelto il primo blocco incontrato nella lista,
 *             purche' sufficientemente grande
 * - BESTFIT: viene scelto il blocco nella lista piu' piccolo (ma
 *            sufficientemente grande per soddisfare la richiesta)
 *
 * Tale comportamento e' determinato in base alla macro POLICY. Con
 * entrambe le modalita' e' possibile indicare di scegliere, in caso
 * di blocchi liberi di pari dimensione, quello ad indirizzo di
 * memoria inferiore. Tale comportamente e' impostato definendo la
 * macro ADDRESS_ORDER.
 * 
 * Ogni blocco restituito ha un indirizzo multiplo di ALIGNMENT.
 *
 * Ogni blocco in ogni momento puo' trovarsi in due stati:
 * - in uso (quindi fuori da qualsiasi lista)
 * - libero (inserito in una lista)
 * e possiede un header di dimensione HEADER_SIZE contenente le seguenti
 * informazioni:
 * - la dimensione del blocco
 * - flag relativo al suo stato
 * - flag relativo allo stato del blocco precedente
 * Gli ultimi due flag sono memorizzati grazie alla tecnica del
 * bit stealing nella dimensione del blocco, che essendo multiplo di
 * ALIGNMENT, ed essendo quest'ultimo almeno pari ad 8, permette
 * di sfruttare fino a 3 bit.
 *
 * Ogni blocco libero contiene nel suo payload le seguenti informazioni:
 * - in testa al blocco un puntatore al blocco libero successivo
 *   nella lista di appartenenza (eventualmente NULL se vuota)
 * - in coda al blocco un puntatore alla sua testa
 *
 * BSA adotta la seguente strategia:
 *
 * - un blocco, quando richiesto con malloc(), viene cercato
 *   nell'appropriata lista. Se quest'ultima non permette la selezione
 *   di un blocco, allora la ricerca prosegue nelle liste successive. Se
 *   nessuna lista presenta un blocco sufficiente per la richiesta
 *   allora viene espanso l'heap (in quantita' pari alla dimensione di
 *   pagina se e' definita la macro PAGE_ALLOCATION o solo per la
 *   dimensione richiesta)
 *
 * - un blocco, quando liberato con free(), viene fuso con eventuali
 *   blocchi liberi (precedente e/o successivo) e inserito
 *   nell'appropriata lista
 *
 * - un blocco, su cui e' stato richiesto un ridimensionamento con
 *   realloc(), viene:
 * 
 *   - liberato se la nuova dimensione e' zero
 *
 *   - non modificato se la nuova dimensione e' minore della
 *     dimensione originale
 *
 *   - espanso (se e' l'ultimo blocco nell'heap) di dimensione pari
 *     alla differenza fra la nuova dimensione e l'originale oppure
 *     di una quantita' multipla della dimensione di pagina se e'
 *     definita la macro PAGE_ALLOCATION. Nel secondo caso, la quantita'
 *     di spazio in eccesso viene staccata per formare un nuovo blocco
 *     libero.
 *
 *   - fuso con il blocco successivo
 *
 *   - liberato ma e' richiesta preventivamente la copia byte a byte
 *     del suo contenuto in un nuovo blocco allocato con malloc()  
 * 
 */

/* Standard header */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Definire queste macro per avere alcune print() di supporto
 * in fase di debugging. E' necessario definire almeno DEBUG in caso
 * di definizione di un'altra macro di debugging.
 */
//#define DEBUG
//#define DEBUG_INIT
//#define DEBUG_MALLOC
//#define DEBUG_FREE
//#define DEBUG_REALLOC
//#define DEBUG_COALESCE
//#define DEBUG_GET_INTEX
//#define DEBUG_IN_LIST
//#define DEBUG_OUT_LIST
//#define DEBUG_SPLIT

/* Allineamento */
#define ALIGNMENT 8

/*
 * Differenza in dimensione blocchi primo livello
 * fra una lista e la successiva
 */
#define FIXED_BLOCK_SIZE 8

/*
 * Dimensione minima di un blocco: un blocco deve poter contenere
 * almeno i due puntatori necessari per lo stato di blocco libero.
 */
#define MIN_BLOCK_SIZE (2 * sizeof(void *))

/* Dimensione massima blocchi primo livello */
#define MAX_FIXED 512
/*
 * Base per blocchi secondo livello
 * Nota: al momento non e' supportata alcuna base diversa da 2 in quanto
 *       poco efficiente nel calcolo delle potenza
 */
#define EXP_BASE 2

/* Massimo esponente per blocchi secondo livello */
#ifdef __i386__
#define MAX_EXP 25
#else
#define MAX_EXP 25
#endif

/*
 * Dimensione dell' header per ogni blocco: essa deve essere pari a
 * sizeof(size_t) e sempre multipla di ALIGNMENT
 */
#define HEADER_SIZE 8

/*
 * Se e' definita PAGE_ALLOCATION, l'heap crescera' a multipli
 * della dimensione di pagina
 */
//#define HEAP_GROWSIZE mem_pagesize()
#define HEAP_GROWSIZE 4096

/* Politica di gestione delle liste */
#define FIRSTFIT 1
#define BESTFIT  2
#define POLICY BESTFIT

/*
 * L'operazione di coalesce(void * ptr, int mode) ha due modalita'
 * (specificata con il secondo parametro):
 * - NORMAL: fusione con il blocco precedente e successivo
 * - FORWARD: fusione con il blocco successivo
 * - BACKWARD: fusion con il blocco precedente
 */
#define NORMAL   1
#define FORWARD  2
#define BACKWARD 3

/*
 * L'operazione di split di un blocco puo' invocare o meno, l'eventuale
 * coalescing sul blocco creato. Il terzo parametro mode e':
 * - NO_COALESCE: no fusione
 * - COALESCE: fusione
 */
#define NO_COALESCE 1
#define COALESCE    2
/*
 * L'allocazione puo' avvenire a blocchi di pagina o in base alla reale
 * necessita'.
 */
#define PAGE_ALLOCATION
/*
 * L'inserimento nelle liste puo' avvennire in address order
 */
//#define ADDRESS_ORDER

/*
 * Alcune macro di servizio
 */

/* Dimensione payload di un blocco */
#define PAYLOAD_SIZE(ptr) (*((size_t *)(((char *)(ptr)) - HEADER_SIZE)) & ~3)
/*
 * Imposta la dimensione di payload per un blocco (non alterando il
 * flag di stato per il blocco precedente)
 */
#define SET_PAYLOAD_SIZE(ptr, size) (*((size_t *)(((char *)(ptr)) - HEADER_SIZE))) = ((size) | (*((size_t *)(((char *)(ptr)) - HEADER_SIZE)) & 3));
/*
 * Imposta la dimensione di payload per un blocco (alterando il
 * flag di stato per il blocco precedente)
 */
#define INIT_SIZE(ptr, size) (*((size_t *)(((char *)(ptr)) - HEADER_SIZE))) = size;
/* Gestione errore fatale */
#define FATAL_ERROR(message) { \
                                fprintf(stderr, "%s\n", message); \
                                exit(EXIT_FAILURE); \
                            }
/* Puntatore al successivo blocco libero della lista di appartenenza*/
#define NEXT_FREE(ptr) *((char **)ptr)
/* Puntatore al successivo blocco */
#define NEXT(ptr) (((char *) ptr)+PAYLOAD_SIZE(ptr)+HEADER_SIZE)
/* Puntatore al precedente blocco */
#define PREV(ptr) *((char **)((char *)ptr - HEADER_SIZE - sizeof(void *)))
/* Imposta in coda al blocco (libero) il puntatore alla sua testa */
#define SET_END_POINTER(ptr) *((char **)((char *)ptr + PAYLOAD_SIZE(ptr) - sizeof(void *))) = ptr;
/*
 * Interpretazione valore per le successive tre macro:
 * - 1 => si
 * - 0 => no
 */
/* Blocco libero */
#define IS_FREE(ptr) (*((size_t *)(((char *)(ptr)) - HEADER_SIZE)) & 1)
/* Primo blocco nell'heap */
#define IS_FIRST_BLOCK(ptr) (((ptr) - HEADER_SIZE) == rm_heap_lo() + heap_allocator_size)
/* Blocco precedente libero */
#define IS_PREV_FREE(ptr) (*((size_t *)(((char *)(ptr)) - HEADER_SIZE)) & 2)
/* Flag di stato per il blocco e quello precedente */
#define GET_FLAG(ptr) (*((size_t *)(((char *)(ptr)) - HEADER_SIZE)) & 3)
/* Marca un blocco come libero:
 * - setta il flag di blocco libero
 * - inserisce alla fine del blocco un puntatore alla sua testa
 * - segnala al blocco successivo di essere libero
*/
#define MARK_AS_FREE(ptr) { \
                            (*((size_t *)(((char *)(ptr)) - HEADER_SIZE))) = (*((size_t *)(((char *)(ptr)) - HEADER_SIZE)) | 1); \
                            SET_END_POINTER(ptr); \
                            ANNOUNCE_FREE_TO_NEXT(ptr); \
                            }
/* Segnala al blocco successivo che e' un blocco libero */                          
#define ANNOUNCE_FREE_TO_NEXT(ptr) { \
                                    if ((void *)NEXT(ptr) < (void *)rm_heap_hi()+1) \
                                        *((size_t *)(((char *)(NEXT(ptr))) - HEADER_SIZE)) = (*((size_t *)(((char *)(NEXT(ptr))) - HEADER_SIZE)) | 2); \
                                    }
/* Segnala al blocco successivo che e' un blocco in uso */
#define ANNOUNCE_USE_TO_NEXT(ptr) { \
                                    if ((void *)NEXT(ptr) < (void *)rm_heap_hi()+1) \
                                        *((size_t *)(((char *)(NEXT(ptr))) - HEADER_SIZE)) = (*((size_t *)(((char *)(NEXT(ptr))) - HEADER_SIZE)) & ~2); \
                                    }
/* Marco un blocco come in uso:
 * - imposta il flag di blocco in uso
 * - annuncio al blocco successivo di essere in uso
 */
#define MARK_AS_USED(ptr) { \
                            (*((size_t *)(((char *)(ptr)) - HEADER_SIZE))) = (*((size_t *)(((char *)(ptr)) - HEADER_SIZE)) & ~1); \
                            ANNOUNCE_USE_TO_NEXT(ptr); \
                            }
/* Arrotonda la dimensione ad un multiplo di ALIGNMENT */
#define ROUND(size) { \
                        if (size < MIN_BLOCK_SIZE) size = MIN_BLOCK_SIZE; \
                        else size = ((size + ALIGNMENT-1) & -ALIGNMENT); \
                    }
                    

/* Dimensione strutture BSA */
static unsigned int heap_allocator_size = 0;
/* Minino esponente utilizzato */
static unsigned int min_exp;
/* Puntatore all'ultimo blocco in heap */
static void * last_block;
/*
 * Dato un blocco e una dimensione, in caso di split divido verso
 * destra o sinistra?
 */
char split_side = 0;
#ifdef DEBUG
/* Funzione di debug dell'heap */
static void debug_heap();
/* Debug */
static unsigned int op = 0;
#endif

/*
 * Funzione privata che data una dimensione di blocco, ritorna
 * l'indice (spiazzamento dall'inizio di heap) della lista di
 * appartenenza.
 */
static unsigned int get_index(size_t size) {

    /* Arrotondiamo la dimensione */
    ROUND(size);

    #ifdef DEBUG_GET_INTEX
    printf("Arrontondato a %u\n", size);
    #endif

    if (size <= MAX_FIXED) {

        #ifdef DEBUG_GET_INTEX
        printf("%u - %u - %u\n", ((size - MIN_BLOCK_SIZE) / FIXED_BLOCK_SIZE), MIN_BLOCK_SIZE, FIXED_BLOCK_SIZE);
        #endif 

        return ((size - MIN_BLOCK_SIZE) / FIXED_BLOCK_SIZE);

    }

    unsigned int exp;
    for(exp = min_exp; exp < MAX_EXP; exp++) {

        double num;
        num = 2 << exp;
         
        if (num >= size) break;

    }

    if (MAX_FIXED != 0 && FIXED_BLOCK_SIZE != 0)
        return ((MAX_FIXED - MIN_BLOCK_SIZE) / FIXED_BLOCK_SIZE) + 1 + exp - min_exp;
    else
        return exp - min_exp;

}

/*
 * Funzione privata che rimuove un blocco libero dalla sua lista
 */
static void out_list(void * ptr) {

    unsigned int index = get_index(PAYLOAD_SIZE(ptr));

    #ifdef DEBUG_OUT_LIST
    printf("Metto fuori lista %u un blocco da %u ad %u\n", index, PAYLOAD_SIZE(ptr), (unsigned int) ptr);
    #endif
    
    void * heap_start = rm_heap_lo();
    void * prev = NULL;
    while(1) {

        #ifdef DEBUG_OUT_LIST
        printf("Scorro lista...\n");
        #endif

        if (prev == NULL) {

            if (NEXT_FREE(heap_start + index) == ptr) {

                #ifdef DEBUG_OUT_LIST
                printf("Blocco in testa alla lista\n");
                //if (NEXT_FREE(ptr) != NULL)
                //    printf("Successivo blocco a %u da %u\n", (unsigned int) NEXT_FREE(ptr), PAYLOAD_SIZE(NEXT_FREE(ptr)));
                #endif

                NEXT_FREE(heap_start + index) = NEXT_FREE(ptr);
                break;

            } else {
                prev = NEXT_FREE(heap_start + index);
            }
        } else {

            if (NEXT_FREE(prev) == ptr) {
                NEXT_FREE(prev) = NEXT_FREE(ptr);
                break;
            } else {
                prev = NEXT_FREE(prev);
            }
            
        }

        if(prev == NULL) {

            #ifdef DEBUG
            FATAL_ERROR("Un blocco dichiarato libero non e' stato trovato nella sua lista!");
            #endif
            
            break;
        }
    }

    return;
}

/*
 * Funzione privata che dato un puntatore ad un blocco lo inserisce
 * nella lista appropriata.
 */
static void in_list(void * ptr) {

    size_t size = PAYLOAD_SIZE(ptr);
    unsigned int index = get_index(size);
    char ** addr_heap = rm_heap_lo();
    NEXT_FREE(ptr) = NULL;

    #ifdef DEBUG_IN_LIST
    printf("Inserisco in lista %u il blocco %u di dimensione %u\n", index, (unsigned int) ptr, size);
    #endif

    if (size <= MAX_FIXED) {

        #ifndef ADDRESS_ORDER
        NEXT_FREE(ptr) = NEXT_FREE(addr_heap + index);
        NEXT_FREE(addr_heap + index) = ptr;
        #else
        NEXT_FREE(ptr) = NEXT_FREE(addr_heap + index);
        void * prev = NULL;
        while(NEXT_FREE(ptr) != NULL) {

            if (ptr < (void *)NEXT_FREE(ptr)) break;
            else {
                prev = NEXT_FREE(ptr);
                NEXT_FREE(ptr) = NEXT_FREE(NEXT_FREE(ptr));
            }
        }
        if (prev == NULL)
            NEXT_FREE(addr_heap + index) = ptr;
        else
            NEXT_FREE(prev) = ptr;
        #endif

    } else {

        #if POLICY == FIRSTFIT

        #ifndef ADDRESS_ORDER
        NEXT_FREE(ptr) = NEXT_FREE(addr_heap + index);
        NEXT_FREE(addr_heap + index) = ptr;
        #else
        NEXT_FREE(ptr) = NEXT_FREE(addr_heap + index);
        void * prev = NULL;
        while(NEXT_FREE(ptr) != NULL) {

            if (ptr < (void *)NEXT_FREE(ptr)) break;
            else {

                prev = NEXT_FREE(ptr);
                NEXT_FREE(ptr) = NEXT_FREE(NEXT_FREE(ptr));

            }

        }
        if (prev == NULL)
            NEXT_FREE(addr_heap + index) = ptr;
        else
            NEXT_FREE(prev) = ptr;
        #endif
        
        #endif
        
        #if POLICY == BESTFIT
        NEXT_FREE(ptr) = NEXT_FREE(addr_heap + index);
        void * prev = NULL;
        while(NEXT_FREE(ptr) != NULL) {

            #ifndef ADDRESS_ORDER
            if (PAYLOAD_SIZE(NEXT_FREE(ptr)) >= size) break;
            #else
            if (PAYLOAD_SIZE(NEXT_FREE(ptr)) > size) break;
            else if (PAYLOAD_SIZE(NEXT_FREE(ptr)) == size && ptr < (void *)NEXT_FREE(ptr)) break;
            #endif
            else {
                prev = NEXT_FREE(ptr);
                NEXT_FREE(ptr) = NEXT_FREE(NEXT_FREE(ptr));
            }
        }
        if (prev == NULL)
            NEXT_FREE(addr_heap + index) = ptr;
        else
            NEXT_FREE(prev) = ptr;
        #endif
        
    }

    #ifdef DEBUG_IN_LIST
    debug_heap();
    #endif

    return;

}

/*
 * Funzione privata che fonde, se liberi, il blocco successivo e/o
 * quello precedente al blocco passato. La modalita' di fusione
 * e' definita in base alle macro NORMAL, FORWARD, BACKWARD che
 * rispettivamente fondono il blocco, se possibile, con:
 * - blocco successivo e/o precedente
 * - blocco successivo
 * - blocco precedente
 * Viene restituito il puntatore alla nuova testa del blocco (con
 * dimensione  e stato correttamente impostato).
 */
static void * coalesce(void * ptr, int mode) {

    if(ptr == NULL) return NULL;

    #ifdef DEBUG_COALESCE
    printf("Invocato coalescing...\n");
    if (mode == NORMAL)
        printf("Modalita' NORMAL\n");
    else if (mode == FORWARD)
        printf("Modalita' FORWARD\n");
    //debug_heap();
    #endif

    /* La fine dell'heap */
    void * end = rm_heap_hi() + 1;

    #ifdef DEBUG_COALESCE
    printf("Inizio a leggere da %u\n", (unsigned int) ptr);
    #endif

    /* Fusione in avanti */
    if (mode == NORMAL || mode == FORWARD) {
        
        /* Cerco di fondere il blocco con il blocco successivo */
        void * next = NEXT(ptr);

        #ifdef DEBUG_COALESCE
        //printf("Localizzato il blocco successivo a %u del blocco %u di %u\n", (unsigned int) next, (unsigned int) ptr, PAYLOAD_SIZE(ptr));
        #endif
        
        if (next < end && IS_FREE(next)) {

            #ifdef DEBUG_COALESCE
            printf("Il blocco successivo locato a %u di %u e' libero\n", (unsigned int) next, PAYLOAD_SIZE(next));
            #endif

            /* Rimuovo il blocco dalla lista di appartenenza */
            out_list(next);
            /* Aggiorno la dimensione del blocco fuso */
            SET_PAYLOAD_SIZE(ptr, PAYLOAD_SIZE(ptr) + HEADER_SIZE + PAYLOAD_SIZE(next));
            if (IS_FREE(ptr)) {
                SET_END_POINTER(ptr);
                ANNOUNCE_FREE_TO_NEXT(ptr);
            } else {
                ANNOUNCE_USE_TO_NEXT(ptr);
            }

            if (last_block == next)
                last_block = ptr;

        }

        #ifdef DEBUG_COALESCE
        printf("Fine tentativo fusione in avanti\n");
        #endif

    }

    /* Fusione all'indietro */
    if ((mode == NORMAL || mode == BACKWARD) && !(IS_FIRST_BLOCK(ptr)) && IS_PREV_FREE(ptr)) {

        #ifdef DEBUG_COALESCE
        //debug_heap();
        printf("Avvio tentativo fusione in indietro...\n");
        #endif

        /* Cerco di fondere il blocco con il blocco precedente */
        void * prev = PREV(ptr);

        #ifdef DEBUG_COALESCE
        printf("Il blocco precedente locato a %u di %u e' libero\n", (unsigned int) prev, PAYLOAD_SIZE(prev));
        #endif

        /* Rimuovo il blocco dalla lista di appartenenza */
        out_list(prev);
        /* Aggiorno la dimensione del blocco fuso */
        SET_PAYLOAD_SIZE(prev, PAYLOAD_SIZE(ptr) + HEADER_SIZE + PAYLOAD_SIZE(prev));

        if (IS_FREE(ptr)) {
            SET_END_POINTER(prev);
            ANNOUNCE_FREE_TO_NEXT(prev);
        } else {
            ANNOUNCE_USE_TO_NEXT(prev);
        }

        if (last_block == ptr)
            last_block = prev;

        /* Il mio blocco ha cambiato "testa" */
        ptr = prev;
        
    }

    #ifdef DEBUG_COALESCE
    printf("Fine coalescing.\n");
    #endif

    return ptr;

}

/*
 * Funzione privata che "stacca" l'eventuale spazio residuo non
 * necessario per la richiesta di allocazione. La dimensione
 * effettivamente richiesta e' indicata con need_size e ptr e'
 * il puntatore al blocco da cui rimuovere il residuo.
 * 
 * La creazione del blocco residuo e' derivata rimuovendo la "parte" in
 * eccesso da:
 * - lato sinistro del blocco se split_side = 1
 * - lato destro del blocco se split_side = 0;
 *
 * L'operazione di split puo' invocare o meno sul nuovo blocco creato un
 * tentativo di fusione a secondo del parametro mode. Tale parametro
 * puo' assumere i valori (macro):
 * - NO_COALESCE: nessun tentativo di fusione
 * - COALESCE: tentativo di fusione
 *
 * Viene restituito il puntatore alla testa del blocco non residuo. Il
 * blocco residuo e' impostato nello stato libero ed inserito
 * nell'opportuna lista.
 */
static void * split(void * ptr, size_t need_size, int mode) {

    #ifdef DEBUG_SPLIT
    printf("Invocato split su %u di %u ma necessari %u\n", (unsigned int) ptr, PAYLOAD_SIZE(ptr), need_size);
    if (mode == NO_COALESCE)
        printf("Modalita' NO_COALESCE\n");
    else if (mode == COALESCE)
        printf("Modalita' COALESCE\n");
    #endif

    if(PAYLOAD_SIZE(ptr) > need_size + HEADER_SIZE + MIN_BLOCK_SIZE) {

        /* Il residuo viene "staccato" a destra del blocco */
        if(!split_side) {

            /* Calcolo la testa del blocco */
            void * block = ptr + need_size + HEADER_SIZE;
            /* Imposto le nuovi dimensioni */
            INIT_SIZE(block, PAYLOAD_SIZE(ptr) - need_size - HEADER_SIZE);
            SET_PAYLOAD_SIZE(ptr, need_size);
            
            if (ptr == last_block && IS_FREE(ptr))
                ANNOUNCE_FREE_TO_NEXT(ptr);
            /* Marco il blocco creato come libero */
            MARK_AS_FREE(block);
            /* Il blocco potrebbe avere un blocco successivo libero */
            if (mode == COALESCE)
                block = coalesce(block, FORWARD);
            /* Inserisco il blocco creato nella lista opportuna */
            in_list(block);
            if(block > last_block)
                last_block = block;

            split_side = !split_side; 
            return ptr;

        } else {

            /* Calcolo la testa del blocco */
            void * block = ptr + PAYLOAD_SIZE(ptr) - need_size;
            /* Imposto le nuove dimensioni */
            INIT_SIZE(block, need_size);
            SET_PAYLOAD_SIZE(ptr, PAYLOAD_SIZE(ptr) - need_size - HEADER_SIZE);
            /* Marco il blocco residuo come libero */
            MARK_AS_FREE(ptr);
            /* Il blocco potrebbe avere un blocco precedente libero */
            if (mode == COALESCE)
                ptr = coalesce(ptr, BACKWARD);
            /* Inserisco il blocco creato nella lista opportuna */
            in_list(ptr);
            if(block > last_block)
                last_block = block;

            split_side = !split_side; 
            return block;

        }

    }
    #ifdef DEBUG_SPLIT
    else printf("Non e' possibile splittare lo spazio in eccesso\n");
    #endif

    return ptr;

}

/* ----------------------------------------------------------------------------
 *  _rm_alloc_init
 * ----------------------------------------------------------------------------
 * Inizializzatore dell'allocatore, in caso di errore viene
 * restituito -1.
 */
int _rm_alloc_init(void) {

    #ifdef DEBUG
    /* Alcuni controlli preliminari */
    if (   (HEADER_SIZE % ALIGNMENT != 0)
        || (MIN_BLOCK_SIZE % ALIGNMENT != 0)
        || (FIXED_BLOCK_SIZE % ALIGNMENT != 0)
        || (MAX_FIXED % FIXED_BLOCK_SIZE) != 0) {
        
        #ifdef DEBUG
        FATAL_ERROR("HEADER_SIZE e/o MIN_BLOCK_SIZE e/o FIXED_BLOCK_SIZE e/o MAX_FIXED dimensionati in modo scorretto.");
        #endif

        return -1;
    }
    #endif

    /*
     * Calcoliamo la dimensione del heap necessaria per l'allocatore:
     * - spazio per l'array di puntatori alle liste di primo livello, a
     *   spiazzamento lineare
     * - spazio per l'array di puntatori alle liste di secondo livello
     *   a spiazzamento esponenziale
     */
    size_t heap_init_size = 0;

    if (MAX_FIXED != 0 && FIXED_BLOCK_SIZE != 0)
        heap_init_size += (((MAX_FIXED - MIN_BLOCK_SIZE) / FIXED_BLOCK_SIZE) +1) * sizeof(void *);

    #ifdef DEBUG_INIT
    printf("MAX_FIXED: %u - MIN_BLOCK_SIZE: %u - FIXED_BLOCK_SIZE: %u\n", MAX_FIXED, MIN_BLOCK_SIZE, FIXED_BLOCK_SIZE);
    printf("Indici per liste esatte: %u\n", ((MAX_FIXED - MIN_BLOCK_SIZE) / FIXED_BLOCK_SIZE) +1);
    #endif

    /* Esponente minimo per lista blocchi esponenziali? */
    for(min_exp = 0; min_exp < MAX_EXP+1; min_exp++) {

        double num;
        num = EXP_BASE << min_exp;
        
        if (num >= MIN_BLOCK_SIZE && num > MAX_FIXED) break;

    }
    if (min_exp != MAX_EXP+1)
        heap_init_size += (MAX_EXP - min_exp + 1) * sizeof(void *);

    #ifdef DEBUG_INIT
    printf("min: %u - indici liste esponenziali %u\n", min_exp, MAX_EXP - min_exp + 1);
    #endif

    ROUND(heap_init_size);
    heap_allocator_size = heap_init_size;

    #ifdef PAGE_ALLOCATION

    // Scelgo una dimensione di base multipla della dimensione di pagina
    size_t size = (heap_init_size + HEAP_GROWSIZE-1) & -HEAP_GROWSIZE;

    /*
     * Sperimentalmente e' risultato conveniente per alcune traccie
     * impostare una minima dimensione 
     */
    #ifdef __i386__
    if (size < 8696) size = 8696;
    #else
    if (size < 8864) size = 8864;
    #endif
    
    #else
    size_t size = heap_init_size;
    #endif
    
    void * addr = _rm_sbrk(size);
    if ((long int) addr == -1) {

        #ifdef DEBUG
        FATAL_ERROR("Errore nell'allocare lo spazio basico");
        #endif

        return -1;

    }

    /* Imposto tutte le liste a NULL */
    unsigned int i = 0;
    while (i < (heap_init_size / sizeof(void *)))
        NEXT_FREE(addr + i++) = NULL;

    /*
     * Ho allocato piu' spazio del dovuto, se possibile, lo spazio
     * che non mi serve lo uso per un blocco libero
     */
    if ((size - heap_init_size) >= MIN_BLOCK_SIZE + HEADER_SIZE) {

        #ifdef DEBUG_INIT
        printf("Creo un blocco di %u\n", size - heap_init_size - HEADER_SIZE);
        #endif
        void * ptr = rm_heap_lo() + heap_init_size + HEADER_SIZE;
        SET_PAYLOAD_SIZE(ptr, size - heap_init_size - HEADER_SIZE);
        MARK_AS_FREE(ptr);
        in_list(ptr);
        last_block = ptr;

    } else {

        last_block = NULL;

    }

    #ifdef DEBUG_INIT
    printf("Dimensione heap iniziale: %u\n", heap_allocator_size);
    printf("Dimensione di pagina: %u\n", HEAP_GROWSIZE);
    printf("Dimensione header: %u\n", HEADER_SIZE);
    printf("Dimensione blocco minimo: %u\n", MIN_BLOCK_SIZE);
    debug_heap();
    #endif

    return 0;
    
}


/* ----------------------------------------------------------------------------
 *  _rm_calloc
 * ----------------------------------------------------------------------------
*/
static void* _rm_calloc(size_t num, size_t size) {
    void* p = _rm_malloc(num * size);
    if(p != NULL) {
        memset(p, 0, num * size);
    }
    return p;
}


/* 
 * Primitiva per l'allocazione di un nuovo blocco di memoria di
 * dimensione pari (o superiore) alla quantita' size. In caso di
 * errore viene restituito l'indirizzo NULL.
 */
void * _rm_malloc(size_t size) {

    #ifdef DEBUG_MALLOC
    printf("Alloco %u operazione %u\n", size, op++);
    fflush(stdout);
    #endif

    /* Arrotondo in eccesso per ottenere un multiplo di ALIGNMENT */
    ROUND(size);
        
    void * addr = NULL;
    unsigned int index = get_index(size);
    char ** addr_heap = rm_heap_lo();
    char block_from_list = 0;
    
    if (size <= MAX_FIXED){

        /*
         * Se esiste cerco di prendere un blocco libero della stessa
         * taglia nelle liste di primo livello
         */
        addr = NEXT_FREE(addr_heap + index);
        if (addr != NULL)
            NEXT_FREE(addr_heap + index) = NEXT_FREE(addr);

    } else {

        /*
         * Cerco un blocco uguale o maggiore in dimensione nelle liste
         * di secondo livello
        */

        addr = NEXT_FREE(addr_heap + index);
        void * prev = NULL;
        while(addr != NULL) {

            if (PAYLOAD_SIZE(addr) >= size) break;
            else {

                prev = addr;
                addr = NEXT_FREE(addr);

            }

        }
        
        if (addr != NULL) {

            if (prev == NULL)
                NEXT_FREE(addr_heap + index) = NEXT_FREE(addr);
            else 
                NEXT_FREE(prev) = NEXT_FREE(addr);
                
        }


    }

    /*
     * Se la precendente ricerca non ha prodotto esiti positivi
     * allora cerco anche nelle successive liste di secondo livello
     */
    if (addr == NULL) {

        unsigned int max_index = get_index(~1 - ALIGNMENT);

        /* Lista successiva */
        index++;
        
        while (index <= max_index && NEXT_FREE(addr_heap + index) == NULL) {

            if (index < max_index) index++;
            else break;

        }

        #ifdef DEBUG_MALLOC
        //printf("max: %u\n", max_index);
        #endif

        /* Ho trovato un blocco libero piu' grande */
        if (index <= max_index && NEXT_FREE(addr_heap + index) != NULL) {

            addr = NEXT_FREE(addr_heap + index);
            NEXT_FREE(addr_heap + index) = NEXT_FREE(addr);

            #ifdef DEBUG_MALLOC
            //printf("Ho trovato un blocco libero a %u di %u\n", (unsigned int) addr, PAYLOAD_SIZE(addr));
            #endif
            
        }

    }

    /*
     * Se ho trovato un blocco della dimensione prevista non
     * devo fare una divisione del residuo successivamente
     */
    if (addr != NULL)
        block_from_list = 1;

    /*
     * Se non e' stato possibile trovare un blocco libero
     * sfruttabile per la richiesta, occorre espandere l'heap.
     * L'heap viene fatto crescere a pagine se e' definita la
     * macro PAGE_ALLOCATION.
     */
    if (addr == NULL) {

        /* Quanto devo far crescere? */
        size_t grow = HEAP_GROWSIZE;

        #ifdef DEBUG_MALLOC
        //printf("Grow iniziale %u\n", grow);
        #endif

        #ifdef PAGE_ALLOCATION
        grow = (size + HEADER_SIZE + HEAP_GROWSIZE-1) & -HEAP_GROWSIZE;
        #else
        grow = size + HEADER_SIZE;
        #endif

        #ifdef DEBUG_MALLOC
        printf("Sto facendo crescere l'heap di %u per un blocco di %u\n", grow, size);
        #endif

        /* Faccio crescere l'heap */
        addr = _rm_sbrk(grow);
        if ((long int) addr == -1) {

            #ifdef DEBUG
            FATAL_ERROR("Impossibile aumentare l'heap ulteriormente.");
            #endif

            return NULL;

        }

        /* Traslo l'indirizzo di HEADER_SIZE */
        addr += HEADER_SIZE;

        /* Scrivo l'header */
        INIT_SIZE(addr, grow - HEADER_SIZE);
        if (last_block != NULL && IS_FREE(last_block)) {
            ANNOUNCE_FREE_TO_NEXT(last_block);
        }

    }

    /* Eventualmente splitto il residuo in eccesso */
    if(block_from_list)
        addr = split(addr, size, NO_COALESCE);
    else
        addr = split(addr, size, COALESCE);

    #ifdef DEBUG_MALLOC
    //printf("Sto marcando %u di %u con flag %u.\n", size, (unsigned int)addr, GET_FLAG(addr));
    #endif

    /* Marco il blocco come in uso */
    MARK_AS_USED(addr);

    if(addr > last_block)
        last_block = addr;

    #ifdef DEBUG_MALLOC
    debug_heap();
    printf("Allocato blocco di %u ad %u con flag %u.\n", size, (unsigned int)addr, GET_FLAG(addr));
    #endif

    #ifdef DEBUG
    if (IS_FREE(addr))
        FATAL_ERROR("Il blocco risulta libero ma e' stato appena allocato!\n");
    #endif

    return addr;

}

/*
 * Primitiva per il rilascio al sistema di un blocco in uso
 * precedentemente allocato con BSA. Prende come parametro il
 * puntatore al blocco e non restuisce alcunche'. Ammessa la
 * possibilita' di ricevere un indirizzo NULL.
 */
void _rm_free(void * ptr) {
    
    if(ptr == NULL) return;

    #ifdef DEBUG_FREE
    printf("Libero blocco a %u di %u, operazione %u\n", (unsigned int) ptr, PAYLOAD_SIZE(ptr), op++);
    //debug_heap();
    fflush(stdout);
    #endif

    /* Fondiamo i blocchi liberi (precedente o successivo) ove possibile */
    ptr = coalesce(ptr, NORMAL);

    /* Marco il nuovo blocco libero */
    MARK_AS_FREE(ptr);

    /* Inserisco il blocco nella lista appropriata */
    in_list(ptr);

    #ifdef DEBUG_FREE
    //printf("Libero blocco a %u di %u, operazione %u\n", (unsigned int) ptr, PAYLOAD_SIZE(ptr), op++);
    debug_heap();
    fflush(stdout);
    #endif

    return;
    
}

/*
 * Primitiva per il ridimensionamento di un blocco precedentemente
 * allocato con BSA. La nuova dimensione size, puo' inferiore o
 * superiore all'originale. Il primo parametro e' il puntatore al
 * blocco ed e' ammesso che esso sia pari a NULL (in tal caso la
 * primitica e' equivalente ad una malloc(size)). Se la nuova dimensione
 * e' nulla, il comportamente e' equivalente ad una free(ptr).
 */
void * _rm_realloc(void * ptr, size_t size) {

    void * newptr = NULL;

    #ifdef DEBUG_REALLOC
    printf("Realloc di %u su %u\n", size, (unsigned int) ptr);
    if (ptr != NULL) printf("Blocco originale di %d\n", PAYLOAD_SIZE(ptr));
    #endif

    if (size > 0) {

        ROUND(size);

        if (ptr != NULL) {
            
            /* Calcola la dimensione effettiva del vecchio blocco */
            size_t oldsize = PAYLOAD_SIZE(ptr);

            if (size > oldsize) {

                /*
                 * Proviamo a vedere se  c'e' possibilita di accorpare
                 * blocchi liberi consecutivi per soddisfare la richiesta
                 */
                ptr = coalesce(ptr, FORWARD);

                /*
                 * Se ho effettuato una fusione, devo segnalare al blocco
                 * successivo che non sono piu' libero
                 */
                if (PAYLOAD_SIZE(ptr) > oldsize) {

                    ANNOUNCE_USE_TO_NEXT(ptr);

                }

                
                /* Sono riuscito nel mio intento */
                if (PAYLOAD_SIZE(ptr) >= size) {

                    #ifdef DEBUG_REALLOC
                    debug_heap();
                    #endif
                    
                    return ptr;
                }

                /*
                 * Se il blocco da espandere e' alla fine dell'heap
                 * proviamo semplicemente ad espandere per la differenza
                 */
                if (ptr == last_block) {

                    //printf("Espando heap...\n");

                    size_t diff = size - oldsize;

                    /* Quanto devo far crescere? */
                    #ifdef PAGE_ALLOCATION
                    size_t grow = (diff + HEAP_GROWSIZE-1) & -HEAP_GROWSIZE;
                    #else
                    size_t grow = diff;
                    #endif
                    
                    void * bp = _rm_sbrk(grow);
                    if ((long int) bp == -1) {

                        #ifdef DEBUG
                        FATAL_ERROR("Impossibile espandere l'heap.");
                        #endif
                        
                        return NULL;
                    }

                    /* Aggiorno la dimensione del blocco */
                    SET_PAYLOAD_SIZE(ptr, PAYLOAD_SIZE(ptr) + grow);
                    /*
                     * Non voglio che il blocco residuo eventuale
                     * sia "staccato" a sinistra, altrimenti perderei
                     * la parte che contiene i precedenti dati
                     * contenuti nel blocco e sarebbe necessario
                     * farne una copia
                     */
                    #ifdef PAGE_ALLOCATION
                    split_side = 0;
                    ptr = split(ptr, size, NO_COALESCE);
                    #endif
                    return ptr;
                }

            } else {

                /* Effetto un trancamento, creando un blocco libero */
                //split(ptr, size, COALESCE);
                return ptr;
            }


            /*
             * Lo spazio contiguo non e' sufficiente, occorre
             * allocare nuovo spazio
            */
            newptr = _rm_malloc(size);
            if (newptr == NULL) {

                #ifdef DEBUG
                FATAL_ERROR("Ottenuto indirizzo non valido in fase di realloc.");
                #endif
                
                return NULL;

            }

            if (ptr != NULL) {

                if (oldsize < size)
                    memcpy(newptr, ptr, oldsize);
                else
                    memcpy(newptr, ptr, size);
            }
        }
    }

    _rm_free(ptr);
    return newptr;
}

#ifdef DEBUG
/*
 * Funzione di debug che stampa in output:
 * - indirizzo di inizio, fine dell'heap
 * - dimensione dell'heap
 * - il numero di liste della lookup table
 * - i blocchi presenti in ciascuna lista (con relative informazioni
 *   su di essi)
 * - i blocchi nell'heap
 */
static void debug_heap(){

    unsigned int head = (unsigned int) rm_heap_lo();
    unsigned int end = (unsigned int) rm_heap_hi() + 1;
    unsigned int size = end - head;
    printf("Heap -> inizio %u - fine: %u - size: %u\n", head, end, size);

    unsigned int num_list = get_index(~1 - ALIGNMENT) + 1;
    printf("Sono presenti %u liste\n", num_list);

    char stop = 0;

    unsigned index;
    for(index = 0; index < num_list; index++) {

        void * ptr = NEXT_FREE(head + index);
        if (ptr != NULL) printf("(%u) -> Lista[%u]:\n", index, (unsigned int)((char **)head + index));
        unsigned int block_num = 0;
        while(ptr != NULL) {

            printf("\tBlocco[%u] -> inizio: %u - size: %u - libero: %u - flag: %u - next: %u", block_num, (unsigned int) ptr, PAYLOAD_SIZE(ptr), IS_FREE(ptr), GET_FLAG(ptr), (unsigned int) NEXT(ptr));
            if (!IS_FIRST_BLOCK(ptr)) { 
                if(IS_PREV_FREE(ptr))
                    printf(" - prev: %u\n", (unsigned int) PREV(ptr));
                else
                    printf(" - prev: in uso\n");
            } else
                printf("\n");
                
            if (!IS_FREE(ptr)) {
                printf("\t\tIl blocco non risulta libero!\n");
                stop = 1;
            }

            ptr = NEXT_FREE(ptr);
            block_num++;
            
        }

    
    }

    void * ptr = (char *)head + heap_allocator_size + HEADER_SIZE;
    printf("Blocco in heap:\n");
    index = 0;
    while (ptr < (void *)end) {

        printf("\tBlocco[%u] -> inizio: %u - size: %u - libero: %u - flag: %u - next: %u ",index, (unsigned int) ptr, PAYLOAD_SIZE(ptr), IS_FREE(ptr), GET_FLAG(ptr), (unsigned int) NEXT(ptr));
        if (!IS_FIRST_BLOCK(ptr)) { 
            if(IS_PREV_FREE(ptr))
                printf(" - prev: %u ", (unsigned int) PREV(ptr));
            else
                printf(" - prev: in uso ");
        }
        if(IS_FREE(ptr))
            if (ptr != *((char **)((char *)ptr + PAYLOAD_SIZE(ptr) - sizeof(void *)))) {
                printf("- end_pointer: NON VALIDO ");
                //stop = 1;
            }

        if (IS_FREE(ptr))
            printf("- next_free: %u", (unsigned int) NEXT_FREE(ptr));
        printf("\n");

        ptr = NEXT(ptr);
        index++;

    }

    if (stop) exit(EXIT_FAILURE);

}
#endif


/* Copyright (c) 2010 the rm team

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
