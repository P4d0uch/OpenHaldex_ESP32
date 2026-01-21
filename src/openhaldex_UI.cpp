
#include <openhaldex.h>

AsyncWebServer server(80);

/**
    Setup the UI and its functions
 */
void setUpUI()
{
    
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html); });

    server.on("/custom", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", custom_html); });


    // API: Get status - All status variables to pass to UI
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        StaticJsonDocument<256> doc;
        doc["mode"] = state.mode;
        doc["lock_target"] = lock_target;
        doc["haldex_engagement"] = haldex_engagement;
        doc["vehicle_speed"] = vehicle_speed;
		doc["pedal_value"] = ped_value;
        doc["vehicle_rpm"] = vehicle_RPM;
        doc["received_report_clutch1"] = received_report_clutch1;
        doc["received_temp_protection"] = received_temp_protection;
        doc["received_report_clutch2"] = received_report_clutch2;
        doc["received_coupling_open"] = received_coupling_open;
        doc["received_speed_limit"] = received_speed_limit;
        doc["received_status_bit_4"] = received_status_bit_4;
        doc["received_status_bit_5"] = received_status_bit_5;
        doc["received_status_bit_7"] = received_status_bit_7;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    // API: Set mode - switch the modes
    server.on("/api/setMode", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
            StaticJsonDocument<256> doc;
            deserializeJson(doc, (const char*)data);
            
            int mode = doc["mode"];
            if (mode >= MODE_STOCK && mode <= MODE_CUSTOM) {
                state.mode = (openhaldex_mode_id)mode;
                Serial.printf("Mode set to: %d\n", mode);
            }
            
            request->send(200, "application/json", "{\"success\":true}"); });

    // API: Get custom mode - Load saved data
    server.on("/api/getCustomMode", HTTP_GET, [](AsyncWebServerRequest *request)
              {
		loadCustomMode();
        DynamicJsonDocument doc(2048);
        doc["lockpoint_count"] = state.custom_mode.lockpoint_count;
        
        JsonArray lockpoints = doc.createNestedArray("lockpoints");
        for (int i = 0; i < state.custom_mode.lockpoint_count; i++) {
            JsonObject point = lockpoints.createNestedObject();
            point["speed"] = state.custom_mode.lockpoints[i].speed;
            point["lock"] = state.custom_mode.lockpoints[i].lock;

        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });


    server.on("/canlog", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", canlog_html); });

    // API: Set custom mode - Save and apply the custom lock
    server.on("/api/setCustomMode", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
            DynamicJsonDocument doc(2048);
            deserializeJson(doc, (const char*)data);
            
            state.custom_mode.lockpoint_count = doc["lockpoint_count"];
            JsonArray lockpoints = doc["lockpoints"];
            
            int idx = 0;
            for (JsonObject point : lockpoints) {
                if (idx >= NUM_LOCK_POINTS) break;
                state.custom_mode.lockpoints[idx].speed = point["speed"];
                state.custom_mode.lockpoints[idx].lock = point["lock"];
                idx++;
            }
            

			saveCustomMode();
            request->send(200, "application/json", "{\"success\":true}"); });
    

    // API: Get can history and send its data
    server.on("/api/canHistory", HTTP_GET, [](AsyncWebServerRequest *request)
              {
            StaticJsonDocument<4096> doc;
            JsonArray haldex_in = doc.createNestedArray("haldex_inbox");
            JsonArray haldex_out = doc.createNestedArray("haldex_outbox");
            JsonArray body_in = doc.createNestedArray("body_inbox");
            JsonArray body_out = doc.createNestedArray("body_outbox");

            auto addFrame = [](JsonArray arr, const can_frame &f) {
                JsonObject obj = arr.createNestedObject();
                obj["id"] = f.id;
                obj["len"] = f.len;

                char dataStr[3 * 8 + 1] = {0};
                for (byte i = 0; i < f.len; i++) {
                char tmp[4];
                sprintf(tmp, "%02X ", f.data.bytes[i]);
                strcat(dataStr, tmp);
                
                }
                obj["data"] = dataStr;
            };

            for (int i = 0; i < 200; i++) {
                addFrame(haldex_in, haldex_inbox_history[i]);
                addFrame(haldex_out, haldex_outbox_history[i]);
                addFrame(body_in, body_inbox_history[i]);
                addFrame(body_out, body_outbox_history[i]);
            }

            String response;
            serializeJson(doc, response);
            request->send(200, "application/json", response); });

    server.begin();
    Serial.println("Web server started!");
}
