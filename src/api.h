/**
 * @author Marius Orcsik <marius@habarnam.ro>
 */
#ifndef MPRIS_SCROBBLER_API_H
#define MPRIS_SCROBBLER_API_H

#include <expat.h>
#include <inttypes.h>
#include <json-c/json.h>
#include <stdbool.h>
#include <openssl/md5.h>

#define MAX_HEADERS                     10
#define MAX_HEADER_LENGTH               256
#define MAX_URL_LENGTH                  2048
#define MAX_BODY_SIZE                   16384

#define CONTENT_TYPE_XML "application/xml"
#define CONTENT_TYPE_JSON "application/json"

#define API_XML_ROOT_NODE_NAME          "lfm"
#define API_XML_TOKEN_NODE_NAME         "token"
#define API_XML_SESSION_NODE_NAME       "session"
#define API_XML_NAME_NODE_NAME          "name"
#define API_XML_KEY_NODE_NAME           "key"
#define API_XML_SUBSCRIBER_NODE_NAME    "subscriber"
#define API_XML_NOWPLAYING_NODE_NAME    "nowplaying"
#define API_XML_SCROBBLES_NODE_NAME     "scrobbles"
#define API_XML_SCROBBLE_NODE_NAME      "scrobble"
#define API_XML_TIMESTAMP_NODE_NAME     "timestamp"
#define API_XML_TRACK_NODE_NAME         "track"
#define API_XML_ARTIST_NODE_NAME        "artist"
#define API_XML_ALBUM_NODE_NAME         "album"
#define API_XML_ALBUMARTIST_NODE_NAME   "albumArtist"
#define API_XML_IGNORED_NODE_NAME       "ignoredMessage"
#define API_XML_STATUS_ATTR_NAME        "status"
#define API_XML_STATUS_VALUE_OK         "ok"
#define API_XML_STATUS_VALUE_FAILED     "failed"
#define API_XML_ERROR_NODE_NAME         "error"
#define API_XML_ERROR_MESSAGE_NAME      "message"
#define API_XML_ERROR_CODE_ATTR_NAME    "code"

#define API_METHOD_NOW_PLAYING          "track.updateNowPlaying"
#define API_METHOD_SCROBBLE             "track.scrobble"
#define API_METHOD_GET_TOKEN            "auth.getToken"
#define API_METHOD_GET_SESSION          "auth.getSession"


#define LASTFM_API_KEY "990909c4e451d6c1ee3df4f5ee87e6f4"
#define LASTFM_API_SECRET "8bde8774564ef206edd9ef9722742a72"

#define LIBREFM_API_KEY "299dead99beef992"
#define LIBREFM_API_SECRET "c0ffee1511fe"

#define LISTENBRAINZ_API_KEY "299dead99beef992"
#define LISTENBRAINZ_API_SECRET "c0ffee1511fe"

typedef enum api_return_statuses {
    status_failed  = 0, // failed == bool false
    status_ok           // ok == bool true
} api_return_status;

struct xml_attribute {
    char *name;
    char *value;
};

struct xml_state {
    struct xml_node *doc;
    struct xml_node *current_node;
};

enum api_node_types {
    api_node_type_unknown,
    // all
    api_node_type_document,
    api_node_type_root,
    api_node_type_error,
    // auth.getToken
    api_node_type_token,
    // track.nowPlaying
    api_node_type_now_playing,
    api_node_type_track,
    api_node_type_artist,
    api_node_type_album,
    api_node_type_album_artist,
    api_node_type_ignored_message,
    // track.scrobble
    api_node_type_scrobbles,
    api_node_type_scrobble,
    api_node_type_timestamp,
    // auth.getSession
    api_node_type_session,
    api_node_type_session_name,
    api_node_type_session_key,
    api_node_type_session_subscriber,
} api_node_type;

struct xml_node {
    enum api_node_types type;
    char *name;
    struct xml_node *parent;
    size_t attributes_count;
    size_t children_count;
    size_t content_length;
    size_t name_length;
    char *content;
    struct xml_attribute *attributes[MAX_XML_ATTRIBUTES];
    struct xml_node *children[MAX_XML_NODES];
};

typedef enum api_return_codes {
    unavaliable             = 1, //The service is temporarily unavailable, please try again.
    invalid_service         = 2, //Invalid service - This service does not exist
    invalid_method          = 3, //Invalid Method - No method with that name in this package
    authentication_failed   = 4, //Authentication Failed - You do not have permissions to access the service
    invalid_format          = 5, //Invalid format - This service doesn't exist in that format
    invalid_parameters      = 6, //Invalid parameters - Your request is missing a required parameter
    invalid_resource        = 7, //Invalid resource specified
    operation_failed        = 8, //Operation failed - Something else went wrong
    invalid_session_key     = 9, //Invalid session key - Please re-authenticate
    invalid_apy_key         = 10, //Invalid API key - You must be granted a valid key by last.fm
    service_offline         = 11, //Service Offline - This service is temporarily offline. Try again later.
    invalid_signature       = 13, //Invalid method signature supplied
    temporary_error         = 16, //There was a temporary error processing your request. Please try again
    suspended_api_key       = 26, //Suspended API key - Access for your account has been suspended, please contact Last.fm
    rate_limit_exceeded     = 29, //Rate limit exceeded - Your IP has made too many requests in a short period
} api_return_code;

struct api_error {
    api_return_code code;
    char *message;
};

struct api_response {
    api_return_status status;
    struct api_error *error;
};

typedef enum message_types {
    api_call_authenticate,
    api_call_now_playing,
    api_call_scrobble,
} message_type;

struct http_header {
    char *name;
    char *value;
};

struct http_response {
    unsigned short code;
    struct xml_node *doc;
    char *body;
    size_t body_length;
    struct http_header *headers[MAX_HEADERS];
    size_t headers_count;
};

