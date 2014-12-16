#ifndef DUKCURL_H
#define DUKCURL_H

#include "duktape.h"

// The refs system is from dukluv.  The refs.c file can be linked in here
// directly if dukluv isn't used.
#ifndef DUV_REFS_H
#define DUV_REFS_H

// Create a global array refs in the heap stash.
void duv_ref_setup(duk_context *ctx);
// like luaL_ref, but assumes storage in "refs" property of heap stash
int duv_ref(duk_context *ctx);
void duv_push_ref(duk_context *ctx, int ref);
void duv_unref(duk_context *ctx, int ref);

#endif


#include <curl/curl.h>

duk_ret_t dukopen_curl(duk_context *ctx);

#endif
