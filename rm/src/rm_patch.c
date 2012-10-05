/* ============================================================================
 *  rm_patch.c
 * ============================================================================

 *  Author:         (c) 2010 the rm team
 *  License:        See the end of this file for license information
 *  Created:        July 21, 2010
 *  Note:           code patching module

 *  Last changed:   $Date: 2011/01/06 17:24:42 $
 *  Changed by:     $Author: alex $
 *  Revision:       $Revision: 1.16 $
*/

#include "_rm_private.h"
#include "_rm_asm.h"

#define MIN_BB_SIZE     5

extern char _etext;

Elf32_Ehdr *elf_header;     /* ELF header */
Elf *elf;                   /* Our Elf pointer for libelf */
Elf_Scn *scn;               /* Section Descriptor */
Elf_Data *edata;            /* Data Descriptor */
GElf_Sym sym;               /* Symbol */
GElf_Shdr shdr;             /* Section Header */

/*array for code cache...*/
GArray* g_cc_array;

GArray* mov_array;          // Array delle istruzioni move (che potrebbero essere patchate)
GArray* branch_array;       // Array delle istruzioni di branch (jmp e call relativi) che devono essere rilocate nelle patch
GArray* BB_array;           // Array dei Basic Block
GHashTable* mov_hash;


// Funzione per il confronto tra indirizzi, utilizzata per l'ordinamento
gint compareAddress(gconstpointer a, gconstpointer b) {
    unsigned int address_a = *(unsigned int*)a;
    unsigned int address_b = *(unsigned int*)b;
    if(address_a < address_b) return -1;
    else if(address_a > address_b) return 1;
    else return 0;  // if(address_a = address_b)
}


// private function prototypes
static void _insert_mov_in_cache(void* movAddress);
static int _get_func_addr(GArray* out_jmp_array);




/* ----------------------------------------------------------------------------
 *  _rm_patch_install
 * ----------------------------------------------------------------------------
*/
int _rm_patch_install(ucontext_t* mycontext) {
    mov_rec* the_mov_instr;
    the_mov_instr = g_hash_table_lookup(mov_hash, (void*)mycontext->uc_mcontext.gregs[14]);
    if(the_mov_instr) {
        
        void *the_futureBB_start;
        unsigned int the_futureBB_pos = 0;
        int jmp_disp, jmp_disp2;
        BB_rec* the_BB = &g_array_index(BB_array, BB_rec, the_mov_instr->BB_index);
        unsigned int the_patch_size = the_mov_instr->patch_size_func(the_mov_instr);
        int i;
        
#if rm_STAT == 1
        g_stats.actually_patched_instr_count++;
        if(the_BB->originalBB_start == the_BB->BB_start) {  // BB non patchato
            g_stats.actually_patched_BB_count++;
        }
#endif
        
        
#if PATCH_DEBUG == 1
        printf("**********************mov hash*********************************\n");
        printf("the_mov_instr address = %x\n", (unsigned int)the_mov_instr->address);
        printf("the_mov_instr address_end = %x\n", (unsigned int)the_mov_instr->address_end);
        printf("the_mov_instr BB_index = %d\n", the_mov_instr->BB_index);
        
        printf("the_BB originalBB_start = %x\n", (unsigned int)the_BB->originalBB_start);
        printf("the_BB originalBB_end = %x\n", (unsigned int)the_BB->originalBB_end);
        printf("the_BB BB_start = %x\n", (unsigned int)the_BB->BB_start);
        printf("the_BB BB_end = %x\n", (unsigned int)the_BB->BB_end);
        printf("the_BB mov_index = %d\n", the_BB->mov_index);
        printf("the_BB mov_number = %d\n", the_BB->mov_number);
#endif
        
        
        // Allocazione dinamica della memoria in cui inseriremo il nuovo codice
        unsigned int the_futureBB_size = (the_mov_instr->address - the_BB->BB_start) + the_patch_size + (the_BB->BB_end - the_mov_instr->address_end) + 5;
        if(the_BB->originalBB_start == the_BB->BB_start) {  // BB non patchato
            for(i = the_BB->branch_index; i < the_BB->branch_index + the_BB->branch_number; i++) {
                branch_rec* the_branch = &g_array_index(branch_array, branch_rec, i);
                if(the_branch->size_to_add_in_patch != 0) { // short jump
                    the_futureBB_size = the_futureBB_size + the_branch->size_to_add_in_patch;
                }
            }
        }
        the_futureBB_start = _rm_exec_sbrk(the_futureBB_size);
        if(the_futureBB_start == (void*)-1) {
            return -1;
        }
        
        
        // Copia codice originale (prima dell'operazione da patchare)
        if(the_BB->originalBB_start == the_BB->BB_start) {  // BB non patchato
            void* the_source_addr = the_BB->BB_start;
            for(i = the_BB->branch_index; i < the_BB->branch_index + the_BB->branch_number; i++) {
                branch_rec* the_branch = &g_array_index(branch_array, branch_rec, i);
                if(the_branch->address_end > the_mov_instr->address) {
                    break;
                }
                if(the_branch->size_to_add_in_patch != 0) { // short jump
                    memcpy(the_futureBB_start + the_futureBB_pos, the_source_addr, the_branch->address_end - the_source_addr);
                    the_futureBB_pos = the_futureBB_pos + (the_branch->address_end - the_source_addr) + the_branch->size_to_add_in_patch;
                    the_source_addr = the_branch->address_end;
                }
            }
            if(the_mov_instr->address > the_source_addr) {
                memcpy(the_futureBB_start + the_futureBB_pos, the_source_addr, the_mov_instr->address - the_source_addr);
                the_futureBB_pos = the_futureBB_pos + (the_mov_instr->address - the_source_addr);
            }
        }
        else {  // BB patchato
            memcpy(the_futureBB_start + the_futureBB_pos, the_BB->BB_start, the_mov_instr->address - the_BB->BB_start);
            the_futureBB_pos = the_futureBB_pos + (the_mov_instr->address - the_BB->BB_start);
        }
        
        unsigned int the_patch_start_pos = the_futureBB_pos;
        
        
        // Patch del codice originale (operazione da patchare)
        the_mov_instr->patch_func(the_mov_instr, the_futureBB_start + the_futureBB_pos);
        the_futureBB_pos = the_futureBB_pos + the_patch_size;
        
        
        // Copia codice originale (dopo l'operazione da patchare)
        if(the_BB->originalBB_start == the_BB->BB_start) {  // BB non patchato
            void* the_source_addr = the_mov_instr->address_end;
            for(i = the_BB->branch_index; i < the_BB->branch_index + the_BB->branch_number; i++) {
                branch_rec* the_branch = &g_array_index(branch_array, branch_rec, i);
                if(the_branch->address_end < the_mov_instr->address_end) {
                    continue;
                }
                if(the_branch->size_to_add_in_patch != 0) { // short jump
                    memcpy(the_futureBB_start + the_futureBB_pos, the_source_addr, the_branch->address_end - the_source_addr);
                    the_futureBB_pos = the_futureBB_pos + (the_branch->address_end - the_source_addr) + the_branch->size_to_add_in_patch;
                    the_source_addr = the_branch->address_end;
                }
            }
            if(the_BB->BB_end > the_source_addr) {
                memcpy(the_futureBB_start + the_futureBB_pos, the_source_addr, the_BB->BB_end - the_source_addr);
                the_futureBB_pos = the_futureBB_pos + (the_BB->BB_end - the_source_addr);
            }
        }
        else {  // BB patchato
            memcpy(the_futureBB_start + the_futureBB_pos, the_mov_instr->address_end, the_BB->BB_end - the_mov_instr->address_end);
            the_futureBB_pos = the_futureBB_pos + (the_BB->BB_end - the_mov_instr->address_end);
        }
        
        
        // Inserimento del salto alla fine del codice originale (salto con indirizzo relativo)
        // N.B. inserisco manualmente il codice binario in memoria (l'opcode del jump)
        memset(the_futureBB_start + the_futureBB_pos, 0xe9, 1);
    //  the_futureBB_pos = the_futureBB_pos + 1;
        the_futureBB_pos++;
        
        jmp_disp = the_BB->originalBB_end - (the_futureBB_start + the_futureBB_pos + 4);
        memcpy(the_futureBB_start + the_futureBB_pos, &jmp_disp, 4);
        the_futureBB_pos = the_futureBB_pos + 4;
        
        
        // Sprotegge pagina del codice originale
        unsigned int page_start = ((unsigned int)the_BB->originalBB_start & ~(getpagesize()-1));
        unsigned int page_end = ((unsigned int)(the_BB->originalBB_end-1) & ~(getpagesize()-1)) + getpagesize();
        if(-1 == mprotect((void*)page_start, page_end-page_start, PROT_READ|PROT_WRITE|PROT_EXEC)) {
            _rm_error(rm_MPROTECT_FAIL, "_rm_patch_install: mprotect() failed!\n");
            return -1;
        }
        
        
        // Modifica il codice originale inserendo un salto al nuovo codice (salto con indirizzo relativo)
        // N.B. inserisco manualmente il codice binario in memoria (l'opcode del jump)
        if(the_BB->originalBB_start == the_BB->BB_start) {  // BB non patchato
            memset(the_BB->originalBB_start, 0xe9, 1);
        }
        
    //  jmp_disp2 = the_futureBB_start - (the_BB->originalBB_start + 1 + 4);
        jmp_disp2 = the_futureBB_start - (the_BB->originalBB_start + 5);
        memcpy(the_BB->originalBB_start + 1, &jmp_disp2, 4);
        
        
        // Azzera il BB originale (dopo il jmp)
        if(the_BB->originalBB_start == the_BB->BB_start) {  // BB non patchato
            memset(the_BB->originalBB_start + 5, rm_ILLEGAL_INSTRUCTION, the_BB->originalBB_end - (the_BB->originalBB_start + 5));
        }
        
        
        // Reimposta protezione pagina del codice originale
        if(-1 == mprotect((void*)page_start, page_end-page_start, PROT_READ|PROT_EXEC)) {
            _rm_error(rm_MPROTECT_FAIL, "_rm_patch_install: mprotect() failed!\n");
            return -1;
        }
        
        
#if PATCH_DEBUG == 1
        // Print PATCH (only for debug)
        printf("***** BEFORE PRINT *****\n");

        printf("A*** the_mov_instr->address_end (x): %x\n", (unsigned int)the_mov_instr->address_end);
        printf("A*** the_mov_instr->address (x): %x\n", (unsigned int)the_mov_instr->address);
        unsigned int PRINT_BUF_SIZE = the_patch_size;
        printf("PRINT_BUF_SIZE: %d\n", PRINT_BUF_SIZE);
        unsigned char print_buf[PRINT_BUF_SIZE];
        
        memcpy(print_buf, the_futureBB_start + the_patch_start_pos, PRINT_BUF_SIZE);
        
        printf("***** print PATCH: ***** start\n");
        
#if rm_DISASSEMBLER == rm_LIBDISASM
        int PRINT_LINE_SIZE = 1000;
        char print_line[PRINT_LINE_SIZE];
        unsigned int print_pos = 0;
        int print_size;
        x86_insn_t print_insn;
        while ( print_pos < PRINT_BUF_SIZE ) {
            /* disassemble address */
            print_size = x86_disasm(print_buf, PRINT_BUF_SIZE, 0, print_pos, &print_insn);
            if(print_size) {
                /* print instruction */
                x86_format_insn(&print_insn, print_line, PRINT_LINE_SIZE, att_syntax);
                printf("%s\n", print_line);
        
        //printf("opcode: ");
        //int i;
        //for(i=0; i<print_size; i++) {
        //  printf("%02x ", print_buf[print_pos+i]);
        //}
        //printf("\n");
        
                print_pos += print_size;
            }
            else {
                printf("Invalid instruction\n");
                print_pos++;
            }
        }

#else

        _DInst insn[rm_MAX_DIS_INSTRUCTIONS];   /* instructions */
        unsigned int instructions_count = 0;
        unsigned int next = 0;
        _DecodedInst inst;                      /* instruction (string) */
        
        _CodeInfo ci;
        ci.dt = Decode32Bits;
        ci.features = DF_NONE;
        
        ci.code = print_buf;
        ci.codeLen = PRINT_BUF_SIZE;
        ci.codeOffset = (unsigned int)print_buf;
        ci.nextOffset = 0;
        
        while(ci.codeLen > 0) {
            /* disassemble address */
            distorm_decompose(&ci, insn, rm_MAX_DIS_INSTRUCTIONS, &instructions_count);
            
            int k;
            for(k=0; k<instructions_count; k++) {
                if(insn[k].flags == FLAG_NOT_DECODABLE) {
                    printf("Invalid instruction\n");
                }
                else {
                    /* print instruction */
                    distorm_format(&ci, &insn[k], &inst);
                    printf("%s\t%s\n", inst.mnemonic.p, inst.operands.p);
            
            //printf("opcode: ");
            //int i;
            //for(i=0; i<insn[k].size; i++) {
            //  printf("%02x ", print_buf[insn[k].addr-print_buf+i]);
            //}
            //printf("\n");
                }
            }
            
            next = ci.nextOffset - ci.codeOffset;
            ci.code += next;
            ci.codeLen -= next;
            ci.codeOffset += next;
        }
#endif
        
        printf("***** print PATCH: ***** end\n");
#endif
        
        
        // Aggiorna i valori della mov patchata (flag che indica che è stata applicata la patch)
        the_mov_instr->patched = TRUE;
        
        
        // Aggiorna i valori delle mov e delle istruzioni di branch presenti nel BB patchato (indirizzi nel nuovo blocco)
        int the_BB_offset = the_futureBB_start - the_BB->BB_start;
        void* the_mov_instr_addr = the_mov_instr->address;
        void* the_mov_instr_addr_end = the_mov_instr->address_end;
        unsigned int the_added_size = 0;
        branch_rec* the_branch;
        int the_branch_disp;
        int j = the_BB->branch_index;
        for(i = the_BB->mov_index; i < the_BB->mov_index + the_BB->mov_number; i++) {
            mov_rec* the_mov;
            the_mov = &g_array_index(mov_array, mov_rec, i);
            // *** AGGIORNA ISTRUZIONI DI BRANCH ***
            while(j<the_BB->branch_index + the_BB->branch_number && 
                (the_branch = &g_array_index(branch_array, branch_rec, j))->address < the_mov->address) {   // istruzioni di branch precedenti alla mov
                
                if(the_branch->address <= the_mov_instr_addr) { // istruzione di branch precedente alla mov patchata
                    the_branch->address = the_branch->address + the_BB_offset + the_added_size;
                    the_branch->address_end = the_branch->address_end + the_BB_offset + the_added_size;
                }
                else { // istruzione di branch successiva alla mov patchata
                    the_branch->address = the_branch->address + the_BB_offset - (the_mov_instr_addr_end - the_mov_instr_addr) + the_patch_size + the_added_size;
                    the_branch->address_end = the_branch->address_end + the_BB_offset - (the_mov_instr_addr_end - the_mov_instr_addr) + the_patch_size + the_added_size;
                }
                
                if(the_BB->originalBB_start == the_BB->BB_start && the_branch->size_to_add_in_patch != 0) { // short jump in BB non patchato
                    the_added_size = the_added_size + the_branch->size_to_add_in_patch;
                    the_branch->address_end = the_branch->address_end + the_branch->size_to_add_in_patch;
                    // modifica opcode da short a long
                    memcpy(the_branch->address, the_branch->new_opcode, the_branch->new_opcode_size);
                }
                
                // modifica displacement del branch
                the_branch_disp = the_branch->jump_target - the_branch->address_end;
                memcpy(the_branch->address_end - 4, &the_branch_disp, 4);
                
                j++;
            }
            // *** AGGIORNA MOV ***
            if(!the_mov->patched) {
                g_hash_table_remove(mov_hash, the_mov->address);
                if(the_mov->address <= the_mov_instr_addr) { // mov precedente alla mov patchata
                    the_mov->address = the_mov->address + the_BB_offset + the_added_size;
                    the_mov->address_end = the_mov->address_end + the_BB_offset + the_added_size;
                }
                else { // mov successiva alla mov patchata
                    the_mov->address = the_mov->address + the_BB_offset - (the_mov_instr_addr_end - the_mov_instr_addr) + the_patch_size + the_added_size;
                    the_mov->address_end = the_mov->address_end + the_BB_offset - (the_mov_instr_addr_end - the_mov_instr_addr) + the_patch_size + the_added_size;
                }
                g_hash_table_insert(mov_hash, the_mov->address, the_mov);
            }
#if rm_DEBUG == 1
            // Aggiorna anche i valori delle mov patchate (serve per il dump file)
            else {
                g_hash_table_remove(mov_hash, the_mov->address);
                if(the_mov->address <= the_mov_instr_addr) { // mov precedente alla mov patchata o proprio mov patchata
                    the_mov->address = the_mov->address + the_BB_offset + the_added_size;
                    the_mov->address_end = the_mov->address_end + the_BB_offset + the_added_size;
                }
                else { // mov successiva alla mov patchata
                    the_mov->address = the_mov->address + the_BB_offset - (the_mov_instr_addr_end - the_mov_instr_addr) + the_patch_size + the_added_size;
                    the_mov->address_end = the_mov->address_end + the_BB_offset - (the_mov_instr_addr_end - the_mov_instr_addr) + the_patch_size + the_added_size;
                }
                g_hash_table_insert(mov_hash, the_mov->address, the_mov);
            }
#endif
        }
        // *** AGGIORNA ISTRUZIONI DI BRANCH ***
        while(j<the_BB->branch_index + the_BB->branch_number) { // istruzioni di branch successive all'ultima mov
            the_branch = &g_array_index(branch_array, branch_rec, j);
            
            if(the_branch->address <= the_mov_instr_addr) { // istruzione di branch precedente alla mov patchata
                the_branch->address = the_branch->address + the_BB_offset + the_added_size;
                the_branch->address_end = the_branch->address_end + the_BB_offset + the_added_size;
            }
            else { // istruzione di branch successiva alla mov patchata
                the_branch->address = the_branch->address + the_BB_offset - (the_mov_instr_addr_end - the_mov_instr_addr) + the_patch_size + the_added_size;
                the_branch->address_end = the_branch->address_end + the_BB_offset - (the_mov_instr_addr_end - the_mov_instr_addr) + the_patch_size + the_added_size;
            }
            
            if(the_BB->originalBB_start == the_BB->BB_start && the_branch->size_to_add_in_patch != 0) { // short jump in BB non patchato
                the_added_size = the_added_size + the_branch->size_to_add_in_patch;
                the_branch->address_end = the_branch->address_end + the_branch->size_to_add_in_patch;
                // modifica opcode da short a long
                memcpy(the_branch->address, the_branch->new_opcode, the_branch->new_opcode_size);
            }
            
            // modifica displacement del branch
            the_branch_disp = the_branch->jump_target - the_branch->address_end;
            memcpy(the_branch->address_end - 4, &the_branch_disp, 4);
            
            j++;
        }
        
        
        // Aggiorna i valori del BB patchato (indirizzi del nuovo blocco)
        the_BB->BB_start = the_futureBB_start;
        the_BB->BB_end = the_futureBB_start + the_futureBB_pos - 5; //*** il jmp inserito alla fine non viene considerato parte del BB ***
        
        
        
        /*advances program counter...*/
        /*advances EIP register (offset 14 in gregs struct as documented in sys/ucontext.h)...*/
    //  mycontext->uc_mcontext.gregs[14] = mycontext->uc_mcontext.gregs[14] + the_BB_offset;
        mycontext->uc_mcontext.gregs[14] = (unsigned int)the_futureBB_start + the_patch_start_pos;
        //printf ("context changed...new EIP is %p\n", mycontext->uc_mcontext.gregs[14]);
        
        return 1;
    }
    
    return 0;
}