typedef enum http_request_types {
    http_get,
    http_post,
    http_put,
    http_head,
    http_patch,
} http_request_type;

struct api_endpoint {
    char *scheme;
    char *host;
    char *path;
};

struct http_request {
    http_request_type request_type;
    struct api_endpoint *end_point;
    char *query;
    char *body;
    size_t body_length;
    char *headers[MAX_HEADERS];
    size_t header_count;
};

#if 0
struct xml_node *xml_node_new(void);
static void xml_node_free(struct xml_node*);
struct xml_attribute *xml_attribute_new(const char*, size_t, const char*, size_t);
struct xml_node *xml_node_append_child(struct xml_node*, struct xml_node*);
static void XMLCALL begin_element(void *data, const char *element_name, const char **attributes)
{
    if (NULL == data) { return; }
    if (NULL == element_name) { return; }

    struct xml_state *state = data;

    struct xml_node *node = xml_node_new();
    _trace("xml::begin_element(%p:%p): %s", state, node, element_name);
    node->name_length = strlen(element_name);
    if (node->name_length == 0) {
        xml_node_free(node);
        return;
    }

    node->name = get_zero_string(node->name_length);
    strncpy(node->name, element_name, node->name_length);
    if (strncmp(element_name, API_XML_ROOT_NODE_NAME, strlen(API_XML_ROOT_NODE_NAME)) == 0) {
        node->type = api_node_type_root;
    }
    if (strncmp(element_name, API_XML_ERROR_NODE_NAME, strlen(API_XML_ERROR_NODE_NAME)) == 0) {
        node->type = api_node_type_error;
    }
    if (strncmp(element_name, API_XML_TOKEN_NODE_NAME, strlen(API_XML_TOKEN_NODE_NAME)) == 0) {
        node->type = api_node_type_token;
    }
    if (strncmp(element_name, API_XML_SESSION_NODE_NAME, strlen(API_XML_SESSION_NODE_NAME)) == 0) {
        node->type = api_node_type_session;
    }
    if (strncmp(element_name, API_XML_NAME_NODE_NAME, strlen(API_XML_NAME_NODE_NAME)) == 0) {
        node->type = api_node_type_session_name;
    }
    if (strncmp(element_name, API_XML_KEY_NODE_NAME, strlen(API_XML_KEY_NODE_NAME)) == 0) {
        node->type = api_node_type_session_key;
    }
    if (strncmp(element_name, API_XML_SUBSCRIBER_NODE_NAME, strlen(API_XML_SUBSCRIBER_NODE_NAME)) == 0) {
        node->type = api_node_type_session_subscriber;
    }
    if (strncmp(element_name, API_XML_NOWPLAYING_NODE_NAME, strlen(API_XML_NOWPLAYING_NODE_NAME)) == 0) {
        node->type = api_node_type_now_playing;
    }
    if (strncmp(element_name, API_XML_TRACK_NODE_NAME, strlen(API_XML_TRACK_NODE_NAME)) == 0) {
        node->type = api_node_type_track;
    }
    if (strncmp(element_name, API_XML_ARTIST_NODE_NAME, strlen(API_XML_ARTIST_NODE_NAME)) == 0) {
        node->type = api_node_type_artist;
    }
    if (strncmp(element_name, API_XML_ALBUM_NODE_NAME, strlen(API_XML_ALBUM_NODE_NAME)) == 0) {
        node->type = api_node_type_album_artist;
    }
    if (strncmp(element_name, API_XML_ALBUMARTIST_NODE_NAME, strlen(API_XML_ALBUMARTIST_NODE_NAME)) == 0) {
        node->type = api_node_type_album_artist;
    }
    if (strncmp(element_name, API_XML_IGNORED_NODE_NAME, strlen(API_XML_IGNORED_NODE_NAME)) == 0) {
        node->type = api_node_type_ignored_message;
    }
    if (strncmp(element_name, API_XML_SCROBBLES_NODE_NAME, strlen(API_XML_SCROBBLES_NODE_NAME)) == 0) {
        node->type = api_node_type_scrobbles;
    }
    if (strncmp(element_name, API_XML_SCROBBLE_NODE_NAME, strlen(API_XML_SCROBBLE_NODE_NAME)) == 0) {
        node->type = api_node_type_scrobble;
    }
    if (strncmp(element_name, API_XML_TIMESTAMP_NODE_NAME, strlen(API_XML_TIMESTAMP_NODE_NAME)) == 0) {
        node->type = api_node_type_timestamp;
    }
    for (int i = 0; attributes[i]; i += 2) {
        const char *name = attributes[i];
        const char *value = attributes[i+1];

        _trace("xml::element_attributes(%p) %s = %s", data, name, value);
        struct xml_attribute *attr = xml_attribute_new(name, strlen(name), value, strlen(value));
        if (NULL != attr) {
            node->attributes[node->attributes_count] = attr;
            node->attributes_count++;
        }
    }

    node->parent = state->current_node;
    _trace("xml::current_node::is(%p):%s", state->current_node, state->current_node->name);
    xml_node_append_child(state->current_node, node);
    state->current_node = node;
}

static void XMLCALL end_element(void *data, const char *element_name)
{
    if (NULL == data) { return; }
    struct xml_state *state = data;
    struct xml_node *current_node = state->current_node;
    _trace("xml::end_element(%p): %s", current_node, element_name);
#if 0
    if (strncmp(element_name, API_XML_ROOT_NODE_NAME, strlen(API_XML_ROOT_NODE_NAME)) == 0) {
        // lfm
    }
    if (strncmp(element_name, API_XML_ERROR_NODE_NAME, strlen(API_XML_ERROR_NODE_NAME)) == 0) {
        // error
    }
#endif
    // now that we build the node, we append it to either the document or the current node
    state->current_node = current_node->parent;
}

