#include "iotDATABASE.h"

WiFiClient sClient;
HTTPClient sHttp;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffset * 3600);

void InitializeFirebase()
{

  config.api_key = DB_API_KEY;
  config.database_url = DB_HOST;
  config.token_status_callback = tokenStatusCallback;

  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASS;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);

  timeClient.begin();
}

void SaveFirebase(float fT, float fH)
{
  timeClient.update();
  dataTime = timeClient.getEpochTime();
  String path = (String)DB_NODE + "/" + String(dataTime);
  if (sClient.connect(DB_HOST, SERVER_PORT))
  {

    Serial.println(Firebase.setFloat(fbdo, path + "/temp", fT) ? "// ==== Saving Temperature" : fbdo.errorReason().c_str());
    Serial.println(Firebase.setFloat(fbdo, path + "/humi", fH) ? "// ==== Saving Humidity" : fbdo.errorReason().c_str());
  }
}

void InitializeThingSpeak()
{
  if (sClient.connect(THINK_SERVER, SERVER_PORT))
  {
    ThingSpeak.begin(sClient);
  }
}

void SaveThingSpeak(float sT, float sH)
{

  Serial.println("// ==== Saving Temperature");
  ThingSpeak.setField(1, sT);
  Serial.println("// ==== Saving Humidity");
  ThingSpeak.setField(2, sH);
  ThingSpeak.writeFields(THINK_CHANNEL, THINK_KEY);
}
