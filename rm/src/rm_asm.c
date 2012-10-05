/* ============================================================================
 *  rm_asm.c
 * ============================================================================

 *  Author:         (c) 2010 the rm team
 *  License:        See the end of this file for license information
 *  Created:        July 21, 2010

 *  Last changed:   $Date: 2011/01/06 17:24:41 $
 *  Changed by:     $Author: alex $
 *  Revision:       $Revision: 1.15 $
*/

#include "_rm_private.h"
#include "_rm_asm.h"

//**** COMMENT - start ***********************************************************************************************************************
        // LEA ---> 1000 1101 : modA reg r/m
        
        // MOV ---> #byte: 1 (opcode), 1 (modR/M), 0-1 (SIB), 0-4 (displacement), 0-4 (immediate)
        // memory to reg ---> 1000 101w : mod reg r/m
        // reg to memory ---> 1000 100w : mod reg r/m
        // immediate to memory ---> 1100 011w : mod 000 r/m : immediate data
        
        //*********** l'operand-size attribute serve per capire se l'operando è da 16 o 32 bit: dove è definito? ****************
//**** COMMENT - end *************************************************************************************************************************

#define PATCH(case, mode, reg) \
    \
    unsigned char the_lea_opcode_buf[20]; \
    unsigned int the_lea_opcode_size; \
    size_t the_handler_call_size; \
    unsigned char opcode_buf2[4]; \
    char disp = 0; \
    unsigned char compare_jmp_opcode[2]; \
    int long_disp = 0; \
    unsigned int the_pos = 0; \
    \
    /* Creazione opcode LEA */ \
    the_lea_opcode_size = instr->address_end - instr->address - instr->immediate_data_size; \
    memcpy(the_lea_opcode_buf, instr->address, the_lea_opcode_size); \
    the_lea_opcode_buf[0] = 0x8d; \
    the_lea_opcode_buf[1] = the_lea_opcode_buf[1] & 0xC7; /* 0xC7 = 11000111 // eax = 000 */ \
    if(instr->eax_is_in_use) { \
        the_lea_opcode_buf[1] = the_lea_opcode_buf[1] | 0x08; /* 0x08 = 00001000 // ecx = 001 */ \
    } \
    \
    \
    /* Copia patch (1/3) */ \
    memcpy(dest_addr + the_pos, &patch_code_start_##case, &compare_##case - &patch_code_start_##case); \
    the_pos = the_pos + (&compare_##case - &patch_code_start_##case); \
    \
    /* Copia opcode LEA */ \
    memcpy(dest_addr + the_pos, the_lea_opcode_buf, the_lea_opcode_size); \
    the_pos = the_pos + the_lea_opcode_size; \
    \
    /* Copia patch (2/3) */ \
    memcpy(dest_addr + the_pos, &compare_##case, &handler_call_start_##case - &compare_##case); \
    the_pos = the_pos + (&handler_call_start_##case - &compare_##case); \
    \
    /* Copia handler call */ \
    if(g_##mode##_handler) { /* default patch */ \
        the_handler_call_size = (&handler_call_end_##case - &handler_call_start_##case); \
        memcpy(dest_addr + the_pos, &handler_call_start_##case, the_handler_call_size); \
    } \
    else if(g_size_table[rm_##mode##_##reg##_patch][rm_size_4] > 0) { /* inline patch */ \
        the_handler_call_size = g_size_table[rm_##mode##_##reg##_patch][rm_size_4]; \
        memcpy(dest_addr + the_pos, g_patch_table[rm_##mode##_##reg##_patch][rm_size_4], the_handler_call_size); \
    } \
    else { /* no handler */ \
        the_handler_call_size = 0; \
    } \
    the_pos = the_pos + the_handler_call_size; \
    \
    /* Copia patch (3/3) */ \
    memcpy(dest_addr + the_pos, &handler_call_end_##case, &else_end_label_##case - &handler_call_end_##case); \
    the_pos = the_pos + (&else_end_label_##case - &handler_call_end_##case); \
    \
    /* Copia mov originale */ \
    memcpy(dest_addr + the_pos, instr->address, instr->address_end - instr->address); \
    the_pos = the_pos + (instr->address_end - instr->address); \
    \
    \
    /* Creazione opcode modified_MOV */ \
    if(instr->immediate_data_size == 0) { /* reg */ \
        /* Creazione opcode modified_MOV (solo il secondo byte, per modificare il reg) */ \
        memcpy(opcode_buf2+1, instr->address+1, 1); \
        opcode_buf2[1] = opcode_buf2[1] & 0x38; /* 0x38 = 00111000 */ \
        memcpy(opcode_buf2, &modified_mov_##case+1, 1); \
        /*opcode_buf2[0] = opcode_buf2[0] & 0xC7; // 0xC7 = 11000111 // non serve perchè usiamo eax che ha già codice 000 */ \
        opcode_buf2[0] = opcode_buf2[0] | opcode_buf2[1]; \
    } \
    else { /* immediate data */ \
        /* Creazione opcode modified_MOV (solo ultimi 4 byte, per modificare l'immediate data) //****** TO DO: gestire casi con imm_data da 1 o 2 byte ******** */ \
        memcpy(opcode_buf2, instr->address_end - instr->immediate_data_size, instr->immediate_data_size); \
    } \
    \
    /* Modifica della modified_MOV */ \
    if(instr->immediate_data_size == 0) { /* reg */ \
    /*  memcpy(&modified_mov_##case+1, opcode_buf2, 1); */ \
        memcpy(dest_addr + the_lea_opcode_size + (&modified_mov_##case - &patch_code_start_##case) + 1, opcode_buf2, 1); \
    } \
    else { /* immediate data */ \
    /*  memcpy(&modified_mov_##case+2, opcode_buf2, instr->immediate_data_size); */ \
    /*  memcpy(dest_addr + the_lea_opcode_size + (&modified_mov_##case - &patch_code_start_##case) + 2, opcode_buf2, instr->immediate_data_size); */ \
        memcpy(dest_addr + the_lea_opcode_size + (&modified_mov_##case - &patch_code_start_##case) + 6, opcode_buf2, instr->immediate_data_size); /*** ipotizzo offset da 4 byte ***/ \
    } \
    \
    /* Modifica del jmp */ \
    /************ assumo che il jmp sia uno short (EB) positivo --> Es. EB 2F ******** */ \
/*  memcpy(&disp, &modified_jmp_##case+1, &if_end_label_##case - &modified_jmp_##case - 1); */ \
    memcpy(&disp, &modified_jmp_##case+1, 1); \
    disp = disp + (instr->address_end - instr->address); \
/*  memcpy(&modified_jmp_##case+1, &disp, 1); */ \
    memcpy(dest_addr + the_lea_opcode_size + (&handler_call_start_##case - &patch_code_start_##case) + \
        the_handler_call_size + (&modified_jmp_##case - &handler_call_end_##case) + 1, &disp, 1); \
    \
    /* Modifica del handler_call */ \
    if(g_##mode##_handler) { /* default patch */ \
        memcpy(dest_addr + the_lea_opcode_size + (&handler_call_##case - &patch_code_start_##case) + 1, &g_##mode##_handler, 4); \
    } \
    /* Modifica del compare_jmp */ \
    else if(g_size_table[rm_##mode##_##reg##_patch][rm_size_4] > 0) { /* inline patch */ \
        /*** trasformo il jbe da short (76) a long (0f86) ***/ \
        compare_jmp_opcode[0] = 0x0f; \
        compare_jmp_opcode[1] = 0x86; \
        memcpy(dest_addr + the_lea_opcode_size + (&compare_jmp_##case - &patch_code_start_##case), compare_jmp_opcode, 2); \
        /*** modifico il displacement del jbe ***/ \
        memcpy(&long_disp, &compare_jmp_##case+1, 1); \
        long_disp = long_disp - 4 - (&handler_call_end_##case - &handler_call_start_##case) + the_handler_call_size; \
        memcpy(dest_addr + the_lea_opcode_size + (&compare_jmp_##case - &patch_code_start_##case) + 2, &long_disp, 4); \
    } \


#define PATCH_SIZE(case, mode, reg) \
size_t _rm_patch_size_##case (mov_rec* instr) { \
    unsigned int the_lea_opcode_size; \
    size_t the_handler_call_size; \
    the_lea_opcode_size = instr->address_end - instr->address - instr->immediate_data_size; \
    if(g_##mode##_handler) { /* default patch */ \
        the_handler_call_size = &handler_call_end_##case - &handler_call_start_##case; \
    } \
    else if(g_size_table[rm_##mode##_##reg##_patch][rm_size_4] > 0) { /* inline patch */ \
        the_handler_call_size = g_size_table[rm_##mode##_##reg##_patch][rm_size_4]; \
    } \
    else { /* no handler */ \
        the_handler_call_size = 0; \
    } \
    return the_lea_opcode_size + (&handler_call_start_##case - &patch_code_start_##case) + \
        the_handler_call_size + (&else_end_label_##case - &handler_call_end_##case) + (instr->address_end - instr->address); \
}


#define PATCH_CODE_1(case, reg) \
    __asm__ __volatile__ ( \
            "jmp else_end_label_"case";" \
    ); \
    __asm__ __volatile__ ( \
        "patch_code_start_"case":" \
            "push   %"reg";" \
            "pushf;" \
    ); \
/*  __asm__ __volatile__ ( 
            "leal   mem, %"reg";"   // l'opcode di questa istruzione è contenuta in the_lea_opcode_buf
    );*/ \
    __asm__ __volatile__ ( \
        "compare_"case":" \
            "cmp    $"rm_MAX_NOT_REACTIVE_MEMORY_ADDR_STR", %"reg";" \
        "compare_jmp_"case":" \
            "jbe    if_end_label_"case";" \
            "nop; nop; nop; nop;" /* spazio necessario per trasformare il jbe da short (2 byte) a long (6 byte) */ \
    );


#define PATCH_CODE_2(case, reg, reg2) \
    __asm__ __volatile__ ( \
        "handler_call_start_"case":" \
    /***        "call   HANDLER;" */ \
            "push   %"reg2";" \
            "push   %edx;" \
            "push   $0x4;"          /* parameter #2 */ \
            "push   %"reg";"        /* parameter #1 */ \
        "handler_call_"case":" \
            "movl   $0x08000000, %"reg";"   /* l'indirizzo di memoria sarà modificato */ \
            "call   *%"reg";" \
            "addl   $0x8, %esp;"        /* remove the parameters from stack */ \
            "pop    %edx;" \
            "pop    %"reg2";" \
    /***        "call   HANDLER;" */ \
        "handler_call_end_"case":" \
            "popf;" \
            "pop    %"reg";" \
        "modified_jmp_"case":" \
            "jmp    else_end_label_"case";" \
        "if_end_label_"case":" \
            "popf;" \
            "pop    %"reg";" \
    ); \
/*  __asm__ __volatile__ ( 
            "movl   mem, REG;"  // l'istruzione originale
    );*/ \
    __asm__ __volatile__ ( \
        "else_end_label_"case":" \
    );





#define PATCH_DOUBLE_ACCESS(case, reg) \
    \
    unsigned char the_lea_opcode_buf[20]; \
    unsigned int the_lea_opcode_size; \
    size_t the_handler_call_size; \
    unsigned char opcode_buf2[4]; \
    char disp = 0; \
    unsigned char compare_jmp_opcode[2]; \
    int long_disp = 0; \
    unsigned int the_pos = 0; \
    \
    /* Creazione opcode LEA */ \
    the_lea_opcode_size = instr->address_end - instr->address - instr->immediate_data_size; \
    memcpy(the_lea_opcode_buf, instr->address, the_lea_opcode_size); \
    the_lea_opcode_buf[0] = 0x8d; \
    the_lea_opcode_buf[1] = the_lea_opcode_buf[1] & 0xC7; /* 0xC7 = 11000111 // eax = 000 */ \
    if(instr->eax_is_in_use) { \
        the_lea_opcode_buf[1] = the_lea_opcode_buf[1] | 0x08; /* 0x08 = 00001000 // ecx = 001 */ \
    } \
    \
    \
    /* Copia patch (1/3) */ \
    memcpy(dest_addr + the_pos, &patch_code_start_##case, &compare_##case - &patch_code_start_##case); \
    the_pos = the_pos + (&compare_##case - &patch_code_start_##case); \
    \
    /* Copia opcode LEA */ \
    memcpy(dest_addr + the_pos, the_lea_opcode_buf, the_lea_opcode_size); \
    the_pos = the_pos + the_lea_opcode_size; \
    \
    /* Copia patch (2/3) */ \
    memcpy(dest_addr + the_pos, &compare_##case, &handler_call_start_##case - &compare_##case); \
    the_pos = the_pos + (&handler_call_start_##case - &compare_##case); \
    \
    /* Copia handler call */ \
    if(g_read_handler) { /* default patch */ \
        the_handler_call_size = (&handler_call_end_##case - &handler_call_start_##case); \
        memcpy(dest_addr + the_pos, &handler_call_start_##case, the_handler_call_size); \
    } \
    else if(g_size_table[rm_rd_wr_##reg##_patch][rm_size_4] > 0) { /* inline patch */ \
        the_handler_call_size = g_size_table[rm_rd_wr_##reg##_patch][rm_size_4]; \
        memcpy(dest_addr + the_pos, g_patch_table[rm_rd_wr_##reg##_patch][rm_size_4], the_handler_call_size); \
    } \
    else { /* no handler */ \
        the_handler_call_size = 0; \
    } \
    the_pos = the_pos + the_handler_call_size; \
    \
    /* Copia patch (3/3) */ \
    memcpy(dest_addr + the_pos, &handler_call_end_##case, &else_end_label_##case - &handler_call_end_##case); \
    the_pos = the_pos + (&else_end_label_##case - &handler_call_end_##case); \
    \
    /* Copia mov originale */ \
    memcpy(dest_addr + the_pos, instr->address, instr->address_end - instr->address); \
    the_pos = the_pos + (instr->address_end - instr->address); \
    \
    \
    /* Creazione opcode modified_MOV */ \
    if(instr->immediate_data_size == 0) { /* reg */ \
        /* Creazione opcode modified_MOV (solo il secondo byte, per modificare il reg) */ \
        memcpy(opcode_buf2+1, instr->address+1, 1); \
        opcode_buf2[1] = opcode_buf2[1] & 0x38; /* 0x38 = 00111000 */ \
        memcpy(opcode_buf2, &modified_mov_##case+1, 1); \
        /*opcode_buf2[0] = opcode_buf2[0] & 0xC7; // 0xC7 = 11000111 // non serve perchè usiamo eax che ha già codice 000 */ \
        opcode_buf2[0] = opcode_buf2[0] | opcode_buf2[1]; \
    } \
    else { /* immediate data */ \
        /* Creazione opcode modified_MOV (solo ultimi 4 byte, per modificare l'immediate data) //****** TO DO: gestire casi con imm_data da 1 o 2 byte ******** */ \
        memcpy(opcode_buf2, instr->address_end - instr->immediate_data_size, instr->immediate_data_size); \
    } \
    \
    /* Modifica della modified_MOV */ \
    if(instr->immediate_data_size == 0) { /* reg */ \
    /*  memcpy(&modified_mov_##case+1, opcode_buf2, 1); */ \
        memcpy(dest_addr + the_lea_opcode_size + (&modified_mov_##case - &patch_code_start_##case) + 1, opcode_buf2, 1); \
    } \
    else { /* immediate data */ \
    /*  memcpy(&modified_mov_##case+2, opcode_buf2, instr->immediate_data_size); */ \
    /*  memcpy(dest_addr + the_lea_opcode_size + (&modified_mov_##case - &patch_code_start_##case) + 2, opcode_buf2, instr->immediate_data_size); */ \
        memcpy(dest_addr + the_lea_opcode_size + (&modified_mov_##case - &patch_code_start_##case) + 6, opcode_buf2, instr->immediate_data_size); /*** ipotizzo offset da 4 byte ***/ \
    } \
    \
    /* Modifica del jmp */ \
    /************ assumo che il jmp sia uno short (EB) positivo --> Es. EB 2F ******** */ \
/*  memcpy(&disp, &modified_jmp_##case+1, &if_end_label_##case - &modified_jmp_##case - 1); */ \
    memcpy(&disp, &modified_jmp_##case+1, 1); \
    disp = disp + (instr->address_end - instr->address); \
/*  memcpy(&modified_jmp_##case+1, &disp, 1); */ \
    memcpy(dest_addr + the_lea_opcode_size + (&handler_call_start_##case - &patch_code_start_##case) + \
        the_handler_call_size + (&modified_jmp_##case - &handler_call_end_##case) + 1, &disp, 1); \
    \
    /* Modifica del handler_call */ \
    if(g_read_handler) { /* default patch */ \
        /* Modifica del read_handler_call */ \
        memcpy(dest_addr + the_lea_opcode_size + (&read_handler_call_##case - &patch_code_start_##case) + 1, &g_read_handler, 4); \
        /* Modifica del write_handler_call */ \
        memcpy(dest_addr + the_lea_opcode_size + (&write_handler_call_##case - &patch_code_start_##case) + 1, &g_write_handler, 4); \
    } \
    /* Modifica del compare_jmp */ \
    else if(g_size_table[rm_rd_wr_##reg##_patch][rm_size_4] > 0) { /* inline patch */ \
        /*** trasformo il jbe da short (76) a long (0f86) ***/ \
        compare_jmp_opcode[0] = 0x0f; \
        compare_jmp_opcode[1] = 0x86; \
        memcpy(dest_addr + the_lea_opcode_size + (&compare_jmp_##case - &patch_code_start_##case), compare_jmp_opcode, 2); \
        /*** modifico il displacement del jbe ***/ \
        memcpy(&long_disp, &compare_jmp_##case+1, 1); \
        long_disp = long_disp - 4 - (&handler_call_end_##case - &handler_call_start_##case) + the_handler_call_size; \
        memcpy(dest_addr + the_lea_opcode_size + (&compare_jmp_##case - &patch_code_start_##case) + 2, &long_disp, 4); \
    } \


#define PATCH_SIZE_DOUBLE_ACCESS(case, reg) \
size_t _rm_patch_size_##case (mov_rec* instr) { \
    unsigned int the_lea_opcode_size; \
    size_t the_handler_call_size; \
    the_lea_opcode_size = instr->address_end - instr->address - instr->immediate_data_size; \
    if(g_read_handler) { /* default patch */ \
        the_handler_call_size = &handler_call_end_##case - &handler_call_start_##case; \
    } \
    else if(g_size_table[rm_rd_wr_##reg##_patch][rm_size_4] > 0) { /* inline patch */ \
        the_handler_call_size = g_size_table[rm_rd_wr_##reg##_patch][rm_size_4]; \
    } \
    else { /* no handler */ \
        the_handler_call_size = 0; \
    } \
    return the_lea_opcode_size + (&handler_call_start_##case - &patch_code_start_##case) + \
        the_handler_call_size + (&else_end_label_##case - &handler_call_end_##case) + (instr->address_end - instr->address); \
}


#define PATCH_CODE_1_DOUBLE_ACCESS(case, reg) \
    __asm__ __volatile__ ( \
            "jmp else_end_label_"case";" \
    ); \
    __asm__ __volatile__ ( \
        "patch_code_start_"case":" \
            "push   %"reg";" \
            "pushf;" \
    ); \
/*  __asm__ __volatile__ ( 
            "leal   mem, %"reg";"   // l'opcode di questa istruzione è contenuta in the_lea_opcode_buf
    );*/ \
    __asm__ __volatile__ ( \
        "compare_"case":" \
            "cmp    $"rm_MAX_NOT_REACTIVE_MEMORY_ADDR_STR", %"reg";" \
        "compare_jmp_"case":" \
            "jbe    if_end_label_"case";" \
            "nop; nop; nop; nop;" /* spazio necessario per trasformare il jbe da short (2 byte) a long (6 byte) */ \
    );


#define PATCH_CODE_2_DOUBLE_ACCESS(case, reg, reg2) \
    __asm__ __volatile__ ( \
        "handler_call_start_"case":" \
    /***        "call   READ_HANDLER;" */ \
            "push   %"reg2";" \
            "push   %edx;" \
            "push   $0x4;"          /* parameter #2 */ \
            "push   %"reg";"        /* parameter #1 */ \
        "read_handler_call_"case":" \
            "movl   $0x08000000, %"reg";"   /* l'indirizzo di memoria sarà modificato */ \
            "call   *%"reg";" \
        /*  "addl   $0x8, %esp;"        /* remove the parameters from stack */ \
        /*  "pop    %edx;" */ \
        /*  "pop    %"reg2";" */ \
    /***        "call   READ_HANDLER;" */ \
    /***        "call   WRITE_HANDLER;" */ \
        /*  "push   %"reg2";" */ \
        /*  "push   %edx;" */ \
        /*  "push   $0x4;"          /* parameter #2 */ \
        /*  "push   %"reg";"        /* parameter #1 */ \
        "write_handler_call_"case":" \
            "movl   $0x08000000, %"reg";"   /* l'indirizzo di memoria sarà modificato */ \
            "call   *%"reg";" \
            "addl   $0x8, %esp;"        /* remove the parameters from stack */ \
            "pop    %edx;" \
            "pop    %"reg2";" \
    /***        "call   WRITE_HANDLER;" */ \
        "handler_call_end_"case":" \
            "popf;" \
            "pop    %"reg";" \
        "modified_jmp_"case":" \
            "jmp    else_end_label_"case";" \
        "if_end_label_"case":" \
            "popf;" \
            "pop    %"reg";" \
    ); \
/*  __asm__ __volatile__ ( 
            "addl   REG, mem;"  // l'istruzione originale
    );*/ \
    __asm__ __volatile__ ( \
        "else_end_label_"case":" \
    );


/* ----------------------------------------------------------------------------
 *  _rm_patch_SA
 * ----------------------------------------------------------------------------
*//*
void _rm_patch_SA(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** READ, eax NOT in use *****\n");
#endif
    
    PATCH(SA, read)
    
    
    // Codice patch da inserire
    __asm__ __volatile__ ( 
            "jmp else_end_label_SA;"
    );
    
    //***************************************************************************
    //******************** case SA (read, eax NOT in use) ***********************
    //***************************************************************************
    __asm__ __volatile__ ( 
        "patch_code_start_SA:"
            "push   %eax;"
            "pushf;" // flags
    );
/*  __asm__ __volatile__ ( 
            "leal   mem, %eax;" // l'opcode di questa istruzione è contenuta in the_lea_opcode_buf
    );*//*
    
    // if(ea >= 0xC0000000) //****** perchè il compilatore mette "ea-1" e usa "jbe" invece di mettere "ea" e usare "jb"?? ************
    __asm__ __volatile__ ( 
        "compare_SA:"
            "cmp    $"rm_MAX_NOT_REACTIVE_MEMORY_ADDR_STR", %eax;"
            "jbe    if_end_label_SA;"
        "modified_mov_SA:"
            "movl   -"rm_OFFSET_STR"(%eax), %eax;" // il reg sarà modificato; usiamo eax perchè ha codice 000
    //***       "call   READ_HANDLER;"
        //  "push   %eax;"
            "push   %ecx;"
            "push   %edx;"
            //****** push register eax, ecx, edx? ************************************
            "push   $0x4;"      // parameter #2
            "push   %eax;"      // parameter #1
        "read_handler_call_SA:"
        //  "call   0x08000000;"    // l'indirizzo di memoria sarà modificato
            "movl   $0x08000000, %eax;" // l'indirizzo di memoria sarà modificato
            "call   *%eax;"
            "addl   $0x8, %esp;"    // remove the parameters from stack
            //****** pop register eax, ecx, edx? *************************************
            "pop    %edx;"
            "pop    %ecx;"
        //  "pop    %eax;"
    //***       "call   READ_HANDLER;"
            "popf;" // flags
            "pop    %eax;"
        "modified_jmp_SA:"
            "jmp    else_end_label_SA;"
        "if_end_label_SA:"
            "popf;" // flags
            "pop    %eax;"
    );
/*  __asm__ __volatile__ ( 
            "movl   mem, REG;" // la mov originale
    );*//*
    __asm__ __volatile__ ( 
        "else_end_label_SA:"
    );
    
    return;
}*/


/* ----------------------------------------------------------------------------
 *  _rm_patch_MOV_SA
 * ----------------------------------------------------------------------------
*/
void _rm_patch_MOV_SA(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case MOV_SA (MOV, read, memory is source, eax NOT in use) *****\n");
#endif
    
    PATCH(MOV_SA, read, eax)
    
    
    // Codice patch da inserire
    PATCH_CODE_1("MOV_SA", "eax")
    __asm__ __volatile__ ( 
        "modified_mov_MOV_SA:"
            "movl   -"rm_OFFSET_STR"(%eax), %eax;" // il reg sarà modificato; usiamo eax perchè ha codice 000
    );
    PATCH_CODE_2("MOV_SA", "eax", "ecx")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_MOV_SA
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE(MOV_SA, read, eax)


/* ----------------------------------------------------------------------------
 *  _rm_patch_MOV_SC
 * ----------------------------------------------------------------------------
*/
void _rm_patch_MOV_SC(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case MOV_SC (MOV, read, memory is source, eax in use) *****\n");
#endif
    
    PATCH(MOV_SC, read, ecx)
    
    
    // Codice patch da inserire
    PATCH_CODE_1("MOV_SC", "ecx")
    __asm__ __volatile__ ( 
        "modified_mov_MOV_SC:"
            "movl   -"rm_OFFSET_STR"(%ecx), %eax;" // il reg sarà modificato; usiamo eax perchè ha codice 000
    );
    PATCH_CODE_2("MOV_SC", "ecx", "eax")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_MOV_SC
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE(MOV_SC, read, ecx)


/* ----------------------------------------------------------------------------
 *  _rm_patch_MOV_DA
 * ----------------------------------------------------------------------------
*/
void _rm_patch_MOV_DA(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case MOV_DA (MOV, write, memory is destination, eax NOT in use) *****\n");
#endif
    
    PATCH(MOV_DA, write, eax)
    
    
    // Codice patch da inserire
    PATCH_CODE_1("MOV_DA", "eax")
    __asm__ __volatile__ ( 
        "modified_mov_MOV_DA:"
            "movl   %eax, -"rm_OFFSET_STR"(%eax);" // il reg sarà modificato; usiamo eax perchè ha codice 000
    );
    PATCH_CODE_2("MOV_DA", "eax", "ecx")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_MOV_DA
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE(MOV_DA, write, eax)


/* ----------------------------------------------------------------------------
 *  _rm_patch_MOV_DC
 * ----------------------------------------------------------------------------
*/
void _rm_patch_MOV_DC(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case MOV_DC (MOV, write, memory is destination, eax in use) *****\n");
#endif
    
    PATCH(MOV_DC, write, ecx)
    
    
    // Codice patch da inserire
    PATCH_CODE_1("MOV_DC", "ecx")
    __asm__ __volatile__ ( 
        "modified_mov_MOV_DC:"
            "movl   %eax, -"rm_OFFSET_STR"(%ecx);" // il reg sarà modificato; usiamo eax perchè ha codice 000
    );
    PATCH_CODE_2("MOV_DC", "ecx", "eax")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_MOV_DC
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE(MOV_DC, write, ecx)


/* ----------------------------------------------------------------------------
 *  _rm_patch_MOV_DI
 * ----------------------------------------------------------------------------
*/
void _rm_patch_MOV_DI(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case MOV_DI (MOV, write, memory is destination, immediate data) *****\n");
#endif
    
    PATCH(MOV_DI, write, eax)
    
    
    // Codice patch da inserire
    PATCH_CODE_1("MOV_DI", "eax")
    __asm__ __volatile__ ( 
        "modified_mov_MOV_DI:"
            "movl   $0x08000000, -"rm_OFFSET_STR"(%eax);" // l'immediate data sarà modificato (bisogna inserire l'immediate data che era presente nella mov originale)
    );
    PATCH_CODE_2("MOV_DI", "eax", "ecx")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_MOV_DI
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE(MOV_DI, write, eax)


/* ----------------------------------------------------------------------------
 *  _rm_patch_CMP_SA
 * ----------------------------------------------------------------------------
*/
void _rm_patch_CMP_SA(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case CMP_SA (CMP, read, memory is source, eax NOT in use) *****\n");
#endif
    
    PATCH(CMP_SA, read, eax)
    
    
    // Codice patch da inserire
    PATCH_CODE_1("CMP_SA", "eax")
    __asm__ __volatile__ ( 
            "popf;"
        "modified_mov_CMP_SA:"
            "cmpl   -"rm_OFFSET_STR"(%eax), %eax;" // il reg sarà modificato; usiamo eax perchè ha codice 000
            "pushf;"
    );
    PATCH_CODE_2("CMP_SA", "eax", "ecx")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_CMP_SA
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE(CMP_SA, read, eax)


/* ----------------------------------------------------------------------------
 *  _rm_patch_CMP_SC
 * ----------------------------------------------------------------------------
*/
void _rm_patch_CMP_SC(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case CMP_SC (CMP, read, memory is source, eax in use) *****\n");
#endif
    
    PATCH(CMP_SC, read, ecx)
    
    
    // Codice patch da inserire
    PATCH_CODE_1("CMP_SC", "ecx")
    __asm__ __volatile__ ( 
            "popf;"
        "modified_mov_CMP_SC:"
            "cmpl   -"rm_OFFSET_STR"(%ecx), %eax;" // il reg sarà modificato; usiamo eax perchè ha codice 000
            "pushf;"
    );
    PATCH_CODE_2("CMP_SC", "ecx", "eax")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_CMP_SC
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE(CMP_SC, read, ecx)


/* ----------------------------------------------------------------------------
 *  _rm_patch_CMP_DA
 * ----------------------------------------------------------------------------
*/
void _rm_patch_CMP_DA(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case CMP_DA (CMP, read, memory is destination, eax NOT in use) *****\n");
#endif
    
    PATCH(CMP_DA, read, eax)
    
    
    // Codice patch da inserire
    PATCH_CODE_1("CMP_DA", "eax")
    __asm__ __volatile__ ( 
            "popf;"
        "modified_mov_CMP_DA:"
            "cmpl   %eax, -"rm_OFFSET_STR"(%eax);" // il reg sarà modificato; usiamo eax perchè ha codice 000
            "pushf;"
    );
    PATCH_CODE_2("CMP_DA", "eax", "ecx")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_CMP_DA
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE(CMP_DA, read, eax)


/* ----------------------------------------------------------------------------
 *  _rm_patch_CMP_DC
 * ----------------------------------------------------------------------------
*/
void _rm_patch_CMP_DC(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case CMP_DC (CMP, read, memory is destination, eax in use) *****\n");
#endif
    
    PATCH(CMP_DC, read, ecx)
    
    
    // Codice patch da inserire
    PATCH_CODE_1("CMP_DC", "ecx")
    __asm__ __volatile__ ( 
            "popf;"
        "modified_mov_CMP_DC:"
            "cmpl   %eax, -"rm_OFFSET_STR"(%ecx);" // il reg sarà modificato; usiamo eax perchè ha codice 000
            "pushf;"
    );
    PATCH_CODE_2("CMP_DC", "ecx", "eax")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_CMP_DC
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE(CMP_DC, read, ecx)


/* ----------------------------------------------------------------------------
 *  _rm_patch_CMP_DI
 * ----------------------------------------------------------------------------
*/
void _rm_patch_CMP_DI(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case CMP_DI (CMP, read, memory is destination, immediate data) *****\n");
#endif
    
    PATCH(CMP_DI, read, eax)
    if(instr->immediate_data_size == 1) {   //****** TO DO: gestire casi con imm_data da 2 byte ******** */
        // inserisco dei nop per completare i 4 byte preallocati per l'immediate_data
        memset(dest_addr + the_lea_opcode_size + (&modified_mov_CMP_DI - &patch_code_start_CMP_DI) + 6 + instr->immediate_data_size, 0x90, 4 - instr->immediate_data_size); /* NOP = 0x90 */    /*** ipotizzo offset da 4 byte ***/
        // modifico l'opcode dell'istruzione modificata in modo che legga il giusto numero di byte per l'immediate_data
        memcpy(dest_addr + the_lea_opcode_size + (&modified_mov_CMP_DI - &patch_code_start_CMP_DI), instr->address, 1);
    }
    
    
    // Codice patch da inserire
    PATCH_CODE_1("CMP_DI", "eax")
    __asm__ __volatile__ ( 
            "popf;"
        "modified_mov_CMP_DI:"
            "cmpl   $0x08000000, -"rm_OFFSET_STR"(%eax);" // l'immediate data sarà modificato (bisogna inserire l'immediate data che era presente nella mov originale)
            "pushf;"
    );
    PATCH_CODE_2("CMP_DI", "eax", "ecx")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_CMP_DI
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE(CMP_DI, read, eax)


/* ----------------------------------------------------------------------------
 *  _rm_patch_ADD_SA
 * ----------------------------------------------------------------------------
*/
void _rm_patch_ADD_SA(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case ADD_SA (ADD, read, memory is source, eax NOT in use) *****\n");
#endif
    
    PATCH(ADD_SA, read, eax)
    
    
    // Codice patch da inserire
    PATCH_CODE_1("ADD_SA", "eax")
    __asm__ __volatile__ ( 
            "popf;"
        "modified_mov_ADD_SA:"
            "addl   -"rm_OFFSET_STR"(%eax), %eax;" // il reg sarà modificato; usiamo eax perchè ha codice 000
            "pushf;"
    );
    PATCH_CODE_2("ADD_SA", "eax", "ecx")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_ADD_SA
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE(ADD_SA, read, eax)


/* ----------------------------------------------------------------------------
 *  _rm_patch_ADD_SC
 * ----------------------------------------------------------------------------
*/
void _rm_patch_ADD_SC(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case ADD_SC (ADD, read, memory is source, eax in use) *****\n");
#endif
    
    PATCH(ADD_SC, read, ecx)
    
    
    // Codice patch da inserire
    PATCH_CODE_1("ADD_SC", "ecx")
    __asm__ __volatile__ ( 
            "popf;"
        "modified_mov_ADD_SC:"
            "addl   -"rm_OFFSET_STR"(%ecx), %eax;" // il reg sarà modificato; usiamo eax perchè ha codice 000
            "pushf;"
    );
    PATCH_CODE_2("ADD_SC", "ecx", "eax")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_ADD_SC
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE(ADD_SC, read, ecx)


/* ----------------------------------------------------------------------------
 *  _rm_patch_ADD_DA
 * ----------------------------------------------------------------------------
*/
void _rm_patch_ADD_DA(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case ADD_DA (ADD, read_write, memory is destination, eax NOT in use) *****\n");
#endif
    
    PATCH_DOUBLE_ACCESS(ADD_DA, eax)
    
    
    // Codice patch da inserire
    PATCH_CODE_1_DOUBLE_ACCESS("ADD_DA", "eax")
    __asm__ __volatile__ ( 
            "popf;"
        "modified_mov_ADD_DA:"
            "addl   %eax, -"rm_OFFSET_STR"(%eax);" // il reg sarà modificato; usiamo eax perchè ha codice 000
            "pushf;"
    );
    PATCH_CODE_2_DOUBLE_ACCESS("ADD_DA", "eax", "ecx")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_ADD_DA
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE_DOUBLE_ACCESS(ADD_DA, eax)


/* ----------------------------------------------------------------------------
 *  _rm_patch_ADD_DC
 * ----------------------------------------------------------------------------
*/
void _rm_patch_ADD_DC(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case ADD_DC (ADD, read_write, memory is destination, eax in use) *****\n");
#endif
    
    PATCH_DOUBLE_ACCESS(ADD_DC, ecx)
    
    
    // Codice patch da inserire
    PATCH_CODE_1_DOUBLE_ACCESS("ADD_DC", "ecx")
    __asm__ __volatile__ ( 
            "popf;"
        "modified_mov_ADD_DC:"
            "addl   %eax, -"rm_OFFSET_STR"(%ecx);" // il reg sarà modificato; usiamo eax perchè ha codice 000
            "pushf;"
    );
    PATCH_CODE_2_DOUBLE_ACCESS("ADD_DC", "ecx", "eax")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_ADD_DC
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE_DOUBLE_ACCESS(ADD_DC, ecx)


/* ----------------------------------------------------------------------------
 *  _rm_patch_ADD_DI
 * ----------------------------------------------------------------------------
*/
void _rm_patch_ADD_DI(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case ADD_DI (ADD, read_write, memory is destination, immediate data) *****\n");
#endif
    
    PATCH_DOUBLE_ACCESS(ADD_DI, eax)
    if(instr->immediate_data_size == 1) {   //****** TO DO: gestire casi con imm_data da 2 byte ******** */
        // inserisco dei nop per completare i 4 byte preallocati per l'immediate_data
        memset(dest_addr + the_lea_opcode_size + (&modified_mov_ADD_DI - &patch_code_start_ADD_DI) + 6 + instr->immediate_data_size, 0x90, 4 - instr->immediate_data_size); /* NOP = 0x90 */    /*** ipotizzo offset da 4 byte ***/
        // modifico l'opcode dell'istruzione modificata in modo che legga il giusto numero di byte per l'immediate_data
        memcpy(dest_addr + the_lea_opcode_size + (&modified_mov_ADD_DI - &patch_code_start_ADD_DI), instr->address, 1);
    }
    
    
    // Codice patch da inserire
    PATCH_CODE_1_DOUBLE_ACCESS("ADD_DI", "eax")
    __asm__ __volatile__ ( 
            "popf;"
        "modified_mov_ADD_DI:"
            "addl   $0x08000000, -"rm_OFFSET_STR"(%eax);" // l'immediate data sarà modificato (bisogna inserire l'immediate data che era presente nella mov originale)
            "pushf;"
    );
    PATCH_CODE_2_DOUBLE_ACCESS("ADD_DI", "eax", "ecx")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_ADD_DI
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE_DOUBLE_ACCESS(ADD_DI, eax)


/* ----------------------------------------------------------------------------
 *  _rm_patch_SUB_SA
 * ----------------------------------------------------------------------------
*/
void _rm_patch_SUB_SA(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case SUB_SA (SUB, read, memory is source, eax NOT in use) *****\n");
#endif
    
    PATCH(SUB_SA, read, eax)
    
    
    // Codice patch da inserire
    PATCH_CODE_1("SUB_SA", "eax")
    __asm__ __volatile__ ( 
            "popf;"
        "modified_mov_SUB_SA:"
            "subl   -"rm_OFFSET_STR"(%eax), %eax;" // il reg sarà modificato; usiamo eax perchè ha codice 000
            "pushf;"
    );
    PATCH_CODE_2("SUB_SA", "eax", "ecx")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_SUB_SA
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE(SUB_SA, read, eax)


/* ----------------------------------------------------------------------------
 *  _rm_patch_SUB_SC
 * ----------------------------------------------------------------------------
*/
void _rm_patch_SUB_SC(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case SUB_SC (SUB, read, memory is source, eax in use) *****\n");
#endif
    
    PATCH(SUB_SC, read, ecx)
    
    
    // Codice patch da inserire
    PATCH_CODE_1("SUB_SC", "ecx")
    __asm__ __volatile__ ( 
            "popf;"
        "modified_mov_SUB_SC:"
            "subl   -"rm_OFFSET_STR"(%ecx), %eax;" // il reg sarà modificato; usiamo eax perchè ha codice 000
            "pushf;"
    );
    PATCH_CODE_2("SUB_SC", "ecx", "eax")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_SUB_SC
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE(SUB_SC, read, ecx)


/* ----------------------------------------------------------------------------
 *  _rm_patch_SUB_DA
 * ----------------------------------------------------------------------------
*/
void _rm_patch_SUB_DA(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case SUB_DA (SUB, read_write, memory is destination, eax NOT in use) *****\n");
#endif
    
    PATCH_DOUBLE_ACCESS(SUB_DA, eax)
    
    
    // Codice patch da inserire
    PATCH_CODE_1_DOUBLE_ACCESS("SUB_DA", "eax")
    __asm__ __volatile__ ( 
            "popf;"
        "modified_mov_SUB_DA:"
            "subl   %eax, -"rm_OFFSET_STR"(%eax);" // il reg sarà modificato; usiamo eax perchè ha codice 000
            "pushf;"
    );
    PATCH_CODE_2_DOUBLE_ACCESS("SUB_DA", "eax", "ecx")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_SUB_DA
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE_DOUBLE_ACCESS(SUB_DA, eax)


/* ----------------------------------------------------------------------------
 *  _rm_patch_SUB_DC
 * ----------------------------------------------------------------------------
*/
void _rm_patch_SUB_DC(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case SUB_DC (SUB, read_write, memory is destination, eax in use) *****\n");
#endif
    
    PATCH_DOUBLE_ACCESS(SUB_DC, ecx)
    
    
    // Codice patch da inserire
    PATCH_CODE_1_DOUBLE_ACCESS("SUB_DC", "ecx")
    __asm__ __volatile__ ( 
            "popf;"
        "modified_mov_SUB_DC:"
            "subl   %eax, -"rm_OFFSET_STR"(%ecx);" // il reg sarà modificato; usiamo eax perchè ha codice 000
            "pushf;"
    );
    PATCH_CODE_2_DOUBLE_ACCESS("SUB_DC", "ecx", "eax")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_SUB_DC
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE_DOUBLE_ACCESS(SUB_DC, ecx)


/* ----------------------------------------------------------------------------
 *  _rm_patch_SUB_DI
 * ----------------------------------------------------------------------------
*/
void _rm_patch_SUB_DI(mov_rec* instr, void* dest_addr) {
    
#if PATCH_DEBUG == 1
    printf("***** case SUB_DI (SUB, read_write, memory is destination, immediate data) *****\n");
#endif
    
    PATCH_DOUBLE_ACCESS(SUB_DI, eax)
    if(instr->immediate_data_size == 1) {   //****** TO DO: gestire casi con imm_data da 2 byte ******** */
        // inserisco dei nop per completare i 4 byte preallocati per l'immediate_data
        memset(dest_addr + the_lea_opcode_size + (&modified_mov_SUB_DI - &patch_code_start_SUB_DI) + 6 + instr->immediate_data_size, 0x90, 4 - instr->immediate_data_size); /* NOP = 0x90 */    /*** ipotizzo offset da 4 byte ***/
        // modifico l'opcode dell'istruzione modificata in modo che legga il giusto numero di byte per l'immediate_data
        memcpy(dest_addr + the_lea_opcode_size + (&modified_mov_SUB_DI - &patch_code_start_SUB_DI), instr->address, 1);
    }
    
    
    // Codice patch da inserire
    PATCH_CODE_1_DOUBLE_ACCESS("SUB_DI", "eax")
    __asm__ __volatile__ ( 
            "popf;"
        "modified_mov_SUB_DI:"
            "subl   $0x08000000, -"rm_OFFSET_STR"(%eax);" // l'immediate data sarà modificato (bisogna inserire l'immediate data che era presente nella mov originale)
            "pushf;"
    );
    PATCH_CODE_2_DOUBLE_ACCESS("SUB_DI", "eax", "ecx")
    
    return;
}


/* ----------------------------------------------------------------------------
 *  _rm_patch_size_SUB_DI
 * ----------------------------------------------------------------------------
*/
PATCH_SIZE_DOUBLE_ACCESS(SUB_DI, eax)





/* ----------------------------------------------------------------------------
 *  _rm_decode
 * ----------------------------------------------------------------------------
*/
#if rm_DISASSEMBLER == rm_LIBDISASM
int _rm_decode(x86_insn_t insn, void* addr, mov_rec* out_instr) {
        x86_op_t *dest, *src;
    
    /*if instr without prefixes...*/
    if (insn.prefix==0 && insn.explicit_count==2) {
        
        /*retrieves operands...*/
        dest = x86_get_dest_operand(&insn);
        src = x86_get_src_operand(&insn);
        
        /*if instr is a move...*/
        if (insn.type==insn_mov) {  // istruzione mov (anche lea, lahf, ...)
            
            // skip bad instructions (lea)
            if (*(unsigned char*)addr == 0x8d) return 0;
            
            /*if src is address and dest is not...*/ //if mov reg, mem...
            if ((src->type >= 5 && src->type <= 7) && x86_operand_size(src)==4 && dest->type==1) {
                out_instr->address = addr;
                out_instr->address_end = addr + insn.size;
                
                //if mov eax, mem...
                if (strcmp(dest->data.reg.name, "eax") == 0) {
                    out_instr->eax_is_in_use = TRUE;
                    out_instr->patch_func = _rm_patch_MOV_SC;
                    out_instr->patch_size_func = _rm_patch_size_MOV_SC;
                }
                //else
                else {
                    out_instr->eax_is_in_use = FALSE;
                    out_instr->patch_func = _rm_patch_MOV_SA;
                    out_instr->patch_size_func = _rm_patch_size_MOV_SA;
                }
                
                out_instr->write = FALSE;
                out_instr->immediate_data_size = 0;
                
                return 1;
            }
            
            /*if dest is address and src is not...*/ //if mov mem, reg...
            else if ((dest->type >= 5 && dest->type <= 7) && x86_operand_size(dest)==4 && src->type==1) {
                out_instr->address = addr;
                out_instr->address_end = addr + insn.size;
                
                //if mov mem, eax...
                if (strcmp(src->data.reg.name, "eax") == 0) {
                    out_instr->eax_is_in_use = TRUE;
                    out_instr->patch_func = _rm_patch_MOV_DC;
                    out_instr->patch_size_func = _rm_patch_size_MOV_DC;
                }
                //else
                else {
                    out_instr->eax_is_in_use = FALSE;
                    out_instr->patch_func = _rm_patch_MOV_DA;
                    out_instr->patch_size_func = _rm_patch_size_MOV_DA;
                }
                
                out_instr->write = TRUE;
                out_instr->immediate_data_size = 0;
                
                return 1;
            }
            
            /*if dest is address and src is not...*/ //if mov mem, #const...
            else if ((dest->type >= 5 && dest->type <= 7) && x86_operand_size(dest)==4 && src->type==2) {
                out_instr->address = addr;
                out_instr->address_end = addr + insn.size;
                
                out_instr->eax_is_in_use = FALSE;
                out_instr->patch_func = _rm_patch_MOV_DI;
                out_instr->patch_size_func = _rm_patch_size_MOV_DI;
                
                out_instr->write = TRUE;
                
                if(src->datatype == op_byte)
                    out_instr->immediate_data_size = 1;
                else if(src->datatype == op_word)
                    out_instr->immediate_data_size = 2;
                else// if(src->datatype == op_dword)
                    out_instr->immediate_data_size = 4;
                
                return 1;
            }
        }
        
        /*if instr is a compare...*/
        else if (insn.type==insn_cmp) {
            
            /*if src is address and dest is not...*/ //if cmp reg, mem...
            if ((src->type >= 5 && src->type <= 7) && x86_operand_size(src)==4 && dest->type==1) {
                out_instr->address = addr;
                out_instr->address_end = addr + insn.size;
                
                //if cmp eax, mem...
                if (strcmp(dest->data.reg.name, "eax") == 0) {
                    out_instr->eax_is_in_use = TRUE;
                    out_instr->patch_func = _rm_patch_CMP_SC;
                    out_instr->patch_size_func = _rm_patch_size_CMP_SC;
                }
                //else
                else {
                    out_instr->eax_is_in_use = FALSE;
                    out_instr->patch_func = _rm_patch_CMP_SA;
                    out_instr->patch_size_func = _rm_patch_size_CMP_SA;
                }
                
                out_instr->write = FALSE;
                out_instr->immediate_data_size = 0;
                
                return 1;
            }
            
            /*if dest is address and src is not...*/ //if cmp mem, reg...
            else if ((dest->type >= 5 && dest->type <= 7) && x86_operand_size(dest)==4 && src->type==1) {
                out_instr->address = addr;
                out_instr->address_end = addr + insn.size;
                
                //if cmp mem, eax...
                if (strcmp(src->data.reg.name, "eax") == 0) {
                    out_instr->eax_is_in_use = TRUE;
                    out_instr->patch_func = _rm_patch_CMP_DC;
                    out_instr->patch_size_func = _rm_patch_size_CMP_DC;
                }
                //else
                else {
                    out_instr->eax_is_in_use = FALSE;
                    out_instr->patch_func = _rm_patch_CMP_DA;
                    out_instr->patch_size_func = _rm_patch_size_CMP_DA;
                }
                
                out_instr->write = FALSE;
                out_instr->immediate_data_size = 0;
                
                return 1;
            }
            
            /*if dest is address and src is not...*/ //if cmp mem, #const...
            else if ((dest->type >= 5 && dest->type <= 7) && x86_operand_size(dest)==4 && src->type==2) {
                out_instr->address = addr;
                out_instr->address_end = addr + insn.size;
                
                out_instr->eax_is_in_use = FALSE;
                out_instr->patch_func = _rm_patch_CMP_DI;
                out_instr->patch_size_func = _rm_patch_size_CMP_DI;
                
                out_instr->write = FALSE;
                
                if(src->datatype == op_byte)
                    out_instr->immediate_data_size = 1;
                else if(src->datatype == op_word)
                    out_instr->immediate_data_size = 2;
                else// if(src->datatype == op_dword)
                    out_instr->immediate_data_size = 4;
                
                return 1;
            }
        }
        
        /*if instr is an add...*/
        else if (insn.type==insn_add) {
            
            /*if src is address and dest is not...*/ //if add reg, mem...
            if ((src->type >= 5 && src->type <= 7) && x86_operand_size(src)==4 && dest->type==1) {
                out_instr->address = addr;
                out_instr->address_end = addr + insn.size;
                
                //if add eax, mem...
                if (strcmp(dest->data.reg.name, "eax") == 0) {
                    out_instr->eax_is_in_use = TRUE;
                    out_instr->patch_func = _rm_patch_ADD_SC;
                    out_instr->patch_size_func = _rm_patch_size_ADD_SC;
                }
                //else
                else {
                    out_instr->eax_is_in_use = FALSE;
                    out_instr->patch_func = _rm_patch_ADD_SA;
                    out_instr->patch_size_func = _rm_patch_size_ADD_SA;
                }
                
                out_instr->write = FALSE;
                out_instr->immediate_data_size = 0;
                
                return 1;
            }
            
            /*if dest is address and src is not...*/ //if add mem, reg...
            else if ((dest->type >= 5 && dest->type <= 7) && x86_operand_size(dest)==4 && src->type==1) {
                out_instr->address = addr;
                out_instr->address_end = addr + insn.size;
                
                //if add mem, eax...
                if (strcmp(src->data.reg.name, "eax") == 0) {
                    out_instr->eax_is_in_use = TRUE;
                    out_instr->patch_func = _rm_patch_ADD_DC;
                    out_instr->patch_size_func = _rm_patch_size_ADD_DC;
                }
                //else
                else {
                    out_instr->eax_is_in_use = FALSE;
                    out_instr->patch_func = _rm_patch_ADD_DA;
                    out_instr->patch_size_func = _rm_patch_size_ADD_DA;
                }
                
                out_instr->write = TRUE;
                out_instr->immediate_data_size = 0;
                
                return 1;
            }
            
            /*if dest is address and src is not...*/ //if add mem, #const...
            else if ((dest->type >= 5 && dest->type <= 7) && x86_operand_size(dest)==4 && src->type==2) {
                out_instr->address = addr;
                out_instr->address_end = addr + insn.size;
                
                out_instr->eax_is_in_use = FALSE;
                out_instr->patch_func = _rm_patch_ADD_DI;
                out_instr->patch_size_func = _rm_patch_size_ADD_DI;
                
                out_instr->write = TRUE;
                
                if(src->datatype == op_byte)
                    out_instr->immediate_data_size = 1;
                else if(src->datatype == op_word)
                    out_instr->immediate_data_size = 2;
                else// if(src->datatype == op_dword)
                    out_instr->immediate_data_size = 4;
                
                return 1;
            }
        }
        
        /*if instr is a sub...*/
        else if (insn.type==insn_sub) {
            
            /*if src is address and dest is not...*/ //if sub reg, mem...
            if ((src->type >= 5 && src->type <= 7) && x86_operand_size(src)==4 && dest->type==1) {
                out_instr->address = addr;
                out_instr->address_end = addr + insn.size;
                
                //if sub eax, mem...
                if (strcmp(dest->data.reg.name, "eax") == 0) {
                    out_instr->eax_is_in_use = TRUE;
                    out_instr->patch_func = _rm_patch_SUB_SC;
                    out_instr->patch_size_func = _rm_patch_size_SUB_SC;
                }
                //else
                else {
                    out_instr->eax_is_in_use = FALSE;
                    out_instr->patch_func = _rm_patch_SUB_SA;
                    out_instr->patch_size_func = _rm_patch_size_SUB_SA;
                }
                
                out_instr->write = FALSE;
                out_instr->immediate_data_size = 0;
                
                return 1;
            }
            
            /*if dest is address and src is not...*/ //if sub mem, reg...
            else if ((dest->type >= 5 && dest->type <= 7) && x86_operand_size(dest)==4 && src->type==1) {
                out_instr->address = addr;
                out_instr->address_end = addr + insn.size;
                
                //if sub mem, eax...
                if (strcmp(src->data.reg.name, "eax") == 0) {
                    out_instr->eax_is_in_use = TRUE;
                    out_instr->patch_func = _rm_patch_SUB_DC;
                    out_instr->patch_size_func = _rm_patch_size_SUB_DC;
                }
                //else
                else {
                    out_instr->eax_is_in_use = FALSE;
                    out_instr->patch_func = _rm_patch_SUB_DA;
                    out_instr->patch_size_func = _rm_patch_size_SUB_DA;
                }
                
                out_instr->write = TRUE;
                out_instr->immediate_data_size = 0;
                
                return 1;
            }
            
            /*if dest is address and src is not...*/ //if sub mem, #const...
            else if ((dest->type >= 5 && dest->type <= 7) && x86_operand_size(dest)==4 && src->type==2) {
                out_instr->address = addr;
                out_instr->address_end = addr + insn.size;
                
                out_instr->eax_is_in_use = FALSE;
                out_instr->patch_func = _rm_patch_SUB_DI;
                out_instr->patch_size_func = _rm_patch_size_SUB_DI;
                
                out_instr->write = TRUE;
                
                if(src->datatype == op_byte)
                    out_instr->immediate_data_size = 1;
                else if(src->datatype == op_word)
                    out_instr->immediate_data_size = 2;
                else// if(src->datatype == op_dword)
                    out_instr->immediate_data_size = 4;
                
                return 1;
            }
        }
    }
    
    return 0;
}

#else

int _rm_decode(_DInst insn, mov_rec* out_instr) {
        _Operand *dest, *src;
    
    /*retrieves operands count...*/
    int i;
    int operand_count = 0;
    for (i = 0; i < OPERANDS_NO; i++) {
        if (insn.ops[i].type != O_NONE) {
            operand_count++;
        }
    }
    
    /*if instr without prefixes...*/
    if (FLAG_GET_PREFIX(insn.flags)==0 && operand_count==2) {
        
        /*retrieves operands...*/
        dest = &insn.ops[0];
        src = &insn.ops[1];
        
        /*if instr is a move...*/
        if (insn.opcode==I_MOV) {   // istruzione mov
            
            /*if src is address and dest is not...*/ //if mov reg, mem...
            if ((src->type == O_DISP || src->type == O_SMEM || src->type == O_MEM) && src->size==32 && dest->type==O_REG) {
                out_instr->address = (void*)insn.addr;
                out_instr->address_end = (void*)insn.addr + insn.size;
                
                //if mov eax, mem...
                if (dest->index == R_EAX) {
                    out_instr->eax_is_in_use = TRUE;
                    out_instr->patch_func = _rm_patch_MOV_SC;
                    out_instr->patch_size_func = _rm_patch_size_MOV_SC;
                }
                //else
                else {
                    out_instr->eax_is_in_use = FALSE;
                    out_instr->patch_func = _rm_patch_MOV_SA;
                    out_instr->patch_size_func = _rm_patch_size_MOV_SA;
                }
                
                out_instr->write = FALSE;
                out_instr->immediate_data_size = 0;
                
                return 1;
            }
            
            /*if dest is address and src is not...*/ //if mov mem, reg...
            else if ((dest->type == O_DISP || dest->type == O_SMEM || dest->type == O_MEM) && dest->size==32 && src->type==O_REG) {
                out_instr->address = (void*)insn.addr;
                out_instr->address_end = (void*)insn.addr + insn.size;
                
                //if mov mem, eax...
                if (src->index == R_EAX) {
                    out_instr->eax_is_in_use = TRUE;
                    out_instr->patch_func = _rm_patch_MOV_DC;
                    out_instr->patch_size_func = _rm_patch_size_MOV_DC;
                }
                //else
                else {
                    out_instr->eax_is_in_use = FALSE;
                    out_instr->patch_func = _rm_patch_MOV_DA;
                    out_instr->patch_size_func = _rm_patch_size_MOV_DA;
                }
                
                out_instr->write = TRUE;
                out_instr->immediate_data_size = 0;
                
                return 1;
            }
            
            /*if dest is address and src is not...*/ //if mov mem, #const...
            else if ((dest->type == O_DISP || dest->type == O_SMEM || dest->type == O_MEM) && dest->size==32 && src->type==O_IMM) {
                out_instr->address = (void*)insn.addr;
                out_instr->address_end = (void*)insn.addr + insn.size;
                
                out_instr->eax_is_in_use = FALSE;
                out_instr->patch_func = _rm_patch_MOV_DI;
                out_instr->patch_size_func = _rm_patch_size_MOV_DI;
                
                out_instr->write = TRUE;
                
                if(src->size == 8)
                    out_instr->immediate_data_size = 1;
                else if(src->size == 16)
                    out_instr->immediate_data_size = 2;
                else// if(src->size == 32)
                    out_instr->immediate_data_size = 4;
                
                return 1;
            }
        }
        
        /*if instr is a compare...*/
        else if (insn.opcode==I_CMP) {
            
            /*if src is address and dest is not...*/ //if cmp reg, mem...
            if ((src->type == O_DISP || src->type == O_SMEM || src->type == O_MEM) && src->size==32 && dest->type==O_REG) {
                out_instr->address = (void*)insn.addr;
                out_instr->address_end = (void*)insn.addr + insn.size;
                
                //if cmp eax, mem...
                if (dest->index == R_EAX) {
                    out_instr->eax_is_in_use = TRUE;
                    out_instr->patch_func = _rm_patch_CMP_SC;
                    out_instr->patch_size_func = _rm_patch_size_CMP_SC;
                }
                //else
                else {
                    out_instr->eax_is_in_use = FALSE;
                    out_instr->patch_func = _rm_patch_CMP_SA;
                    out_instr->patch_size_func = _rm_patch_size_CMP_SA;
                }
                
                out_instr->write = FALSE;
                out_instr->immediate_data_size = 0;
                
                return 1;
            }
            
            /*if dest is address and src is not...*/ //if cmp mem, reg...
            else if ((dest->type == O_DISP || dest->type == O_SMEM || dest->type == O_MEM) && dest->size==32 && src->type==O_REG) {
                out_instr->address = (void*)insn.addr;
                out_instr->address_end = (void*)insn.addr + insn.size;
                
                //if cmp mem, eax...
                if (src->index == R_EAX) {
                    out_instr->eax_is_in_use = TRUE;
                    out_instr->patch_func = _rm_patch_CMP_DC;
                    out_instr->patch_size_func = _rm_patch_size_CMP_DC;
                }
                //else
                else {
                    out_instr->eax_is_in_use = FALSE;
                    out_instr->patch_func = _rm_patch_CMP_DA;
                    out_instr->patch_size_func = _rm_patch_size_CMP_DA;
                }
                
                out_instr->write = FALSE;
                out_instr->immediate_data_size = 0;
                
                return 1;
            }
            
            /*if dest is address and src is not...*/ //if cmp mem, #const...
            else if ((dest->type == O_DISP || dest->type == O_SMEM || dest->type == O_MEM) && dest->size==32 && src->type==O_IMM) {
                out_instr->address = (void*)insn.addr;
                out_instr->address_end = (void*)insn.addr + insn.size;
                
                out_instr->eax_is_in_use = FALSE;
                out_instr->patch_func = _rm_patch_CMP_DI;
                out_instr->patch_size_func = _rm_patch_size_CMP_DI;
                
                out_instr->write = FALSE;
                
                if(src->size == 8)
                    out_instr->immediate_data_size = 1;
                else if(src->size == 16)
                    out_instr->immediate_data_size = 2;
                else// if(src->size == 32)
                    out_instr->immediate_data_size = 4;
                
                return 1;
            }
        }
        
        /*if instr is an add...*/
        else if (insn.opcode==I_ADD) {
            
            /*if src is address and dest is not...*/ //if add reg, mem...
            if ((src->type == O_DISP || src->type == O_SMEM || src->type == O_MEM) && src->size==32 && dest->type==O_REG) {
                out_instr->address = (void*)insn.addr;
                out_instr->address_end = (void*)insn.addr + insn.size;
                
                //if add eax, mem...
                if (dest->index == R_EAX) {
                    out_instr->eax_is_in_use = TRUE;
                    out_instr->patch_func = _rm_patch_ADD_SC;
                    out_instr->patch_size_func = _rm_patch_size_ADD_SC;
                }
                //else
                else {
                    out_instr->eax_is_in_use = FALSE;
                    out_instr->patch_func = _rm_patch_ADD_SA;
                    out_instr->patch_size_func = _rm_patch_size_ADD_SA;
                }
                
                out_instr->write = FALSE;
                out_instr->immediate_data_size = 0;
                
                return 1;
            }
            
            /*if dest is address and src is not...*/ //if add mem, reg...
            else if ((dest->type == O_DISP || dest->type == O_SMEM || dest->type == O_MEM) && dest->size==32 && src->type==O_REG) {
                out_instr->address = (void*)insn.addr;
                out_instr->address_end = (void*)insn.addr + insn.size;
                
                //if add mem, eax...
                if (src->index == R_EAX) {
                    out_instr->eax_is_in_use = TRUE;
                    out_instr->patch_func = _rm_patch_ADD_DC;
                    out_instr->patch_size_func = _rm_patch_size_ADD_DC;
                }
                //else
                else {
                    out_instr->eax_is_in_use = FALSE;
                    out_instr->patch_func = _rm_patch_ADD_DA;
                    out_instr->patch_size_func = _rm_patch_size_ADD_DA;
                }
                
                out_instr->write = TRUE;
                out_instr->immediate_data_size = 0;
                
                return 1;
            }
            
            /*if dest is address and src is not...*/ //if add mem, #const...
            else if ((dest->type == O_DISP || dest->type == O_SMEM || dest->type == O_MEM) && dest->size==32 && src->type==O_IMM) {
                out_instr->address = (void*)insn.addr;
                out_instr->address_end = (void*)insn.addr + insn.size;
                
                out_instr->eax_is_in_use = FALSE;
                out_instr->patch_func = _rm_patch_ADD_DI;
                out_instr->patch_size_func = _rm_patch_size_ADD_DI;
                
                out_instr->write = TRUE;
                
                if(src->size == 8)
                    out_instr->immediate_data_size = 1;
                else if(src->size == 16)
                    out_instr->immediate_data_size = 2;
                else// if(src->size == 32)
                    out_instr->immediate_data_size = 4;
                
                return 1;
            }
        }
        
        /*if instr is a sub...*/
        else if (insn.opcode==I_SUB) {
            
            /*if src is address and dest is not...*/ //if sub reg, mem...
            if ((src->type == O_DISP || src->type == O_SMEM || src->type == O_MEM) && src->size==32 && dest->type==O_REG) {
                out_instr->address = (void*)insn.addr;
                out_instr->address_end = (void*)insn.addr + insn.size;
                
                //if sub eax, mem...
                if (dest->index == R_EAX) {
                    out_instr->eax_is_in_use = TRUE;
                    out_instr->patch_func = _rm_patch_SUB_SC;
                    out_instr->patch_size_func = _rm_patch_size_SUB_SC;
                }
                //else
                else {
                    out_instr->eax_is_in_use = FALSE;
                    out_instr->patch_func = _rm_patch_SUB_SA;
                    out_instr->patch_size_func = _rm_patch_size_SUB_SA;
                }
                
                out_instr->write = FALSE;
                out_instr->immediate_data_size = 0;
                
                return 1;
            }
            
            /*if dest is address and src is not...*/ //if sub mem, reg...
            else if ((dest->type == O_DISP || dest->type == O_SMEM || dest->type == O_MEM) && dest->size==32 && src->type==O_REG) {
                out_instr->address = (void*)insn.addr;
                out_instr->address_end = (void*)insn.addr + insn.size;
                
                //if sub mem, eax...
                if (src->index == R_EAX) {
                    out_instr->eax_is_in_use = TRUE;
                    out_instr->patch_func = _rm_patch_SUB_DC;
                    out_instr->patch_size_func = _rm_patch_size_SUB_DC;
                }
                //else
                else {
                    out_instr->eax_is_in_use = FALSE;
                    out_instr->patch_func = _rm_patch_SUB_DA;
                    out_instr->patch_size_func = _rm_patch_size_SUB_DA;
                }
                
                out_instr->write = TRUE;
                out_instr->immediate_data_size = 0;
                
                return 1;
            }
            
            /*if dest is address and src is not...*/ //if sub mem, #const...
            else if ((dest->type == O_DISP || dest->type == O_SMEM || dest->type == O_MEM) && dest->size==32 && src->type==O_IMM) {
                out_instr->address = (void*)insn.addr;
                out_instr->address_end = (void*)insn.addr + insn.size;
                
                out_instr->eax_is_in_use = FALSE;
                out_instr->patch_func = _rm_patch_SUB_DI;
                out_instr->patch_size_func = _rm_patch_size_SUB_DI;
                
                out_instr->write = TRUE;
                
                if(src->size == 8)
                    out_instr->immediate_data_size = 1;
                else if(src->size == 16)
                    out_instr->immediate_data_size = 2;
                else// if(src->size == 32)
                    out_instr->immediate_data_size = 4;
                
                return 1;
            }
        }
    }
    
    return 0;
}
#endif


/* ----------------------------------------------------------------------------
 *  _rm_is_branch
 * ----------------------------------------------------------------------------
*/
#if rm_DISASSEMBLER == rm_LIBDISASM
int _rm_is_branch(x86_insn_t insn, void* addr, branch_rec* out_branch_instr, GArray* out_jmp_array) {
    void* jmp_address;
    
    if (insn.type == insn_jmp || insn.type == insn_jcc) {   // istruzioni jmp (salto incondizionato) e jcc (salti condizionati, es. jz, jnz, jc, ...)
        if (insn.operands->op.type == op_relative_near) {   // short relativi
            jmp_address = addr + insn.size + insn.operands->op.data.relative_near;
            g_array_append_val(out_jmp_array, jmp_address);
            out_branch_instr->address = addr;
            out_branch_instr->address_end = addr + insn.size;
            out_branch_instr->jump_target = jmp_address;
            unsigned char the_opcode = *(unsigned char*)addr;
            if (the_opcode == 0xeb) {   // jmp (eb --> e9)
                out_branch_instr->size_to_add_in_patch = 3;
                out_branch_instr->new_opcode[0] = 0xe9;
                out_branch_instr->new_opcode_size = 1;
            }
            else if ((the_opcode & 0xf0) == 0x70) { // jcc (7x --> 0f8x)
                out_branch_instr->size_to_add_in_patch = 4;
                out_branch_instr->new_opcode[0] = 0x0f;
                out_branch_instr->new_opcode[1] = (the_opcode & 0x0f) | 0x80;
                out_branch_instr->new_opcode_size = 2;
            }
            else {  // probably jecxz (opcode == e3)
                jmp_address = addr;
                g_array_append_val(out_jmp_array, jmp_address);
                jmp_address = addr + insn.size;
                g_array_append_val(out_jmp_array, jmp_address);
                return 0;   // serve per evitare che out_branch_instr venga inserita come istruzione di branch rilocabile
            }
            return 1;
        }
        else if (insn.operands->op.type == op_relative_far) {   // long relativi
            jmp_address = addr + insn.size + insn.operands->op.data.relative_far;
            g_array_append_val(out_jmp_array, jmp_address);
            out_branch_instr->address = addr;
            out_branch_instr->address_end = addr + insn.size;
            out_branch_instr->jump_target = jmp_address;
            out_branch_instr->size_to_add_in_patch = 0;
            return 1;
        }
#if rm_DEBUG > 1
        else if (insn.operands->op.type == op_absolute) {
            printf("case op_absolute, TO DO\n"); //*******************************************************************
            //getchar();
        }
        else if (insn.operands->op.type == op_expression) {
            printf("case op_expression, TO DO\n"); //*******************************************************************
            //getchar();
        }
        else if (insn.operands->op.type == op_offset) {
            printf("case op_offset, TO DO\n"); //*******************************************************************
            //getchar();
        }
//else {
//printf("******************* JMP non gestito **************\n");
//getchar();
//}
#endif
    }
    
    else if (insn.type == insn_call || insn.type == insn_callcc) {  // istruzioni call (salto incondizionato) e callcc (salti condizionati)
        jmp_address = addr + insn.size; // le istruzioni ret in pratica sono salti in entrata alla fine delle istruzioni call
        g_array_append_val(out_jmp_array, jmp_address);
        if (insn.operands->op.type == op_relative_far) {    // call relativi
            out_branch_instr->address = addr;
            out_branch_instr->address_end = addr + insn.size;
            out_branch_instr->jump_target = addr + insn.size + insn.operands->op.data.relative_far;
            out_branch_instr->size_to_add_in_patch = 0;
            return 1;
        }
    }
    
    return 0;
}

#else

int _rm_is_branch(_DInst insn, branch_rec* out_branch_instr, GArray* out_jmp_array) {
    void* jmp_address;
    
    if (META_GET_FC(insn.meta) == FC_UNC_BRANCH || META_GET_FC(insn.meta) == FC_CND_BRANCH) {   // istruzioni jmp (salto incondizionato) e jcc (salti condizionati, es. jz, jnz, jc, ...)
        if (insn.ops[0].type == O_PC) { // jmp e jcc relativi
            if (insn.ops[0].size == 8) {    // short relativi
                jmp_address = (void*)INSTRUCTION_GET_TARGET(&insn);
                g_array_append_val(out_jmp_array, jmp_address);
                out_branch_instr->address = (void*)insn.addr;
                out_branch_instr->address_end = (void*)insn.addr + insn.size;
                out_branch_instr->jump_target = jmp_address;
                unsigned char the_opcode = *(unsigned char*)insn.addr;
                if (the_opcode == 0xeb) {   // jmp (eb --> e9)
                    out_branch_instr->size_to_add_in_patch = 3;
                    out_branch_instr->new_opcode[0] = 0xe9;
                    out_branch_instr->new_opcode_size = 1;
                }
                else if ((the_opcode & 0xf0) == 0x70) { // jcc (7x --> 0f8x)
                    out_branch_instr->size_to_add_in_patch = 4;
                    out_branch_instr->new_opcode[0] = 0x0f;
                    out_branch_instr->new_opcode[1] = (the_opcode & 0x0f) | 0x80;
                    out_branch_instr->new_opcode_size = 2;
                }
                else {  // probably jecxz (opcode == e3)
                    jmp_address = (void*)insn.addr;
                    g_array_append_val(out_jmp_array, jmp_address);
                    jmp_address = (void*)insn.addr + insn.size;
                    g_array_append_val(out_jmp_array, jmp_address);
                    return 0;   // serve per evitare che out_branch_instr venga inserita come istruzione di branch rilocabile
                }
                return 1;
            }
            else {  // long relativi
                jmp_address = (void*)INSTRUCTION_GET_TARGET(&insn);
                g_array_append_val(out_jmp_array, jmp_address);
                out_branch_instr->address = (void*)insn.addr;
                out_branch_instr->address_end = (void*)insn.addr + insn.size;
                out_branch_instr->jump_target = jmp_address;
                out_branch_instr->size_to_add_in_patch = 0;
                return 1;
            }
        }
#if rm_DEBUG > 1
        else if (insn.ops[0].type == O_PTR) {
            printf("case op_absolute (O_PTR), TO DO\n"); //*******************************************************************
            //getchar();
        }
        else if (insn.ops[0].type == O_SMEM || insn.ops[0].type == O_MEM) {
            printf("case op_expression (O_SMEM, O_MEM), TO DO\n"); //*******************************************************************
            //getchar();
        }
        else if (insn.ops[0].type == O_DISP) {
            printf("case op_offset (O_DISP), TO DO\n"); //*******************************************************************
            //getchar();
        }
//else {
//printf("******************* JMP non gestito **************\n");
//getchar();
//}
#endif
    }
    
    else if (META_GET_FC(insn.meta) == FC_CALL) {   // istruzioni call
        jmp_address = (void*)insn.addr + insn.size; // le istruzioni ret in pratica sono salti in entrata alla fine delle istruzioni call
        g_array_append_val(out_jmp_array, jmp_address);
        if (insn.ops[0].type == O_PC) { // call relativi
            out_branch_instr->address = (void*)insn.addr;
            out_branch_instr->address_end = (void*)insn.addr + insn.size;
            out_branch_instr->jump_target = (void*)INSTRUCTION_GET_TARGET(&insn);
            out_branch_instr->size_to_add_in_patch = 0;
            return 1;
        }
    }
    
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
