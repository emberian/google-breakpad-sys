//
//  breakpad_c.c
//  Breakpad
//
//  Created by Corey Richardson on 5/11/15.
//
//

#include "breakpad_c.h"
#include "client/mac/crash_generation/crash_generation_client.h"
#include "client/mac/crash_generation/crash_generation_server.h"
#include "client/mac/handler/exception_handler.h"

using namespace google_breakpad;

// ugh.
#define ExceptionHandler google_breakpad::ExceptionHandler

struct HandlerWrapperContext {
    breakpad_minidump_cb mcb;
    breakpad_filter_cb fcb;
    void* real_context;
    
    HandlerWrapperContext(breakpad_filter_cb fcb, breakpad_minidump_cb mcb, void *context) : mcb(mcb), fcb(fcb), real_context(context) { }
};

static bool filter_wrapper_cb(void* context) {
    HandlerWrapperContext* cont = reinterpret_cast<HandlerWrapperContext*>(context);
    
    return cont->fcb ? cont->fcb(cont->real_context) : true;
}

static bool minidump_wrapper_cb(const char *dump_dir, const char *minidump_id, void* context, bool succeeded) {
    HandlerWrapperContext* cont = reinterpret_cast<HandlerWrapperContext*>(context);
    
    return cont->mcb ? cont->mcb(dump_dir, minidump_id, cont->real_context, succeeded) : succeeded;
}


struct ServerWrapperContext {
    breakpad_filter_cb filter_cb;
    breakpad_on_client_dump_request_cb dump_cb;
    breakpad_on_client_exiting_cb exit_cb;
    void *filter_context;
    void *dump_context;
    void *exit_context;
    
    ServerWrapperContext(breakpad_filter_cb filter_cb, breakpad_on_client_dump_request_cb dump_cb, breakpad_on_client_exiting_cb exit_cb, void *filter_context, void *dump_context, void *exit_context) : filter_cb(filter_cb), dump_cb(dump_cb), exit_cb(exit_cb), dump_context(dump_context), exit_context(exit_context) { }
};

static bool server_filter_callback_wrapper(void *context) {
    ServerWrapperContext* cont = reinterpret_cast<ServerWrapperContext*>(context);
    if (NULL != cont->filter_cb) {
        return cont->filter_cb(cont->filter_context);
    } else {
        return false;
    }
}

static void exit_callback_wrapper(void* context, const ClientInfo &client_info) {
    ServerWrapperContext* cont = reinterpret_cast<ServerWrapperContext*>(context);
    if (NULL != cont->exit_cb) {
        cont->exit_cb(cont->exit_context, reinterpret_cast<const breakpad_client_info*>(&client_info));
    }
}

static void dump_request_callback_wrapper(void* context, const ClientInfo &client_info, const string &file_path) {
    ServerWrapperContext* cont = reinterpret_cast<ServerWrapperContext*>(context);
    if (NULL != cont->dump_cb) {
        cont->dump_cb(cont->dump_context, reinterpret_cast<const breakpad_client_info*>(&client_info), file_path.c_str());
    }
}

extern "C" {
    breakpad_eh* breakpad_eh_create(const char *dump_path,
                                    breakpad_filter_cb filter,
                                    breakpad_minidump_cb cb,
                                    void *cb_context,
                                    bool install_handler,
                                    const char *port_name) {
        HandlerWrapperContext *cont = new HandlerWrapperContext(filter, cb, cb_context);
        return reinterpret_cast<breakpad_eh*>(new ExceptionHandler(string(dump_path), filter_wrapper_cb, minidump_wrapper_cb, cont, install_handler, port_name));
    }
    
    void breakpad_eh_destroy(breakpad_eh *eh) {
        delete reinterpret_cast<ExceptionHandler*>(eh);
    }
    
    const char *breakpad_eh_dump_path(breakpad_eh *eh) {
        return reinterpret_cast<ExceptionHandler&>(eh).dump_path().c_str();
    }
    
    void breakpad_eh_set_dump_path(breakpad_eh *eh, char *dump_path) {
        reinterpret_cast<ExceptionHandler&>(eh).set_dump_path(string(dump_path));
    }
    
    // write_exception_stream should probably be false? not sure what it does.
    bool breakpad_eh_write_minidump(breakpad_eh *eh, bool write_exception_stream) {
        return reinterpret_cast<ExceptionHandler&>(eh).WriteMinidump(write_exception_stream);
    }
    
    breakpad_crash_generation_client* breakpad_crash_generation_client_create(const char *mach_port_name) {
        return reinterpret_cast<breakpad_crash_generation_client*>(new CrashGenerationClient(mach_port_name));
    }
    
    bool breakpad_crash_generation_client_request_dump_for_exception(breakpad_crash_generation_client* client, int exception_type, int exception_code, int exception_subcode, mach_port_t crashing_thread) {
        return reinterpret_cast<CrashGenerationClient&>(client).RequestDumpForException(exception_type, exception_code, exception_subcode, crashing_thread);
    }
    
    breakpad_crash_generation_server* breakpad_crash_generation_server_create(const char *mach_port_name, breakpad_filter_cb filter, void *filter_context, breakpad_on_client_dump_request_cb dump_cb, void *dump_context, breakpad_on_client_exiting_cb exit_cb, void *exit_context, bool generate_dumps, const char *dump_path) {
        
        ServerWrapperContext *cont = new ServerWrapperContext(filter, dump_cb, exit_cb, filter_context, dump_context, exit_context);
        return reinterpret_cast<breakpad_crash_generation_server*>(new CrashGenerationServer(mach_port_name, server_filter_callback_wrapper, cont, dump_request_callback_wrapper, cont, exit_callback_wrapper, cont, generate_dumps, string(dump_path)));
    }
    
    void breakpad_crash_generation_server_destroy(breakpad_crash_generation_server* server) {
        delete reinterpret_cast<CrashGenerationServer*>(server);
    }
    
    bool breakpad_crash_generation_server_start(breakpad_crash_generation_server* server) {
        reinterpret_cast<CrashGenerationServer&>(server).Start();
    }
    
    void breakpad_crash_generation_server_stop(breakpad_crash_generation_server* server) {
        reinterpret_cast<CrashGenerationServer&>(server).Stop();
    }
}