/* ----------------------------------------------------------------------------
 *  _rm_patch_init
 * ----------------------------------------------------------------------------
*/
int _rm_patch_init(void) {
    
//  void* code_start = (void*)rm_CODE_START;
    void* code_end = &_etext;
//  unsigned int BUF_SIZE = code_end-code_start;
//  unsigned char buf[BUF_SIZE];    /* buffer of bytes to disassemble */
    
    GArray* func_array;     // Array degli indirizzi delle funzioni
    func_array = g_array_new(FALSE, FALSE, sizeof(void*));
    
    GArray* jmp_array;      // Array degli indirizzi di arrivo dei salti (che serviranno per dividere il codice in Basic Block)
    jmp_array = g_array_new(FALSE, FALSE, sizeof(void*));
    
    mov_rec movNext;
//  GArray* mov_array;      // Array delle istruzioni move (che potrebbero essere patchate)
    movNext.patched = FALSE;
    mov_array = g_array_new(FALSE, FALSE, sizeof(mov_rec));
    
    branch_rec branchNext;
    branch_array = g_array_new(FALSE, FALSE, sizeof(branch_rec));
    
    BB_rec BBNext;
//  GArray* BB_array;       // Array dei Basic Block
    
    
    /* get addresses of all the functions */
    if (_get_func_addr(jmp_array) == -1) {
        return -1;
    }
    
    // Array salti: aggiungo anche code_end
    g_array_append_val(jmp_array, code_end);
    
    /* copy addresses of all the functions in func_array */
    g_array_append_vals(func_array, jmp_array->data, jmp_array->len);
    
    // Array funzioni: Ordinamento
    g_array_sort(func_array, compareAddress);
    
    int i;
    // Array funzioni: Rimozione doppioni
    for(i=1; i<func_array->len; i++) {
        if(g_array_index(func_array, void*, i) == g_array_index(func_array, void*, i-1)) {
            g_array_remove_index(func_array, i);
            i--;
        }
    }
    
    
    
    //*********************** COPIA MEMORIA - inizio ***********************************
/*#if PATCH_DEBUG == 1
    printf("BUF_SIZE: %d\n", BUF_SIZE);
#endif
    memcpy(buf, code_start, BUF_SIZE);
    
/*  __asm__ ("pro_start:");
    int a = 5;
    __asm__ ("pro_end:");
    int end = &pro_end;
    int start = &pro_start;
    BUF_SIZE = end-start;
    printf("end: %x\n", end);
    printf("start: %x\n", start);
    printf("BUF_SIZE: %d\n", BUF_SIZE);
    memcpy(buf, &pro_start, BUF_SIZE);*/
    //*********************** COPIA MEMORIA - fine *************************************
    
    
    
#if rm_DISASSEMBLER == rm_LIBDISASM
//  int LINE_SIZE = 1000;
//  char line[LINE_SIZE];       /* buffer of line to print */
    unsigned int pos = 0;       /* current position in buffer */
    int size;                   /* size of instruction */
    x86_insn_t insn;            /* instruction */
    
    /*initializes libdisasm...*/
    x86_init(opt_none, NULL, NULL);
    
    for(i=0; i<func_array->len-1; i++) {
        void *func_start, *func_end;
        func_start = g_array_index(func_array, void*, i);
        func_end = g_array_index(func_array, void*, i+1);
        pos = 0;
        while (pos < func_end-func_start) {
            /* disassemble address */
            size = x86_disasm(func_start, func_end-func_start, 0, pos, &insn);
        //  size = x86_disasm(buf, BUF_SIZE, 0, pos, &insn);    //*********************** provare a disassemblare senza usare buffer ****************************
            if (size) {
                /* print instruction */
            //  x86_format_insn(&insn, line, LINE_SIZE, att_syntax);
            //  printf("%s\n", line);
                
                if (_rm_decode(insn, func_start+pos, &movNext) == 1) {
                    g_array_append_val(mov_array, movNext);
                }
                else if (_rm_is_branch(insn, func_start+pos, &branchNext, jmp_array) == 1) {
                    g_array_append_val(branch_array, branchNext);
            //      printf("*******************************jmp_array address = %x\n", (unsigned int)g_array_index(jmp_array, void*, jmp_array->len - 1));
                }
                
                pos += size;
            }
            
            else {
    #if PATCH_DEBUG == 1
                printf("Invalid instruction\n");
                printf("pos: %d\n", pos);
                printf("func_start+pos: %x\n", (unsigned int)func_start+pos);
    #endif
                pos++;
            }
            
        //  x86_oplist_free(&insn); //************** va inserita per ripulire la lista di operandi della insn prima di disassemblare la successiva istruzione ****************
                                    //************** (Caller is responsible for calling x86_oplist_free() on a reused "insn" to avoid leaking memory when calling x86_disasm() repeatedly) **********
        }
    }
    
    /*uninitializes libdisasm...*/
//  x86_cleanup();

#else

    _DInst insn[rm_MAX_DIS_INSTRUCTIONS];   /* instructions */
    unsigned int instructions_count = 0;
    unsigned int next = 0;
//  _DecodedInst inst;                      /* instruction (string) */
    
    _CodeInfo ci;
    ci.dt = Decode32Bits;
    ci.features = DF_NONE;
    
    for(i=0; i<func_array->len-1; i++) {
        void *func_start, *func_end;
        func_start = g_array_index(func_array, void*, i);
        func_end = g_array_index(func_array, void*, i+1);
        next = 0;
        
        ci.code = func_start;
        ci.codeLen = func_end-func_start;
        ci.codeOffset = (unsigned int)func_start;
        ci.nextOffset = 0;
        
        while(ci.codeLen > 0) {
            /* disassemble address */
            distorm_decompose(&ci, insn, rm_MAX_DIS_INSTRUCTIONS, &instructions_count);
            
            int k;
            for(k=0; k<instructions_count; k++) {
                if(insn[k].flags == FLAG_NOT_DECODABLE) {
    #if PATCH_DEBUG == 1
                    printf("Invalid instruction\n");
                    printf("ci.code: %x\n", (unsigned int)ci.code);
                    printf("insn[k].addr: %x\n", (unsigned int)insn[k].addr);
    #endif
                /*  _rm_error(rm_INVALID_INSN, "_rm_patch_init: invalid instruction\n");
                    g_array_free(func_array, TRUE);
                    g_array_free(jmp_array, TRUE);
                    return -1;*/
                }
                else {
                    /* print instruction */
                //  distorm_format(&ci, &insn[k], &inst);
                //  printf("%s\t%s\n", inst.mnemonic.p, inst.operands.p);
                    
                    if (_rm_decode(insn[k], &movNext) == 1) {
                        g_array_append_val(mov_array, movNext);
                    }
                    else if (_rm_is_branch(insn[k], &branchNext, jmp_array) == 1) {
                        g_array_append_val(branch_array, branchNext);
                //      printf("*******************************jmp_array address = %x\n", (unsigned int)g_array_index(jmp_array, void*, jmp_array->len - 1));
                    }
                }
            }
            
            next = ci.nextOffset - ci.codeOffset;
            ci.code += next;
            ci.codeLen -= next;
            ci.codeOffset += next;
        }
    }
#endif
    
    
    // Array salti: Ordinamento
    g_array_sort(jmp_array, compareAddress);
    
//  int i;
#if PATCH_DEBUG == 1
    printf("**********************ordinato***************\n");
    for(i=0; i<jmp_array->len; i++) {
        printf("jmp_array address index (%d) = %x\n", i, (unsigned int)g_array_index(jmp_array, void*, i));
    }
#endif
    
    // Array salti: Rimozione doppioni
    for(i=1; i<jmp_array->len; i++) {
        if(g_array_index(jmp_array, void*, i) == g_array_index(jmp_array, void*, i-1)) {
            g_array_remove_index(jmp_array, i);
            i--;
        }
    }
    
#if PATCH_DEBUG == 1
    printf("**********************senza doppioni***************\n");
    for(i=0; i<jmp_array->len; i++) {
        printf("jmp_array address index (%d) = %x\n", i, (unsigned int)g_array_index(jmp_array, void*, i));
    }
#endif
    
    // Creazione array istruzioni patchabili, cache, BB
    BB_array = g_array_new(FALSE, FALSE, sizeof(BB_rec));
    g_cc_array = g_array_new(FALSE, FALSE, sizeof(rm_CODECACHEDATA));
    GArray* the_cc_addr_array = g_array_new(FALSE, FALSE, sizeof(void*));
    int j = 1;
    void* movAddress;
    for(i=0; i<mov_array->len; i++) {
        movAddress = g_array_index(mov_array, mov_rec, i).address;
        while(movAddress >= g_array_index(jmp_array, void*, j)) {
            j++;
        }
        BBNext.originalBB_start = g_array_index(jmp_array, void*, j-1);
        BBNext.originalBB_end = g_array_index(jmp_array, void*, j);
        if(BBNext.originalBB_end - BBNext.originalBB_start >= MIN_BB_SIZE) {    // BB abbastanza grande --> istruzione patchabile
            if(BB_array->len == 0 || 
               BBNext.originalBB_start != g_array_index(BB_array, BB_rec, BB_array->len - 1).originalBB_start) {    // BB nuovo
                BBNext.BB_start = BBNext.originalBB_start;
                BBNext.BB_end = BBNext.originalBB_end;
                BBNext.mov_index = i;
                BBNext.mov_number = 1;
                g_array_append_val(BB_array, BBNext);
            }
            else {  // BB esistente
                g_array_index(BB_array, BB_rec, BB_array->len - 1).mov_number++;
            }
            g_array_index(mov_array, mov_rec, i).BB_index = BB_array->len - 1;
        }
        else {  // BB troppo piccolo --> istruzione in cache (non patchabile)
#if PATCH_DEBUG == 1
            printf("********************** istruzione in cache (non patchabile): %d\n", i);
#endif
            
            // insert mov in cache
            _insert_mov_in_cache(movAddress);
            g_array_append_val(the_cc_addr_array, movAddress);
            
            // remove from mov_array
            g_array_remove_index(mov_array, i);
            i--;
        }
    }
    
    // Binding branch-BB
    j = 0;
    for(i=0; i<BB_array->len; i++) {
        BB_rec* theBB = &g_array_index(BB_array, BB_rec, i);
        theBB->branch_index = 0;
        theBB->branch_number = 0;
        while(j<branch_array->len && g_array_index(branch_array, branch_rec, j).address < theBB->originalBB_start) {  // istruzioni di branch precedenti al BB
            g_array_remove_index(branch_array, j);
        }
        while(j<branch_array->len && g_array_index(branch_array, branch_rec, j).address < theBB->originalBB_end) {  // istruzioni di branch nel BB
            g_array_index(branch_array, branch_rec, j).BB_index = i;
            if(theBB->branch_number == 0) {
                theBB->branch_index = j;
            }
            theBB->branch_number++;
            j++;
        }
    }
    while(j<branch_array->len) {  // istruzioni di branch successive all'ultimo BB
        g_array_remove_index(branch_array, j);
    }
    
    // Creazione hash table (PATCH)
    mov_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
    for(i=0; i<mov_array->len; i++) {
        g_hash_table_insert(mov_hash, g_array_index(mov_array, mov_rec, i).address, &g_array_index(mov_array, mov_rec, i));
    }
    
    // Creazione hash table (CACHE)
    g_cchash = g_hash_table_new(g_direct_hash, g_direct_equal);
    for(i=0; i<g_cc_array->len; i++) {
        g_hash_table_insert(g_cchash, g_array_index(the_cc_addr_array, void*, i), &g_array_index(g_cc_array, rm_CODECACHEDATA, i));
    }
    
#if PATCH_DEBUG == 1
    printf("**********************mov array*********************************\n");
    for(i=0; i<mov_array->len; i++) {
        printf("mov_array address index (%d) = %x\n", i, (unsigned int)g_array_index(mov_array, mov_rec, i).address);
        printf("mov_array address_end index (%d) = %x\n", i, (unsigned int)g_array_index(mov_array, mov_rec, i).address_end);
        printf("mov_array BB_index index (%d) = %d\n", i, g_array_index(mov_array, mov_rec, i).BB_index);
        
        printf("BB_array originalBB_start = %x\n", (unsigned int)g_array_index(BB_array, BB_rec, g_array_index(mov_array, mov_rec, i).BB_index).originalBB_start);
        printf("BB_array originalBB_end = %x\n", (unsigned int)g_array_index(BB_array, BB_rec, g_array_index(mov_array, mov_rec, i).BB_index).originalBB_end);
        printf("BB_array BB_start = %x\n", (unsigned int)g_array_index(BB_array, BB_rec, g_array_index(mov_array, mov_rec, i).BB_index).BB_start);
        printf("BB_array BB_end = %x\n", (unsigned int)g_array_index(BB_array, BB_rec, g_array_index(mov_array, mov_rec, i).BB_index).BB_end);
        printf("BB_array mov_index = %d\n", g_array_index(BB_array, BB_rec, g_array_index(mov_array, mov_rec, i).BB_index).mov_index);
        printf("BB_array mov_number = %d\n", g_array_index(BB_array, BB_rec, g_array_index(mov_array, mov_rec, i).BB_index).mov_number);
    }
    
    printf("**********************mov hash*********************************\n");
    mov_rec* prova_hash = g_hash_table_lookup(mov_hash, g_array_index(mov_array, mov_rec, 0).address);
    printf("mov_array address = %x\n", (unsigned int)prova_hash->address);
    printf("mov_array address_end = %x\n", (unsigned int)prova_hash->address_end);
    printf("mov_array BB_index = %d\n", prova_hash->BB_index);
#endif

#if rm_STAT == 1
    g_stats.patch_instr_number = mov_array->len;
    g_stats.cache_instr_number = g_cc_array->len;
    g_stats.BB_number = BB_array->len;
    
    BB_rec theBB;
    unsigned int theBB_size;
    g_stats.BB_min_size = 0xffffffff;
    g_stats.BB_min_instr_number = 0xffffffff;
    for(i=0; i<BB_array->len; i++) {
        theBB = g_array_index(BB_array, BB_rec, i);
        theBB_size = (unsigned int)(theBB.originalBB_end - theBB.originalBB_start);
        if(theBB_size < g_stats.BB_min_size) {
            g_stats.BB_min_size = theBB_size;
        }
        if(theBB_size > g_stats.BB_max_size) {
            g_stats.BB_max_size = theBB_size;
        }
        g_stats.BB_med_size += theBB_size;
        
        if(theBB.mov_number < g_stats.BB_min_instr_number) {
            g_stats.BB_min_instr_number = theBB.mov_number;
        }
        if(theBB.mov_number > g_stats.BB_max_instr_number) {
            g_stats.BB_max_instr_number = theBB.mov_number;
        }
        g_stats.BB_med_instr_number += theBB.mov_number;
    }
    g_stats.BB_med_size = g_stats.BB_med_size / g_stats.BB_number;
    g_stats.BB_med_instr_number = g_stats.BB_med_instr_number / g_stats.BB_number;
#endif
    
    
#if rm_DISASSEMBLER == rm_LIBDISASM
    /*uninitializes libdisasm...*/
    x86_cleanup();
#endif
    
    g_array_free(func_array, TRUE);
    g_array_free(jmp_array, TRUE);
    g_array_free(the_cc_addr_array, TRUE);
    
    return 0;
}


