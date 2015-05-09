#include "client/linux/handler/exception_handler.h"
#include "client/linux/handler/minidump_descriptor.h"
#include "client/linux/crash_generation/crash_generation_client.h"
#include "client/linux/crash_generation/crash_generation_server.h"

#include "breakpad_c.h"

using namespace google_breakpad;


struct HandlerWrapperContext {
    breakpad_minidump_cb mcb;
    breakpad_filter_cb fcb;
    void* real_context;

    HandlerWrapperContext(breakpad_filter_cb fcb, breakpad_minidump_cb mcb, void *context) : mcb(mcb), fcb(fcb), real_context(context) { }
};

static bool filter_callback_wrapper(void* context) {
    HandlerWrapperContext* cont = reinterpret_cast<HandlerWrapperContext*>(context);

    return cont->fcb ? cont->fcb(cont->real_context) : true;
}

static bool minidump_callback_wrapper(const MinidumpDescriptor& desc, void* context, bool succeeded) {
    HandlerWrapperContext* cont = reinterpret_cast<HandlerWrapperContext*>(context);

    return cont->mcb ? cont->mcb(reinterpret_cast<const breakpad_minidump_descriptor*>(&desc), cont->real_context, succeeded) : succeeded;
}

struct ServerWrapperContext {
    breakpad_on_client_dump_request_cb dump_cb;
    breakpad_on_client_exiting_cb exit_cb;
    void* dump_context;
    void* exit_context;

    ServerWrapperContext(breakpad_on_client_dump_request_cb dump_cb, breakpad_on_client_exiting_cb exit_cb, void* dump_context, void* exit_context) : dump_cb(dump_cb), exit_cb(exit_cb), dump_context(dump_context), exit_context(exit_context) { }
};

static void exit_callback_wrapper(void* context, const ClientInfo* client_info) {
    ServerWrapperContext* cont = reinterpret_cast<ServerWrapperContext*>(context);
    if (NULL != cont->exit_cb) {
        cont->exit_cb(cont->exit_context, reinterpret_cast<const breakpad_client_info*>(client_info));
    }
}

static void dump_request_callback_wrapper(void* context, const ClientInfo* client_info, const string* file_path) {
    ServerWrapperContext* cont = reinterpret_cast<ServerWrapperContext*>(context);
    if (NULL != cont->dump_cb) {
        cont->dump_cb(cont->dump_context, reinterpret_cast<const breakpad_client_info*>(client_info), file_path ? file_path->c_str() : NULL);
    }
}

