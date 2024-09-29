#include <HTTPClient.h>
#include "FS.h"
//#include <SPI.h>
#include <TFT_eSPI.h>


#include <WiFi.h>

#include <Arduino_JSON.h>
//#include <Math.h>

#include <SpotifyCustom.h>
#include <TJpg_Decoder.h>
#include "Web_Fetch.h"


const char* ssid = "WI-FI name here";
const char* password = "WI-FI password here";
const String SP_DC = "Spotify cookie here";

TaskHandle_t Core0Task;

uint16_t Color = TFT_BLACK;

TFT_eSPI tft = TFT_eSPI();

int SongTime;
int CurrTime;
int LastMillis;
String OldTime;

int WifiLook;
int Tab;




bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  // Stop further decoding as image is running off bottom of screen
  if (y >= tft.height()) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);

  // Return 1 to decode next block
  return 1;
}

int HalfPxH;
int HalfPxV;
void makeSlider(int x, int y, int PxLen, int CVal, int MVal, int Vert) {
  //Vert=0 -> horiz
  int H=8;
  int W=PxLen;
  int Ch=0;
  if (CVal==-100) {
    Ch=1;
  }
  if (Vert) {
    H=PxLen;
    W=8;
  }
  if (MVal <= 0) {
    return;
  }
  if(CVal<0) {
    CVal=0;
  } else if (CVal>MVal) {
    CVal=MVal;
  }
  int HalfPx = PxLen * CVal / MVal;
  if (Vert) {
    if (HalfPx == HalfPxV) {
      return;
    }
    if (Ch) {
      HalfPx=HalfPxV;
    } else {
      HalfPxV=HalfPx;
    }
    tft.fillRect(x, y, 8,PxLen, 0xD69A);
    tft.fillRect(x, y+PxLen-HalfPx, 8, HalfPx, TFT_GREEN);
    if (HalfPx > 2) {
      tft.drawPixel(x, y + PxLen - HalfPx, 0xD69A);
      tft.drawPixel(x + 7, y + PxLen - HalfPx, 0xD69A);
    }
  } else {
    if (HalfPx == HalfPxH) {
      return;
    }
    HalfPxH=HalfPx;
    tft.fillRect(x, y, PxLen, 8, 0xD69A);
    tft.fillRect(x, y, HalfPx, 8, TFT_GREEN);
    if (HalfPx > 2) {
      tft.drawPixel(x + HalfPx - 1, y, 0xD69A);
      tft.drawPixel(x + HalfPx - 1, y + 7, 0xD69A);
    }
  }

  tft.drawPixel(x, y, Color);
  tft.drawPixel(x + 1, y, Color);
  tft.drawPixel(x, y + 1, Color);
  tft.drawPixel(x, y + H-2, Color);
  tft.drawPixel(x, y + H-1, Color);
  tft.drawPixel(x + 1, y + H-1, Color);

  tft.drawPixel(x + W - 1, y, Color);
  tft.drawPixel(x + W - 2, y, Color);
  tft.drawPixel(x + W - 1, y + 1, Color);
  tft.drawPixel(x + W - 1, y + H-2, Color);
  tft.drawPixel(x + W- 1, y + H-1, Color);
  tft.drawPixel(x + W - 2, y + H-1, Color);
}
void OpenInfo1() {
    JSONVar Feat=getFeatures(TrackId);

    makeInfoSlider(101,(double) Feat["energy"]);
    makeInfoSlider(120,(double) Feat["valence"]);
    makeInfoSlider(139,(double) Feat["danceability"]);
    makeInfoSlider(158,(double) Feat["instrumentalness"]);
    makeInfoSlider(177,((double) Feat["loudness"]+60)/60);
    makeInfoSlider(196,(double) Feat["acousticness"]);
    char Key[12][3] ={"C","C#","D","D#","E","F","F#","G","G#","A#","A#","B"};
    tft.setTextDatum(TR_DATUM);
    int Signature=Feat["time_signature"];
    tft.drawString((String(Signature)+ "/4"), 311, 13, 2);
    int key=Feat["key"];
    if (key>=0) {
      tft.drawString(Key[key], 311, 33, 2);
    }
    int Tempo=((double) Feat["tempo"]+0.5);
    tft.drawString((String(Tempo)+ " BPM"), 311, 53, 2);
    int Major=Feat["mode"];
    if (Major) {
      tft.drawString("Major", 311, 73, 2);
    } else {
      tft.drawString("Minor", 311, 73, 2);
    }
}