static void xml_state_free(struct xml_state *state)
{
    if (NULL == state) { return; }
    free(state);
}

static struct xml_state *xml_state_new(void)
{
    struct xml_state *state = malloc(sizeof(struct xml_state));
    state->doc = NULL;
    state->current_node = NULL;
    return state;
}

size_t string_trim(char**, size_t, const char*);
static void XMLCALL text_handle(void *data, const char *incoming, int length)
{
    if (length == 0) { return; }

    if (NULL == data) { return; }
    if (NULL == incoming) { return; }

    struct xml_state *state = data;
    if (NULL == state->current_node) { return; }

    char *local_cont = get_zero_string(length);
    strncpy(local_cont, incoming, length);
    size_t new_len = string_trim((char**)&local_cont, length, NULL);
    if (new_len == 0) {
        free(local_cont);
        return;
    } else {
        struct xml_node *node = state->current_node;

        if (node->type == api_node_type_error) { }

        node->content_length = length;
        node->content = get_zero_string(new_len);
        strncpy (node->content, local_cont, new_len);
        _trace("xml::text_handle(%p//%u):%s", node, new_len, node->content);
    }
    free(local_cont);
}

static void http_response_parse_xml_body(struct http_response *res, struct xml_node *document)
{
    if (NULL == res) { return; }
    if (res->code >= 500) { return; }

    struct xml_state *state = xml_state_new();
    state->doc = document;
    state->current_node = document;

    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser, state);
    XML_SetElementHandler(parser, begin_element, end_element);
    XML_SetCharacterDataHandler(parser, text_handle);

    if (XML_Parse(parser, res->body, res->body_length, XML_TRUE) == XML_STATUS_ERROR) {
        _warn("xml::parse_error");
    }

    XML_ParserFree(parser);
    xml_state_free(state);
}
#endif

#define HTTP_HEADER_CONTENT_TYPE "ContentType"
char *http_response_headers_content_type(struct http_response *res)
{

    for (size_t i = 0; i < res->headers_count; i++) {
        struct http_header *current = res->headers[i];

        if(strncmp(current->name, HTTP_HEADER_CONTENT_TYPE, strlen(HTTP_HEADER_CONTENT_TYPE)) == 0) {
             return current->value;
        }
    }
    return NULL;
}

#if 0
static void http_response_parse_json_body(struct http_response *res)
{
    if (NULL == res) { return; }
    if (res->code >= 500) { return; }

    json_object *obj = json_tokener_parse(res->body);
    if (NULL == obj) {
        _warn("json::parse_error");
    }
    if (json_object_array_length(obj) < 1) {
        _warn("json::invalid_json_message");
    }
    _trace("json::obj_cnt: %d", json_object_array_length(obj));
#if 0
    if (json_object_type(object, json_type_object)) {
        json_object_object_get_ex();
    }
#endif
}
#endif

static bool api_is_valid_doc(struct xml_node *doc)
{
    if (NULL == doc) { return false; }
    if (doc->children_count != 1) { return false; }

    struct xml_node *root = doc->children[0];
    if (NULL == root) { return false; }
    if (root->type != api_node_type_root) { return false; }
    if (root->children_count != 1) { return false; }

    return true;
}

static inline struct xml_node *api_get_root_node(struct xml_node *doc)
{
    return api_is_valid_doc(doc) ? doc->children[0] : NULL;
}

#if 0
char *api_response_get_name(struct xml_node *doc)
{
    struct xml_node *root = api_get_root_node(doc);
    return NULL;
}
#endif

const char *api_response_get_session_key_json(const char *buffer, const size_t length)
{
    const char *session_key = NULL;
    json_object *root = json_tokener_parse(buffer);
    if (NULL == root) {
        _warn("json::invalid_json_message");
        return NULL;
    }
    if (json_object_object_length(root) < 1) {
        _warn("json::no_root_object");
        return NULL;
    }
    json_object *sess_object = NULL;
    json_object_object_get_ex(root, API_XML_SESSION_NODE_NAME, &sess_object);
    if(NULL == sess_object) {
        _warn("json:missing_session_object");
        return NULL;
    }
    if(!json_object_is_type(sess_object, json_type_object)) {
        _warn("json::session_is_not_object");
        return NULL;
    }
    json_object *key_object = NULL;
    json_object_object_get_ex(sess_object, API_XML_KEY_NODE_NAME, &key_object);
    if(NULL == key_object) {
        _warn("json:missing_key");
        return NULL;
    }
    if(!json_object_is_type(key_object, json_type_string)) {
        _warn("json::key_is_not_string");
        return NULL;
    }
    session_key = json_object_get_string(key_object);
    _info("json::loaded_session_key: %s", session_key);

    return session_key;
}

char *api_response_get_session_key_xml(struct xml_node *doc)
{
    struct xml_node *root = api_get_root_node(doc);
    if (NULL == root) { return NULL; }
    if (root->children_count == 0) { return NULL; }

    struct xml_node *session = root->children[0];
    if (NULL == session) { return NULL; }
    if (session->type != api_node_type_session) { return NULL; }

    struct xml_node *key = NULL;
    for (size_t i = 0; i < session->children_count; i++) {
        struct xml_node *child = session->children[i];
        if (NULL == child) { continue; }
        if (child->type != api_node_type_session_key) { continue; }
        key = child;
    }
    if (NULL == key) { return NULL; }

    size_t len_session = strlen(key->content);
    char *response = get_zero_string(len_session);
    strncpy(response, key->content, len_session);
    return response;
}

