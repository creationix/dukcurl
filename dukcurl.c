#include "dukcurl.h"

#ifndef bool
  typedef enum { false, true } bool;
#endif

#define DIPROP_CURL "\xff\xff" "curl"

static char error_buf[CURL_ERROR_SIZE];


// Helper to verify item at index is curl instance and get it's pointer
static CURL* dcurl_require_pointer(duk_context *ctx, int index) {
  CURL *curl;
  duk_get_prop_string(ctx, index, DIPROP_CURL);
  curl = duk_get_pointer(ctx, -1);
  duk_pop(ctx);
  if (!curl) {
    duk_error(ctx, DUK_ERR_TYPE_ERROR, "Expected Curl at index %d", index);
    return NULL;
  }
  return curl;
}

static CURL* dcurl_check(duk_context *ctx) {
  CURL *curl;
  duk_push_this(ctx);
  curl = dcurl_require_pointer(ctx, -1);
  duk_pop(ctx);
  return curl;
}

static void dcurl_push(duk_context *ctx, CURL* curl) {
  // Create a new instance of CurlPrototype
  duk_push_object(ctx);
  duk_get_global_string(ctx, "CurlPrototype");
  duk_set_prototype(ctx, -2);

  // Store the pointer inside it
  duk_push_pointer(ctx, curl);
  duk_put_prop_string(ctx, -2, DIPROP_CURL);
}

// Create a new curl instance
static duk_ret_t dcurl_easy_init(duk_context *ctx) {
  CURL *curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buf);
  dcurl_push(ctx, curl);
  return 1;
}

// Finalizer that's called when a curl instance is garbage collected.
static duk_ret_t dcurl_easy_cleanup(duk_context *ctx) {
  curl_easy_cleanup(dcurl_require_pointer(ctx, 0));
  return 0;
}


#define charinfo(name, constant)      \
  if (strcmp(str, name) == 0) {       \
    char* str = NULL;                 \
    if (curl_easy_getinfo(curl,       \
        constant, str)) {             \
      goto fail;                      \
    }                                 \
    duk_push_string(ctx, str);        \
    return 1;                         \
  }

#define longinfo(name, constant)      \
  if (strcmp(str, name) == 0) {       \
    long num = 0;                     \
    if (curl_easy_getinfo(curl,       \
        constant, &num)) {            \
      goto fail;                      \
    }                                 \
    duk_push_int(ctx, num);           \
    return 1;                         \
  }

#define doubleinfo(name, constant)    \
  if (strcmp(str, name) == 0) {       \
    double num = 0;                   \
    if (curl_easy_getinfo(curl,       \
        constant, &num)) {            \
      goto fail;                      \
    }                                 \
    duk_push_number(ctx, num);        \
    return 1;                         \
  }

#define OPT(type, name, constant) \
  if (strcmp(str, name) == 0) {   \
    opt = constant;               \
    goto process##type;           \
  }