void OpenInfo() {

  
  TJpgDec.drawFsJpg(23, 102, "/InfoLImg.jpg");
  TJpgDec.drawFsJpg(288, 102, "/InfoRImg.jpg");
  

  tft.setTextDatum(TL_DATUM);
  tft.drawString(TrackName, 93, 13, 2);
  tft.drawString(TrackAuth, 93, 53, 2);
  if (1) {
    JSONVar TrackDet=getSong(TrackId);

    String AlbName=TrackDet["album"]["name"];
    tft.drawString(AlbName, 93, 33, 2);
    String AlbDate=TrackDet["album"]["release_date"];
    tft.drawString((AlbDate.substring(8,10)+"/"+AlbDate.substring(5,7)+"/"+AlbDate.substring(0,4)), 93, 73, 2);

    String ArtId=TrackDet["artists"][0]["id"];
    if (ArtId!="") {
      tft.drawString(getGenre(ArtId), 11, 216, 2);
    }
  }
  
  //JSONVar AlbInfo=getArtist(ArtId);
  //Serial.println(AlbInfo);


  if (SPIFFS.exists("/albumArt.jpg") == true) {
    TJpgDec.setJpgScale(4);
    TJpgDec.drawFsJpg(9, 14, "/albumArt.jpg");
    TJpgDec.setJpgScale(1);
  }
  /*tft.setCursor(104,205);
  for (int x = 0; x < (Feat).keys().length(); x++) {
    tft.print()
  }*/
}



char KeyB[30]={'Q','W','E','R','T','Y','U','I','O','P',
             'A','S','D','F','G','H','J','K','L',27,
             'Z','X','C','V',' ','B','N','M',127,17};

void OpenSearch() {

  tft.drawFastHLine(0, 147, 320, TFT_WHITE);
  tft.drawFastHLine(0, 178, 320, TFT_WHITE);
  tft.drawFastHLine(0, 209, 320, TFT_WHITE);
    for (int x = 0; x < 9; x++) {
    tft.drawFastVLine(32+(32*x), 147, 90, TFT_WHITE);
  }
  for (int x = 0; x < 30; x++) {
    tft.drawChar(12+32*(x%10),157+31*(x/10),KeyB[x],TFT_WHITE,Color,2);
  }
  tft.setTextSize(1);
}
String Id0Sh;
String Id1Sh;