const char *api_response_get_token_json(const char *buffer, const size_t length)
{
    // {"token":"NQH5C24A6RbIOx1xWUcty1N6yOHcKcRk"}
    const char *token = NULL;
    json_object *root = json_tokener_parse(buffer);
    if (NULL == root) {
        _warn("json::invalid_json_message");
        return NULL;
    }
    if (json_object_object_length(root) < 1) {
        _warn("json::no_root_object");
        return NULL;
    }
    json_object *tok_object;
    json_object_object_get_ex(root, API_XML_TOKEN_NODE_NAME, &tok_object);
    if(NULL == tok_object) {
        _warn("json:missing_token_key");
        return NULL;
    }
    if(!json_object_is_type(tok_object, json_type_string)) {
        _warn("json::token_is_not_string");
        return NULL;
    }
    token = json_object_get_string(tok_object);
    _info("json::loaded_token: %s", token);
    return token;
}

char *api_response_get_token_xml(struct xml_node *doc)
{
    struct xml_node *root = api_get_root_node(doc);
    if (root->children_count == 0) { return NULL; }

    struct xml_node *token = root->children[0];
    if (NULL == token) { return NULL; }
    if (token->type != api_node_type_token) { return NULL; }

    size_t len_token = strlen(token->content);
    char *response = get_zero_string(len_token);
    strncpy(response, token->content, len_token);
    return response;
}

static void api_endpoint_free(struct api_endpoint *api)
{
    free(api);
}

static struct api_endpoint *api_endpoint_new(enum api_type type)
{
    struct api_endpoint *result = malloc(sizeof(struct api_endpoint));
    result->scheme = "https";
    switch (type) {
        case  lastfm:
            result->host = LASTFM_API_BASE_URL;
            result->path = "/" LASTFM_API_VERSION "/";
            break;
        case librefm:
            result->host = LIBREFM_API_BASE_URL;
            result->path = "/" LIBREFM_API_VERSION "/";
            break;
        case listenbrainz:
            result->host = LISTENBRAINZ_API_BASE_URL;
            result->path = "/" LISTENBRAINZ_API_VERSION "/";
            break;
        case unknown:
        default:
            break;
    }

    return result;
}

#if 0
static const char *get_api_error_message(api_return_code code)
{
    switch (code) {
        case unavaliable:
            return "The service is temporarily unavailable, please try again.";
        case invalid_service:
            return "Invalid service - This service does not exist";
        case invalid_method:
            return "Invalid Method - No method with that name in this package";
        case authentication_failed:
            return "Authentication Failed - You do not have permissions to access the service";
        case invalid_format:
            return "Invalid format - This service doesn't exist in that format";
        case invalid_parameters:
            return "Invalid parameters - Your request is missing a required parameter";
        case invalid_resource:
            return "Invalid resource specified";
        case operation_failed:
            return "Operation failed - Something else went wrong";
        case invalid_session_key:
            return "Invalid session key - Please re-authenticate";
        case invalid_apy_key:
            return "Invalid API key - You must be granted a valid key by last.fm";
        case service_offline:
            return "Service Offline - This service is temporarily offline. Try again later.";
        case invalid_signature:
            return "Invalid method signature supplied";
        case temporary_error:
            return "There was a temporary error processing your request. Please try again";
        case suspended_api_key:
            return "Suspended API key - Access for your account has been suspended, please contact Last.fm";
        case rate_limit_exceeded:
            return "Rate limit exceeded - Your IP has made too many requests in a short period";
    }
    return "Unkown";
}
#endif

void http_request_free(struct http_request *req)
{
    if (NULL == req) { return; }
    if (NULL != req->body) { free(req->body); }
    if (NULL != req->query) { free(req->query); }
    for (size_t i = 0; i < req->header_count; i++) {
        if (NULL != req->headers[i]) { free(req->headers[i]); }
    }
    api_endpoint_free(req->end_point);
    free(req);
}

#if 0
static struct http_header *http_header_new(void)
{
    struct http_header *header = malloc(sizeof(struct http_header));
    header->name = get_zero_string(MAX_HEADER_LENGTH);
    header->value = get_zero_string(MAX_HEADER_LENGTH);
    return header;
}
#endif

static struct http_request *http_request_new(void)
{
    struct http_request *req = malloc(sizeof(struct http_request));
    req->body = NULL;
    req->body_length = 0;
    req->query = NULL;
    req->end_point = NULL;
    req->headers[0] = NULL;
    req->header_count = 0;

    return req;
}

static char *api_get_signature(const char *string, const char *secret)
{
    size_t len = strlen(string) + strlen(secret);
    char *sig = get_zero_string(len);
    snprintf(sig, len + 1, "%s%s", string, secret);

    unsigned char sig_hash[MD5_DIGEST_LENGTH];
    MD5_CTX h;
    MD5_Init(&h);
    MD5_Update(&h, sig, len);
    MD5_Final(sig_hash, &h);
    free(sig);

    char *result = get_zero_string(MD5_DIGEST_LENGTH * 2 + 2);
    for (size_t n = 0; n < MD5_DIGEST_LENGTH; n++) {
        snprintf(result + 2 * n, 3, "%02x", sig_hash[n]);
    }

    return result;
}

/*
 * artist (Required) : The artist name.
 * track (Required) : The track name.
 * album (Optional) : The album name.
 * trackNumber (Optional) : The track number of the track on the album.
 * context (Optional) : Sub-client version (not public, only enabled for certain API keys)
 * mbid (Optional) : The MusicBrainz Track ID.
 * duration (Optional) : The length of the track in seconds.
 * albumArtist (Optional) : The album artist - if this differs from the track artist.
 * api_key (Required) : A Last.fm API key.
 * api_sig (Required) : A Last.fm method signature. See authentication for more information.
 * sk (Required) : A session key generated by authenticating a user via the authentication protocol.
 */
struct http_request *api_build_request_now_playing(const struct scrobble *track, CURL *handle, enum api_type type, const char *api_key, const char *secret, const char *sk)
{
    if (NULL == handle) { return NULL; }

