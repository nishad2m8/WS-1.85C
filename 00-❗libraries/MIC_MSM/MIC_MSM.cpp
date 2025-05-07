#include "MIC_MSM.h"
// #include "LVGL_Music.h"
// English wakeword : Hi ESP！！！！
// Chinese wakeword : Hi 乐鑫！！！！
// 英文唤醒词 : Hi ESP！！！！
// 中文唤醒词 : Hi 乐鑫！！！！


I2SClass i2s;

// Generated using the following command:
// python3 tools/gen_sr_commands.py "Turn on the light,Switch on the light;Turn off the light,Switch off the light,Go dark;Start fan;Stop fan"
enum {
  SR_CMD_TURN_ON_THE_BACKLIGHT,
  SR_CMD_TURN_OFF_THE_BACKLIGHT,
  SR_CMD_BACKLIGHT_IS_BRIGHTEST,
  SR_CMD_BACKLIGHT_IS_DARKEST,
  SR_CMD_PLAY_MUSIC,
};
static const sr_cmd_t sr_commands[] = {
  {0, "Turn on the backlight", "TkN nN jc BaKLiT"},                 // English
  {1, "Turn off the backlight", "TkN eF jc BaKLiT"},                // English
  {2, "backlight is brightest", "BaKLiT gZ BRiTcST"},               // English
  {3, "backlight is darkest", "BaKLiT gZ DnRKcST"},                 // English
  {4, "play music", "PLd MYoZgK"},                                  // English

  // {0, "bei guang zui liang", "bei guang zui liang"},            // chinese
  // {1, "bei guang zui an", "bei guang zui an"},                  // chinese
  // {2, "guan bi bei guang", "guan bi bei guang"},                // chinese
  // {3, "da kai bei guang", "da kai bei guang"},                  // chinese
  // {4, "bo fang yin yue", "bo fang yin yue"},                    // chinese
};
bool play_Music_Flag = 0;
uint8_t LCD_Backlight_original = 0;
void Awaken_Event(sr_event_t event, int command_id, int phrase_id) {
  switch (event) {
    case SR_EVENT_WAKEWORD: 
      // if(ACTIVE_TRACK_CNT)
      //   _lv_demo_music_pause();
      printf("WakeWord Detected!\r\n"); 
      LCD_Backlight_original = LCD_Backlight;
      break;
    case SR_EVENT_WAKEWORD_CHANNEL:
      printf("WakeWord Channel %d Verified!\r\n", command_id);
      ESP_SR.setMode(SR_MODE_COMMAND);  // Switch to Command detection
      LCD_Backlight = 35;
      break;
    case SR_EVENT_TIMEOUT:
      printf("Timeout Detected!\r\n");
      ESP_SR.setMode(SR_MODE_WAKEWORD);  // Switch back to WakeWord detection
      LCD_Backlight = LCD_Backlight_original;
      if(play_Music_Flag){
        play_Music_Flag = 0;
        // if(ACTIVE_TRACK_CNT)
        //   _lv_demo_music_resume();   
        // else
        //   printf("No MP3 file found in SD card!\r\n");    
      }
      break;
    case SR_EVENT_COMMAND:
      printf("Command %d Detected! %s\r\n", command_id, sr_commands[phrase_id].str);
      switch (command_id) {
        case SR_CMD_TURN_ON_THE_BACKLIGHT:      
          LCD_Backlight = 100;  
          break;
        case SR_CMD_TURN_OFF_THE_BACKLIGHT:     
          LCD_Backlight = 0;    
          break;
        case SR_CMD_BACKLIGHT_IS_BRIGHTEST:     
          LCD_Backlight = 100;  
          break;
        case SR_CMD_BACKLIGHT_IS_DARKEST:       
          LCD_Backlight = 30;   
          break;
        case SR_CMD_PLAY_MUSIC:                 
          play_Music_Flag = 1;              
          break;
        default:                        printf("Unknown Command!\r\n"); break;
      }
      ESP_SR.setMode(SR_MODE_COMMAND);  // Allow for more commands to be given, before timeout
      // ESP_SR.setMode(SR_MODE_WAKEWORD); // Switch back to WakeWord detection
      break;
    default: printf("Unknown Event!\r\n"); break;
  }
}

void _MIC_Init() {
  i2s.setPins(I2S_PIN_BCK, I2S_PIN_WS, I2S_PIN_DOUT, I2S_PIN_DIN);
  i2s.setTimeout(1000);
  i2s.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);

  ESP_SR.onEvent(Awaken_Event);
  ESP_SR.begin(i2s, sr_commands, sizeof(sr_commands) / sizeof(sr_cmd_t), SR_CHANNELS_STEREO, SR_MODE_WAKEWORD);
}
void MICTask(void *parameter) {
  _MIC_Init();
  esp_task_wdt_add(NULL);
  while(1){
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  vTaskDelete(NULL);
  
}
void MIC_Init() {
  xTaskCreatePinnedToCore(
    MICTask,     
    "MICTask",  
    4096,                
    NULL,                 
    5,                   
    NULL,                 
    0                     
  );
}