extern "C" {
    /*
    typedef struct breakpad_minidump_descriptor {
        MinidumpDescriptor desc;
    } breakpad_minidump_descriptor;

    typedef struct breakpad_eh {
        ExceptionHandler eh;
    } breakpad_eh;

    typedef struct breakpad_crash_generation_client {
        CrashGenerationClient clent;
    } breakpad_crash_generation_client;

    typedef struct breakpad_crash_generation_server {
        CrashGenerationServer server;
    } breakpad_crash_generationserver;
    */

    breakpad_eh* breakpad_eh_create(const breakpad_minidump_descriptor* descriptor,
                                    breakpad_filter_cb filter,
                                    breakpad_minidump_cb cb,
                                    void* cb_context,
                                    bool install_handler,
                                    int server_fd) {
        if (NULL == descriptor) {
            return NULL;
        }
        return reinterpret_cast<breakpad_eh*>(new
                ExceptionHandler(reinterpret_cast<const MinidumpDescriptor&>(descriptor),
                                 filter_callback_wrapper,
                                 minidump_callback_wrapper,
                                 new HandlerWrapperContext(filter, cb, cb_context),
                                 install_handler,
                                 server_fd));
    }

    void breakpad_eh_destroy(breakpad_eh *eh) {
        if (NULL == eh) {
            return;
        }
        delete reinterpret_cast<ExceptionHandler*>(eh);
    }

    const breakpad_minidump_descriptor* breakpad_eh_minidump_descriptor(const breakpad_eh* eh) {
        if (NULL == eh) {
            return NULL;
        }
        return reinterpret_cast<const breakpad_minidump_descriptor*>(&reinterpret_cast<const ExceptionHandler*>(eh)->minidump_descriptor());
    }

    void breakpad_eh_set_minidump_descriptor(breakpad_eh* eh, const breakpad_minidump_descriptor* descriptor) {
        if (NULL == eh) {
            return;
        }
        assert(NULL != descriptor);
        reinterpret_cast<ExceptionHandler*>(eh)->set_minidump_descriptor(reinterpret_cast<const MinidumpDescriptor&>(descriptor));
    }

    /*
    void breakpad_eh_set_crash_handler(breakpad_eh* eh, breakpad_handler_cb cb) {
        if (NULL == eh) {
            return;
        }
        reinterpret_
    }
    */

    void breakpad_eh_set_crash_generation_client(breakpad_eh *eh, breakpad_crash_generation_client* client) {
        if (NULL == eh) {
            return;
        }
        reinterpret_cast<ExceptionHandler*>(eh)->set_crash_generation_client(reinterpret_cast<CrashGenerationClient*>(client));
    }

    bool breakpad_eh_write_minidump(breakpad_eh *eh) {
        if (NULL == eh) {
            return false;
        }
        return reinterpret_cast<ExceptionHandler*>(eh)->WriteMinidump();
    }

    void breakpad_eh_add_mapping_info(breakpad_eh *eh,
                                      const char* name,
                                      const uint8_t identifier[sizeof(MDGUID)],
                                      uintptr_t start_address,
                                      size_t mapping_size,
                                      size_t file_offset) {
        if (NULL == eh) {
            return;
        }
        reinterpret_cast<ExceptionHandler*>(eh)->AddMappingInfo(string(name), identifier, start_address, mapping_size, file_offset);
    }

    void breakpad_eh_register_app_memory(breakpad_eh *eh,
                                         void* ptr,
                                         size_t len) {
        if (NULL == eh) {
            return;
        }
        reinterpret_cast<ExceptionHandler*>(eh)->RegisterAppMemory(ptr, len);
    }

    void breakpad_eh_unregister_app_memory(breakpad_eh *eh, void* ptr) {
        if (NULL == eh) {
            return;
        }
        reinterpret_cast<ExceptionHandler*>(eh)->UnregisterAppMemory(ptr);
    }

    void breakpad_eh_handle_signal(breakpad_eh *eh, int sig, siginfo_t* info, void* uc) {
        if (NULL == eh) {
            return;
        }
        reinterpret_cast<ExceptionHandler*>(eh)->HandleSignal(sig, info, uc);
    }

    breakpad_minidump_descriptor* breakpad_minidump_descriptor_create_from_path(const char* directory) {
        return reinterpret_cast<breakpad_minidump_descriptor*>(new MinidumpDescriptor(string(directory)));
    }

    breakpad_minidump_descriptor* breakpad_minidump_descriptor_create_from_fd(int fd) {
        return reinterpret_cast<breakpad_minidump_descriptor*>(new MinidumpDescriptor(fd));
    }

    breakpad_minidump_descriptor* breakpad_minidump_descriptor_create_microdump_on_console() {
        return reinterpret_cast<breakpad_minidump_descriptor*>(new MinidumpDescriptor(MinidumpDescriptor::kMicrodumpOnConsole));
    }

    bool breakpad_minidump_descriptor_is_fd(const breakpad_minidump_descriptor* descriptor) {
        if (NULL == descriptor) {
            return false;
        }
        return reinterpret_cast<const MinidumpDescriptor&>(descriptor).IsFD();
    }

    int breakpad_minidump_descriptor_get_fd(const breakpad_minidump_descriptor* descriptor) {
        if (NULL == descriptor) {
            return -1;
        }
        return reinterpret_cast<const MinidumpDescriptor&>(descriptor).fd();
    }

    const char* breakpad_minidump_descriptor_get_path(const breakpad_minidump_descriptor* descriptor) {
        if (NULL == descriptor) {
            return NULL;
        }
        return reinterpret_cast<const MinidumpDescriptor&>(descriptor).path();
    }

    bool breakpad_minidump_descriptor_is_microdomp_on_console(const breakpad_minidump_descriptor* descriptor) {
        if (NULL == descriptor) {
            return false;
        }
        return reinterpret_cast<const MinidumpDescriptor&>(descriptor).IsMicrodumpOnConsole();
    }

    void breakpad_minidump_descriptor_set_size_limit(breakpad_minidump_descriptor* descriptor, off_t limit) {
        if (NULL == descriptor) {
            return;
        }
        return reinterpret_cast<MinidumpDescriptor&>(descriptor).set_size_limit(limit);
    }

    off_t breakpad_minidump_descriptor_get_size_limit(const breakpad_minidump_descriptor* descriptor) {
        if (NULL == descriptor) {
            return 0;
        }
        return reinterpret_cast<const MinidumpDescriptor&>(descriptor).size_limit();
    }

    breakpad_crash_generation_client* breakpad_crash_generation_client_try_create(int server_fd) {
        return reinterpret_cast<breakpad_crash_generation_client*>(CrashGenerationClient::TryCreate(server_fd));
    }

    bool breakpad_crash_generation_client_request_dump(breakpad_crash_generation_client* client, const void* blob, size_t blob_size) {
        if (NULL == client) {
            return false;
        }
        return reinterpret_cast<CrashGenerationClient&>(client).RequestDump(blob, blob_size);
    }

    breakpad_crash_generation_server* breakpad_crash_generation_server_create(int listen_fd,
                                                                              breakpad_on_client_dump_request_cb dump_cb,
                                                                              void* dump_context,
                                                                              breakpad_on_client_exiting_cb exit_cb,
                                                                              void* exit_context,
                                                                              bool generate_dumps,
                                                                              const char* dump_path) {
        ServerWrapperContext* cont = new ServerWrapperContext(dump_cb, exit_cb, dump_context, exit_context);
        return reinterpret_cast<breakpad_crash_generation_server*>(
                new CrashGenerationServer(listen_fd,
                                          dump_request_callback_wrapper,
                                          cont,
                                          exit_callback_wrapper,
                                          cont,
                                          generate_dumps,
                                          new string(dump_path)));
    }

    bool breakpad_crash_generation_server_create_report_channel(int* server_fd, int* client_fd) {
        return CrashGenerationServer::CreateReportChannel(server_fd, client_fd);
    }

    void breakpad_crash_generation_server_destroy(breakpad_crash_generation_server* server) {
        delete reinterpret_cast<CrashGenerationServer*>(server);
    }

    bool breakpad_crash_generation_server_start(breakpad_crash_generation_server* server) {
        return reinterpret_cast<CrashGenerationServer*>(server)->Start();
    }

    void breakpad_crash_generation_server_stop(breakpad_crash_generation_server* server) {
        return reinterpret_cast<CrashGenerationServer*>(server)->Stop();
    }
}
