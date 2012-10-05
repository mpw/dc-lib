/* =====================================================================
 *  test-driver.c
 * =====================================================================

 *  Author:         (c) 2010-2011 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        October 23, 2010
 *  Module:         dc/test

 *  Last changed:   $Date: 2011/04/03 11:35:29 $
 *  Changed by:     $Author: demetres $
 *  Revision:       $Revision: 1.25 $
*/


#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "logfile.h"
#include "test-driver.h"
#include "profile.h"


#if DC
#include "rm.h"
#include "dc.h"
#include "dc_inspect.h"
#include "dc_profile.h"
#endif


// config
#define DUMP_PROC_STATUS 0


// driver structure
typedef struct {
    size_t          input_size;
    size_t          num_updates;
    char*           log_file_name;
    char*           test_name;
    char*           input_family;
    char*           update_family;
    int             seed;
    size_t          trials;
    test_t*         test;
    size_t          base_vm_peak;
    size_t          vm_peak;
    elapsed_time_t  input_time;
    elapsed_time_t  init_time;
    elapsed_time_t  update_time;
    elapsed_time_t  delete_time;

    #if CONV
    elapsed_time_t  conv_input_time;
    elapsed_time_t  conv_eval_time;
    #endif

    #if DC

    // vm peak after dc_init()
    size_t          dc_init_vm_peak;

    // number of cached instructions executed during from-scratch 
    // evaluation
    unsigned long long init_exec_cached;

    // number of cached instructions executed during update sequence
    unsigned long long update_exec_cached; 

    dc_profile post_init;   // post-init profiling snapshot
    dc_profile post_update; // post-update profiling snapshot
    dc_profile post_delete; // post-delete profiling snapshot
    #endif
} driver_t;


// ---------------------------------------------------------------------
// print_msg
// ---------------------------------------------------------------------
static void print_msg(FILE* fp, char* msg, va_list args) {
    fprintf(fp, "[test-driver] ");
	vfprintf(fp, msg, args);
    fprintf(fp, "\n");
}


// ---------------------------------------------------------------------
// message
// ---------------------------------------------------------------------
static void message(char* msg, ...) {
    va_list args;
    va_start(args, msg);
    print_msg(stderr, msg, args);
    va_end(args);
}


// ---------------------------------------------------------------------
// panic
// ---------------------------------------------------------------------
static void panic(char* msg, ...) {
    va_list args;
    va_start(args, msg);
    print_msg(stderr, msg, args);
    va_end(args);
    exit(1);
}

#if DC && MAKE_ASM_DUMP
void do_make_binary_dump(driver_t* driver, char* suffix) {
    char buf[128];
    dc_init();
    sprintf(buf, "%s%s-%s%s.dump", 
        LOGS_DIR, driver->test_name, OPT, suffix);
    rm_make_dump_file(buf);
}
#endif


