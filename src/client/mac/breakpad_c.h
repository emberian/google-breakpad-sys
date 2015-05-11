//
//  breakpad_c.h
//  Breakpad
//
//  Created by Corey Richardson on 5/11/15.
//
//

#ifndef __Breakpad__breakpad_c__
#define __Breakpad__breakpad_c__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <signal.h>
#include <sys/types.h>
#include <mach/mach.h>

#ifdef __cplusplus
extern "C" {
#endif
    typedef struct breakpad_eh breakpad_eh;
    typedef struct breakpad_crash_generation_client breakpad_crash_generation_client;
    typedef struct breakpad_crash_generation_server breakpad_crash_generation_server;
    typedef struct breakpad_client_info breakpad_client_info;
    
    typedef bool (*breakpad_filter_cb)(void* context);
    typedef bool (*breakpad_minidump_cb)(const char *dump_dir, const char *minidump_id, void* context, bool succeeded);
    typedef bool (*breakpad_handler_cb)(const void* crash_context, size_t crash_context_size, void* context);
    
    typedef bool (*breakpad_on_client_dump_request_cb)(void* context, const breakpad_client_info *client_info, const char *file_path);
    typedef bool (*breakpad_on_client_exiting_cb)(void* context, const breakpad_client_info* client_info);
    
    
    
    breakpad_eh* breakpad_eh_create(const char *dump_path,
                                    breakpad_filter_cb filter,
                                    breakpad_minidump_cb cb,
                                    void *cb_context,
                                    bool install_handler,
                                    const char *port_name);
    
    void breakpad_eh_destroy(breakpad_eh *eh);
    
    const char *breakpad_eh_dump_path(breakpad_eh *eh);
    
    void breakpad_eh_set_dump_path(breakpad_eh *eh, char *dump_path);
    
    // write_exception_stream should probably be false? not sure what it does.
    bool breakpad_eh_write_minidump(breakpad_eh *eh, bool write_exception_stream);
    
    breakpad_crash_generation_client* breakpad_crash_generation_client_create(const char *mach_port_name);
    
    bool breakpad_crash_generation_client_request_dump_for_exception(breakpad_crash_generation_client* client, int exception_type, int exception_code, int exception_subcode, mach_port_t crashing_thread);
    
    breakpad_crash_generation_server* breakpad_crash_generation_server_create(const char *mach_port_name, breakpad_filter_cb filter, void *filter_context, breakpad_on_client_dump_request_cb dump_cb, void *dump_context, breakpad_on_client_exiting_cb exit_cb, void *exit_context, bool generate_dumps, const char *dump_path);
    
    void breakpad_crash_generation_server_destroy(breakpad_crash_generation_server* server);
    
    bool breakpad_crash_generation_server_start(breakpad_crash_generation_server* server);
    
    void breakpad_crash_generation_server_stop(breakpad_crash_generation_server* server);
#ifdef __cplusplus
}
#endif


#endif /* defined(__Breakpad__breakpad_c__) */