/* ----------------------------------------------------------------------------
 *  _get_func_addr
 * ----------------------------------------------------------------------------
*/
static int _get_func_addr(GArray* out_jmp_array) {
    int fd;                 // File Descriptor
    char *base_ptr;         // ptr to our object in memory
    int symbol_count;
    FILE* f;
    char file[256];
    int i=0;
    
    /*retrieves file name...*/
    f = fopen("/proc/self/cmdline", "r");
    fscanf(f, "%c", &file[i]);
    while (!feof(f) && file[i]!='\0') {
        i++;
        fscanf(f, "%c", &file[i]);
    }
    fclose(f);
    
    
    struct stat elf_stats;  // fstat struct
    
        if((fd = open(file, O_RDONLY)) == -1) {
        _rm_error(rm_CANNOT_OPEN_ELF, "_get_func_addr: could not open %s\n", file);
        return -1;
        }

        if((fstat(fd, &elf_stats))) {
        _rm_error(rm_CANNOT_OPEN_ELF, "_get_func_addr: could not fstat %s\n", file);
                close(fd);
        return -1;
        }

        if((base_ptr = (char*)malloc(elf_stats.st_size)) == NULL) {
        _rm_error(rm_CANNOT_OPEN_ELF, "_get_func_addr: could not malloc\n");
                close(fd);
        return -1;
        }

        if((read(fd, base_ptr, elf_stats.st_size)) < elf_stats.st_size) {
        _rm_error(rm_CANNOT_OPEN_ELF, "_get_func_addr: could not read %s\n", file);
                free(base_ptr);
                close(fd);
        return -1;
        }

    /* Check libelf version first */
    if(elf_version(EV_CURRENT) == EV_NONE) {
#if rm_DEBUG == 1
        printf("WARNING Elf Library is out of date!\n");
#endif
    }

    elf_header = (Elf32_Ehdr *) base_ptr;   // point elf_header at our object in memory
    elf = elf_begin(fd, ELF_C_READ, NULL);  // Initialize 'elf' pointer to our file descriptor

    while((scn = elf_nextscn(elf, scn)) != NULL) {
        
        gelf_getshdr(scn, &shdr);

        // When we find a section header marked SHT_SYMTAB stop and get symbols
        if(shdr.sh_type == SHT_SYMTAB) {
            
            // edata points to our symbol table
            edata = elf_getdata(scn, edata);

            // how many symbols are there? this number comes from the size of
            // the section divided by the entry size
            symbol_count = shdr.sh_size / shdr.sh_entsize;

            // loop through to grab all symbols
            for(i = 0; i < symbol_count; i++) {
                        
                // libelf grabs the symbol data using gelf_getsym()
                        gelf_getsym(edata, i, &sym);

                    if(ELF32_ST_TYPE(sym.st_info) == STT_FUNC) {
                        
                    if(sym.st_value != 0) {
                        g_array_append_val(out_jmp_array, sym.st_value);
                    }

                    // print out the value and size
                //  printf("%08x", sym.st_value);

                    // the name of the symbol is somewhere in a string table
                    // we know which one using the shdr.sh_link member
                    // libelf grabs the string using elf_strptr()
                //  printf("\t%s\n", elf_strptr(elf, shdr.sh_link, sym.st_name));
                        }
                }
        }
    }
/*
    printf("**********************array***********************\n");
    for(i=0; i<func_array->len; i++) {
        printf("*****func_array address = %x\n", g_array_index(func_array, unsigned int, i));
    }

    // Array func: Ordinamento
    g_array_sort(func_array, compareAddress);
    
    printf("**********************ordinato***************\n");
    for(i=0; i<func_array->len; i++) {
        printf("*****func_array address = %x\n", g_array_index(func_array, unsigned int, i));
    }
    
    // Array func: Rimozione doppioni
    for(i=1; i<func_array->len; i++) {
        if(g_array_index(func_array, unsigned int, i) == g_array_index(func_array, unsigned int, i-1)) {
            g_array_remove_index(func_array, i);
            i--;
        }
    }
    
    printf("**********************senza doppioni***************\n");
    for(i=0; i<func_array->len; i++) {
        printf("*****func_array address = %x\n", g_array_index(func_array, unsigned int, i));
    }*/
    
    return 0;
}


