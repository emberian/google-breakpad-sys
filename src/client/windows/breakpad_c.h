#include <stdlib.h>
#include <windows.h>
#include <dbghelp.h>
#include <rpc.h>

#ifdef __cplusplus
extern "C" {
#endif
    enum breakpad_eh_handler_type {
        HANDLER_NONE = 0,
        HANDLER_EXCEPTION = 1 << 0,          // SetUnhandledExceptionFilter
        HANDLER_INVALID_PARAMETER = 1 << 1,  // _set_invalid_parameter_handler
        HANDLER_PURECALL = 1 << 2,           // _set_purecall_handler
        HANDLER_ALL = HANDLER_EXCEPTION |
            HANDLER_INVALID_PARAMETER |
            HANDLER_PURECALL
    };

    typedef struct breakpad_eh breakpad_eh;
    typedef struct breakpad_crash_generation_client breakpad_crash_generation_client;
    typedef struct breakpad_crash_generation_server breakpad_crash_generation_server;
    typedef struct breakpad_custom_client_info breakpad_custom_client_info;
    typedef struct breakpad_client_info breakpad_client_info;
	typedef struct breakpad_md_raw_assertion_info breakpad_md_raw_assertion_info;

	typedef bool(*breakpad_filter_cb)(void* context, EXCEPTION_POINTERS* exinfo, breakpad_md_raw_assertion_info* assertion);
    typedef bool (*breakpad_minidump_cb)(const wchar_t* dump_path, const wchar_t* minidump_id, void* context, EXCEPTION_POINTERS* exinfo,
		breakpad_md_raw_assertion_info* assertion, bool succeeded);

    typedef void (*breakpad_on_client_connected_cb)(void* context,
            const breakpad_client_info* client_info);

    typedef void (*breakpad_on_client_dump_request_cb)(void* context,
            const breakpad_client_info* client_info,
            const wchar_t* file_path);

    typedef void (*breakpad_on_client_exited_cb)(void* context,
            const breakpad_client_info* client_info);

    typedef void (*breakpad_on_client_upload_request_cb)(void* context,
            const DWORD crash_id);

    breakpad_eh* breakpad_eh_create_in_process(const wchar_t* dump_path,
            breakpad_filter_cb filter, breakpad_minidump_cb callback,
            void* callback_context, int handler_types);

    breakpad_eh* breakpad_eh_create_try_out_of_process(const wchar_t* dump_path,
            breakpad_filter_cb filter, breakpad_minidump_cb callback,
            void* callback_context, int handler_types, MINIDUMP_TYPE dump_type,
            HANDLE pipe_handle, const breakpad_custom_client_info* custom_info);

    breakpad_eh* breakpad_eh_create_out_of_process(const wchar_t* dump_path,
            breakpad_filter_cb filter, breakpad_minidump_cb callback,
            void* callback_context, int handler_types, breakpad_crash_generation_client* client);

    void breakpad_eh_destroy(breakpad_eh *eh);

    const wchar_t* breakpad_eh_get_dump_path(const breakpad_eh* eh);

    void breakpad_eh_set_dump_path(breakpad_eh* eh, const wchar_t* dumppath);

    bool breakpad_eh_request_upload(breakpad_eh* eh, DWORD crash_id);

    bool breakpad_eh_write_minidump_for_exception(breakpad_eh* eh, EXCEPTION_POINTERS* exinfo);

    DWORD breakpad_eh_get_requesting_thread_id(const breakpad_eh* eh);

    bool breakpad_eh_get_handle_debug_exceptions(const breakpad_eh* eh);

    void breakpad_eh_set_handle_debug_exceptions(breakpad_eh* eh, bool handle_debug_exceptions);

    bool breakpad_eh_get_consume_invalid_handle_exceptions(const breakpad_eh* eh);

    void breakpad_eh_set_consume_invalid_handle_exceptions(breakpad_eh* eh, bool consume_invalid_handle_exceptions);

    bool breakpad_eh_is_out_of_process(const breakpad_eh* eh);

    void breakpad_eh_register_app_memory(breakpad_eh *eh,
                                         void* ptr,
                                         size_t len);

    void breakpad_eh_unregister_app_memory(breakpad_eh *eh, void* ptr);

    breakpad_crash_generation_client* breakpad_crash_generation_client_create(HANDLE pipe_handle,
            MINIDUMP_TYPE dump_type,
            const breakpad_custom_client_info* custom_info);

    bool breakpad_crash_generation_client_register(breakpad_crash_generation_client* client);

    bool breakpad_crash_generation_client_request_upload(breakpad_crash_generation_client* client, DWORD crash_id);

	bool breakpad_crash_generation_client_request_dump(breakpad_crash_generation_client* client, EXCEPTION_POINTERS* ex_info, breakpad_md_raw_assertion_info* assert_info);

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
            const wchar_t* dump_path);

    bool breakpad_crash_generation_server_start(breakpad_crash_generation_server* server);

    void breakpad_crash_generation_server_pre_fetch_custom_info(breakpad_crash_generation_server* server, bool do_pre_fetch);

#ifdef __cplusplus
}
#endif