static duk_ret_t dcurl_easy_setopt(duk_context *ctx) {
  CURL *curl = dcurl_check(ctx);

  const char *str = duk_require_string(ctx, 0);
  CURLoption opt;

  // BEHAVIOR OPTIONS
  OPT(bool, "verbose", CURLOPT_VERBOSE)
  OPT(bool, "header", CURLOPT_HEADER)
  OPT(bool, "noprogress", CURLOPT_NOPROGRESS)
  OPT(bool, "wildcardmatch", CURLOPT_WILDCARDMATCH)

  // NETWORK OPTIONS
  OPT(char, "url", CURLOPT_URL)
  OPT(char, "proxy", CURLOPT_PROXY)
  OPT(long, "proxyport", CURLOPT_PROXYPORT)
  OPT(char, "noproxy", CURLOPT_NOPROXY)
  OPT(long, "httpproxytunnel", CURLOPT_HTTPPROXYTUNNEL)

  OPT(char, "interface", CURLOPT_INTERFACE)
  OPT(long, "localport", CURLOPT_LOCALPORT)
  OPT(long, "localportrange", CURLOPT_LOCALPORTRANGE)
  OPT(long, "dns-cache-timeout", CURLOPT_DNS_CACHE_TIMEOUT)
  OPT(long, "buffersize", CURLOPT_BUFFERSIZE)
  OPT(long, "port", CURLOPT_PORT)
  OPT(bool, "tcp-nodelay", CURLOPT_TCP_NODELAY)
  OPT(long, "address_scope", CURLOPT_ADDRESS_SCOPE)
  OPT(bool, "tcp-keepalive", CURLOPT_TCP_KEEPALIVE)
  OPT(long, "tcp-keepidle", CURLOPT_TCP_KEEPIDLE)
  OPT(long, "tcp-keepintvl", CURLOPT_TCP_KEEPINTVL)
#ifdef CURLOPT_UNIX_SOCKET_PATH
  OPT(char, "unix-socket-path", CURLOPT_UNIX_SOCKET_PATH)
#endif

  // Auth options
  OPT(char, "userpwd", CURLOPT_USERPWD)
  OPT(char, "proxyuserpwd", CURLOPT_PROXYUSERPWD)
  OPT(char, "username", CURLOPT_USERNAME)
  OPT(char, "password", CURLOPT_PASSWORD)
  OPT(char, "login_options", CURLOPT_LOGIN_OPTIONS)
  OPT(char, "proxyusername", CURLOPT_PROXYUSERNAME)
  OPT(char, "proxypassword", CURLOPT_PROXYPASSWORD)


  OPT(bool, "nobody", CURLOPT_NOBODY)
  OPT(long, "infilesize", CURLOPT_INFILESIZE)
  OPT(bool, "upload", CURLOPT_UPLOAD)
  OPT(bool, "ssl-verifypeer", CURLOPT_SSL_VERIFYPEER)
  OPT(long, "ssl-verifyhost", CURLOPT_SSL_VERIFYHOST)

  // HTTP Options
  OPT(bool, "autoreferer", CURLOPT_AUTOREFERER)
  OPT(char, "accept-encoding", CURLOPT_ACCEPT_ENCODING)
  OPT(bool, "transfer-encoding", CURLOPT_TRANSFER_ENCODING)
  OPT(bool, "followlocation", CURLOPT_FOLLOWLOCATION)
  OPT(bool, "unrestricted-auth", CURLOPT_UNRESTRICTED_AUTH)
  OPT(long, "maxredirs", CURLOPT_MAXREDIRS)
  OPT(long, "post", CURLOPT_POST)
  OPT(char, "postfields", CURLOPT_POSTFIELDS)
  OPT(long, "postfieldsize", CURLOPT_POSTFIELDSIZE)
  OPT(char, "referer", CURLOPT_REFERER)
  OPT(char, "useragent", CURLOPT_USERAGENT)
  OPT(curl_slist, "httpheader", CURLOPT_HTTPHEADER)
  OPT(char, "customrequest", CURLOPT_CUSTOMREQUEST)
  OPT(char, "cookie", CURLOPT_COOKIE)
  OPT(bool, "http-content-decoding", CURLOPT_HTTP_CONTENT_DECODING)
  OPT(bool, "http-transfer-decoding", CURLOPT_HTTP_TRANSFER_DECODING)

  duk_error(ctx, DUK_ERR_REFERENCE_ERROR, "Unknown or unsupported curlopt");
  return 0;

  processchar: {
    if (curl_easy_setopt(curl, opt, (char*)duk_require_string(ctx, 1))) goto fail;
    return 0;
  }

  processlong: {
    if (curl_easy_setopt(curl, opt, (long)duk_require_int(ctx, 1))) goto fail;
    return 0;
  }

  processbool: {
    if (curl_easy_setopt(curl, opt, (long)duk_require_boolean(ctx, 1))) goto fail;
    return 0;
  }

  processcurl_slist: {
    int i, l;
    struct curl_slist *slist = NULL;

    if (!duk_is_array(ctx, 1)) {
      duk_error(ctx, DUK_ERR_TYPE_ERROR, "Expected array of strings");
      return 0;
    }
    l = duk_get_length(ctx, 1);
    for (i = 0; i < l; ++i) {
      duk_get_prop_index(ctx, 1, i);
      slist = curl_slist_append(slist, duk_get_string(ctx, -1));
      duk_pop(ctx);
    }
    if (curl_easy_setopt(curl, opt, slist)) goto fail;
    return 0;
  }

  fail: {
    duk_error(ctx, DUK_ERR_ERROR, error_buf);
    return 0;
  }
}

static duk_ret_t dcurl_easy_perform(duk_context *ctx) {
  CURL *curl = dcurl_check(ctx);

  if(curl_easy_perform(curl)) {
    duk_error(ctx, DUK_ERR_ERROR, error_buf);
  }
  return 0;
}

#define INFO(type, name, constant) \
  if (strcmp(str, name) == 0) {    \
    info = constant;               \
    goto process##type;            \
  }

