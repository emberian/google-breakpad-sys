#include "client/windows/handler/exception_handler.h"
#include "client/windows/crash_generation/crash_generation_client.h"
#include "client/windows/crash_generation/crash_generation_server.h"

#include "breakpad_c.h"

using namespace google_breakpad;

struct HandlerWrapperContext {
    breakpad_minidump_cb mcb;
    breakpad_filter_cb fcb;
    void* real_context;

    HandlerWrapperContext(breakpad_filter_cb fcb, breakpad_minidump_cb mcb, void *context) : mcb(mcb), fcb(fcb), real_context(context) { }
};

static bool filter_callback_wrapper(void* context, EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion) {
    HandlerWrapperContext* cont = reinterpret_cast<HandlerWrapperContext*>(context);

    return cont->fcb ? cont->fcb(cont->real_context, exinfo, reinterpret_cast<breakpad_md_raw_assertion_info*>(assertion)) : true;
}

static bool minidump_callback_wrapper(const wchar_t* dump_path, const wchar_t* minidump_id, void* context, EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool succeeded) {
    HandlerWrapperContext* cont = reinterpret_cast<HandlerWrapperContext*>(context);

	return cont->mcb ? cont->mcb(dump_path, minidump_id, cont->real_context, exinfo, reinterpret_cast<breakpad_md_raw_assertion_info*>(assertion), succeeded) : succeeded;
}

struct ServerWrapperContext {
    breakpad_on_client_dump_request_cb dump_cb;
    breakpad_on_client_exited_cb exit_cb;
    breakpad_on_client_upload_request_cb upload_cb;
    breakpad_on_client_connected_cb connect_cb;
    void* dump_context;
    void* exit_context;
    void* upload_context;
    void* connect_context;

    ServerWrapperContext( breakpad_on_client_dump_request_cb dump_cb,
            breakpad_on_client_exited_cb exit_cb,
            breakpad_on_client_upload_request_cb upload_cb,
            breakpad_on_client_connected_cb connect_cb, void* dump_context,
            void* exit_context, void* upload_context, void* connect_context) :

        dump_cb(dump_cb), exit_cb(exit_cb), upload_cb(upload_cb),
        connect_cb(connect_cb), dump_context(dump_context),
        exit_context(exit_context), upload_context(upload_context),
        connect_context(connect_context) { }
};

static void exit_callback_wrapper(void* context, const ClientInfo* client_info) {
    ServerWrapperContext* cont = reinterpret_cast<ServerWrapperContext*>(context);
    if (NULL != cont->exit_cb) {
        cont->exit_cb(cont->exit_context, reinterpret_cast<const breakpad_client_info*>(client_info));
    }
}

static void dump_request_callback_wrapper(void* context, const ClientInfo* client_info, const wstring* file_path) {
    ServerWrapperContext* cont = reinterpret_cast<ServerWrapperContext*>(context);
    if (NULL != cont->dump_cb) {
        cont->dump_cb(cont->dump_context, reinterpret_cast<const breakpad_client_info*>(client_info), file_path ? file_path->c_str() : NULL);
    }
}

static void upload_callback_wrapper(void* context, const DWORD crash_id) {
    ServerWrapperContext* cont = reinterpret_cast<ServerWrapperContext*>(context);
    if (NULL != cont->dump_cb) {
        cont->upload_cb(cont->upload_context, crash_id);
    }
}

static void connect_callback_wrapper(void* context, const ClientInfo* client_info) {
    ServerWrapperContext* cont = reinterpret_cast<ServerWrapperContext*>(context);
    if (NULL != cont->dump_cb) {
        cont->connect_cb(cont->connect_context, reinterpret_cast<const breakpad_client_info*>(client_info));
    }
}