// ---------------------------------------------------------------------
// do_trials
// ---------------------------------------------------------------------
void do_trials(driver_t* driver) {

    time_rec_t start_time, end_time;
    int k;
    char* res;

    // init time counters
    reset_elapsed_time(&driver->input_time);
    reset_elapsed_time(&driver->init_time);
    reset_elapsed_time(&driver->update_time);
    reset_elapsed_time(&driver->delete_time);
    #if CONV
    reset_elapsed_time(&driver->conv_input_time);
    reset_elapsed_time(&driver->conv_eval_time);
    #endif

    // make trials
    for (k = 1; k <= driver->trials; ++k) {

        // make test object
        driver->test = make_test(driver->input_size, driver->seed, 
                                 driver->input_family,
                                 driver->update_family, 0);
        if (driver->test == NULL) panic("cannot create test object");

        // starting test
        message("starting trial #%d: %s, input-size=%u, "
                "input-family=\"%s\", update-family=\"%s\", "
                "seed=%d, opt=%s", 
            k, driver->test_name, driver->input_size, 
                driver->input_family, driver->update_family, 
                driver->seed, OPT);

        // make updatable input for evaluation
        message("make updatable input...");
        get_time(&start_time);      // get initial time
        res = make_updatable_input(driver->test);
        if (res != NULL) panic(res);
        get_time(&end_time);        // get final time
        add_to_elapsed_time(&start_time, &end_time, 
            &driver->input_time);
        message("...done.");
    
        // initial from-scratch evaluation of updatable input
        message("performing from-scratch evaluation...");
        #if DC
        if (k==1) {            
            driver->init_exec_cached = g_stats.exec_cache_instr_count;
        }
        #endif
        get_time(&start_time);      // get initial time
        res = do_from_scratch_eval(driver->test);
        if (res != NULL) panic(res);
        get_time(&end_time);        // get final time
        add_to_elapsed_time(&start_time, &end_time, &driver->init_time);
        #if DC
        if (k==1) {
            if (dc_profile_on())
                dc_fetch_profile_info(&driver->post_init);
            driver->init_exec_cached = 
                g_stats.exec_cache_instr_count - 
                    driver->init_exec_cached;
        }
        #endif
        message("...done.");

        // do sequence of updates on updatable input
        message("performing update sequence...");
        #if DC
        if (k==1) {
            driver->update_exec_cached = g_stats.exec_cache_instr_count;
        }
        #endif
        get_time(&start_time);      // get initial time
        res = do_updates(driver->test);
        if (res != NULL) panic(res);
        get_time(&end_time);        // get final time
        add_to_elapsed_time(&start_time, &end_time, 
                            &driver->update_time);
        #if DC
        if (k==1) {
            if (dc_profile_on())
                dc_fetch_profile_info(&driver->post_update);
            driver->update_exec_cached = 
                g_stats.exec_cache_instr_count - 
                    driver->update_exec_cached;
        }
        #endif
        message("...done.");

        // get peak memory used by process after first trial
        // (ignoring conventional evaluation)
        if (k==1) driver->vm_peak = get_vm_peak();

        // fetch actual number of updates performed
        driver->num_updates = get_num_updates(driver->test);

        #if CONV
        // make conventional input for evaluation
        message("making conventional input...");
        get_time(&start_time);      // get initial time
        res = make_conv_input(driver->test);
        if (res != NULL) panic(res);
        get_time(&end_time);        // get final time
        add_to_elapsed_time(&start_time, &end_time, 
                            &driver->conv_input_time);

        // conventional evaluation
        message("performing conventional evaluation...");
        get_time(&start_time);      // get initial time
        res = do_conv_eval(driver->test);
        if (res != NULL) panic(res);
        get_time(&end_time);        // get final time
        add_to_elapsed_time(&start_time, &end_time, 
                            &driver->conv_eval_time);
        #endif

        // deallocate test object
        message("deallocating test object...");
        get_time(&start_time);      // get initial time
        //del_test(driver->test);
        get_time(&end_time);        // get final time
        add_to_elapsed_time(&start_time, &end_time, 
                            &driver->delete_time);
        #if DC
        if (k==1 && dc_profile_on()) 
            dc_fetch_profile_info(&driver->post_delete);
        #endif
        message("...done.");
    }

    // compute average time per trial
    divide_elapsed_time_by(&driver->input_time,  driver->trials);
    divide_elapsed_time_by(&driver->init_time,   driver->trials);
    divide_elapsed_time_by(&driver->update_time, driver->trials);
    divide_elapsed_time_by(&driver->delete_time, driver->trials);
    #if CONV
    divide_elapsed_time_by(&driver->conv_input_time, driver->trials);
    divide_elapsed_time_by(&driver->conv_eval_time, driver->trials);
    #endif
}