void addResults() {
  //Serial.println(OldTime);
  if (OldTime=="" ){
    return;
  }
  Serial.print(OldTime);
  tft.setTextDatum(TL_DATUM);
  for (int i = 0; i < 2; i++) {
    String res=searchItem(OldTime,i);
    //String res=searchItem("i want it that way");
    //Serial.println(res);
  
    int N;
    
    //Name(song,alb,auth),duration,id,imglink
    N=res.indexOf("64,");
    //Serial.println(N);
    //Link
    //Serial.println(res.substring(N+23,res.indexOf(",",N+3)-1));
    String Sh=res.substring(N+23,res.indexOf(",",N+3)-1);

    N=res.indexOf("name",N+2);
    //Album
    //Serial.println(res.substring(N+9,res.indexOf(",",N)-1));
    tft.drawString(res.substring(N+9,res.indexOf(",",N)-1)+"                            ", 70, 24+66*i, 2);
    N=res.indexOf("name",N+2);
    //Artist
    //Serial.println(res.substring(N+9,res.indexOf(",",N)-1));
    tft.drawString(res.substring(N+9,res.indexOf(",",N)-1)+"                            ", 70, 44+66*i, 2);
    N=res.indexOf("market",N+2);
    N=res.indexOf("id",N+2);
    N=res.indexOf("id",N+2);
    //Id
    //Serial.println(res.substring(N+7,res.indexOf(",",N)-1));
    String IdTemp=res.substring(N+7,res.indexOf(",",N)-1);
    Serial.println(IdTemp);
    if (i) {
      if (IdTemp!=Id1Sh) {
        SPIFFS.remove("/albumArtS1.jpg");
        getFile(Sh, "/albumArtS1.jpg");
        TJpgDec.drawFsJpg(3, 66, "/albumArtS1.jpg");
        Id1Sh=IdTemp;
      } 
    } else {
      if (IdTemp!=Id0Sh) {
        SPIFFS.remove("/albumArtS0.jpg");
        getFile(Sh, "/albumArtS0.jpg");
        TJpgDec.drawFsJpg(3, 0, "/albumArtS0.jpg");
        Id0Sh=IdTemp;
      } 
    }
    N=res.indexOf("name",N+2);
    //Song
    //Serial.println(res.substring(N+9,res.indexOf(",",N)-1));
    tft.drawString(res.substring(N+9,res.indexOf(",",N)-1)+"                            ", 70, 4+66*i, 2);


    tft.setTextSize(3);
    tft.drawString("+", 298, 23+66*i, 1);
    tft.setTextSize(1);

    TJpgDec.drawFsJpg(298, 6+66*i, "/PlayImg.jpg");
    if (isSaved(IdTemp)) {
      //tft.pushImage(22, 134, 16, 16, SavedImg);
      TJpgDec.drawFsJpg(298, 46+66*i, "/SavedImg.jpg");
    } else {
      //tft.pushImage(22, 134, 16, 16, NotSavedImg);
      TJpgDec.drawFsJpg(298, 46+66*i, "/NotSavedImg.jpg");
    }
  }
}



void buildUI(int VolPercent) {
  tft.fillScreen(Color);
  makeSlider(253, 45,106, VolPercent, 100, 1);
  //weird behaviour
  //String TRId=play["item"]["id"];
  //CurrentSaved=isSaved(TRId);
  if (1) {
    TJpgDec.setJpgScale(1);
    //tft.pushImage(22,46,16,16,ShuffleImg);
    if (Shuffle) {
      //tft.pushImage(22, 46, 16, 16, ShuffleImgOn);
      TJpgDec.drawFsJpg(22, 46, "/ShuffleImgOn.jpg");
    } else {
      //tft.pushImage(22, 46, 16, 16, ShuffleImg);
      TJpgDec.drawFsJpg(22, 46, "/ShuffleImg.jpg");
    }
    //tft.pushImage(55, 46, 16, 16, NextImg);
    //tft.pushImage(55, 90, 16, 16, PrevImg);
    TJpgDec.drawFsJpg(55, 46, "/NextImg.jpg");
    TJpgDec.drawFsJpg(55, 90, "/PrevImg.jpg");
    TJpgDec.drawFsJpg(282, 46, "/WiFiImg.jpg");
    TJpgDec.drawFsJpg(282, 90, "/SearchImg.jpg");
    TJpgDec.drawFsJpg(282, 134, "/InfoImg.jpg");
    if (Playing) {
      //tft.pushImage(55, 134, 16, 16, PauseImg);
      TJpgDec.drawFsJpg(55, 134, "/PauseImg.jpg");
    } else {
      //tft.pushImage(55, 134, 16, 16, PlayImg);
      TJpgDec.drawFsJpg(55, 134, "/PlayImg.jpg");
    }
    if (CurrentSaved) {
      //tft.pushImage(22, 134, 16, 16, SavedImg);
      TJpgDec.drawFsJpg(22, 134, "/SavedImg.jpg");
    } else {
      //tft.pushImage(22, 134, 16, 16, NotSavedImg);
      TJpgDec.drawFsJpg(22, 134, "/NotSavedImg.jpg");
    }
    /*if (Loop == 0) {
      tft.pushImage(22, 90, 16, 16, Loop0Img);
    } else if (Loop == 1) {
      tft.pushImage(22, 90, 16, 16, Loop1Img);
    } else {
      tft.pushImage(22, 90, 16, 16, Loop2Img);
    }*/
    TJpgDec.drawFsJpg(22, 90, "/Loop"+String(Loop)+"Img.jpg");
    //getPic("https://i.scdn.co/image/ab67616d00001e02da7b326bd7ffa7b6b95b0660");
    //String TrUrl = play["item"]["album"]["images"][1]["url"];
    //Serial.println(TrUrl=="https://i.scdn.co/image/ab67616d00001e0236346a1a5bc20cba58358836");
    //TrUrl="https://i.scdn.co/image/ab67616d00001e0236346a1a5bc20cba58358836";
    //getPic(TrUrl);
  }
}