extern "C" {
    breakpad_eh* breakpad_eh_create_in_process(const wchar_t* dump_path,
            breakpad_filter_cb filter, breakpad_minidump_cb callback,
            void* callback_context, int handler_types) {

        HandlerWrapperContext* cont = new HandlerWrapperContext(filter, callback, callback_context);

        return reinterpret_cast<breakpad_eh*>(new ExceptionHandler(wstring(dump_path),
                    filter_callback_wrapper,
                    minidump_callback_wrapper,
                    cont,
                    handler_types));
    }

    breakpad_eh* breakpad_eh_create_try_out_of_process(const wchar_t* dump_path,
            breakpad_filter_cb filter, breakpad_minidump_cb callback,
            void* callback_context, int handler_types, MINIDUMP_TYPE dump_type,
            HANDLE pipe_handle, const breakpad_custom_client_info* custom_info) {

        HandlerWrapperContext* cont = new HandlerWrapperContext(filter, callback, callback_context);
		return reinterpret_cast<breakpad_eh*>(new ExceptionHandler(wstring(dump_path), filter_callback_wrapper, minidump_callback_wrapper, cont, handler_types, dump_type, pipe_handle, reinterpret_cast<const CustomClientInfo*>(custom_info)));
    }

    breakpad_eh* breakpad_eh_create_out_of_process(const wchar_t* dump_path,
            breakpad_filter_cb filter, breakpad_minidump_cb callback,
            void* callback_context, int handler_types, breakpad_crash_generation_client* client) {

        HandlerWrapperContext* cont = new HandlerWrapperContext(filter, callback, callback_context);

        return reinterpret_cast<breakpad_eh*>(new ExceptionHandler(wstring(dump_path),
                    filter_callback_wrapper,
                    minidump_callback_wrapper,
                    cont,
                    handler_types,
                    reinterpret_cast<CrashGenerationClient*>(client)));
    }

    void breakpad_eh_destroy(breakpad_eh *eh) {
        delete reinterpret_cast<ExceptionHandler*>(eh);
    }

    const wchar_t* breakpad_eh_get_dump_path(const breakpad_eh* eh) {
        return reinterpret_cast<const ExceptionHandler&>(eh).dump_path().c_str();
    }

    void breakpad_eh_set_dump_path(breakpad_eh* eh, const wchar_t* dump_path) {
        reinterpret_cast<ExceptionHandler&>(eh).set_dump_path(wstring(dump_path));
    }

    bool breakpad_eh_request_upload(breakpad_eh* eh, DWORD crash_id) {
        return reinterpret_cast<ExceptionHandler&>(eh).RequestUpload(crash_id);
    }

    bool breakpad_eh_write_minidump_for_exception(breakpad_eh* eh, EXCEPTION_POINTERS* exinfo) {
        return reinterpret_cast<ExceptionHandler&>(eh).WriteMinidumpForException(exinfo);
    }

    DWORD breakpad_eh_get_requesting_thread_id(const breakpad_eh* eh){
        return reinterpret_cast<ExceptionHandler&>(eh).get_requesting_thread_id();
    }

    bool breakpad_eh_get_handle_debug_exceptions(const breakpad_eh* eh) {
        return reinterpret_cast<ExceptionHandler&>(eh).get_handle_debug_exceptions();
    }

    void breakpad_eh_set_handle_debug_exceptions(breakpad_eh* eh, bool handle_debug_exceptions) {
        return reinterpret_cast<ExceptionHandler&>(eh).set_handle_debug_exceptions(handle_debug_exceptions);
    }

    bool breakpad_eh_get_consume_invalid_handle_exceptions(const breakpad_eh* eh) {
        return reinterpret_cast<ExceptionHandler&>(eh).get_consume_invalid_handle_exceptions();
    }

    void breakpad_eh_set_consume_invalid_handle_exceptions(breakpad_eh* eh, bool cihe) {
        return reinterpret_cast<ExceptionHandler&>(eh).set_consume_invalid_handle_exceptions(cihe);
    }

    bool breakpad_eh_is_out_of_process(const breakpad_eh* eh) {
        return reinterpret_cast<ExceptionHandler&>(eh).IsOutOfProcess();
    }

    void breakpad_eh_register_app_memory(breakpad_eh *eh,
                                         void* ptr,
                                         size_t len) {
        return reinterpret_cast<ExceptionHandler&>(eh).RegisterAppMemory(ptr, len);
    }

    void breakpad_eh_unregister_app_memory(breakpad_eh *eh, void* ptr) {
        return reinterpret_cast<ExceptionHandler&>(eh).UnregisterAppMemory(ptr);
    }

    breakpad_crash_generation_client* breakpad_crash_generation_client_create(HANDLE pipe_handle,
            MINIDUMP_TYPE dump_type,
            const breakpad_custom_client_info* custom_info) {
        return reinterpret_cast<breakpad_crash_generation_client*>(new CrashGenerationClient(pipe_handle,
                    dump_type, reinterpret_cast<const CustomClientInfo*>(custom_info)));
    }

    bool breakpad_crash_generation_client_register(breakpad_crash_generation_client* client) {
        return reinterpret_cast<CrashGenerationClient&>(client).Register();
    }

    bool breakpad_crash_generation_client_request_upload(breakpad_crash_generation_client* client, DWORD crash_id) {
        return reinterpret_cast<CrashGenerationClient&>(client).RequestUpload(crash_id);
    }

    bool breakpad_crash_generation_client_request_dump(breakpad_crash_generation_client* client, EXCEPTION_POINTERS* ex_info, breakpad_md_raw_assertion_info* assert_info) {
        return reinterpret_cast<CrashGenerationClient&>(client).RequestDump(ex_info, reinterpret_cast<MDRawAssertionInfo*>(assert_info));
    }

    breakpad_crash_generation_server* breakpad_crash_generation_server_create(const wchar_t* pipe_name,
            SECURITY_ATTRIBUTES* pipe_sec_attrs,
            breakpad_on_client_connected_cb connect_cb,
            void* connect_context,
            breakpad_on_client_dump_request_cb dump_callback,
            void* dump_context,
            breakpad_on_client_exited_cb exit_callback,
            void* exit_context,
            breakpad_on_client_upload_request_cb upload_request_callback,
            void* upload_context,
            bool generate_dumps,
            const wchar_t* dump_path) {

        ServerWrapperContext* cont = new ServerWrapperContext(dump_callback, exit_callback, upload_request_callback, connect_cb,
                dump_context, exit_context, upload_context, connect_context);

        return reinterpret_cast<breakpad_crash_generation_server*>(new CrashGenerationServer(wstring(pipe_name),
                    pipe_sec_attrs, connect_callback_wrapper, cont, dump_request_callback_wrapper, cont,
                    exit_callback_wrapper, cont, upload_callback_wrapper, cont, generate_dumps, new wstring(dump_path)));
    }

    bool breakpad_crash_generation_server_start(breakpad_crash_generation_server* server) {
        return reinterpret_cast<CrashGenerationServer&>(server).Start();
    }

    void breakpad_crash_generation_server_pre_fetch_custom_info(breakpad_crash_generation_server* server, bool do_pre_fetch) {
        return reinterpret_cast<CrashGenerationServer&>(server).pre_fetch_custom_info(do_pre_fetch);
    }
}