// ---------------------------------------------------------------------
// do_checks
// ---------------------------------------------------------------------
void do_checks(driver_t* driver) {

    // perform final checks
    #if CHECK

    char* res;

    // make test object
    driver->test = make_test(driver->input_size, 
                             driver->seed, driver->input_family, 
                             driver->update_family, 1);
    if (driver->test == NULL) panic("cannot create test object");

    // starting correctness check    
    message("checking correctness: %s, input size=%u, "
            "input-family=\"%s\", update-family=\"%s\", seed=%d opt=%s", 
        driver->test_name, driver->input_size, 
        driver->input_family, driver->update_family, driver->seed, OPT);

    // check updatable input construction
    message("checking updatable input construction...");
    res = make_updatable_input(driver->test);
    if (res != NULL) panic("...failed (%s)", res);
    else message("...passed");

    // check initial from-scratch evaluation of updatable input
    message("checking from-scratch evaluation...");
    res = do_from_scratch_eval(driver->test);
    if (res != NULL) panic("...failed (%s)", res);
    else message("...passed");

    // do sequence of updates on updatable input
    message("checking update sequence...");
    res = do_updates(driver->test);
    if (res != NULL) panic("...failed (%s)", res);
    else message("...passed");

    // deallocate test object
    message("deallocating test object...");
    del_test(driver->test);
    message("...done.");

    #endif
}