void buildUIFull(int VolPercent) {
  //Serial.println(VolPercent);
  buildUI(VolPercent);
  String TrackNamePR = TrackName;
  TrackNamePR.replace("é", "e");

  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_WHITE, Color);
  tft.setTextSize(2);
  //tft.print(Name);

  tft.drawString(TrackNamePR, 160, 169, 2);
  tft.setTextSize(1);
  tft.drawString(TrackAuth, 160, 200, 2);

  tft.setTextDatum(BL_DATUM);
  tft.setTextSize(2);
  //tft.print(Name);
  Serial.println(convTime(SongTime));
  tft.drawString(convTime(SongTime), 250, 238, 2);

  tft.setTextDatum(BR_DATUM);
  tft.drawString(convTime(0), 70, 238, 2);



  if (SPIFFS.exists("/albumArt.jpg") == true) {
    TJpgDec.setJpgScale(2);
    TJpgDec.drawFsJpg(85, 23, "/albumArt.jpg");
    TJpgDec.setJpgScale(1);
  }

}

void makeInfoSlider(int y,float val) {
  //Serial.println(val);
  tft.fillRect(36, y, 2,13, TFT_WHITE);
  tft.fillRect(282, y, 2,13, TFT_WHITE);
  tft.fillRect(38, y+5, 244,3, TFT_WHITE);

  tft.fillRect(36+(244*val+0.5), y, 4,13, TFT_GREEN);
}

/*void changeCol(unsigned short ImgArr[], uint16_t ColorCh) {
  for (int i = 0; i < 256; i++) {
    if (ImgArr[i] != 0x0000) {
      ImgArr[i] = ColorCh;
    }
  }
}*/

int getRepeat(String State) {
  if (State == "off") {
    return 0;
  } else if (State == "context") {
    return 1;
  } else {
    return 2;
  }
}

void getPic(String url) {
  if (SPIFFS.exists("/albumArt.jpg") == true) {
    SPIFFS.remove("/albumArt.jpg");
  }
  int Attempts = 2;
  // Serial.println("trying to get album art");
  //bool loaded_ok = getFile("https://i.scdn.co/image/ab67616d00001e02bc032f86301fe5f26eacfe55", "/albumArt.jpg"); // Note name preceded with "/"
  bool loaded_ok = getFile(url, "/albumArt.jpg");
  while ((!loaded_ok) && (Attempts)) {
    Attempts--;
    loaded_ok = getFile(url, "/albumArt.jpg");
  }
  Serial.println("Image load was: ");
  Serial.println(loaded_ok);

  if (SPIFFS.exists("/albumArt.jpg") == true) {
    TJpgDec.setJpgScale(2);
    TJpgDec.drawFsJpg(85, 23, "/albumArt.jpg");
    TJpgDec.setJpgScale(1);
  }
}

