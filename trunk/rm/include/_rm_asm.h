/* ============================================================================
 *  _rm_asm.h
 * ============================================================================

 *  Author:         (c) 2010 the rm team
 *  License:        See the end of this file for license information
 *  Created:        July 21, 2010

 *  Last changed:   $Date: 2011/01/06 17:24:19 $
 *  Changed by:     $Author: alex $
 *  Revision:       $Revision: 1.11 $
*/


#ifndef ___rm_asm__
#define ___rm_asm__

#ifdef __cplusplus
extern "C" {
#endif


#ifndef PATCH_DEBUG
#define PATCH_DEBUG     1
#endif


// *** MOV labels ***
extern char patch_code_start_MOV_SA;
extern char compare_MOV_SA;
extern char compare_jmp_MOV_SA;
extern char modified_mov_MOV_SA;
extern char modified_jmp_MOV_SA;
extern char else_end_label_MOV_SA;
extern char handler_call_start_MOV_SA;
extern char handler_call_end_MOV_SA;
extern char handler_call_MOV_SA;

extern char patch_code_start_MOV_SC;
extern char compare_MOV_SC;
extern char compare_jmp_MOV_SC;
extern char modified_mov_MOV_SC;
extern char modified_jmp_MOV_SC;
extern char else_end_label_MOV_SC;
extern char handler_call_start_MOV_SC;
extern char handler_call_end_MOV_SC;
extern char handler_call_MOV_SC;

extern char patch_code_start_MOV_DA;
extern char compare_MOV_DA;
extern char compare_jmp_MOV_DA;
extern char modified_mov_MOV_DA;
extern char modified_jmp_MOV_DA;
extern char else_end_label_MOV_DA;
extern char handler_call_start_MOV_DA;
extern char handler_call_end_MOV_DA;
extern char handler_call_MOV_DA;

extern char patch_code_start_MOV_DC;
extern char compare_MOV_DC;
extern char compare_jmp_MOV_DC;
extern char modified_mov_MOV_DC;
extern char modified_jmp_MOV_DC;
extern char else_end_label_MOV_DC;
extern char handler_call_start_MOV_DC;
extern char handler_call_end_MOV_DC;
extern char handler_call_MOV_DC;

extern char patch_code_start_MOV_DI;
extern char compare_MOV_DI;
extern char compare_jmp_MOV_DI;
extern char modified_mov_MOV_DI;
extern char modified_jmp_MOV_DI;
extern char else_end_label_MOV_DI;
extern char handler_call_start_MOV_DI;
extern char handler_call_end_MOV_DI;
extern char handler_call_MOV_DI;


// *** CMP labels ***
extern char patch_code_start_CMP_SA;
extern char compare_CMP_SA;
extern char compare_jmp_CMP_SA;
extern char modified_mov_CMP_SA;
extern char modified_jmp_CMP_SA;
extern char else_end_label_CMP_SA;
extern char handler_call_start_CMP_SA;
extern char handler_call_end_CMP_SA;
extern char handler_call_CMP_SA;

extern char patch_code_start_CMP_SC;
extern char compare_CMP_SC;
extern char compare_jmp_CMP_SC;
extern char modified_mov_CMP_SC;
extern char modified_jmp_CMP_SC;
extern char else_end_label_CMP_SC;
extern char handler_call_start_CMP_SC;
extern char handler_call_end_CMP_SC;
extern char handler_call_CMP_SC;

extern char patch_code_start_CMP_DA;
extern char compare_CMP_DA;
extern char compare_jmp_CMP_DA;
extern char modified_mov_CMP_DA;
extern char modified_jmp_CMP_DA;
extern char else_end_label_CMP_DA;
extern char handler_call_start_CMP_DA;
extern char handler_call_end_CMP_DA;
extern char handler_call_CMP_DA;

extern char patch_code_start_CMP_DC;
extern char compare_CMP_DC;
extern char compare_jmp_CMP_DC;
extern char modified_mov_CMP_DC;
extern char modified_jmp_CMP_DC;
extern char else_end_label_CMP_DC;
extern char handler_call_start_CMP_DC;
extern char handler_call_end_CMP_DC;
extern char handler_call_CMP_DC;

extern char patch_code_start_CMP_DI;
extern char compare_CMP_DI;
extern char compare_jmp_CMP_DI;
extern char modified_mov_CMP_DI;
extern char modified_jmp_CMP_DI;
extern char else_end_label_CMP_DI;
extern char handler_call_start_CMP_DI;
extern char handler_call_end_CMP_DI;
extern char handler_call_CMP_DI;


// *** ADD labels ***
extern char patch_code_start_ADD_SA;
extern char compare_ADD_SA;
extern char compare_jmp_ADD_SA;
extern char modified_mov_ADD_SA;
extern char modified_jmp_ADD_SA;
extern char else_end_label_ADD_SA;
extern char handler_call_start_ADD_SA;
extern char handler_call_end_ADD_SA;
extern char handler_call_ADD_SA;

extern char patch_code_start_ADD_SC;
extern char compare_ADD_SC;
extern char compare_jmp_ADD_SC;
extern char modified_mov_ADD_SC;
extern char modified_jmp_ADD_SC;
extern char else_end_label_ADD_SC;
extern char handler_call_start_ADD_SC;
extern char handler_call_end_ADD_SC;
extern char handler_call_ADD_SC;

extern char patch_code_start_ADD_DA;
extern char compare_ADD_DA;
extern char compare_jmp_ADD_DA;
extern char modified_mov_ADD_DA;
extern char modified_jmp_ADD_DA;
extern char else_end_label_ADD_DA;
extern char handler_call_start_ADD_DA;
extern char handler_call_end_ADD_DA;
extern char read_handler_call_ADD_DA;
extern char write_handler_call_ADD_DA;

extern char patch_code_start_ADD_DC;
extern char compare_ADD_DC;
extern char compare_jmp_ADD_DC;
extern char modified_mov_ADD_DC;
extern char modified_jmp_ADD_DC;
extern char else_end_label_ADD_DC;
extern char handler_call_start_ADD_DC;
extern char handler_call_end_ADD_DC;
extern char read_handler_call_ADD_DC;
extern char write_handler_call_ADD_DC;

extern char patch_code_start_ADD_DI;
extern char compare_ADD_DI;
extern char compare_jmp_ADD_DI;
extern char modified_mov_ADD_DI;
extern char modified_jmp_ADD_DI;
extern char else_end_label_ADD_DI;
extern char handler_call_start_ADD_DI;
extern char handler_call_end_ADD_DI;
extern char read_handler_call_ADD_DI;
extern char write_handler_call_ADD_DI;


// *** SUB labels ***
extern char patch_code_start_SUB_SA;
extern char compare_SUB_SA;
extern char compare_jmp_SUB_SA;
extern char modified_mov_SUB_SA;
extern char modified_jmp_SUB_SA;
extern char else_end_label_SUB_SA;
extern char handler_call_start_SUB_SA;
extern char handler_call_end_SUB_SA;
extern char handler_call_SUB_SA;

extern char patch_code_start_SUB_SC;
extern char compare_SUB_SC;
extern char compare_jmp_SUB_SC;
extern char modified_mov_SUB_SC;
extern char modified_jmp_SUB_SC;
extern char else_end_label_SUB_SC;
extern char handler_call_start_SUB_SC;
extern char handler_call_end_SUB_SC;
extern char handler_call_SUB_SC;

extern char patch_code_start_SUB_DA;
extern char compare_SUB_DA;
extern char compare_jmp_SUB_DA;
extern char modified_mov_SUB_DA;
extern char modified_jmp_SUB_DA;
extern char else_end_label_SUB_DA;
extern char handler_call_start_SUB_DA;
extern char handler_call_end_SUB_DA;
extern char read_handler_call_SUB_DA;
extern char write_handler_call_SUB_DA;

extern char patch_code_start_SUB_DC;
extern char compare_SUB_DC;
extern char compare_jmp_SUB_DC;
extern char modified_mov_SUB_DC;
extern char modified_jmp_SUB_DC;
extern char else_end_label_SUB_DC;
extern char handler_call_start_SUB_DC;
extern char handler_call_end_SUB_DC;
extern char read_handler_call_SUB_DC;
extern char write_handler_call_SUB_DC;

extern char patch_code_start_SUB_DI;
extern char compare_SUB_DI;
extern char compare_jmp_SUB_DI;
extern char modified_mov_SUB_DI;
extern char modified_jmp_SUB_DI;
extern char else_end_label_SUB_DI;
extern char handler_call_start_SUB_DI;
extern char handler_call_end_SUB_DI;
extern char read_handler_call_SUB_DI;
extern char write_handler_call_SUB_DI;


// Istruzione move (che potrebbe essere patchata)
typedef struct mov_rec mov_rec;
struct mov_rec {
    void* address;
    void* address_end;  //***** forse meglio size al posto di end *****
    int BB_index;                       // indice del BB a cui appartiene questa istruzione
    char patched;                       // indica che la mov è stata patchata
    char write;                         // indica che la mov esegue una scrittura in memoria
    unsigned char immediate_data_size;  // size dell'immediate data
    char eax_is_in_use;                 // indica che il registro eax viene usato nell'istruzione
    void (*patch_func) (mov_rec* instr, void* dest_addr);
    size_t (*patch_size_func) (mov_rec* instr);
};


// Istruzione di branch (jmp o call relativo) che deve essere rilocata nelle patch
typedef struct {
    void* address;
    void* address_end;  //***** forse meglio size al posto di end *****
    int BB_index;                       // indice del BB a cui appartiene questa istruzione
    void* jump_target;                  // indirizzo obiettivo del salto
    unsigned char size_to_add_in_patch; // numero di byte da aggiungere all'istruzione per trasformarla da short a long jump (0 per long jump originali)
    unsigned char new_opcode[2];        // nuovo opcode dell'istruzione per trasformarla da short a long jump (x per long jump originali)
    unsigned char new_opcode_size;      // numero di byte del nuovo opcode dell'istruzione per trasformarla da short a long jump (x per long jump originali)
} branch_rec;


// Basic Block
typedef struct {
    void* originalBB_start;
    void* originalBB_end;
    void* BB_start;
    void* BB_end;
    int mov_index;                      // indice della prima mov appartenente a questo BB
    unsigned int mov_number;            // numero di mov appartenenti a questo BB
    int branch_index;                   // indice della prima istruzione di branch appartenente a questo BB
    unsigned int branch_number;         // numero di istruzioni di branch appartenenti a questo BB
} BB_rec;


// Patch function prototypes
void _rm_patch_MOV_SA(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_MOV_SA(mov_rec* instr);
void _rm_patch_MOV_SC(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_MOV_SC(mov_rec* instr);
void _rm_patch_MOV_DA(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_MOV_DA(mov_rec* instr);
void _rm_patch_MOV_DC(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_MOV_DC(mov_rec* instr);
void _rm_patch_MOV_DI(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_MOV_DI(mov_rec* instr);

void _rm_patch_CMP_SA(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_CMP_SA(mov_rec* instr);
void _rm_patch_CMP_SC(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_CMP_SC(mov_rec* instr);
void _rm_patch_CMP_DA(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_CMP_DA(mov_rec* instr);
void _rm_patch_CMP_DC(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_CMP_DC(mov_rec* instr);
void _rm_patch_CMP_DI(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_CMP_DI(mov_rec* instr);

void _rm_patch_ADD_SA(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_ADD_SA(mov_rec* instr);
void _rm_patch_ADD_SC(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_ADD_SC(mov_rec* instr);
void _rm_patch_ADD_DA(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_ADD_DA(mov_rec* instr);
void _rm_patch_ADD_DC(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_ADD_DC(mov_rec* instr);
void _rm_patch_ADD_DI(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_ADD_DI(mov_rec* instr);

void _rm_patch_SUB_SA(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_SUB_SA(mov_rec* instr);
void _rm_patch_SUB_SC(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_SUB_SC(mov_rec* instr);
void _rm_patch_SUB_DA(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_SUB_DA(mov_rec* instr);
void _rm_patch_SUB_DC(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_SUB_DC(mov_rec* instr);
void _rm_patch_SUB_DI(mov_rec* instr, void* dest_addr);
size_t _rm_patch_size_SUB_DI(mov_rec* instr);

#if rm_DISASSEMBLER == rm_LIBDISASM
int _rm_decode(x86_insn_t insn, void* addr, mov_rec* out_instr);
int _rm_is_branch(x86_insn_t insn, void* addr, branch_rec* out_branch_instr, GArray* out_jmp_array);
#else
int _rm_decode(_DInst insn, mov_rec* out_instr);
int _rm_is_branch(_DInst insn, branch_rec* out_branch_instr, GArray* out_jmp_array);
#endif



#ifdef __cplusplus
}
#endif

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