/* ----------------------------------------------------------------------------
 *  _insert_mov_in_cache
 * ----------------------------------------------------------------------------
*/
#if rm_DISASSEMBLER == rm_LIBDISASM
static void _insert_mov_in_cache(void* movAddress) {
    rm_CODECACHEDATA ccNext;
    x86_insn_t insn;
    x86_op_t *dest, *src;
    
    /* disassemble address */
    ccNext.instr_size = x86_disasm(movAddress, rm_SINGLE_INSN_MAX_SIZE, 0, 0, &insn);
    
    /*retrieves operands...*/
    dest = x86_get_dest_operand(&insn);
    src = x86_get_src_operand(&insn);
    
    /*if instr is a move...*/
    if (insn.type==insn_mov) {  // istruzione mov (anche lea, lahf, ...)
        
        /*if src is address and dest is not...*/ //if mov reg, mem...
        if ((src->type >= 5 && src->type <= 7) && x86_operand_size(src)==4 && dest->type==1) {
            //if mov eax, mem...
            if (strcmp(dest->data.reg.name, "eax") == 0) {
                ccNext.sim_func = mov_eax_mem;
            }
            
            //if mov ecx, mem...
            else if (strcmp(dest->data.reg.name, "ecx") == 0) {
                ccNext.sim_func = mov_ecx_mem;
            }
            
            //if mov edx, mem...
            else if (strcmp(dest->data.reg.name, "edx") == 0) {
                ccNext.sim_func = mov_edx_mem;
            }
            
            //if mov ebx, mem...
            else if (strcmp(dest->data.reg.name, "ebx") == 0) {
                ccNext.sim_func = mov_ebx_mem;
            }
            
            //if mov esi, mem...
            else if (strcmp(dest->data.reg.name, "esi") == 0) {
                ccNext.sim_func = mov_esi_mem;
            }
            
            //if mov edi, mem...
            else if (strcmp(dest->data.reg.name, "edi") == 0) {
                ccNext.sim_func = mov_edi_mem;
            }
        }
        
        /*if dest is address and src is not...*/ //if mov mem, reg...
        else if ((dest->type >= 5 && dest->type <= 7) && x86_operand_size(dest)==4 && src->type==1) {
            //if mov mem, eax...
            if (strcmp(src->data.reg.name, "eax") == 0) {
                ccNext.sim_func = mov_mem_eax;
            }
            
            //if mov mem, ecx...
            else if (strcmp(src->data.reg.name, "ecx") == 0) {
                ccNext.sim_func = mov_mem_ecx;
            }
            
            //if mov mem, edx...
            else if (strcmp(src->data.reg.name, "edx") == 0) {
                ccNext.sim_func = mov_mem_edx;
            }
            
            //if mov mem, ebx...
            else if (strcmp(src->data.reg.name, "ebx") == 0) {
                ccNext.sim_func = mov_mem_ebx;
            }
            
            //if mov mem, esi...
            else if (strcmp(src->data.reg.name, "esi") == 0) {
                ccNext.sim_func = mov_mem_esi;
            }
            
            //if mov mem, edi...
            else if (strcmp(src->data.reg.name, "edi") == 0) {
                ccNext.sim_func = mov_mem_edi;
            }
        }
        
        /*if dest is address and src is not...*/ //if mov mem, #const...
        else if ((dest->type >= 5 && dest->type <= 7) && x86_operand_size(dest)==4 && src->type==2) {
            ccNext.sim_func = mov_mem_imm;
            ccNext.imm_data = src->data.dword;
        }
    }
    
    /*if instr is a compare...*/
    else if (insn.type==insn_cmp) {
        
        /*if src is address and dest is not...*/ //if cmp reg, mem...
        if ((src->type >= 5 && src->type <= 7) && x86_operand_size(src)==4 && dest->type==1) {
            //if cmp eax, mem...
            if (strcmp(dest->data.reg.name, "eax") == 0) {
                ccNext.sim_func = cmp_eax_mem;
            }
            
            //if cmp ecx, mem...
            else if (strcmp(dest->data.reg.name, "ecx") == 0) {
                ccNext.sim_func = cmp_ecx_mem;
            }
            
            //if cmp edx, mem...
            else if (strcmp(dest->data.reg.name, "edx") == 0) {
                ccNext.sim_func = cmp_edx_mem;
            }
            
            //if cmp ebx, mem...
            else if (strcmp(dest->data.reg.name, "ebx") == 0) {
                ccNext.sim_func = cmp_ebx_mem;
            }
            
            //if cmp esi, mem...
            else if (strcmp(dest->data.reg.name, "esi") == 0) {
                ccNext.sim_func = cmp_esi_mem;
            }
            
            //if cmp edi, mem...
            else if (strcmp(dest->data.reg.name, "edi") == 0) {
                ccNext.sim_func = cmp_edi_mem;
            }
        }
        
        /*if dest is address and src is not...*/ //if cmp mem, reg...
        else if ((dest->type >= 5 && dest->type <= 7) && x86_operand_size(dest)==4 && src->type==1) {
            //if cmp mem, eax...
            if (strcmp(src->data.reg.name, "eax") == 0) {
                ccNext.sim_func = cmp_mem_eax;
            }
            
            //if cmp mem, ecx...
            else if (strcmp(src->data.reg.name, "ecx") == 0) {
                ccNext.sim_func = cmp_mem_ecx;
            }
            
            //if cmp mem, edx...
            else if (strcmp(src->data.reg.name, "edx") == 0) {
                ccNext.sim_func = cmp_mem_edx;
            }
            
            //if cmp mem, ebx...
            else if (strcmp(src->data.reg.name, "ebx") == 0) {
                ccNext.sim_func = cmp_mem_ebx;
            }
            
            //if cmp mem, esi...
            else if (strcmp(src->data.reg.name, "esi") == 0) {
                ccNext.sim_func = cmp_mem_esi;
            }
            
            //if cmp mem, edi...
            else if (strcmp(src->data.reg.name, "edi") == 0) {
                ccNext.sim_func = cmp_mem_edi;
            }
        }
        
        /*if dest is address and src is not...*/ //if cmp mem, #const...
        else if ((dest->type >= 5 && dest->type <= 7) && x86_operand_size(dest)==4 && src->type==2) {
            ccNext.sim_func = cmp_mem_imm;
        //  ccNext.imm_data = src->data.dword;  //****** TO DO: gestire casi con imm_data da 1 o 2 byte ********
            
            if(src->datatype == op_byte)
                ccNext.imm_data = src->data.sbyte;
            else if(src->datatype == op_word)
                ccNext.imm_data = src->data.sword;
            else// if(src->datatype == op_dword)
                ccNext.imm_data = src->data.sdword;
        }
    }
    
    /*if instr is an add...*/
    else if (insn.type==insn_add) {
        
        /*if src is address and dest is not...*/ //if add reg, mem...
        if ((src->type >= 5 && src->type <= 7) && x86_operand_size(src)==4 && dest->type==1) {
            //if add eax, mem...
            if (strcmp(dest->data.reg.name, "eax") == 0) {
                ccNext.sim_func = add_eax_mem;
            }
            
            //if add ecx, mem...
            else if (strcmp(dest->data.reg.name, "ecx") == 0) {
                ccNext.sim_func = add_ecx_mem;
            }
            
            //if add edx, mem...
            else if (strcmp(dest->data.reg.name, "edx") == 0) {
                ccNext.sim_func = add_edx_mem;
            }
            
            //if add ebx, mem...
            else if (strcmp(dest->data.reg.name, "ebx") == 0) {
                ccNext.sim_func = add_ebx_mem;
            }
            
            //if add esi, mem...
            else if (strcmp(dest->data.reg.name, "esi") == 0) {
                ccNext.sim_func = add_esi_mem;
            }
            
            //if add edi, mem...
            else if (strcmp(dest->data.reg.name, "edi") == 0) {
                ccNext.sim_func = add_edi_mem;
            }
        }
        
        /*if dest is address and src is not...*/ //if add mem, reg...
        else if ((dest->type >= 5 && dest->type <= 7) && x86_operand_size(dest)==4 && src->type==1) {
            //if add mem, eax...
            if (strcmp(src->data.reg.name, "eax") == 0) {
                ccNext.sim_func = add_mem_eax;
            }
            
            //if add mem, ecx...
            else if (strcmp(src->data.reg.name, "ecx") == 0) {
                ccNext.sim_func = add_mem_ecx;
            }
            
            //if add mem, edx...
            else if (strcmp(src->data.reg.name, "edx") == 0) {
                ccNext.sim_func = add_mem_edx;
            }
            
            //if add mem, ebx...
            else if (strcmp(src->data.reg.name, "ebx") == 0) {
                ccNext.sim_func = add_mem_ebx;
            }
            
            //if add mem, esi...
            else if (strcmp(src->data.reg.name, "esi") == 0) {
                ccNext.sim_func = add_mem_esi;
            }
            
            //if add mem, edi...
            else if (strcmp(src->data.reg.name, "edi") == 0) {
                ccNext.sim_func = add_mem_edi;
            }
        }
        
        /*if dest is address and src is not...*/ //if add mem, #const...
        else if ((dest->type >= 5 && dest->type <= 7) && x86_operand_size(dest)==4 && src->type==2) {
            ccNext.sim_func = add_mem_imm;
        //  ccNext.imm_data = src->data.dword;  //****** TO DO: gestire casi con imm_data da 1 o 2 byte ********
            
            if(src->datatype == op_byte)
                ccNext.imm_data = src->data.sbyte;
            else if(src->datatype == op_word)
                ccNext.imm_data = src->data.sword;
            else// if(src->datatype == op_dword)
                ccNext.imm_data = src->data.sdword;
        }
    }
    
    /*if instr is a sub...*/
    else if (insn.type==insn_sub) {
        
        /*if src is address and dest is not...*/ //if sub reg, mem...
        if ((src->type >= 5 && src->type <= 7) && x86_operand_size(src)==4 && dest->type==1) {
            //if sub eax, mem...
            if (strcmp(dest->data.reg.name, "eax") == 0) {
                ccNext.sim_func = sub_eax_mem;
            }
            
            //if sub ecx, mem...
            else if (strcmp(dest->data.reg.name, "ecx") == 0) {
                ccNext.sim_func = sub_ecx_mem;
            }
            
            //if sub edx, mem...
            else if (strcmp(dest->data.reg.name, "edx") == 0) {
                ccNext.sim_func = sub_edx_mem;
            }
            
            //if sub ebx, mem...
            else if (strcmp(dest->data.reg.name, "ebx") == 0) {
                ccNext.sim_func = sub_ebx_mem;
            }
            
            //if sub esi, mem...
            else if (strcmp(dest->data.reg.name, "esi") == 0) {
                ccNext.sim_func = sub_esi_mem;
            }
            
            //if sub edi, mem...
            else if (strcmp(dest->data.reg.name, "edi") == 0) {
                ccNext.sim_func = sub_edi_mem;
            }
        }
        
        /*if dest is address and src is not...*/ //if sub mem, reg...
        else if ((dest->type >= 5 && dest->type <= 7) && x86_operand_size(dest)==4 && src->type==1) {
            //if sub mem, eax...
            if (strcmp(src->data.reg.name, "eax") == 0) {
                ccNext.sim_func = sub_mem_eax;
            }
            
            //if sub mem, ecx...
            else if (strcmp(src->data.reg.name, "ecx") == 0) {
                ccNext.sim_func = sub_mem_ecx;
            }
            
            //if sub mem, edx...
            else if (strcmp(src->data.reg.name, "edx") == 0) {
                ccNext.sim_func = sub_mem_edx;
            }
            
            //if sub mem, ebx...
            else if (strcmp(src->data.reg.name, "ebx") == 0) {
                ccNext.sim_func = sub_mem_ebx;
            }
            
            //if sub mem, esi...
            else if (strcmp(src->data.reg.name, "esi") == 0) {
                ccNext.sim_func = sub_mem_esi;
            }
            
            //if sub mem, edi...
            else if (strcmp(src->data.reg.name, "edi") == 0) {
                ccNext.sim_func = sub_mem_edi;
            }
        }
        
        /*if dest is address and src is not...*/ //if sub mem, #const...
        else if ((dest->type >= 5 && dest->type <= 7) && x86_operand_size(dest)==4 && src->type==2) {
            ccNext.sim_func = sub_mem_imm;
        //  ccNext.imm_data = src->data.dword;  //****** TO DO: gestire casi con imm_data da 1 o 2 byte ********
            
            if(src->datatype == op_byte)
                ccNext.imm_data = src->data.sbyte;
            else if(src->datatype == op_word)
                ccNext.imm_data = src->data.sword;
            else// if(src->datatype == op_dword)
                ccNext.imm_data = src->data.sdword;
        }
    }
    
    // insert in g_cc_array
    g_array_append_val(g_cc_array, ccNext);
}