// ---------------------------------------------------------------------
// do_write_output
// ---------------------------------------------------------------------
void do_write_output(driver_t* driver) {

    message("writing experiment log...");

    logfile_t* f = logfile_open(driver->log_file_name, 1);
    if (f == NULL)
        panic("can't open log file %s", driver->log_file_name);    

    logfile_add_to_header(f, "%-14s ", "test name");
    logfile_add_to_row   (f, "%-14s ", driver->test_name);

    logfile_add_to_header(f, "%-11s ", "input size");
    logfile_add_to_row   (f, "%-11u ", driver->input_size);

    logfile_add_to_header(f, "%-14s ", "input family");
    logfile_add_to_row   (f, "%-14s ", driver->input_family);

    logfile_add_to_header(f, "%-14s ", "update family");
    logfile_add_to_row   (f, "%-14s ", driver->update_family);

    logfile_add_to_header(f, "%-12s ", "num updates");
    logfile_add_to_row   (f, "%-12u ", driver->num_updates);

    logfile_add_to_header(f, "%-8s ", "seed");
    logfile_add_to_row   (f, "%-8d ", driver->seed);

    logfile_add_to_header(f, "%-4s ", "opt");
    logfile_add_to_row   (f, "%-4s ", OPT);

    logfile_add_to_header(f, "%-7s ", "trials");
    logfile_add_to_row   (f, "%-7d ", driver->trials);

    #if DC
    if (!dc_profile_on()) {
    #endif

        #if WALL_CLOCK_TIME
        logfile_add_to_header(f, "%-16s ",   "input real (s)");
        logfile_add_to_row   (f, "%-16.6f ", driver->input_time.real);
    
        logfile_add_to_header(f, "%-16s ",   "init real (s)");
        logfile_add_to_row   (f, "%-16.6f ", driver->init_time.real);
    
        logfile_add_to_header(f, "%-16s ",   "update real (s)");
        logfile_add_to_row   (f, "%-16.6f ", driver->update_time.real);
    
        logfile_add_to_header(f, "%-16s ",   "delete real (s)");
        logfile_add_to_row   (f, "%-16.6f ", driver->delete_time.real);
        #endif

        #if USER_SYS_TIME
        logfile_add_to_header(f, "%-16s ",   "input u+s (s)");
        logfile_add_to_row   (f, "%-16.6f ", 
            driver->input_time.user + driver->input_time.system);
    
        logfile_add_to_header(f, "%-16s ",   "init u+s (s)");
        logfile_add_to_row   (f, "%-16.6f ", 
            driver->init_time.user + driver->init_time.system);
    
        logfile_add_to_header(f, "%-16s ",   "update u+s (s)");
        logfile_add_to_row   (f, "%-16.6f ", 
            driver->update_time.user + driver->update_time.system);
    
        logfile_add_to_header(f, "%-16s ",   "delete u+s (s)");
        logfile_add_to_row   (f, "%-16.6f ", 
            driver->delete_time.user + driver->delete_time.system);
        #endif

        #if CONV
    
        #if WALL_CLOCK_TIME
        logfile_add_to_header(f, "%-16s ",   "con inp re (s)");
        logfile_add_to_row   (f, "%-16.6f ", 
            driver->conv_input_time.real);

        logfile_add_to_header(f, "%-16s ",   "con eva re (s)");
        logfile_add_to_row   (f, "%-16.6f ", 
            driver->conv_eval_time.real);
        #endif

        #if USER_SYS_TIME
        logfile_add_to_header(f, "%-16s ",   "con inp u+s (s)");
        logfile_add_to_row   (f, "%-16.6f ", 
            driver->conv_input_time.user + driver->conv_input_time.system);
    
        logfile_add_to_header(f, "%-16s ",   "con eva u+s (s)");
        logfile_add_to_row   (f, "%-16.6f ", 
            driver->conv_eval_time.user + driver->conv_eval_time.system);
        #endif
        
        #endif

    #if DC
    }
    #endif

    #if !DC
    logfile_add_to_header(f, "%-17s ", "base vm peak (KB)");
    logfile_add_to_row   (f, "%-17u ", driver->base_vm_peak);

    logfile_add_to_header(f, "%-13s ", "vm peak (KB)");
    logfile_add_to_row   (f, "%-13u ", driver->vm_peak);
    #endif

    #if DC

    // vm peak is higher if profiling is turned on, so it would be an
    // unrealiable measure
    if (!dc_profile_on()) {
        logfile_add_to_header(f, "%-17s ", "base vm peak (KB)");
        logfile_add_to_row   (f, "%-17u ", driver->base_vm_peak);

        logfile_add_to_header(f, "%-17s ", "dc vm peak (KB)");
        logfile_add_to_row   (f, "%-17u ", driver->dc_init_vm_peak);

        logfile_add_to_header(f, "%-13s ", "vm peak (KB)");
        logfile_add_to_row   (f, "%-13u ", driver->vm_peak);
    }

    if (dc_profile_on()) {
        dc_profile update_prof;
        dc_profile_diff(&driver->post_init, 
                        &driver->post_update, 
                        &update_prof);

        // sampling
        logfile_add_to_header(f, "%-14s ", "init norm samp");
        logfile_add_to_row   (f, "%-14llu ",
            driver->post_init.normal_samples);

        logfile_add_to_header(f, "%-14s ", "init solv samp");
        logfile_add_to_row   (f, "%-14llu ",
            driver->post_init.solver_samples);

        logfile_add_to_header(f, "%-17s ", "init cons ex samp");
        logfile_add_to_row   (f, "%-17llu ",
            driver->post_init.cons_exec_samples);

        logfile_add_to_header(f, "%-13s ", "upd norm samp");
        logfile_add_to_row   (f, "%-13llu ",
            update_prof.normal_samples);

        logfile_add_to_header(f, "%-13s ", "upd solv samp");
        logfile_add_to_row   (f, "%-13llu ",
            update_prof.solver_samples);

        logfile_add_to_header(f, "%-16s ", "upd cons ex samp");
        logfile_add_to_row   (f, "%-16llu ",
            update_prof.cons_exec_samples);

        // cell objects
        logfile_add_to_header(f, "%-14s ", "cell peak num");
        logfile_add_to_row   (f, "%-14u ",
            driver->post_update.cell_peak_num);

        logfile_add_to_header(f, "%-14s ", "cell peak (KB)");
        logfile_add_to_row   (f, "%-14u ", 
            driver->post_update.cell_peak_size);

        // dependency objects
        logfile_add_to_header(f, "%-12s ", "init dep num");
        logfile_add_to_row   (f, "%-12u ", 
            driver->post_init.dep_num);

        logfile_add_to_header(f, "%-18s ", "init stale dep num");
        logfile_add_to_row   (f, "%-18u ", 
            driver->post_init.stale_dep_num);

        logfile_add_to_header(f, "%-18s ", "init stale cleanup");
        logfile_add_to_row   (f, "%-18u ", 
            driver->post_init.stale_cleanup_num);

        logfile_add_to_header(f, "%-12s ", "upd dep num");
        logfile_add_to_row   (f, "%-12u ", 
            driver->post_update.dep_num);

        logfile_add_to_header(f, "%-18s ", "upd stale dep num");
        logfile_add_to_row   (f, "%-18u ", 
            driver->post_update.stale_dep_num);

        logfile_add_to_header(f, "%-18s ", "upd stale cleanup");
        logfile_add_to_row   (f, "%-18u ", 
            update_prof.stale_cleanup_num);

        logfile_add_to_header(f, "%-13s ", "dep peak num");
        logfile_add_to_row   (f, "%-13u ", 
            driver->post_update.dep_peak_num);

        logfile_add_to_header(f, "%-13s ", "dep peak (KB)");
        logfile_add_to_row   (f, "%-13u ", 
            driver->post_update.dep_peak_size);

        // constraints
        logfile_add_to_header(f, "%-13s ", "init cons");
        logfile_add_to_row   (f, "%-13llu ", 
            driver->post_init.cons_peak_num);

        logfile_add_to_header(f, "%-13s ", "init new_cons");
        logfile_add_to_row   (f, "%-13llu ", 
            driver->post_init.num_new_cons);

        logfile_add_to_header(f, "%-13s ", "init del_cons");
        logfile_add_to_row   (f, "%-13llu ", 
            driver->post_init.num_del_cons);

        logfile_add_to_header(f, "%-13s ", "upd cons");
        logfile_add_to_row   (f, "%-13llu ", 
            driver->post_init.cons_peak_num);

        logfile_add_to_header(f, "%-13s ", "upd new_cons");
        logfile_add_to_row   (f, "%-13llu ", 
            update_prof.num_new_cons);

        logfile_add_to_header(f, "%-13s ", "upd del_cons");
        logfile_add_to_row   (f, "%-13llu ", 
            update_prof.num_del_cons);

        logfile_add_to_header(f, "%-14s ", "cons peak (KB)");
        logfile_add_to_row   (f, "%-14u ", 
            driver->post_update.cons_peak_size);

        // constraint executions
        logfile_add_to_header(f, "%-13s ", "init ex cons");
        logfile_add_to_row   (f, "%-13llu ", 
            driver->post_init.num_exec_cons);

        logfile_add_to_header(f, "%-17s ", "init dis ex cons");
        logfile_add_to_row   (f, "%-17llu ", 
            driver->post_init.num_distinct_exec_cons);

        logfile_add_to_header(f, "%-13s ", "upd ex cons");
        logfile_add_to_row   (f, "%-13llu ", 
            update_prof.num_exec_cons);

        logfile_add_to_header(f, "%-17s ", "upd dis ex cons");
        logfile_add_to_row   (f, "%-17llu ", 
            update_prof.num_distinct_exec_cons);

        logfile_add_to_header(f, "%-13s ", "avg con x upd");
        logfile_add_to_row   (f, "%-13.1f ", 
            (double)update_prof.num_exec_cons/driver->num_updates);

        // reactive blocks
        logfile_add_to_header(f, "%-13s ", "init rblocks");
        logfile_add_to_row   (f, "%-13llu ", 
            driver->post_init.peak_num_reactive_blocks);

        logfile_add_to_header(f, "%-13s ", "init dc_alloc");
        logfile_add_to_row   (f, "%-13llu ", 
            driver->post_init.num_alloc);

        logfile_add_to_header(f, "%-13s ", "init dc_free");
        logfile_add_to_row   (f, "%-13llu ", 
            driver->post_init.num_free);

        logfile_add_to_header(f, "%-13s ", "upd rblocks");
        logfile_add_to_row   (f, "%-13llu ", 
            driver->post_update.peak_num_reactive_blocks);

        logfile_add_to_header(f, "%-13s ", "upd dc_alloc");
        logfile_add_to_row   (f, "%-13llu ", 
            update_prof.num_alloc);

        logfile_add_to_header(f, "%-13s ", "upd dc_free");
        logfile_add_to_row   (f, "%-13llu ", 
            update_prof.num_free);

        logfile_add_to_header(f, "%-13s ", "rvm peak (KB)");
        logfile_add_to_row   (f, "%-13u ", 
            driver->post_update.rmem_size);

        // cached instructions
        logfile_add_to_header(f, "%-14s ", "init ex cached");
        logfile_add_to_row   (f, "%-14u ", driver->init_exec_cached);

        logfile_add_to_header(f, "%-13s ", "upd ex cached");
        logfile_add_to_row   (f, "%-13u ", driver->update_exec_cached);
    }
    #endif

    logfile_close(f);

    message("...done.");
}