    struct http_request *req = http_request_new();

    char *sig_base = get_zero_string(MAX_BODY_SIZE);
    char *body = get_zero_string(MAX_BODY_SIZE);

    size_t album_len = strlen(track->album);
    char *esc_album = curl_easy_escape(handle, track->album, album_len);
    size_t esc_album_len = strlen(esc_album);
    strncat(body, "album=", 6);
    strncat(body, esc_album, esc_album_len);
    strncat(body, "&", 1);

    strncat(sig_base, "album", 5);
    strncat(sig_base, track->album, album_len);
    free(esc_album);

    size_t api_key_len = strlen(api_key);
    char *esc_api_key = curl_easy_escape(handle, api_key, api_key_len);
    size_t esc_api_key_len = strlen(esc_api_key);
    strncat(body, "api_key=", 8);
    strncat(body, esc_api_key, esc_api_key_len);
    strncat(body, "&", 1);

    strncat(sig_base, "api_key", 7);
    strncat(sig_base, api_key, api_key_len);
    free(esc_api_key);

    size_t artist_len = strlen(track->artist);
    char *esc_artist = curl_easy_escape(handle, track->artist, artist_len);
    size_t esc_artist_len = strlen(esc_artist);
    strncat(body, "artist=", 7);
    strncat(body, esc_artist, esc_artist_len);
    strncat(body, "&", 1);

    strncat(sig_base, "artist", 6);
    strncat(sig_base, track->artist, artist_len);
    free(esc_artist);

    const char *method = API_METHOD_NOW_PLAYING;

    size_t method_len = strlen(method);
    strncat(body, "method=", 7);
    strncat(body, method, method_len);
    strncat(body, "&", 1);

    strncat(sig_base, "method", 6);
    strncat(sig_base, method, method_len);

    strncat(body, "sk=", 3);
    size_t sk_len = strlen(sk);
    strncat(body, sk, sk_len);
    strncat(body, "&", 1);

    strncat(sig_base, "sk", 2);
    strncat(sig_base, sk, sk_len);

    size_t title_len = strlen(track->title);
    char *esc_title = curl_easy_escape(handle, track->title, title_len);
    size_t esc_title_len = strlen(esc_title);
    strncat(body, "track=", 6);
    strncat(body, esc_title, esc_title_len);
    strncat(body, "&", 1);

    strncat(sig_base, "track", 5);
    strncat(sig_base, track->title, title_len);
    free(esc_title);

    char *query = get_zero_string(MAX_BODY_SIZE);
    strncat(query, "format=json", 12);

    char *sig = (char*)api_get_signature(sig_base, secret);
    strncat(body, "api_sig=", 8);
    strncat(body, sig, strlen(sig));
    free(sig_base);
    free(sig);

    req->body = body;
    req->body_length = strlen(body);
    req->query = query;
    req->end_point = api_endpoint_new(type);
    return req;
}

bool credentials_valid(struct api_credentials *c)
{
    switch (c->end_point) {
        case librefm:
#ifndef LIBREFM_API_SECRET
            return false;
#endif
            break;
        case lastfm:
#ifndef LASTFM_API_SECRET
            return false;
#endif
            break;
        case listenbrainz:
#ifndef LISTENBRAINZ_API_SECRET
            return false;
#endif
            break;
        case unknown:
        default:
            return false;
            break;
    }
    return (c->enabled && c->authenticated);
}

const char *api_get_application_secret(enum api_type type)
{
    switch (type) {
        case librefm:
            return LIBREFM_API_SECRET;
            break;
        case lastfm:
            return LASTFM_API_SECRET;
            break;
        case listenbrainz:
            return LISTENBRAINZ_API_SECRET;
            break;
        default:
            return NULL;
    }
}

const char *api_get_application_key(enum api_type type)
{
    switch (type) {
        case librefm:
            return LIBREFM_API_KEY;
            break;
        case lastfm:
            return LASTFM_API_KEY;
            break;
        case listenbrainz:
            return LISTENBRAINZ_API_KEY;
            break;
        default:
            return NULL;
    }
}

char *api_get_auth_url(enum api_type type, const char *token)
{
    if (NULL == token) { return NULL; }

    const char *base_url = NULL;
    switch(type) {
        case lastfm:
            base_url = LASTFM_AUTH_URL;
            break;
        case librefm:
            base_url = LIBREFM_AUTH_URL;
            break;
        case listenbrainz:
            base_url = LISTENBRAINZ_AUTH_URL;
            break;
        case unknown:
        default:
            return NULL;
    }
    const char *api_key = api_get_application_key(type);
    size_t token_len = strlen(token);
    size_t key_len = strlen(api_key);
    size_t base_url_len = strlen(base_url);

    size_t url_len = base_url_len + token_len + key_len;
    char *url = get_zero_string(url_len);

    snprintf(url, url_len, base_url, api_key, token);

    return url;
}

/*
 * api_key (Required) : A Last.fm API key.
 * api_sig (Required) : A Last.fm method signature. See [authentication](https://www.last.fm/api/authentication) for more information.
*/
struct http_request *api_build_request_get_token(CURL *handle, enum api_type type)
{
    if (NULL == handle) { return NULL; }

    const char *api_key = api_get_application_key(type);
    const char *secret = api_get_application_secret(type);

    struct http_request *request = http_request_new();

    char *sig_base = get_zero_string(MAX_BODY_SIZE);
    char *query = get_zero_string(MAX_BODY_SIZE);

    char *escaped_api_key = curl_easy_escape(handle, api_key, strlen(api_key));
    size_t escaped_key_len = strlen(escaped_api_key);
    strncat(query, "api_key=", 8);
    strncat(query, escaped_api_key, escaped_key_len);
    strncat(query, "&", 1);

    strncat(sig_base, "api_key", 7);
    strncat(sig_base, escaped_api_key, escaped_key_len);
    free(escaped_api_key);

