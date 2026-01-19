#include "openhaldex.h"

void connectWifi()
{
    int connect_timeout;

    if (!USE_HOTSPOT)
    {
        WiFi.begin(MY_SSID, MY_PASSWORD);
        Serial.print("Connecting to WiFi");
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(10);
            Serial.print(".");
        }
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {

        WiFi.setHostname(HOSTNAME);

        Serial.println("Begin wifi...");

        // Load credentials from EEPROM
        if (!(FORCE_USE_HOTSPOT))
        {
            yield();

            connect_timeout = 28; // 7 seconds
            while (WiFi.status() != WL_CONNECTED && connect_timeout > 0)
            {
                delay(10);
                Serial.print(".");
                connect_timeout--;
            }
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println(WiFi.localIP());
            Serial.println("Wifi started");

            if (!MDNS.begin(HOSTNAME))
            {
                Serial.println("Error setting up MDNS responder!");
            }
        }
        else
        {
            Serial.println("\nCreating access point...");
            WiFi.mode(WIFI_AP);
            WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
            WiFi.softAP(HOSTNAME);

            connect_timeout = 20;
            do
            {
                delay(10);
                Serial.print(",");
                connect_timeout--;
            } while (connect_timeout);
        }
    }
}

// Utilities
//
// If you are here just to see examples of how to use ESPUI, you can ignore the following functions
//------------------------------------------------------------------------------------------------

template <typename T>
bool EEPROM_writeAnything(int address, const T &data)
{
    if (address + sizeof(T) > EEPROM_SIZE)
    {
        Serial.println("EEPROM write error: data too large!");
        return false;
    }
    EEPROM.put(address, data);
    return EEPROM.commit();
}

template <typename T>
bool EEPROM_readAnything(int address, T &data)
{
    if (address + sizeof(T) > EEPROM_SIZE)
    {
        Serial.println("EEPROM read error: data too large!");
        return false;
    }
    EEPROM.get(address, data);
    return true;
}

void saveCustomMode()
{
    if (EEPROM_writeAnything(EEPROM_ADDR_CUSTOM_MODE, state.custom_mode))
    {
        printCustomMode();
        Serial.println("Custom mode saved successfully.");
    }
    else
    {
        Serial.println("Failed to save custom mode!");
    }
}

void loadCustomMode()
{
    if (EEPROM_readAnything(EEPROM_ADDR_CUSTOM_MODE, state.custom_mode))
    {
        Serial.println("Custom mode loaded successfully.");
        printCustomMode();
    }
    else
    {
        Serial.println("Failed to load custom mode!");
    }
}

void printCustomMode()
{
    Serial.printf("Custom Mode: %d lockpoints\n", state.custom_mode.lockpoint_count);
    for (int i = 0; i < state.custom_mode.lockpoint_count; i++)
    {
        Serial.printf("  Point %d: Speed=%d, Lock=%d\n", i,
                      state.custom_mode.lockpoints[i].speed,
                      state.custom_mode.lockpoints[i].lock);
    }
}