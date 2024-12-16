/*
SpotifyArduino - An Arduino library to wrap the Spotify API
*/

#ifndef SpotifyCustom_h
#define SpotifyCustom_h


// Prints the JSON received to serial (only use for debugging as it will be slow)
//#define SPOTIFY_PRINT_JSON_PARSE 1

#include <Arduino.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

static HTTPClient httpCom;


//char Token[346];
String Token;
String Device;
boolean Shuffle;
boolean Playing;
boolean CurrentSaved;
int Loop;
//String Playback;
String TrackId;
String OldTrackUrl;
String TrackUrl;
String TrackName;
String TrackAuth;



/*void getTokenOld(String sp_dc) {
    httpCom.begin("https://open.spotify.com/get_access_token?reason=transport&productType=web-player");

    httpCom.addHeader("Cookie", "sp_dc=" + String(sp_dc));
    httpCom.addHeader("cache-control", "max-age=0");
    httpCom.addHeader("user-agent", "ESP32HTTPClient");
    int response = httpCom.GET();

    if (response > 0) {
        String payload = httpCom.getString();
        //Serial.println(payload);
        int tokenIndex = payload.indexOf("accessToken");

        if ((payload.substring(tokenIndex + 350, tokenIndex + 351)) == "\"") {
            strcpy(Token, "Bearer ");
            strcat(Token, payload.substring(tokenIndex + 14, tokenIndex + 350).c_str());
            Serial.println(Token);
        }
        else {
            Serial.println("Token Failed");

        }
        //Serial.println(payload.substring(tokenIndex + 14, tokenIndex + 14 + 320).c_str());
    }

    //Serial.println("");
    //Serial.println(response);
    httpCom.end();
}*/

void getToken(String sp_dc) {
    httpCom.begin("https://open.spotify.com/get_access_token?reason=transport&productType=web-player");

    httpCom.addHeader("Cookie", "sp_dc=" + String(sp_dc));
    int response = httpCom.GET();

    if (response > 0) {
        JSONVar responseJson = JSON.parse(httpCom.getString());

        if (JSON.typeof(responseJson) != "undefined") {
            Token ="Bearer "+ String((const char*)responseJson["accessToken"]);

            Serial.println(Token);
        }
        else {
            Serial.println("Token Failed");

        }
        //Serial.println(payload.substring(tokenIndex + 14, tokenIndex + 14 + 320).c_str());
    }

    //Serial.println("");
    //Serial.println(response);
    httpCom.end();
}


JSONVar getPlaybackState() {
  HTTPClient httpPlay;
  httpPlay.begin("https://api.spotify.com/v1/me/player");
  httpPlay.addHeader("authorization", Token);
  int response = httpPlay.GET();
  if (response==200) {
    String payload = httpPlay.getString();
    //Serial.println(payload);
    httpPlay.end();
    //Playback=payload;
    //Serial.println(payload);
    JSONVar doc = JSON.parse(payload);
    //Playback=doc;
    //Serial.println(doc["item"]["name"]);
    return doc;
  }
  else {
Serial.println("No player active");
    httpPlay.end();
    JSONVar doc;
    doc["is_playing"]=2;
   return doc;
  }
}


int sendCommand(String endpoint,String value) {
  if (endpoint=="set_options") {
    if ((value.toInt()<0)||(value.toInt()>2)) {
      return -1;
    }
  }


  Serial.println(endpoint);
  String url="https://gew4-spclient.spotify.com/connect-state/v1/player/command/from/1234567890123456789012345678901234567890/to/";
  //String more=getPlaybackState()["device"]["id"];
  //String more="852daaa6057fd8a361934bfc133655332676f7c2";
  url+=Device;
  //Serial.println("SetUrl");
  //Serial.println(url);
  httpCom.begin(url);
  httpCom.addHeader("authorization", Token);
  httpCom.addHeader("Content-Type", "application/json");
  String JsonPost;
  if (endpoint=="add_to_queue") {
    JsonPost="{\"command\":{\"track\": {\"uri\":\"spotify:track:"+value+"\",\"provider\":\"queue\"},\"endpoint\":\""+endpoint+"\"}}";
  }else if (endpoint=="set_options") {
    String Tr="false";
    String Con="false";
    if (value=="1") {
      Con="true";
    } else if (value=="2") {
      Con="true";
      Tr="true";
    }
    JsonPost="{\"command\":{\"repeating_context\":"+Con+",\"repeating_track\":"+Tr+",\"endpoint\":\""+endpoint+"\"}}";
  }else {
    JsonPost="{\"command\":{\"value\":"+value+",\"endpoint\":\""+endpoint+"\"}}";
  }
  //Serial.println(JsonPost);
  int httpResponseCode = httpCom.POST(JsonPost);
  Serial.println(httpResponseCode);
  if (httpResponseCode==200) {
    if (endpoint=="pause") {
      Playing=false;
    } else if (endpoint=="resume") {
      Playing=true;
    }else if (endpoint=="set_shuffling_context") {
      if (value=="true") {
        Shuffle=true;
      } else {
        Shuffle=false;
      }
    }else if (endpoint=="set_options") {
      Loop=value.toInt();
    }
  }
  String payload = httpCom.getString();
  Serial.println(payload);
  httpCom.end();
  return httpResponseCode;
}


JSONVar searchItem(String name,int offset) {
  String url="https://api.spotify.com/v1/search?q=";
  name.replace(" ", "+");
  url+=name;
  url+="&type=track&limit=1&offset="+String(offset);
  //Serial.println(url);
  httpCom.begin(url);
  httpCom.addHeader("authorization", Token);
  int response = httpCom.GET();
  Serial.print("Search ");
  Serial.println(response);
  if (response==200) {
    String payload = httpCom.getString();
    //Serial.println(payload);
    httpCom.end();

    payload=payload.substring(payload.indexOf("["),payload.lastIndexOf("]")+1);
    //Serial.println(payload);
    //JSONVar doc = JSON.parse(payload);
    //Serial.println(doc);

    return payload;
  } else {
	  httpCom.end();
	  return JSONVar();
  }
}