    const char *method = API_METHOD_GET_TOKEN;
    size_t method_len = strlen(method);
    strncat(query, "method=", 7);
    strncat(query, method, method_len);
    strncat(query, "&",1);

    strncat(sig_base, "method", 6);
    strncat(sig_base, method, method_len);

    strncat(query, "format=json&", 12);

    char *sig = (char*)api_get_signature(sig_base, secret);
    strncat(query, "api_sig=", 8);
    strncat(query, sig, strlen(sig));
    free(sig);
    free(sig_base);

    request->query = query;
    request->end_point = api_endpoint_new(type);
    return request;
}

/*
 * token (Required) : A 32-character ASCII hexadecimal MD5 hash returned by step 1 of the authentication process (following the granting of permissions to the application by the user)
 * api_key (Required) : A Last.fm API key.
 * api_sig (Required) : A Last.fm method signature. See authentication for more information.
 * Return codes
 *  2 : Invalid service - This service does not exist
 *  3 : Invalid Method - No method with that name in this package
 *  4 : Authentication Failed - You do not have permissions to access the service
 *  4 : Invalid authentication token supplied
 *  5 : Invalid format - This service doesn't exist in that format
 *  6 : Invalid parameters - Your request is missing a required parameter
 *  7 : Invalid resource specified
 *  8 : Operation failed - Something else went wrong
 *  9 : Invalid session key - Please re-authenticate
 * 10 : Invalid API key - You must be granted a valid key by last.fm
 * 11 : Service Offline - This service is temporarily offline. Try again later.
 * 13 : Invalid method signature supplied
 * 14 : This token has not been authorized
 * 15 : This token has expired
 * 16 : There was a temporary error processing your request. Please try again
 * 26 : Suspended API key - Access for your account has been suspended, please contact Last.fm
 * 29 : Rate limit exceeded - Your IP has made too many requests in a short period*
 */
struct http_request *api_build_request_get_session(CURL *handle, enum api_type type, const char *token)
{

    if (NULL == handle) { return NULL; }
    if (NULL == token) { return NULL; }

    const char *api_key = api_get_application_key(type);
    const char *secret = api_get_application_secret(type);

    struct http_request *request = http_request_new();

    const char *method = API_METHOD_GET_SESSION;

    char *sig_base = get_zero_string(MAX_BODY_SIZE);
    char *query = get_zero_string(MAX_BODY_SIZE);

    char *escaped_api_key = curl_easy_escape(handle, api_key, strlen(api_key));
    size_t escaped_key_len = strlen(escaped_api_key);
    strncat(query, "api_key=", 8);
    strncat(query, escaped_api_key, escaped_key_len);
    strncat(query, "&", 1);

    strncat(sig_base, "api_key", 7);
    strncat(sig_base, escaped_api_key, escaped_key_len);
    free(escaped_api_key);

    size_t method_len = strlen(method);
    strncat(query, "method=", 7);
    strncat(query, method, method_len);
    strncat(query, "&", 1);

    strncat(sig_base, "method", 6);
    strncat(sig_base, method, method_len);

    size_t token_len =strlen(token);
    strncat(query, "token=", 6);
    strncat(query, token, token_len);
    strncat(query, "&", 1);

    strncat(sig_base, "token", 5);
    strncat(sig_base, token, token_len);

    strncat(query, "format=json&", 12);

    char *sig = (char*)api_get_signature(sig_base, secret);
    strncat(query, "api_sig=", 8);
    strncat(query, sig, strlen(sig));
    free(sig);
    free(sig_base);

    request->query = query;
    request->end_point = api_endpoint_new(type);
    return request;
}

/*
 * artist[i] (Required) : The artist name.
 * track[i] (Required) : The track name.
 * timestamp[i] (Required) : The time the track started playing, in UNIX timestamp format (integer number of seconds since 00:00:00, January 1st 1970 UTC). This must be in the UTC time zone.
 * album[i] (Optional) : The album name.
 * context[i] (Optional) : Sub-client version (not public, only enabled for certain API keys)
 * streamId[i] (Optional) : The stream id for this track received from the radio.getPlaylist service, if scrobbling Last.fm radio
 * chosenByUser[i] (Optional) : Set to 1 if the user chose this song, or 0 if the song was chosen by someone else (such as a radio station or recommendation service). Assumes 1 if not specified
 * trackNumber[i] (Optional) : The track number of the track on the album.
 * mbid[i] (Optional) : The MusicBrainz Track ID.
 * albumArtist[i] (Optional) : The album artist - if this differs from the track artist.
 * duration[i] (Optional) : The length of the track in seconds.
 * api_key (Required) : A Last.fm API key.
 * api_sig (Required) : A Last.fm method signature. See authentication for more information.
 * sk (Required) : A session key generated by authenticating a user via the authentication protocol.
 *
 */
struct http_request *api_build_request_scrobble(const struct scrobble *tracks[], size_t track_count, CURL *handle, enum api_type type, const char *api_key, const char *secret, const char *sk)
{
    if (NULL == handle) { return NULL; }

    struct http_request *request = http_request_new();

    const char *method = API_METHOD_SCROBBLE;

    char *sig_base = get_zero_string(MAX_BODY_SIZE);
    char *body = get_zero_string(MAX_BODY_SIZE);

    for (size_t i = 0; i < track_count; i++) {
        const struct scrobble *track = tracks[i];

        size_t album_len = strlen(track->album);
        char *esc_album = curl_easy_escape(handle, track->album, album_len);
        size_t esc_album_len = strlen(esc_album);
        strncat(body, "album[]=", 8);
        strncat(body, esc_album, esc_album_len);
        strncat(body, "&", 1);

        strncat(sig_base, "album[]", 7);
        strncat(sig_base, track->album, album_len);
        free(esc_album);
    }