#else

static void _insert_mov_in_cache(void* movAddress) {
    rm_CODECACHEDATA ccNext;
    _DInst insn;
    unsigned int instructions_count = 0;
    _Operand *dest, *src;
    
    _CodeInfo ci;
    ci.code = movAddress;
    ci.codeLen = rm_SINGLE_INSN_MAX_SIZE;
    ci.codeOffset = (unsigned int)movAddress;
    ci.dt = Decode32Bits;
    ci.features = DF_NONE;
    
    /* disassemble address */
    distorm_decompose(&ci, &insn, 1, &instructions_count);
    ccNext.instr_size = insn.size;
    
    /*retrieves operands...*/
    dest = &insn.ops[0];
    src = &insn.ops[1];
    
    /*if instr is a move...*/
    if (insn.opcode==I_MOV) {   // istruzione mov
        
        /*if src is address and dest is not...*/ //if mov reg, mem...
        if ((src->type == O_DISP || src->type == O_SMEM || src->type == O_MEM) && src->size==32 && dest->type==O_REG) {
            //if mov eax, mem...
            if (dest->index == R_EAX) {
                ccNext.sim_func = mov_eax_mem;
            }
            
            //if mov ecx, mem...
            else if (dest->index == R_ECX) {
                ccNext.sim_func = mov_ecx_mem;
            }
            
            //if mov edx, mem...
            else if (dest->index == R_EDX) {
                ccNext.sim_func = mov_edx_mem;
            }
            
            //if mov ebx, mem...
            else if (dest->index == R_EBX) {
                ccNext.sim_func = mov_ebx_mem;
            }
            
            //if mov esi, mem...
            else if (dest->index == R_ESI) {
                ccNext.sim_func = mov_esi_mem;
            }
            
            //if mov edi, mem...
            else if (dest->index == R_EDI) {
                ccNext.sim_func = mov_edi_mem;
            }
        }
        
        /*if dest is address and src is not...*/ //if mov mem, reg...
        else if ((dest->type == O_DISP || dest->type == O_SMEM || dest->type == O_MEM) && dest->size==32 && src->type==O_REG) {
            //if mov mem, eax...
            if (dest->index == R_EAX) {
                ccNext.sim_func = mov_mem_eax;
            }
            
            //if mov mem, ecx...
            else if (dest->index == R_ECX) {
                ccNext.sim_func = mov_mem_ecx;
            }
            
            //if mov mem, edx...
            else if (dest->index == R_EDX) {
                ccNext.sim_func = mov_mem_edx;
            }
            
            //if mov mem, ebx...
            else if (dest->index == R_EBX) {
                ccNext.sim_func = mov_mem_ebx;
            }
            
            //if mov mem, esi...
            else if (dest->index == R_ESI) {
                ccNext.sim_func = mov_mem_esi;
            }
            
            //if mov mem, edi...
            else if (dest->index == R_EDI) {
                ccNext.sim_func = mov_mem_edi;
            }
        }
        
        /*if dest is address and src is not...*/ //if mov mem, #const...
        else if ((dest->type == O_DISP || dest->type == O_SMEM || dest->type == O_MEM) && dest->size==32 && src->type==O_IMM) {
            ccNext.sim_func = mov_mem_imm;
            ccNext.imm_data = insn.imm.dword;
        }
    }
    
    /*if instr is a compare...*/
    else if (insn.opcode==I_CMP) {
        
        /*if src is address and dest is not...*/ //if cmp reg, mem...
        if ((src->type == O_DISP || src->type == O_SMEM || src->type == O_MEM) && src->size==32 && dest->type==O_REG) {
            //if cmp eax, mem...
            if (dest->index == R_EAX) {
                ccNext.sim_func = cmp_eax_mem;
            }
            
            //if cmp ecx, mem...
            else if (dest->index == R_ECX) {
                ccNext.sim_func = cmp_ecx_mem;
            }
            
            //if cmp edx, mem...
            else if (dest->index == R_EDX) {
                ccNext.sim_func = cmp_edx_mem;
            }
            
            //if cmp ebx, mem...
            else if (dest->index == R_EBX) {
                ccNext.sim_func = cmp_ebx_mem;
            }
            
            //if cmp esi, mem...
            else if (dest->index == R_ESI) {
                ccNext.sim_func = cmp_esi_mem;
            }
            
            //if cmp edi, mem...
            else if (dest->index == R_EDI) {
                ccNext.sim_func = cmp_edi_mem;
            }
        }
        
        /*if dest is address and src is not...*/ //if cmp mem, reg...
        else if ((dest->type == O_DISP || dest->type == O_SMEM || dest->type == O_MEM) && dest->size==32 && src->type==O_REG) {
            //if cmp mem, eax...
            if (dest->index == R_EAX) {
                ccNext.sim_func = cmp_mem_eax;
            }
            
            //if cmp mem, ecx...
            else if (dest->index == R_ECX) {
                ccNext.sim_func = cmp_mem_ecx;
            }
            
            //if cmp mem, edx...
            else if (dest->index == R_EDX) {
                ccNext.sim_func = cmp_mem_edx;
            }
            
            //if cmp mem, ebx...
            else if (dest->index == R_EBX) {
                ccNext.sim_func = cmp_mem_ebx;
            }
            
            //if cmp mem, esi...
            else if (dest->index == R_ESI) {
                ccNext.sim_func = cmp_mem_esi;
            }
            
            //if cmp mem, edi...
            else if (dest->index == R_EDI) {
                ccNext.sim_func = cmp_mem_edi;
            }
        }
        
        /*if dest is address and src is not...*/ //if cmp mem, #const...
        else if ((dest->type == O_DISP || dest->type == O_SMEM || dest->type == O_MEM) && dest->size==32 && src->type==O_IMM) {
            ccNext.sim_func = cmp_mem_imm;
        //  ccNext.imm_data = insn.dword;   //****** TO DO: gestire casi con imm_data da 1 o 2 byte ********
            
            if(src->size == 8)
                ccNext.imm_data = insn.imm.sbyte;
            else if(src->size == 16)
                ccNext.imm_data = insn.imm.sword;
            else// if(src->size == 32)
                ccNext.imm_data = insn.imm.sdword;
        }
    }
    
    /*if instr is an add...*/
    else if (insn.opcode==I_ADD) {
        
        /*if src is address and dest is not...*/ //if add reg, mem...
        if ((src->type == O_DISP || src->type == O_SMEM || src->type == O_MEM) && src->size==32 && dest->type==O_REG) {
            //if add eax, mem...
            if (dest->index == R_EAX) {
                ccNext.sim_func = add_eax_mem;
            }
            
            //if add ecx, mem...
            else if (dest->index == R_ECX) {
                ccNext.sim_func = add_ecx_mem;
            }
            
            //if add edx, mem...
            else if (dest->index == R_EDX) {
                ccNext.sim_func = add_edx_mem;
            }
            
            //if add ebx, mem...
            else if (dest->index == R_EBX) {
                ccNext.sim_func = add_ebx_mem;
            }
            
            //if add esi, mem...
            else if (dest->index == R_ESI) {
                ccNext.sim_func = add_esi_mem;
            }
            
            //if add edi, mem...
            else if (dest->index == R_EDI) {
                ccNext.sim_func = add_edi_mem;
            }
        }
        
        /*if dest is address and src is not...*/ //if add mem, reg...
        else if ((dest->type == O_DISP || dest->type == O_SMEM || dest->type == O_MEM) && dest->size==32 && src->type==O_REG) {
            //if add mem, eax...
            if (dest->index == R_EAX) {
                ccNext.sim_func = add_mem_eax;
            }
            
            //if add mem, ecx...
            else if (dest->index == R_ECX) {
                ccNext.sim_func = add_mem_ecx;
            }
            
            //if add mem, edx...
            else if (dest->index == R_EDX) {
                ccNext.sim_func = add_mem_edx;
            }
            
            //if add mem, ebx...
            else if (dest->index == R_EBX) {
                ccNext.sim_func = add_mem_ebx;
            }
            
            //if add mem, esi...
            else if (dest->index == R_ESI) {
                ccNext.sim_func = add_mem_esi;
            }
            
            //if add mem, edi...
            else if (dest->index == R_EDI) {
                ccNext.sim_func = add_mem_edi;
            }
        }
        
        /*if dest is address and src is not...*/ //if add mem, #const...
        else if ((dest->type == O_DISP || dest->type == O_SMEM || dest->type == O_MEM) && dest->size==32 && src->type==O_IMM) {
            ccNext.sim_func = add_mem_imm;
        //  ccNext.imm_data = insn.dword;   //****** TO DO: gestire casi con imm_data da 1 o 2 byte ********
            
            if(src->size == 8)
                ccNext.imm_data = insn.imm.sbyte;
            else if(src->size == 16)
                ccNext.imm_data = insn.imm.sword;
            else// if(src->size == 32)
                ccNext.imm_data = insn.imm.sdword;
        }
    }
    
    /*if instr is a sub...*/
    else if (insn.opcode==I_SUB) {
        
        /*if src is address and dest is not...*/ //if sub reg, mem...
        if ((src->type == O_DISP || src->type == O_SMEM || src->type == O_MEM) && src->size==32 && dest->type==O_REG) {
            //if sub eax, mem...
            if (dest->index == R_EAX) {
                ccNext.sim_func = sub_eax_mem;
            }
            
            //if sub ecx, mem...
            else if (dest->index == R_ECX) {
                ccNext.sim_func = sub_ecx_mem;
            }
            
            //if sub edx, mem...
            else if (dest->index == R_EDX) {
                ccNext.sim_func = sub_edx_mem;
            }
            
            //if sub ebx, mem...
            else if (dest->index == R_EBX) {
                ccNext.sim_func = sub_ebx_mem;
            }
            
            //if sub esi, mem...
            else if (dest->index == R_ESI) {
                ccNext.sim_func = sub_esi_mem;
            }
            
            //if sub edi, mem...
            else if (dest->index == R_EDI) {
                ccNext.sim_func = sub_edi_mem;
            }
        }
        
        /*if dest is address and src is not...*/ //if sub mem, reg...
        else if ((dest->type == O_DISP || dest->type == O_SMEM || dest->type == O_MEM) && dest->size==32 && src->type==O_REG) {
            //if sub mem, eax...
            if (dest->index == R_EAX) {
                ccNext.sim_func = sub_mem_eax;
            }
            
            //if sub mem, ecx...
            else if (dest->index == R_ECX) {
                ccNext.sim_func = sub_mem_ecx;
            }
            
            //if sub mem, edx...
            else if (dest->index == R_EDX) {
                ccNext.sim_func = sub_mem_edx;
            }
            
            //if sub mem, ebx...
            else if (dest->index == R_EBX) {
                ccNext.sim_func = sub_mem_ebx;
            }
            
            //if sub mem, esi...
            else if (dest->index == R_ESI) {
                ccNext.sim_func = sub_mem_esi;
            }
            
            //if sub mem, edi...
            else if (dest->index == R_EDI) {
                ccNext.sim_func = sub_mem_edi;
            }
        }
        
        /*if dest is address and src is not...*/ //if sub mem, #const...
        else if ((dest->type == O_DISP || dest->type == O_SMEM || dest->type == O_MEM) && dest->size==32 && src->type==O_IMM) {
            ccNext.sim_func = sub_mem_imm;
        //  ccNext.imm_data = insn.dword;   //****** TO DO: gestire casi con imm_data da 1 o 2 byte ********
            
            if(src->size == 8)
                ccNext.imm_data = insn.imm.sbyte;
            else if(src->size == 16)
                ccNext.imm_data = insn.imm.sword;
            else// if(src->size == 32)
                ccNext.imm_data = insn.imm.sdword;
        }
    }
    
    // insert in g_cc_array
    g_array_append_val(g_cc_array, ccNext);
}
#endif