int saveTrack(String track) {

  Serial.println(track);

  String url="https://api.spotify.com/v1/me/tracks?ids=";
  url+=track;
  //Serial.println("SetUrl");
  //Serial.println(url);
  httpCom.begin(url);
  httpCom.addHeader("authorization", Token);
  //httpCom.addHeader("accept", "*/*");
  //httpCom.addHeader("accept-encoding", "gzip, deflate");
  httpCom.addHeader("content-length","0");

  //Serial.println(JsonPost);
  int response = httpCom.PUT(String(""));
  Serial.println(response);
  httpCom.end();
  return response;
}


int deleteTrack(String track) {

  Serial.println(track);

  String url="https://api.spotify.com/v1/me/tracks?ids=";
  url+=track;
  //Serial.println("SetUrl");
  //Serial.println(url);
  httpCom.begin(url);
  httpCom.addHeader("authorization", Token);
  httpCom.addHeader("accept", "*/*");
  httpCom.addHeader("accept-encoding", "gzip, deflate");
  httpCom.addHeader("content-length","0");

  //Serial.println(JsonPost);
  int response = httpCom.sendRequest("DELETE","");
  Serial.println(response);
  httpCom.end();
  return response;
}

void startSong(String id) {

    String url = "https://gew4-spclient.spotify.com/connect-state/v1/player/command/from/f59e9112b62e37a5782a2372cb74f8fee1e2a0ed/to/";
    url+=Device;
    httpCom.begin(url);
  
    String payload = "{\"command\":{\"context\":{\"url\":\"context://spotify:track:" + id + "\"},\"endpoint\":\"play\"}}";
    
    httpCom.addHeader("authorization", Token);
    httpCom.addHeader("Content-Type", "application/json");

    int httpResponseCode = httpCom.POST(payload);
    if (httpResponseCode==200){
      Playing=true;
    }
    Serial.println(httpResponseCode);

    httpCom.end();
}

void setVolume(float volume) {
    // Clamp volume between 0 and 100
    volume = constrain(volume, 0, 100);
    
    // Calculate the volume in the Spotify format (0-65535)
    int volumeSpotify = int(volume / 100.0 * 65535);

    String url = "https://gew4-spclient.spotify.com/connect-state/v1/connect/volume/from/1234567890123456789012345678901234567890/to/";
    url += Device;
    httpCom.begin(url);

    // Create JSON payload
    String payload = "{\"volume\":" + String(volumeSpotify) + "}";

    // Set headers
    httpCom.addHeader("authorization", Token);
    httpCom.addHeader("Content-Type", "application/json");

    // Send PUT request with payload
    int httpResponseCode = httpCom.PUT(payload);

    //Serial.println(httpResponseCode);

    httpCom.end();
}

bool isSaved(String id) {
  String url = "https://api.spotify.com/v1/me/tracks/contains?ids=" + id;
  
  httpCom.begin(url);
  httpCom.addHeader("authorization", Token);
  
  int responseCode = httpCom.GET();
  if (responseCode == 200) {
    String response = httpCom.getString();
    httpCom.end();
    // Convert response to boolean
    if (response == "[true]") {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

JSONVar getFeatures(String id) {
  String url = "https://api.spotify.com/v1/audio-features/" + id;
  
  httpCom.begin(url);
  httpCom.addHeader("authorization", Token);
  
  int responseCode = httpCom.GET();
  Serial.print("Feats ");
  Serial.println(responseCode);
  if (responseCode == 200) {
    String response = httpCom.getString();
    JSONVar doc = JSON.parse(response);
    httpCom.end();
    return doc;
  } else {
    httpCom.end();
    return JSONVar();
  }
}

JSONVar getSong(String id) {
  String url = "https://api.spotify.com/v1/tracks/" + id;
  
  httpCom.begin(url);
  httpCom.addHeader("authorization", Token);
  
  int responseCode = httpCom.GET();
  Serial.print("Song ");
  Serial.println(responseCode);
  if (responseCode == 200) {
    String response = httpCom.getString();
    JSONVar doc = JSON.parse(response);
    httpCom.end();
    return doc;
  } else {
    httpCom.end();
    return JSONVar();
  }
}

JSONVar getArtist(String id) {
  String url = "https://api.spotify.com/v1/artists/" + id;
  
  httpCom.begin(url);
  httpCom.addHeader("authorization", Token);
  
  int responseCode = httpCom.GET();
  Serial.print("Artist ");
  Serial.println(responseCode);
  Serial.println(url);
  if (responseCode == 200) {
    String response = httpCom.getString();
    //Serial.println(response);
    JSONVar doc = JSON.parse(response);
    httpCom.end();
    return doc;
  } else {
    httpCom.end();
  }
}

String getGenre(String id) {
  String url = "https://api.spotify.com/v1/artists/" + id;
  
  httpCom.begin(url);
  httpCom.addHeader("authorization", Token);
  
  int responseCode = httpCom.GET();
  Serial.print("Genre ");
  Serial.println(responseCode);
  Serial.println(url);
  if (responseCode == 200) {
    String response = httpCom.getString();
    httpCom.end();
    int St=response.indexOf("genres");
    response=response.substring(St+10,response.indexOf("]",St));
    response.replace("\"","");
    response.replace(",", ", ");
    Serial.println(response);
    if (response=="]") {
	return "";
    }
    return response;
  } else {
    httpCom.end();
    return "";
  }
}


#endif