    size_t api_key_len = strlen(api_key);
    char *esc_api_key = curl_easy_escape(handle, api_key, api_key_len);
    size_t esc_api_key_len = strlen(esc_api_key);
    strncat(body, "api_key=", 8);
    strncat(body, esc_api_key, esc_api_key_len);
    strncat(body, "&", 1);

    strncat(sig_base, "api_key", 7);
    strncat(sig_base, api_key, api_key_len);
    free(esc_api_key);

    for (size_t i = 0; i < track_count; i++) {
        const struct scrobble *track = tracks[i];

        size_t artist_len = strlen(track->artist);
        char *esc_artist = curl_easy_escape(handle, track->artist, artist_len);
        size_t esc_artist_len = strlen(esc_artist);
        strncat(body, "artist[]=", 9);
        strncat(body, esc_artist, esc_artist_len);
        strncat(body, "&", 1);

        strncat(sig_base, "artist[]", 8);
        strncat(sig_base, track->artist, artist_len);
        free(esc_artist);
    }

    size_t method_len = strlen(method);
    strncat(body, "method=", 7);
    strncat(body, method, method_len);
    strncat(body, "&", 1);

    strncat(sig_base, "method", 6);
    strncat(sig_base, method, method_len);

    size_t sk_len = strlen(sk);
    strncat(body, "sk=", 3);
    strncat(body, sk, sk_len);
    strncat(body, "&", 1);

    strncat(sig_base, "sk", 2);
    strncat(sig_base, sk, sk_len);

    for (size_t i = 0; i < track_count; i++) {
        const struct scrobble *track = tracks[i];

        char *timestamp = get_zero_string(32);
        snprintf(timestamp, 32, "%ld", track->start_time);
        strncat(body, "timestamp[]=", 12);
        strncat(body, timestamp, strlen(timestamp));
        strncat(body, "&", 1);

        strncat(sig_base, "timestamp[]", 11);
        strncat(sig_base, timestamp, strlen(timestamp));
        free(timestamp);

        size_t title_len = strlen(track->title);
        char *esc_title = curl_easy_escape(handle, track->title, title_len);
        size_t esc_title_len = strlen(esc_title);
        strncat(body, "track[]=", 8);
        strncat(body, esc_title, esc_title_len);
        strncat(body, "&", 1);

        strncat(sig_base, "track[]", 7);
        strncat(sig_base, track->title, title_len);
        free(esc_title);
    }

    char *query = get_zero_string(MAX_BODY_SIZE);
    strncat(query, "format=json", 12);

    char *sig = (char*)api_get_signature(sig_base, secret);
    strncat(body, "api_sig=", 8);
    strncat(body, sig, strlen(sig));
    free(sig);
    free(sig_base);

    request->query = query;
    request->body = body;
    request->body_length = strlen(body);
    request->end_point = api_endpoint_new(type);

    return request;
}

#if 0
void xml_document_free(struct xml_node*);
#endif
void http_response_free(struct http_response *res)
{
    if (NULL == res) { return; }

    if (NULL != res->body) {
        free(res->body);
        res->body = NULL;
    }
#if 0
    if (NULL != res->doc) {
        xml_document_free(res->doc);
        res->doc = NULL;
    }
#endif

    free(res);
}

void http_response_print(struct http_response *res)
{
}

struct http_response *http_response_new(void)
{
    struct http_response *res = malloc(sizeof(struct http_response));

    res->body = get_zero_string(MAX_BODY_SIZE);
    res->doc = NULL;
    res->code = -1;
    res->body_length = 0;
    res->headers[0] = NULL;
    res->headers_count = 0;

    return res;
}

#if 0
struct http_request *api_build_request(message_type type, void *data)
{
    if (type == api_call_now_playing) {
        const struct scrobble *track = (struct scrobble *)data;
        return api_build_request_now_playing(track);
    }
    if (type == api_call_scrobble) {
        const struct scrobble *tracks = (struct scrobble *)data[];
        return api_build_request_scrobble(tracks);
    }
    return NULL;
}
#endif

static char *http_request_get_url(struct api_endpoint *endpoint, const char *query)
{
    if (NULL == endpoint) { return NULL; }
    char *url = get_zero_string(MAX_URL_LENGTH);

    strncat(url, endpoint->scheme, strlen(endpoint->scheme));
    strncat(url, "://", 3);
    strncat(url, endpoint->host, strlen(endpoint->host));
    strncat(url, endpoint->path, strlen(endpoint->path));
    if (NULL != query) {
        strncat(url, "?", 1);
        strncat(url, query, strlen(query));
    }

    return url;
}

#if 0
void http_header_load_name(const char* data, char *name, size_t length)
{
    char *scol_pos = strchr(data, ':');
    //_trace("mem::loading_header_name[%p:%p:%p]", name, *name, scol_pos, data);

    if (NULL == scol_pos) { return; }

    strncpy(name, data, scol_pos - data);
}

void http_header_load_value(const char* data, char *value, size_t length)
{
    char *scol_pos = strchr(data, ':');

    if (NULL == scol_pos) { return; }

    strncpy(value, data, data + length - scol_pos);
    _trace("mem::loading_header_name[%p:%p:%lu] %s", value, scol_pos, length, value);
}