#if rm_DEBUG == 1
/* ----------------------------------------------------------------------------
 *  rm_make_dump_file
 * ----------------------------------------------------------------------------
*/
int rm_make_dump_file(char* filename) {
    
//  void* code_start = (void*)rm_CODE_START;
    void* code_end = &_etext;
    FILE* the_dump_file;
    int i = 0;
    BB_rec theBB;
    
    GArray* func_array;     // Array degli indirizzi delle funzioni
    func_array = g_array_new(FALSE, FALSE, sizeof(void*));
    
    GHashTable* func_name_hash;
//  func_name_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
    func_name_hash = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, free);
    
    
    //************************ libelf - start ******************************************************************************************
    int fd;                 // File Descriptor
    char *base_ptr;         // ptr to our object in memory
    int symbol_count;
    FILE* f;
    char file[256];
//  int i=0;
    
    /*retrieves file name...*/
    f = fopen("/proc/self/cmdline", "r");
    fscanf(f, "%c", &file[i]);
    while (!feof(f) && file[i]!='\0') {
        i++;
        fscanf(f, "%c", &file[i]);
    }
    fclose(f);
    
    
    struct stat elf_stats;  // fstat struct
    
        if((fd = open(file, O_RDONLY)) == -1) {
        _rm_error(rm_CANNOT_OPEN_ELF, "rm_make_dump_file: could not open %s\n", file);
        return -1;
        }

        if((fstat(fd, &elf_stats))) {
        _rm_error(rm_CANNOT_OPEN_ELF, "rm_make_dump_file: could not fstat %s\n", file);
                close(fd);
        return -1;
        }

        if((base_ptr = (char*)malloc(elf_stats.st_size)) == NULL) {
        _rm_error(rm_CANNOT_OPEN_ELF, "rm_make_dump_file: could not malloc\n");
                close(fd);
        return -1;
        }

        if((read(fd, base_ptr, elf_stats.st_size)) < elf_stats.st_size) {
        _rm_error(rm_CANNOT_OPEN_ELF, "rm_make_dump_file: could not read %s\n", file);
                free(base_ptr);
                close(fd);
        return -1;
        }

    /* Check libelf version first */
    if(elf_version(EV_CURRENT) == EV_NONE) {
//#if rm_DEBUG == 1
        printf("WARNING Elf Library is out of date!\n");
//#endif
    }

    elf_header = (Elf32_Ehdr *) base_ptr;   // point elf_header at our object in memory
    elf = elf_begin(fd, ELF_C_READ, NULL);  // Initialize 'elf' pointer to our file descriptor

    scn = NULL;
    edata = NULL;
    while((scn = elf_nextscn(elf, scn)) != NULL) {
        
        gelf_getshdr(scn, &shdr);

        // When we find a section header marked SHT_SYMTAB stop and get symbols
        if(shdr.sh_type == SHT_SYMTAB) {
            
            // edata points to our symbol table
            edata = elf_getdata(scn, edata);

            // how many symbols are there? this number comes from the size of
            // the section divided by the entry size
            symbol_count = shdr.sh_size / shdr.sh_entsize;

            // loop through to grab all symbols
            for(i = 0; i < symbol_count; i++) {
                        
                // libelf grabs the symbol data using gelf_getsym()
                        gelf_getsym(edata, i, &sym);

                    if(ELF32_ST_TYPE(sym.st_info) == STT_FUNC) {
                        
                    if(sym.st_value != 0) {
                        char* the_name;
                        the_name = (char*)malloc(strlen(elf_strptr(elf, shdr.sh_link, sym.st_name)) + 1);
                        strcpy(the_name, elf_strptr(elf, shdr.sh_link, sym.st_name));
                        g_hash_table_insert(func_name_hash, (void*)(unsigned int)sym.st_value, the_name);
                        
                        g_array_append_val(func_array, sym.st_value);
                    }

                    // print out the value and size
                //  printf("%08x", sym.st_value);

                    // the name of the symbol is somewhere in a string table
                    // we know which one using the shdr.sh_link member
                    // libelf grabs the string using elf_strptr()
                //  printf("\t%s\n", elf_strptr(elf, shdr.sh_link, sym.st_name));
                        }
                }
        }
    }
    //************************ libelf - end ***********************************************************************************************
    
    
    
    // Array funzioni: aggiungo anche code_end
    g_array_append_val(func_array, code_end);
    
    // Array funzioni: Ordinamento
    g_array_sort(func_array, compareAddress);
    
    // Array funzioni: Rimozione doppioni
    for(i=1; i<func_array->len; i++) {
        if(g_array_index(func_array, void*, i) == g_array_index(func_array, void*, i-1)) {
            g_array_remove_index(func_array, i);
            i--;
        }
    }
    
    
    /* open dump file */
    if(filename == NULL) {
        filename = rm_DUMP_FILE_NAME;
    }
    the_dump_file = fopen(filename,"w");
    if(the_dump_file == NULL) {
        _rm_error(rm_CANNOT_OPEN_DUMP, "rm_make_dump_file: could not open %s\n", filename);
        return -1;
    }
    
    
