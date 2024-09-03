#ifndef SPOTIFY_H
#define SPOTIFY_H

#include <SpotifyArduino.h>
#include <SpotifyArduinoCert.h>

void setupSpotify(WiFiClientSecure &client, SpotifyArduino &spotify) {
    client.setCACert(spotify_server_cert);

    Serial.println("Refreshing Access Tokens");
    if (!spotify.refreshAccessToken()) {
        Serial.println("Failed to get access tokens");
    }
}

#endif // SPOTIFY_H