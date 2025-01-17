#ifndef SHIRO_MULTIPARTPARSER_H
#define SHIRO_MULTIPARTPARSER_H

// Disable all warnings from gcc/clang:
#if defined(__clang__)
#pragma clang system_header
#elif defined(__GNUC__)
#pragma GCC system_header
#endif

#include <functional>

typedef struct multipartparser multipartparser;
typedef struct multipartparser_callbacks multipartparser_callbacks;

typedef std::function<int(multipartparser*)> multipart_cb;
typedef std::function<int(multipartparser*, const char*, size_t)> multipart_data_cb;

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

struct multipartparser {
    /** PRIVATE **/
    char        boundary[70];
    int         boundary_length;
    int         index;
    uint16_t    state;

    /** PUBLIC **/
    void* data;
};

struct multipartparser_callbacks {
    multipart_cb      on_body_begin;
    multipart_cb      on_part_begin;
    multipart_data_cb on_header_field;
    multipart_data_cb on_header_value;
    multipart_cb      on_headers_complete;
    multipart_data_cb on_data;
    multipart_cb      on_part_end;
    multipart_cb      on_body_end;
};

void multipartparser_init(multipartparser* parser, const char* boundary);

void multipartparser_callbacks_init(multipartparser_callbacks* callbacks);

size_t multipartparser_execute(multipartparser* parser,
                               multipartparser_callbacks* callbacks,
                               const char* data,
                               size_t size);

#ifdef __cplusplus
}
#endif
#endif // SHIRO_MULTIPARTPARSER_H