#if rm_DISASSEMBLER == rm_LIBDISASM
    int LINE_SIZE = 1000;
    char line[LINE_SIZE];       /* buffer of line to print */
    unsigned int pos = 0;       /* current position in buffer */
    int size;                   /* size of instruction */
    x86_insn_t insn;            /* instruction */
    
    /* initialize libdisasm */
    x86_init(opt_none, NULL, NULL);
    
    i = 0;
    if (i < BB_array->len) {
        theBB = g_array_index(BB_array, BB_rec, i);
    }
    int j;
    for(j=0; j<func_array->len-1; j++) {
        void *func_start, *func_end;
        func_start = g_array_index(func_array, void*, j);
        func_end = g_array_index(func_array, void*, j+1);
        pos = 0;
        
        /* print function name */
        char* the_func_name;
        the_func_name = g_hash_table_lookup(func_name_hash, func_start);
        if (the_func_name) {
            fprintf(the_dump_file, "\n\nFUNCTION: %s\n\n", the_func_name);
        }
        
        while (pos < func_end-func_start) {
            
            /* print BB start */
            if (i < BB_array->len && func_start+pos == theBB.originalBB_start) {
                fprintf(the_dump_file, "********************************************************************************\n");
            }
            
            /* print address */
            fprintf(the_dump_file, "%08x\t", (unsigned int)func_start+pos);
            
            /* disassemble address */
            size = x86_disasm(func_start, func_end-func_start, 0, pos, &insn);
            if (size) {
                /* print instruction */
                x86_format_insn(&insn, line, LINE_SIZE, att_syntax);
                fprintf(the_dump_file, "%s", line);
                
                /* print absolute address for jump */
                if (insn.type == insn_jmp || insn.type == insn_jcc) {   // istruzioni jmp (salto incondizionato) e jcc (salti condizionati, es. jz, jnz, jc, ...)
                    if (insn.operands->op.type == op_relative_near) {
                        fprintf(the_dump_file, "\t\t (absolute address = %08x)", (unsigned int)func_start + pos + insn.size + insn.operands->op.data.relative_near);
                    }
                    else if (insn.operands->op.type == op_relative_far) {
                        fprintf(the_dump_file, "\t\t (absolute address = %08x)", (unsigned int)func_start + pos + insn.size + insn.operands->op.data.relative_far);
                        if (i < BB_array->len && theBB.BB_start != theBB.originalBB_start && theBB.BB_start == func_start + pos + insn.size + insn.operands->op.data.relative_far) {
                            fprintf(the_dump_file, "\t\t <-- PATCH BB %d", i);
                        }
                    }
                /*  else if (insn.operands->op.type == op_absolute) {
                        fprintf(the_dump_file, "\t\t case op_absolute, TO DO"); //*******************************************************************
                    }
                    else if (insn.operands->op.type == op_expression) {
                        fprintf(the_dump_file, "\t\t case op_expression, TO DO"); //*******************************************************************
                    }
                    else if (insn.operands->op.type == op_offset) {
                        fprintf(the_dump_file, "\t\t case op_offset, TO DO"); //*******************************************************************
                    }
                    else {
                        fprintf(the_dump_file, "\t\t ******************* JMP non gestito **************");
                    }*/
                }
                else if (insn.type == insn_call || insn.type == insn_callcc) {  // istruzioni call (salto incondizionato) e callcc (salti condizionati)
                    if (insn.operands->op.type == op_relative_near) {
                        fprintf(the_dump_file, "\t\t (absolute address = %08x)", (unsigned int)func_start + pos + insn.size + insn.operands->op.data.relative_near);
                    }
                    else if (insn.operands->op.type == op_relative_far) {
                        fprintf(the_dump_file, "\t\t (absolute address = %08x)", (unsigned int)func_start + pos + insn.size + insn.operands->op.data.relative_far);
                    }
                /*  else if (insn.operands->op.type == op_absolute) {
                        fprintf(the_dump_file, "\t\t case op_absolute, TO DO"); //*******************************************************************
                    }*/
                }
                
                /* print patchable or cached instruction tag */
                if (g_hash_table_lookup(mov_hash, func_start+pos)) {
                    fprintf(the_dump_file, "\t\t\t <--- patchable instruction (size = %d)", size);
                }
                else if (g_hash_table_lookup(g_cchash, func_start+pos)) {
                    fprintf(the_dump_file, "\t\t\t <--- cached instruction (size = %d)", size);
                }
                
                fprintf(the_dump_file, "\n");
                
                pos += size;
            }
            else {
                fprintf(the_dump_file, "***Invalid instruction***\n");
                pos++;
            }
            
            /* print BB end */
            if (i < BB_array->len && func_start+pos == theBB.originalBB_end) {
                fprintf(the_dump_file, "--------------------------------------------------------------------------------\n");
                i++;
                if (i < BB_array->len) {
                    theBB = g_array_index(BB_array, BB_rec, i);
                }
            }
        }
        
        
        
        /* PRINT ALL PATCHED BB IN THE FUNCTION */
        
        /* find index of first BB in the function */
        int k = i-1;
        while (k>=0 && g_array_index(BB_array, BB_rec, k).originalBB_start >= func_start) k--;
        k++;
        
        /* for all BB in the function */
        while (k < i) {
            BB_rec theFuncBB;
            theFuncBB = g_array_index(BB_array, BB_rec, k);
            /* if BB is patched */
            if (theFuncBB.originalBB_start != theFuncBB.BB_start) {
                unsigned int BB_pos = 0;
                void* patch_end_addr = 0;
                int patch_num = 0;
                
                /* print function name */
                if (the_func_name) {
                    fprintf(the_dump_file, "\n\nFUNCTION: %s\t (PATCH BB %d)\n\n", the_func_name, k);
                }
                
                /* print BB start */
                fprintf(the_dump_file, "********************************************************************************\n");
                
                while (BB_pos < theFuncBB.BB_end + 5 - theFuncBB.BB_start) {    //*** il +5 serve per stampare anche il jmp di ritorno alla fine del BB ***
                    
                    /* print address */
                    fprintf(the_dump_file, "%08x\t", (unsigned int)theFuncBB.BB_start+BB_pos);
                    
                    /* disassemble address */
                    size = x86_disasm(theFuncBB.BB_start, theFuncBB.BB_end+5-theFuncBB.BB_start, 0, BB_pos, &insn);
                    if (size) {
                        /* print instruction */
                        x86_format_insn(&insn, line, LINE_SIZE, att_syntax);
                        fprintf(the_dump_file, "%s", line);
                        
                        /* print absolute address for jump */
                        if (insn.type == insn_jmp || insn.type == insn_jcc) {   // istruzioni jmp (salto incondizionato) e jcc (salti condizionati, es. jz, jnz, jc, ...)
                            if (insn.operands->op.type == op_relative_near) {
                                fprintf(the_dump_file, "\t\t (absolute address = %08x)", (unsigned int)theFuncBB.BB_start + BB_pos + insn.size + insn.operands->op.data.relative_near);
                            }
                            else if (insn.operands->op.type == op_relative_far) {
                                fprintf(the_dump_file, "\t\t (absolute address = %08x)", (unsigned int)theFuncBB.BB_start + BB_pos + insn.size + insn.operands->op.data.relative_far);
                            }
                        /*  else if (insn.operands->op.type == op_absolute) {
                                fprintf(the_dump_file, "\t\t case op_absolute, TO DO"); //*******************************************************************
                            }
                            else if (insn.operands->op.type == op_expression) {
                                fprintf(the_dump_file, "\t\t case op_expression, TO DO"); //*******************************************************************
                            }
                            else if (insn.operands->op.type == op_offset) {
                                fprintf(the_dump_file, "\t\t case op_offset, TO DO"); //*******************************************************************
                            }
                            else {
                                fprintf(the_dump_file, "\t\t ******************* JMP non gestito **************");
                            }*/
                        }
                        else if (insn.type == insn_call || insn.type == insn_callcc) {  // istruzioni call (salto incondizionato) e callcc (salti condizionati)
                            if (insn.operands->op.type == op_relative_near) {
                                fprintf(the_dump_file, "\t\t (absolute address = %08x)", (unsigned int)theFuncBB.BB_start + BB_pos + insn.size + insn.operands->op.data.relative_near);
                            }
                            else if (insn.operands->op.type == op_relative_far) {
                                fprintf(the_dump_file, "\t\t (absolute address = %08x)", (unsigned int)theFuncBB.BB_start + BB_pos + insn.size + insn.operands->op.data.relative_far);
                            }
                        /*  else if (insn.operands->op.type == op_absolute) {
                                fprintf(the_dump_file, "\t\t case op_absolute, TO DO"); //*******************************************************************
                            }*/
                        }
                        
                        /* print patchable or cached instruction tag */
                        mov_rec* the_mov;
                        the_mov = g_hash_table_lookup(mov_hash, theFuncBB.BB_start+BB_pos);
                        if (the_mov) {
                            if (!the_mov->patched) {
                                fprintf(the_dump_file, "\t\t\t <--- patchable instruction (size = %d)", size);
                            }
                            else {
                                /* print patch start */
                                patch_num++;
                                fprintf(the_dump_file, "\t\t\t <--- PATCH %d START (FIRST INSTRUCTION OF THE PATCH)", patch_num);
                                patch_end_addr = the_mov->address + the_mov->patch_size_func(the_mov) - (the_mov->address_end - the_mov->address);
                            }
                        }
                        else if (g_hash_table_lookup(g_cchash, theFuncBB.BB_start+BB_pos)) {
                            fprintf(the_dump_file, "\t\t\t <--- cached instruction (size = %d)", size);
                        }
                        
                        /* print patch end */
                        if (theFuncBB.BB_start+BB_pos == patch_end_addr) {
                            fprintf(the_dump_file, "\t\t\t <--- PATCH %d END (LAST INSTRUCTION OF THE PATCH)", patch_num);
                        }
                        
                        fprintf(the_dump_file, "\n");
                        
                        BB_pos += size;
                    }
                    else {
                        fprintf(the_dump_file, "***Invalid instruction***\n");
                        BB_pos++;
                    }
                }
                
                /* print BB end */
                fprintf(the_dump_file, "--------------------------------------------------------------------------------\n");
            }
            
            k++;
        }
    }
    
    /* uninitialize libdisasm */
    x86_cleanup();

