/**
 * @author Marius Orcsik <marius@habarnam.ro>
 */
#ifndef API_H
#define API_H

#define MAX_BODY_SIZE 1024

void scrobbler_endpoint_destroy(api_endpoint *api)
{
    free(api);
}

api_endpoint *scrobbler_endpoint_new(api_type type)
{
    api_endpoint *result = malloc(sizeof(api_endpoint));
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
api_request *api_build_request_now_playing(const scrobble *track, api_type type)
{
    api_request *request = malloc(sizeof(api_request));

    char* body = get_zero_string(MAX_BODY_SIZE);
    strncpy(body, "artist=", 7);
    strncpy(body, track->artist, strlen(track->artist));
    strncpy(body, "&", 1);

    strncpy(body, "track=", 6);
    strncpy(body, track->artist, strlen(track->title));
    strncpy(body, "&", 1);

    strncpy(body, "album=", 6);
    strncpy(body, track->artist, strlen(track->artist));
    strncpy(body, "&", 1);

    request->body = body;
    //request->end_point = scrobbler_endpoint_new(type);
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
api_request* api_build_request_scrobble(scrobble *tracks[], size_t track_count)
{
    api_request *request = malloc(sizeof(api_request));

    char* body = get_zero_string(MAX_BODY_SIZE);
    strncpy(body, "method=track.scrobble&", 22);

    for (size_t i = 0; i < track_count; i++) {
        scrobble *track = tracks[i];
        strncpy(body, "artist[]=", 9);
        strncpy(body, track->artist, strlen(track->artist));
        strncpy(body, "&", 1);

        strncpy(body, "track[]=", 8);
        strncpy(body, track->artist, strlen(track->title));
        strncpy(body, "&", 1);

        strncpy(body, "album[]=", 8);
        strncpy(body, track->artist, strlen(track->artist));
        strncpy(body, "&", 1);
    }
    request->body = body;

    return request;
}

api_request* api_build_request(message_type type, void* data)
{
    if (type == api_call_now_playing) {
        const scrobble *track = (scrobble *)data;
        return api_build_request_now_playing(track);
    }
    if (type == api_call_scrobble) {
        scrobble *tracks = (scrobble *)data;
        return api_build_request_scrobble((scrobble*)data);
    }
    return NULL;
}

void api_get_request(const api_request *req, api_response *res)
{
}

void api_post_request(const api_request *req, api_response *res)
{
}

static void api_request(const api_request *req, const request_type *t, api_response *res)
{
    if (t == http_post) {
        curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDS, req->body);
    }
    curl_easy_setopt(easyhandle, CURLOPT_URL, req->url);

    curl_easy_perform(easyhandle);
}
#endif // API_H