// ---------------------------------------------------------------------
// main
// ---------------------------------------------------------------------
int main(int argc, char** argv) {

    driver_t d, *driver = &d;

    if (argc < 7)
        panic("usage: %s inputsize inputfamily updatefamily logfile "
              "seed trials\n", 
            argv[0]);

    // get command line parameters
    driver->input_size = atol(argv[1]);
    if (driver->input_size == 0) panic("illegal size parameter");
    driver->input_family = argv[2];
    driver->update_family = argv[3];
    driver->log_file_name = argv[4];
    driver->seed = atol(argv[5]);
    if (driver->seed == 0) panic("illegal seed parameter");
    driver->trials = atol(argv[6]);
    if (driver->trials == 0) panic("illegal trials parameter");
    driver->test_name = get_test_name();

    // get initial vm peak
    driver->base_vm_peak = get_vm_peak();

    // make dump of unpatched dc binary
    #if DC
    dc_init();

    // get vm peak after dc initialization
    driver->dc_init_vm_peak = get_vm_peak();

    #if MAKE_ASM_DUMP
    do_make_binary_dump(driver, "-start");
    #endif
    #endif

    // do performance test
    do_trials(driver);

    // do correctness test
    do_checks(driver);

    // write results to log file
    do_write_output(driver);
    
    // make dump of patched dc binary
    #if DC && MAKE_ASM_DUMP
    do_make_binary_dump(driver, "-end");
    #endif

    // dump final process status
    #if DUMP_PROC_STATUS
    dump_proc_status("final process status");
    #endif

    return 0;
}


// ---------------------------------------------------------------------
//  get_int_param
// ---------------------------------------------------------------------
int get_int_param(char* str, char* delim, int index, int* arg) {
    char* buf = malloc(strlen(str)+1), *token;
    strcpy(buf, str);
    token = strtok(buf, delim);
    while (index > 0 && token != NULL) {
        token = strtok(NULL, delim);
        index--;
    }
    if (token == NULL) goto err;
    *arg = atoi(token);
    free(buf);
    return 1;

  err:
    free(buf);
    return 0;
}


// ---------------------------------------------------------------------
//  cmp_param
// ---------------------------------------------------------------------
int cmp_param(char* str, char* delim, int index, char* str2) {
    char* buf = malloc(strlen(str)+1), *token;
    strcpy(buf, str);
    token = strtok(buf, delim);
    while (index > 0 && token != NULL) {
        token = strtok(NULL, delim);
        index--;
    }
    if (token == NULL) goto err;
    int res = !strcmp(token, str2);
    free(buf);
    return res;

  err:
    free(buf);
    return 0;
}


/* Copyright (C) 2010-2011 Camil Demetrescu

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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  
 * USA
*/