#else

    _DInst insn[rm_MAX_DIS_INSTRUCTIONS];   /* instructions */
    unsigned int instructions_count = 0;
    unsigned int next = 0;
    _DecodedInst inst;                      /* instruction (string) */
    
    _CodeInfo ci;
    ci.dt = Decode32Bits;
    ci.features = DF_NONE;
    
    i = 0;
    if (i < BB_array->len) {
        theBB = g_array_index(BB_array, BB_rec, i);
    }
    int j;
    for(j=0; j<func_array->len-1; j++) {
        void *func_start, *func_end;
        func_start = g_array_index(func_array, void*, j);
        func_end = g_array_index(func_array, void*, j+1);
        next = 0;
        
        ci.code = func_start;
        ci.codeLen = func_end-func_start;
        ci.codeOffset = (unsigned int)func_start;
        ci.nextOffset = 0;
        
        /* print function name */
        char* the_func_name;
        the_func_name = g_hash_table_lookup(func_name_hash, func_start);
        if (the_func_name) {
            fprintf(the_dump_file, "\n\nFUNCTION: %s\n\n", the_func_name);
        }
        
        while(ci.codeLen > 0) {
            /* disassemble address */
            distorm_decompose(&ci, insn, rm_MAX_DIS_INSTRUCTIONS, &instructions_count);
            
            int z;
            for(z=0; z<instructions_count; z++) {
                
                /* print BB start */
                if (i < BB_array->len && (void*)insn[z].addr == theBB.originalBB_start) {
                    fprintf(the_dump_file, "********************************************************************************\n");
                }
                
                /* print address */
                fprintf(the_dump_file, "%08x\t", (unsigned int)insn[z].addr);
                
                if(insn[z].flags == FLAG_NOT_DECODABLE) {
                    fprintf(the_dump_file, "***Invalid instruction***\n");
                }
                else {
                    /* print instruction */
                    distorm_format(&ci, &insn[z], &inst);
                    fprintf(the_dump_file, "%s\t%s", inst.mnemonic.p, inst.operands.p);
                    
                    /* print absolute address for jump */
                    if (META_GET_FC(insn[z].meta) == FC_UNC_BRANCH || META_GET_FC(insn[z].meta) == FC_CND_BRANCH) { // istruzioni jmp (salto incondizionato) e jcc (salti condizionati, es. jz, jnz, jc, ...)
                        if (insn[z].ops[0].type == O_PC) {  // jmp e jcc relativi
                            if (insn[z].ops[0].size == 8) { // short relativi
                                fprintf(the_dump_file, "\t\t (absolute address = %08x)", (unsigned int)INSTRUCTION_GET_TARGET(&insn[z]));
                            }
                            else {  // long relativi
                                fprintf(the_dump_file, "\t\t (absolute address = %08x)", (unsigned int)INSTRUCTION_GET_TARGET(&insn[z]));
                                if (i < BB_array->len && theBB.BB_start != theBB.originalBB_start && theBB.BB_start == (void*)INSTRUCTION_GET_TARGET(&insn[z])) {
                                    fprintf(the_dump_file, "\t\t <-- PATCH BB %d", i);
                                }
                            }
                        }
                    /*  else if (insn[z].ops[0].type == O_PTR) {
                            fprintf(the_dump_file, "\t\t case op_absolute (O_PTR), TO DO"); //*******************************************************************
                        }
                        else if (insn[z].ops[0].type == O_SMEM || insn.ops[0].type == O_MEM) {
                            fprintf(the_dump_file, "\t\t case op_expression (O_SMEM, O_MEM), TO DO"); //*******************************************************************
                        }
                        else if (insn[z].ops[0].type == O_DISP) {
                            fprintf(the_dump_file, "\t\t case op_offset (O_DISP), TO DO"); //*******************************************************************
                        }
                        else {
                            fprintf(the_dump_file, "\t\t ******************* JMP non gestito **************");
                        }*/
                    }
                    else if (META_GET_FC(insn[z].meta) == FC_CALL) {    // istruzioni call
                        if (insn[z].ops[0].type == O_PC) {  // call relativi
                            fprintf(the_dump_file, "\t\t (absolute address = %08x)", (unsigned int)INSTRUCTION_GET_TARGET(&insn[z]));
                        }
                    /*  else if (insn[z].ops[0].type == O_PTR) {
                            fprintf(the_dump_file, "\t\t case op_absolute (O_PTR), TO DO"); //*******************************************************************
                        }*/
                    }
                    
                    /* print patchable or cached instruction tag */
                    if (g_hash_table_lookup(mov_hash, (void*)insn[z].addr)) {
                        fprintf(the_dump_file, "\t\t\t <--- patchable instruction (size = %d)", insn[z].size);
                    }
                    else if (g_hash_table_lookup(g_cchash, (void*)insn[z].addr)) {
                        fprintf(the_dump_file, "\t\t\t <--- cached instruction (size = %d)", insn[z].size);
                    }
                    
                    fprintf(the_dump_file, "\n");
                }
                
                /* print BB end */
                if (i < BB_array->len && ((void*)insn[z].addr + insn[z].size) == theBB.originalBB_end) {
                    fprintf(the_dump_file, "--------------------------------------------------------------------------------\n");
                    i++;
                    if (i < BB_array->len) {
                        theBB = g_array_index(BB_array, BB_rec, i);
                    }
                }
            }
            
            next = ci.nextOffset - ci.codeOffset;
            ci.code += next;
            ci.codeLen -= next;
            ci.codeOffset += next;
        }
        
        
        
        /* PRINT ALL PATCHED BB IN THE FUNCTION */
        
        /* find index of first BB in the function */
        int k = i-1;
        while (k>=0 && g_array_index(BB_array, BB_rec, k).originalBB_start >= func_start) k--;
        k++;
        
        /* for all BB in the function */
        while (k < i) {
            BB_rec theFuncBB;
            theFuncBB = g_array_index(BB_array, BB_rec, k);
            /* if BB is patched */
            if (theFuncBB.originalBB_start != theFuncBB.BB_start) {
                void* patch_end_addr = 0;
                int patch_num = 0;
                next = 0;
                
                ci.code = theFuncBB.BB_start;
                ci.codeLen = theFuncBB.BB_end + 5 - theFuncBB.BB_start; //*** il +5 serve per stampare anche il jmp di ritorno alla fine del BB ***
                ci.codeOffset = (unsigned int)theFuncBB.BB_start;
                ci.nextOffset = 0;
                
                /* print function name */
                if (the_func_name) {
                    fprintf(the_dump_file, "\n\nFUNCTION: %s\t (PATCH BB %d)\n\n", the_func_name, k);
                }
                
                /* print BB start */
                fprintf(the_dump_file, "********************************************************************************\n");
                
                while(ci.codeLen > 0) {
                    /* disassemble address */
                    distorm_decompose(&ci, insn, rm_MAX_DIS_INSTRUCTIONS, &instructions_count);
                    
                    int z;
                    for(z=0; z<instructions_count; z++) {
                        
                        /* print address */
                        fprintf(the_dump_file, "%08x\t", (unsigned int)insn[z].addr);
                        
                        if(insn[z].flags == FLAG_NOT_DECODABLE) {
                            fprintf(the_dump_file, "***Invalid instruction***\n");
                        }
                        else {
                            /* print instruction */
                            distorm_format(&ci, &insn[z], &inst);
                            fprintf(the_dump_file, "%s\t%s", inst.mnemonic.p, inst.operands.p);
                            
                            /* print absolute address for jump */
                            if (META_GET_FC(insn[z].meta) == FC_UNC_BRANCH || META_GET_FC(insn[z].meta) == FC_CND_BRANCH) { // istruzioni jmp (salto incondizionato) e jcc (salti condizionati, es. jz, jnz, jc, ...)
                                if (insn[z].ops[0].type == O_PC) {  // jmp e jcc relativi
                                    if (insn[z].ops[0].size == 8) { // short relativi
                                        fprintf(the_dump_file, "\t\t (absolute address = %08x)", (unsigned int)INSTRUCTION_GET_TARGET(&insn[z]));
                                    }
                                    else {  // long relativi
                                        fprintf(the_dump_file, "\t\t (absolute address = %08x)", (unsigned int)INSTRUCTION_GET_TARGET(&insn[z]));
                                    }
                                }
                            /*  else if (insn[z].ops[0].type == O_PTR) {
                                    fprintf(the_dump_file, "\t\t case op_absolute (O_PTR), TO DO"); //*******************************************************************
                                }
                                else if (insn[z].ops[0].type == O_SMEM || insn.ops[0].type == O_MEM) {
                                    fprintf(the_dump_file, "\t\t case op_expression (O_SMEM, O_MEM), TO DO"); //*******************************************************************
                                }
                                else if (insn[z].ops[0].type == O_DISP) {
                                    fprintf(the_dump_file, "\t\t case op_offset (O_DISP), TO DO"); //*******************************************************************
                                }
                                else {
                                    fprintf(the_dump_file, "\t\t ******************* JMP non gestito **************");
                                }*/
                            }
                            else if (META_GET_FC(insn[z].meta) == FC_CALL) {    // istruzioni call
                                if (insn[z].ops[0].type == O_PC) {  // call relativi
                                    fprintf(the_dump_file, "\t\t (absolute address = %08x)", (unsigned int)INSTRUCTION_GET_TARGET(&insn[z]));
                                }
                            /*  else if (insn[z].ops[0].type == O_PTR) {
                                    fprintf(the_dump_file, "\t\t case op_absolute (O_PTR), TO DO"); //*******************************************************************
                                }*/
                            }
                            
                            /* print patchable or cached instruction tag */
                            mov_rec* the_mov;
                            the_mov = g_hash_table_lookup(mov_hash, (void*)insn[z].addr);
                            if (the_mov) {
                                if (!the_mov->patched) {
                                    fprintf(the_dump_file, "\t\t\t <--- patchable instruction (size = %d)", insn[z].size);
                                }
                                else {
                                    /* print patch start */
                                    patch_num++;
                                    fprintf(the_dump_file, "\t\t\t <--- PATCH %d START (FIRST INSTRUCTION OF THE PATCH)", patch_num);
                                    patch_end_addr = the_mov->address + the_mov->patch_size_func(the_mov) - (the_mov->address_end - the_mov->address);
                                }
                            }
                            else if (g_hash_table_lookup(g_cchash, (void*)insn[z].addr)) {
                                fprintf(the_dump_file, "\t\t\t <--- cached instruction (size = %d)", insn[z].size);
                            }
                            
                            /* print patch end */
                            if ((void*)insn[z].addr == patch_end_addr) {
                                fprintf(the_dump_file, "\t\t\t <--- PATCH %d END (LAST INSTRUCTION OF THE PATCH)", patch_num);
                            }
                            
                            fprintf(the_dump_file, "\n");
                        }
                    }
                    
                    next = ci.nextOffset - ci.codeOffset;
                    ci.code += next;
                    ci.codeLen -= next;
                    ci.codeOffset += next;
                }
                
                /* print BB end */
                fprintf(the_dump_file, "--------------------------------------------------------------------------------\n");
            }
            
            k++;
        }
    }
#endif
    
    /* close dump file */
    fclose(the_dump_file);
    
    g_array_free(func_array, TRUE);
    g_hash_table_destroy(func_name_hash);
    
    return 0;
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