static size_t http_response_write_headers(char *buffer, size_t size, size_t nitems, void* data)
{
    if (NULL == buffer) { return 0; }
    if (NULL == data) { return 0; }
    if (0 == size) { return 0; }
    if (0 == nitems) { return 0; }

    struct http_response *res = (struct http_response*)data;
    res->headers_count = nitems;

//    fprintf(stdout, "HEADER[%lu:%lu]: %s\n", size, nitems, buffer);
//  fprintf(stdout, "[%p]\n", data);

    size_t new_size = size * nitems;

    struct http_header *h = http_header_new();
    char *scol_pos = strchr(buffer, ':');
    _trace("mem::position[%p:%lu]", scol_pos, (ptrdiff_t)(scol_pos - buffer));
    http_header_load_name(buffer, h->name, new_size);
    if (NULL != h->name) { return 0; }

    http_header_load_value(buffer, h->value, new_size);
    if (NULL != h->value) { return 0; }

    _error("header name: %s", h->name);
    _error("header value: %s", h->value);

    res->headers[res->headers_count] = h;
    res->headers_count++;

    return new_size;
}
#endif

static size_t http_response_write_body(void *buffer, size_t size, size_t nmemb, void* data)
{
    if (NULL == buffer) { return 0; }
    if (NULL == data) { return 0; }
    if (0 == size) { return 0; }
    if (0 == nmemb) { return 0; }

    struct http_response *res = (struct http_response*)data;

    size_t new_size = size * nmemb;

    strncat(res->body, buffer, new_size);
    res->body_length += new_size;

    return new_size;
}

struct xml_node *xml_document_new(void);
bool xml_document_is_error(struct xml_node*);
void xml_node_print (struct xml_node*, short unsigned);
static enum api_return_statuses curl_request(CURL *handle, const struct http_request *req, const http_request_type t, struct http_response *res)
{
    enum api_return_statuses ok = status_failed;
    char *url = http_request_get_url(req->end_point, req->query);
    if (t == http_post) {
        curl_easy_setopt(handle, CURLOPT_POSTFIELDS, req->body);
        curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, (long)req->body_length);
    }

    if (NULL == req->body) {
        _trace("curl::request[%s]: %s", (t == http_get ? "G" : "P"), url);
    } else {
        _trace("curl::request[%s]: %s\ncurl::body[%lu]: %s", (t == http_get ? "G" : "P"), url, req->body_length, req->body);
    }

    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_HEADER, 0L);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, http_response_write_body);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, res);
#if 0
    curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, http_response_write_headers);
    curl_easy_setopt(handle, CURLOPT_HEADERDATA, res);
#endif

    CURLcode cres = curl_easy_perform(handle);
    /* Check for errors */
    if(cres != CURLE_OK) {
        _error("curl::error: %s", curl_easy_strerror(cres));
        goto _exit;
    }
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &res->code);
    _trace("curl::response(%p:%lu): %s", res, res->body_length, res->body);

    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &res->code);
    if (res->code < 500) {
        http_response_print(res);
#if 0
        char *content_type = http_response_headers_content_type(res);
        if (true || strncmp(content_type, CONTENT_TYPE_JSON, strlen(CONTENT_TYPE_JSON)) == 0) {
            http_response_parse_json_body(res);
        } else
        if (NULL != content_type && strncmp(content_type, CONTENT_TYPE_XML, strlen(CONTENT_TYPE_XML)) == 0) {
            struct xml_node *document = xml_document_new();
            http_response_parse_xml_body(res, document);
            if (!xml_document_is_error(document)) {
                ok = status_ok;
                res->doc = document;
                xml_node_print(document, 0);
            } else {
                xml_node_print(document, 0);
                xml_document_free(document);
            }
        }
#endif
    }
    if (res->code == 200) { ok = true; }
_exit:
    free(url);
    return ok;
}

enum api_return_statuses api_get_request(CURL *handle, const struct http_request *req, struct http_response *res)
{
    return curl_request(handle, req, http_get, res);
}

enum api_return_statuses api_post_request(CURL *handle, const struct http_request *req, struct http_response *res)
{
    return curl_request(handle, req, http_post, res);
}

bool json_document_is_error(const char *buffer, const size_t length)
{
    // {"error":14,"message":"This token has not yet been authorised"}
    bool result = false;
    json_object *err_object = NULL;
    json_object *msg_object = NULL;

    json_object *root = json_tokener_parse(buffer);
    if (NULL == root || json_object_object_length(root) < 1) {
        return result;
    }
    json_object_object_get_ex(root, API_XML_ERROR_MESSAGE_NAME, &err_object);
    json_object_object_get_ex(root, API_XML_ERROR_NODE_NAME, &err_object);
    if (NULL == err_object || !json_object_is_type(err_object, json_type_string)) {
        return result;
    }
    if (NULL == msg_object || !json_object_is_type(msg_object, json_type_string)) {
        return result;
    }
    result = true;

    return result;
}

#if 0
static bool xml_node_is_error(struct xml_node *node)
{
    if (NULL == node) { return false;}
    bool status = false;
    if (node->type == api_node_type_error) {
        status = true;
    }
    return status;
}

struct xml_attribute *xml_node_get_attribute(struct xml_node*, const char*, size_t);
bool xml_document_is_error(struct xml_node *node)
{
    bool status = true;
    if (NULL == node) { return status;}
    if (node->children_count == 0) { return status; }
    if (node->type != api_node_type_document) { return status; }
    if (node->children_count == 1) {
        struct xml_node *root = node->children[0];
        if (NULL == root) { return status; }
        if (root->type != api_node_type_root) { return status; }
        if (root->children_count == 0) { return status; }
        if (root->children_count == 1) {
            struct xml_node *err = root->children[0];
            if (NULL == err) { return status; }
            if (xml_node_is_error(err)) {
                int status_code = -1;
                struct xml_attribute *status = xml_node_get_attribute(err, API_XML_ERROR_CODE_ATTR_NAME, strlen(API_XML_ERROR_CODE_ATTR_NAME));
                if (NULL != status) {
                    status_code = strtol(status->value, NULL, 0);
                }
                _warn("api::response::status(%d): \"%s\"", status_code, err->content);
                return status;
            } else {
                status = false;
            }
        }
    }
    return status;
}
#endif

#endif // MPRIS_SCROBBLER_API_H
