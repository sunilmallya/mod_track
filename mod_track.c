/*
 * Apache Module to  track the time from connection establishment to completion of data arrival.
 * This also measures PostData send time. (Note that this is longer as apache start sending the first byte to the client early)
 * @Author = Sunil Mallya (mallya16@gmail.com)
 */
#include "httpd.h"
#include "http_config.h"
#include "http_log.h"

#include "apr.h"
#include "apr_strings.h"
#include "apr_lib.h"

#include "ap_config.h"

#include <util_filter.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

static double start_time;
static double output_start_time;
static void log_connection_close(void *);
static ap_filter_rec_t* log_output_start_handle;


static double cur_time(){
        struct timeval tp;

        if ( gettimeofday(&tp, (struct timezone *) NULL) == -1){
                return -1;
        }
        return ((double) (tp.tv_sec)) +
                (((double) tp.tv_usec) * 0.000001 );
}


static int log_pre_conn(conn_rec *c, void *csd){

	start_time =  cur_time();

       // ap_log_cerror(APLOG_MARK, APLOG_ERR, 0, c,
       //               "PRE_ACCEPT= %f", start_time);

	return OK;
}

static int log_post_conn(request_rec *r){

	double end_time = cur_time();

	double total_time = end_time - start_time;

        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                      "[mod_track] Time elapsed since connection esablishment to completion of data arrival = %f ", total_time);

	return OK;
}


static int log_header_parser(request_rec *r){

        double end_time = cur_time();
        double total_time = end_time - start_time;

        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                      "[mod_track] Time elapsed since connection esablishment to start parsing header = %f ", total_time);

        return OK;
}

static int log_output_start(ap_filter_t *f, apr_bucket_brigade *in){

    	request_rec *r = f->r;
    	output_start_time = cur_time();

	//     	ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
	//                    "OUTPUT_TIME_S= %f",output_start_time );

	ap_remove_output_filter(f);
	return ap_pass_brigade(f->next, in);

}


static void log_connection_close(void *data){
     	request_rec *r = (request_rec*) data;
	double end_time = cur_time();
	double total_time = end_time - output_start_time;
	
	ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
					"[mod_track] Total time from sending the first byte to competion of sending data  = %f ",total_time );
}

static int cleanup_handler(request_rec *r){

        ap_add_output_filter_handle(log_output_start_handle,
                                NULL, r, r->connection);

	//Register Cleanup Handler
	apr_pool_cleanup_register(r->pool, (void*)r, log_connection_close,apr_pool_cleanup_null) ;

	return DECLINED;
}

/* Apace Register Hooks */
static void mod_track_register_hooks (apr_pool_t *p){

	//Worth trying this -
	//ap_hook_quick_handler -    called before any request processing, used by cache modules.
       
	//After accepting connection
	ap_hook_pre_connection(log_pre_conn,NULL,NULL,APR_HOOK_FIRST);
 
	//Post request read
	ap_hook_post_read_request(log_post_conn,NULL,NULL,APR_HOOK_LAST);

	//header parser
	ap_hook_header_parser(log_header_parser, NULL, NULL, APR_HOOK_MIDDLE);

	//Hook before output is sent
	log_output_start_handle = ap_register_output_filter("MOD_TRACK",
                                  log_output_start,
                                  NULL,
                                  AP_FTYPE_CONTENT_SET+1);
 
	//cleanup handler
	ap_hook_access_checker(cleanup_handler, NULL, NULL, APR_HOOK_LAST);

}

/* Apache Module Struct */
module AP_MODULE_DECLARE_DATA mod_track_module ={
	STANDARD20_MODULE_STUFF,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	mod_track_register_hooks,			/* callback for registering hooks */
};