static duk_ret_t dcurl_easy_getinfo(duk_context *ctx) {
  CURL *curl = dcurl_check(ctx);
  const char *str = duk_require_string(ctx, 0);
  CURLINFO info;

  INFO(char, "effective-url", CURLINFO_EFFECTIVE_URL)
  INFO(long, "response-code", CURLINFO_RESPONSE_CODE)
  INFO(long, "http-connectcode", CURLINFO_HTTP_CONNECTCODE)
  INFO(long, "filetime", CURLINFO_FILETIME)
  INFO(double, "total-time", CURLINFO_TOTAL_TIME)
  INFO(double, "namelookup-time", CURLINFO_NAMELOOKUP_TIME)
  INFO(double, "connect-time", CURLINFO_CONNECT_TIME)
  INFO(double, "appconnect-time", CURLINFO_APPCONNECT_TIME)
  INFO(double, "pretransfer-time", CURLINFO_PRETRANSFER_TIME)
  INFO(double, "starttransfer-time", CURLINFO_STARTTRANSFER_TIME)
  INFO(double, "redirect-time", CURLINFO_REDIRECT_TIME)
  INFO(long, "redirect-count", CURLINFO_REDIRECT_COUNT)
  INFO(char, "redirect-url", CURLINFO_REDIRECT_URL)
  INFO(double, "size_upload", CURLINFO_SIZE_UPLOAD)
  INFO(double, "size_download", CURLINFO_SIZE_DOWNLOAD)
  INFO(double, "speed_download", CURLINFO_SPEED_DOWNLOAD)
  INFO(double, "speed_upload", CURLINFO_SPEED_UPLOAD)
  INFO(long, "header_size", CURLINFO_HEADER_SIZE)
  INFO(long, "request_size", CURLINFO_REQUEST_SIZE)
  INFO(long, "ssl_verifyresult", CURLINFO_SSL_VERIFYRESULT)
  INFO(double, "content_length_download", CURLINFO_CONTENT_LENGTH_DOWNLOAD)
  INFO(double, "content_length_upload", CURLINFO_CONTENT_LENGTH_UPLOAD)
  INFO(char, "content_type", CURLINFO_CONTENT_TYPE)
  INFO(char, "private", CURLINFO_PRIVATE)
  INFO(long, "os_errno", CURLINFO_OS_ERRNO)
  INFO(long, "num_connects", CURLINFO_NUM_CONNECTS)
  INFO(char, "primary_ip", CURLINFO_PRIMARY_IP)
  INFO(long, "primary_port", CURLINFO_PRIMARY_PORT)
  INFO(char, "local_ip", CURLINFO_LOCAL_IP)
  INFO(long, "local_port", CURLINFO_LOCAL_PORT)

  duk_error(ctx, DUK_ERR_REFERENCE_ERROR, "Unknown or unsupported curlinfo");
  return 0;

  processchar: {
    char* str = NULL;
    if (curl_easy_getinfo(curl, info, str)) goto fail;
    duk_push_string(ctx, str);
    return 1;
  }

  processlong: {
    long num = 0;
    if (curl_easy_getinfo(curl, info, &num)) goto fail;
    duk_push_int(ctx, num);
    return 1;
  }

  processdouble: {
    double num = 0;
    if (curl_easy_getinfo(curl, info, &num)) goto fail;
    duk_push_int(ctx, num);
    return 1;
  }

  fail: {
    duk_error(ctx, DUK_ERR_ERROR, error_buf);
    return 0;
  }
}

static duk_ret_t dcurl_easy_duphandle(duk_context *ctx) {
  CURL *curl = dcurl_check(ctx);
  CURL *dup = curl_easy_duphandle(curl);
  dcurl_push(ctx, dup);
  return 1;
}

static duk_ret_t dcurl_easy_reset(duk_context *ctx) {
  CURL *curl = dcurl_check(ctx);
  curl_easy_reset(curl);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buf);
  return 0;
}

static const duk_function_list_entry dcurl_easy_methods[] = {
  {"setopt", dcurl_easy_setopt, 2},
  {"perform", dcurl_easy_perform, 0},
  {"getinfo", dcurl_easy_getinfo, 1},
  {"duphandle", dcurl_easy_duphandle, 0},
  {"reset", dcurl_easy_reset, 0},
  {NULL, NULL, 0},
};

duk_ret_t dukopen_curl(duk_context *ctx) {

  // Create the handle prototype as global CurlPrototype
  duk_push_object(ctx);
  duk_push_c_function(ctx, dcurl_easy_cleanup, 0);
  duk_set_finalizer(ctx, -1);
  duk_put_function_list(ctx, -1, dcurl_easy_methods);
  duk_put_global_string(ctx, "CurlPrototype");

  // Push init as the module itself
  duk_push_c_function(ctx, dcurl_easy_init, 0);
  return 1;
}
