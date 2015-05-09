#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <signal.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif
    typedef struct breakpad_minidump_descriptor breakpad_minidump_descriptor;
    typedef struct breakpad_eh breakpad_eh;
    typedef struct breakpad_crash_generation_client breakpad_crash_generation_client;
    typedef struct breakpad_crash_generation_server breakpad_crash_generation_server;
    typedef struct breakpad_client_info breakpad_client_info;

    typedef bool (*breakpad_filter_cb)(void* context);
    typedef bool (*breakpad_minidump_cb)(const breakpad_minidump_descriptor* minidump_descriptor, void* context, bool succeeded);
    typedef bool (*breakpad_handler_cb)(const void* crash_context, size_t crash_context_size, void* context);

    typedef bool (*breakpad_on_client_dump_request_cb)(void* context,
                                                       const breakpad_client_info* client_info,
                                                       const char* file_path);
    typedef bool (*breakpad_on_client_exiting_cb)(void* context,
                                                 const breakpad_client_info* client_info);



    breakpad_eh* breakpad_eh_create(const breakpad_minidump_descriptor* descriptor,
                                    breakpad_filter_cb filter,
                                    breakpad_minidump_cb cb,
                                    void* cb_context,
                                    bool install_handler,
                                    int server_fd);

    void breakpad_eh_destroy(breakpad_eh *eh);

    const breakpad_minidump_descriptor* breakpad_eh_minidump_descriptor(const breakpad_eh* eh);

    void breakpad_eh_set_minidump_descriptor(breakpad_eh* eh, const breakpad_minidump_descriptor* descriptor);

    void breakpad_eh_set_crash_handler(breakpad_eh* eh, breakpad_handler_cb cb);

    void breakpad_eh_set_crash_generation_client(breakpad_eh *eh, breakpad_crash_generation_client* client);

    bool breakpad_eh_write_minidump(breakpad_eh *eh);

    void breakpad_eh_add_mapping_info(breakpad_eh *eh,
                                      const char* name,
                                      const uint8_t identifier[16],
                                      uintptr_t start_address,
                                      size_t mapping_size,
                                      size_t file_offset);

    void breakpad_eh_register_app_memory(breakpad_eh *eh,
                                         void* ptr,
                                         size_t len);

    void breakpad_eh_unregister_app_memory(breakpad_eh *eh, void* ptr);

    void breakpad_eh_handle_signal(breakpad_eh *eh, int sig, siginfo_t* info, void* uc);

    breakpad_minidump_descriptor* breakpad_minidump_descriptor_create_from_path(const char* directory);

    breakpad_minidump_descriptor* breakpad_minidump_descriptor_create_from_fd(int fd);

    breakpad_minidump_descriptor* breakpad_minidump_descriptor_create_microdump_on_console();

    bool breakpad_minidump_descriptor_is_fd(const breakpad_minidump_descriptor* descriptor);

    int breakpad_minidump_descriptor_get_fd(const breakpad_minidump_descriptor* descriptor);

    const char* breakpad_minidump_descriptor_get_path(const breakpad_minidump_descriptor* descriptor);

    bool breakpad_minidump_descriptor_is_microdomp_on_console(const breakpad_minidump_descriptor* descriptor);

    void breakpad_minidump_descriptor_set_size_limit(breakpad_minidump_descriptor* descriptor, off_t limit);

    off_t breakpad_minidump_descriptor_get_size_limit(const breakpad_minidump_descriptor* descriptor);

    breakpad_crash_generation_client* breakpad_crash_generation_client_try_create(int server_fd);

    bool breakpad_crash_generation_client_request_dump(breakpad_crash_generation_client* client, const void* blob, size_t blob_size);

    breakpad_crash_generation_server* breakpad_crash_generation_server_create(int listen_fd,
                                                                              breakpad_on_client_dump_request_cb dump_cb,
                                                                              void* dump_context,
                                                                              breakpad_on_client_exiting_cb exit_cb,
                                                                              void* exit_context,
                                                                              bool generate_dumps,
                                                                              const char* dump_path);

    bool breakpad_crash_generation_server_create_report_channel(int* server_fd, int* client_fd);

    void breakpad_crash_generation_server_destroy(breakpad_crash_generation_server* server);

    bool breakpad_crash_generation_server_start(breakpad_crash_generation_server* server);

    void breakpad_crash_generation_server_stop(breakpad_crash_generation_server* server);
#ifdef __cplusplus
}
#endif