String convTime(int Time) {
  int Secs = Time / 1000;
  int Mins = Secs / 60;
  Secs = Secs % 60;
  if (Secs >= 10) {
    return String(Mins) + ":" + String(Secs);
  } else {
    return String(Mins) + ":0" + String(Secs);
  }
}
void codeForCore0Task(void* parameter) {
  HTTPClient httpAlt;
  for (;;) {
    httpAlt.begin("https://api.spotify.com/v1/me/player");
    httpAlt.addHeader("authorization", Token);
    int response = httpAlt.GET();
    //Serial.print(response);
    if (response == 200) {
      String payload = httpAlt.getString();
      //Serial.println(payload);
      httpAlt.end();
      JSONVar doc = JSON.parse(payload);
      String Tr = doc["item"]["id"];
      //Serial.println(Tr);
      CurrTime = doc["progress_ms"];
      LastMillis = millis();
      if ((Tr != TrackId) && (Tr != "")) {
        if (1) {
        TrackId = Tr;
        Serial.print("New: ");
        String TrUrl = doc["item"]["album"]["images"][1]["url"];
        TrackUrl = TrUrl;
        //tft.setCursor(170, 179, 2);
        //tft.setTextDatum(1);
        String Name = doc["item"]["name"];
        TrackName = Name;
        String Auth = doc["item"]["artists"][0]["name"];
        TrackAuth = Auth;
        SongTime = doc["item"]["duration_ms"];
        if (0) {
          tft.setTextDatum(BL_DATUM);
          tft.setTextColor(TFT_WHITE, Color);
          tft.setTextSize(2);
          //tft.print(Name);
          tft.drawString("         " + convTime(SongTime), 228, 67, 2);
        }
        }
        //getPic(TrUrl);
        //getFile(TrUrl, "/albumArt.jpg");
        //Serial.println(doc);
        //songUpdate(doc);
        
      }
      String dev = doc["device"]["id"];
      Device = dev;
    } else {
      httpAlt.end();
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("starting");


  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected. Starting");

  tft.init();

  tft.setRotation(3);
  

  tft.setCursor(20, 0, 2);
  tft.setTextColor(TFT_WHITE, Color);
  tft.setTextSize(1);

  //TJpgDec.setJpgScale(2);

  // The byte order can be swapped (set true for TFT_eSPI)
  TJpgDec.setSwapBytes(true);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);

  if (!SPIFFS.begin()) {
    Serial.println("formatting file system");

    SPIFFS.format();
    SPIFFS.begin();
  }
  http.setReuse(true);
  getToken(SP_DC);
  tft.fillScreen(Color);
  JSONVar play = getPlaybackState();
  while (((int) play["is_playing"])==2) {
    tft.setTextSize(2);
    tft.drawString("Spotify not active",42,104,2);
    delay(500);
    play = getPlaybackState();
  }
  boolean sh = play["shuffle_state"];
  Shuffle = sh;
  String dev = play["device"]["id"];
  Device = dev;
  boolean Pl = play["is_playing"];
  Playing = Pl;
  Loop = getRepeat(play["repeat_state"]);

  //makeHSlider(76,222,168,88774,150233,0);
  buildUI(int(play["device"]["volume_percent"]));

  Serial.println("---");
  xTaskCreatePinnedToCore(
    codeForCore0Task,
    "Core 0 task",
    3600,
    NULL,
    1,
    &Core0Task,
    0);
}

int NotTouching = 1;
int TrackSliding=0;

void loop() {
  if (Tab==0) {
  if (TrackUrl != OldTrackUrl) {
    String TrackNamePR = TrackName;
    TrackNamePR.replace("é", "e");
    Serial.println(TrackName);
    OldTrackUrl = TrackUrl;

    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_WHITE, Color);
    tft.setTextSize(2);
    //tft.print(Name);

    tft.drawString("          " + TrackNamePR + "          ", 160, 169, 2);
    tft.setTextSize(1);
    tft.drawString("                  " + TrackAuth + "                  ", 160, 200, 2);
    CurrentSaved = isSaved(TrackId);
    if (1) {
      tft.setTextDatum(BL_DATUM);
      tft.setTextSize(2);
      //tft.print(Name);
      //Serial.println(convTime(SongTime));
      tft.drawString(convTime(SongTime) + "    ", 250, 238, 2);

      tft.setTextDatum(BR_DATUM);
      tft.drawString("    " + convTime(0), 70, 238, 2);
      makeSlider(76, 222, 168,1, SongTime, 0);
    }
    
    if (CurrentSaved) {
      //tft.pushImage(22, 134, 16, 16, SavedImg);
      TJpgDec.drawFsJpg(22, 134, "/SavedImg.jpg");
    } else {
      //tft.pushImage(22, 134, 16, 16, NotSavedImg);
      TJpgDec.drawFsJpg(22, 134, "/NotSavedImg.jpg");
    }
    getPic(TrackUrl);
    //getFile("https://webhook.site/1ce687ca-04aa-4c0e-9ca5-be28c8c69023", "albumArt.jpg");
    getToken(SP_DC);
  }

  if (Playing) {
    tft.setTextDatum(BR_DATUM);
    tft.setTextColor(TFT_WHITE, Color);
    tft.setTextSize(2);
    //tft.print(Name);
    int TimeF = CurrTime - LastMillis + millis() - 20;
    String CTime=convTime(TimeF);
    if (CTime != OldTime) {
      tft.drawString("    " + convTime(TimeF), 70, 238, 2);
      OldTime=CTime;
    }
    if (TrackSliding!=1) {
      makeSlider(76, 222, 168, TimeF, SongTime, 0);
    }
  }
  }
  uint16_t x, y;
  


  //22,46
  if (tft.getTouch(&x, &y)) {
    x = 320 - x;
    //Serial.print(NotTouching);
    //Serial.print(x);
    //Serial.println(y);
    if (Tab==0) {
    if (TrackSliding==1) {
      makeSlider(76, 222, 168, x-76, 168, 0);
    } else if (TrackSliding==2) {
      makeSlider(253, 45,106, -y+151, 106, 1);
      setVolume(100*(-y+151)/106);
      //Serial.println(100*(-y+151)/106);
    } else if (x > 18 && x < 42 && y > 42 && y < 68 && NotTouching) {
      //Serial.print("A!");
      if (Shuffle) {
        if (sendCommand("set_shuffling_context", "false")==200) {
          //tft.pushImage(22, 46, 16, 16, ShuffleImg);
          TJpgDec.drawFsJpg(22, 46, "/ShuffleImg.jpg");
        }
      } else {
        if (sendCommand("set_shuffling_context", "true")==200) {
          //tft.pushImage(22, 46, 16, 16, ShuffleImgOn);
          TJpgDec.drawFsJpg(22, 46, "/ShuffleImgOn.jpg");
        }
      }
    } else if (x > 51 && x < 75 && y > 42 && y < 68 && NotTouching) {
      //Serial.print("A!");
      sendCommand("skip_next", "0");
    } else if (x > 51 && x < 75 && y > 86 && y < 112 && NotTouching) {
      //Serial.print("A!");
      sendCommand("skip_prev", "0");
    } else if (x > 51 && x < 75 && y > 130 && y < 156 && NotTouching) {
      if (Playing) {
        if (sendCommand("pause", "0")==200) {
          //tft.pushImage(55, 134, 16, 16, PlayImg);
          TJpgDec.drawFsJpg(55, 134, "/PlayImg.jpg");
        }
      } else {
        if (sendCommand("resume", "0")==200) {
          //tft.pushImage(55, 134, 16, 16, PauseImg);
          TJpgDec.drawFsJpg(55, 134, "/PauseImg.jpg");
        }
      }
    } else if (x > 18 && x < 42 && y > 130 && y < 156 && NotTouching) {
      if (CurrentSaved) {
        if (deleteTrack(TrackId) == 200) {
          CurrentSaved = false;
          //tft.pushImage(22, 134, 16, 16, NotSavedImg);
          TJpgDec.drawFsJpg(22, 134, "/NotSavedImg.jpg");
          
        }
      } else {
        if (saveTrack(TrackId) == 200) {
          CurrentSaved = true;
          //ft.pushImage(22, 134, 16, 16, SavedImg);
          TJpgDec.drawFsJpg(22, 134, "/SavedImg.jpg");
        }
      }
    } else if (x > 18 && x < 42 && y > 86 && y < 112 && NotTouching) {
      if (Loop == 2) {
        Loop = -1;
      }
      if (sendCommand("set_options", String(Loop + 1))==200) {
        /*if (Loop == 0) {
          tft.pushImage(22, 90, 16, 16, Loop0Img);
        } else if (Loop == 1) {
          tft.pushImage(22, 90, 16, 16, Loop1Img);
        } else {
          tft.pushImage(22, 90, 16, 16, Loop2Img);
        }*/
        TJpgDec.drawFsJpg(22, 90, "/Loop"+String(Loop)+"Img.jpg");
      }
    } else if (x > 74 && x < 246 && y > 220 && y < 232 && NotTouching) {
      TrackSliding=1;
    } else if (x > 251 && x < 263 && y > 43 && y < 153 && NotTouching) {
      TrackSliding=2;
    } else if (x > 278 && x < 302 && y > 130 && y < 156 && NotTouching) {
      Tab=1;
      tft.fillScreen(Color);
      tft.setTextSize(1);
      OpenInfo1();
      OpenInfo();
    } else if (x > 278 && x < 302 && y > 86 && y < 112 && NotTouching) {
      Tab=2;
      OldTime="";
      tft.fillScreen(Color);
      tft.setCursor(9,131);
      OpenSearch();
    }
    } else if (Tab==2) {
      if (NotTouching) {
        int X=x/32;
        X+=10*int((y-144)/31);
        if (y<144) {
          X=-1;
        }
        if (X==29) {
          Tab=0;
          buildUIFull(-100);
        } else if (X==19) {
          int StrL=OldTime.length();
          int NewX=tft.getCursorX()-tft.textWidth(OldTime.substring(StrL-1,StrL),2);
          tft.fillRect(NewX,tft.getCursorY(),14, 15, Color);
          tft.setCursor(NewX,tft.getCursorY());
          tft.print(KeyB[X]);
          OldTime=OldTime.substring(0,StrL-1);
          //Serial.println(OldTime);
          addResults();
        } else if (X==28) {

        } else if (X>=0) {
          tft.print(KeyB[X]);
          OldTime+=KeyB[X];
          addResults();
        } else if (x > 295 && x < 316 && y > 3 && y < 24){
          startSong(Id0Sh);
        } else if (x > 295 && x < 316 && y > 23 && y < 44){
          if(sendCommand("add_to_queue",Id0Sh)==200) {
            tft.setTextColor(TFT_GREEN, Color);
            tft.setTextSize(3);
            tft.drawString("+", 298, 23, 1);
            tft.setTextSize(1);
            tft.setTextColor(TFT_WHITE, Color);
          }
        } else if (x > 295 && x < 316 && y > 43 && y < 64){
          if (tft.readPixel(305,53)) {
            if (deleteTrack(Id0Sh) == 200) {
              TJpgDec.drawFsJpg(298, 46, "/NotSavedImg.jpg");
            }
          } else {
            if (saveTrack(Id0Sh)==200) {
              TJpgDec.drawFsJpg(298, 46, "/SavedImg.jpg");
            }
          }
        } else if (x > 295 && x < 316 && y > 69 && y < 90){
          startSong(Id1Sh);
        } else if (x > 295 && x < 316 && y > 89 && y < 110){
          if(sendCommand("add_to_queue",Id1Sh)==200) {
            tft.setTextColor(TFT_GREEN, Color);
            tft.setTextSize(3);
            tft.drawString("+", 298, 89, 1);
            tft.setTextSize(1);
            tft.setTextColor(TFT_WHITE, Color);
          }
        } else if (x > 295 && x < 316 && y > 109 && y < 130){
          if (tft.readPixel(305,119)) {
            if (deleteTrack(Id1Sh) == 200) {
              TJpgDec.drawFsJpg(298, 112, "/NotSavedImg.jpg");
            }
          } else {
            if (saveTrack(Id1Sh)==200) {
              TJpgDec.drawFsJpg(298, 112, "/SavedImg.jpg");
            }
          }
        }
      }
    } else {
      //if (x > 297 && y < 25 && NotTouching) {
      if (NotTouching) {
        Tab=0;
        buildUIFull(-100);
      }
    }

    if (0) {
      tft.setTextSize(1);
      tft.setCursor(5, 5, 2);
      tft.printf("x: %i     ", x);
      tft.setCursor(5, 20, 2);
      tft.printf("y: %i    ", y);

      //tft.drawPixel(x, y, color);

    }
    NotTouching = 0;
  } else {
    NotTouching = 1;
    if (TrackSliding==1) {
      //int TimeSeek=SongTime*(x-76)/168

      sendCommand("seek_to",String(SongTime*(x-76)/168));
    }
    TrackSliding=0;
  }
  //delay(1000);
}
