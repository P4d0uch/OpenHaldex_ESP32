#include <openhaldex.h>


// Global CAN message buffer
const int MAX_MSGS = 50;
can_frame replayMessages[MAX_MSGS];
char replayBus[MAX_MSGS][3];  // H or B for each frame

int replayMsgCount = 0;

// --- Serial command task ---
void serialCommandTask(void *pvParameters) {
    String inputBuffer = "";
    Serial.println("Serial command task started. Commands: REPLAY, STOP, STOPTASKS, STARTTASKS");

    for (;;) {
        while (Serial.available() > 0) {
            char c = Serial.read();
            if (c == '\r') continue;
            if (c == '\n') {
                if (inputBuffer.length() == 0) continue;

                inputBuffer.trim();
                inputBuffer.toUpperCase();
                Serial.printf("Command received: %s\n", inputBuffer.c_str());

                if (inputBuffer == "REPLAY") {
                    if (!replayActive) {
                        //stopAllCanTasks();
                        replayMsgCount = 0;
                        replayActive = true;
                    }
                }
                else if (inputBuffer == "START"){
                
                        xTaskCreatePinnedToCore(replayTask, "replayTask", 4096, NULL, 3, &replay_task, 1);
                        Serial.println("Replay mode started. Paste CAN messages and type START.");
                    
                }
                else if (inputBuffer == "STOP") {
                    stopReplayMode();
                }
                else if (inputBuffer == "STOPTASKS") {
                    stopAllCanTasks();
                }
                else if (inputBuffer == "STARTTASKS") {
                    startAllCanTasks();
                }
                else if (replayActive) {
                    handleReplayInput(inputBuffer);
                }
                else {
                    Serial.printf("Unknown command: %s\n", inputBuffer.c_str());
                }

                inputBuffer = "";
            } else {
                inputBuffer += c;
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void handleReplayInput(const String &line) {
    if (line.equalsIgnoreCase("START")) {
        Serial.printf("Starting replay of %d messages...\n", replayMsgCount);
        return;
    }

    if (line.length() < 3) return; // need at least 2 chars for bus + space

    // Extract first two characters as bus ID
    char busChars[3] = { line.charAt(0), line.charAt(1), '\0' };
    String rest = line.substring(2);
    rest.trim();

    // Parse hex CAN frame
    char buf[64];
    rest.toCharArray(buf, sizeof(buf));

    char* token = strtok(buf, " ");
    if (!token) return;

    uint32_t id = strtoul(token, NULL, 16);
    byte data[8];
    int len = 0;

    while ((token = strtok(NULL, " ")) && len < 8) {
        data[len++] = (byte)strtoul(token, NULL, 16);
    }

    if (len > 0 && replayMsgCount < MAX_MSGS) {
        replayMessages[replayMsgCount].id = id;
        replayMessages[replayMsgCount].len = len;
        memcpy(replayMessages[replayMsgCount].data.bytes, data, len);

        strcpy(replayBus[replayMsgCount], busChars); // store 2-char bus ID

        replayMsgCount++;
        Serial.printf("Stored frame [%s] ID=0x%03X, len=%d\n", busChars, id, len);
    } else {
        Serial.println("Invalid frame or buffer full.");
    }
}

// --- Replay task ---
void replayTask(void *param) {
    Serial.println("Replay task active. Type STOP to exit.");
    while (replayActive) {
        //Serial.println("Replay while\n");
        for (int i = 0; i < replayMsgCount; i++) {
            if (strcmp(replayBus[i], "BI") == 0) {
                replayWithoutSending = true;
                xQueueSendToBack(body_can.inbox, &replayMessages[i], 0);
                addToHistory(body_inbox_history,&body_inbox_history_index,replayMessages[i]);
            } else if (strcmp(replayBus[i], "HI") == 0) {
                replayWithoutSending = true;
                xQueueSendToBack(haldex_can.inbox, &replayMessages[i], 0);
                addToHistory(haldex_inbox_history,&haldex_inbox_history_index,replayMessages[i]);
            }
            else if (strcmp(replayBus[i], "BO") == 0) {
                replayWithoutSending = true;
               xQueueSendToBack(body_can.outbox, &replayMessages[i], 0);
               addToHistory(body_outbox_history,&body_outbox_history_index,replayMessages[i]);
            } else if (strcmp(replayBus[i], "HO") == 0) {
                replayWithoutSending = true;
                xQueueSendToBack(haldex_can.outbox, &replayMessages[i], 0);
                addToHistory(haldex_outbox_history,&haldex_outbox_history_index,replayMessages[i]);
            }
            else if (strcmp(replayBus[i], "BS") == 0) {
                replayWithoutSending = false;
                body_can.can_interface->sendMsgBuf(
                replayMessages[i].id, replayMessages[i].len, replayMessages[i].data.bytes);
            } else if (strcmp(replayBus[i], "HS") == 0) {
                replayWithoutSending = false;
                haldex_can.can_interface->sendMsgBuf(
                replayMessages[i].id, replayMessages[i].len, replayMessages[i].data.bytes);
            }
            vTaskDelay(10 / portTICK_PERIOD_MS); // adjustable delay
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    Serial.println("Replay task exiting.");
    vTaskDelete(NULL);
}

// --- Control CAN tasks ---
void stopAllCanTasks() {
    if (body_can.comms_task) vTaskSuspend(body_can.comms_task);
    if (haldex_can.comms_task) vTaskSuspend(haldex_can.comms_task);
    if (haldex_status_update_task) vTaskSuspend(haldex_status_update_task);
    Serial.println("CAN tasks suspended.");
}

void startAllCanTasks() {
    if (body_can.comms_task) vTaskResume(body_can.comms_task);
    if (haldex_can.comms_task) vTaskResume(haldex_can.comms_task);
    if (haldex_status_update_task) vTaskResume(haldex_status_update_task);
    Serial.println("CAN tasks resumed.");
}

void stopReplayMode() {
    if (!replayActive) return;
    replayActive = false;
    if (replay_task) {
        vTaskDelete(replay_task);
        replay_task = NULL;
    }
    xQueueReset(body_can.outbox);
    xQueueReset(haldex_can.outbox);
    xQueueReset(body_can.inbox);
    xQueueReset(haldex_can.inbox);
    startAllCanTasks();

    Serial.println("Exited REPLAY mode. Tasks resumed.");
}