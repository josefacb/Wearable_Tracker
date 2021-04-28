#pragma mark - Depend arduino-LoRa library
/*
cd ~/Arduino/libraries
git clone https://github.com/sandeepmistry/arduino-LoRa.git
*/

#include "config.h"
#include <LoRa.h>


TTGOClass *ttgo;

AXP20X_Class *power;
BMA *sensor;
TFT_eSPI *tft;

bool irq = false, isScreenOn = true;

uint8_t brightness = 150;

uint32_t state = 0, prev_state = 0;
bool isInit = false;
lv_obj_t *btn2, *btn1, *ta1, *gContainer, *btn3, *btn4, *btn5, *btn6, *isConnected, *btn_brightness_plus, *btn_brightness_minus;
SPIClass LoraSPI(HSPI);
uint32_t sendCount = 0;
String recv = "";
char buf[256];


void createWin();
void add_message(const char *txt);

#define LORA_PERIOD 868

#if LORA_PERIOD == 433
#define BAND 433E6
#elif LORA_PERIOD == 868
#define BAND 868E6
#elif LORA_PERIOD == 915
#define BAND 915E6
#else
#define BAND 470E6
#endif

//Function prototypes
void bma();
void batteryState();

/*Thingspeak*/
void lora_thingspeak();
float  tem;
char tem_1[8]={"\0"};   
char *node_id = "<5679>";  //From LG01 via web Local Channel settings on MQTT.Please refer <> dataformat in here. 
uint8_t datasend[36];
unsigned int count = 1; 
unsigned long new_time,old_time=0;
void ir()
{
  tem=digitalRead(3);
  Serial.println(tem);
}
void irWrite()
{
    char data[50] = "\0";
    for(int i = 0; i < 50; i++)
    {
       data[i] = node_id[i];
    }

    dtostrf(tem,0,1,tem_1);
  

    // Serial.println(tem_1);
     strcat(data,"field1=");
     strcat(data,tem_1);
     strcpy((char *)datasend,data);  
     Serial.println((char *)datasend);
}
void SendData()
{
     LoRa.beginPacket();
     LoRa.print((char *)datasend);
     LoRa.endPacket();
     Serial.println("Packet Sent");
}

static void brightness_handler(lv_obj_t *obj, lv_event_t event){
    if (obj == btn_brightness_plus && brightness < 255){
        brightness = brightness + 1;
        ttgo->setBrightness(brightness);
    } else if (obj == btn_brightness_minus && brightness > 0){
        brightness = brightness - 1;
        ttgo->setBrightness(brightness);
    }
}


static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    isInit = false;
    if (!ta1) {
        createWin();
    }
    if (obj == btn1) {
        state = 1;
        add_message("Lora Sender...");
    } else if (obj == btn2) {
        state = 2;
        add_message("Lora Received...");
    } else if (obj == btn3) {
        state = 3;
        add_message("Shutting down screen...");
    } else if (obj == btn4) {
        state = 4;
        //add_message("Battery information...");

        ttgo->displayOff(); //!!!
        ttgo->closeBL();
        
        power = ttgo->power;
        // ADC monitoring must be enabled to use the AXP202 monitoring function
        ttgo->power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);
        
        isConnected = lv_label_create(lv_scr_act(), NULL);
        lv_obj_set_style_local_text_color(isConnected, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        lv_label_set_long_mode(isConnected,   LV_LABEL_LONG_BREAK);
        lv_label_set_text(isConnected, "isConnected");
        lv_obj_set_width(isConnected, 239);
        lv_obj_align(isConnected, NULL, LV_ALIGN_CENTER, 0, 0);
    } else if (obj == btn5){
        state = 5;
    } else if (obj == btn6){

        sensor = ttgo->bma;
        // Accel parameter structure
        Acfg cfg;
        /*!
            Output data rate in Hz, Optional parameters:
                - BMA4_OUTPUT_DATA_RATE_0_78HZ
                - BMA4_OUTPUT_DATA_RATE_1_56HZ
                - BMA4_OUTPUT_DATA_RATE_3_12HZ
                - BMA4_OUTPUT_DATA_RATE_6_25HZ
                - BMA4_OUTPUT_DATA_RATE_12_5HZ
                - BMA4_OUTPUT_DATA_RATE_25HZ
                - BMA4_OUTPUT_DATA_RATE_50HZ
                - BMA4_OUTPUT_DATA_RATE_100HZ
                - BMA4_OUTPUT_DATA_RATE_200HZ
                - BMA4_OUTPUT_DATA_RATE_400HZ
                - BMA4_OUTPUT_DATA_RATE_800HZ
                - BMA4_OUTPUT_DATA_RATE_1600HZ
        */
        cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
        /*!
            G-range, Optional parameters:
                - BMA4_ACCEL_RANGE_2G
                - BMA4_ACCEL_RANGE_4G
                - BMA4_ACCEL_RANGE_8G
                - BMA4_ACCEL_RANGE_16G
        */
        cfg.range = BMA4_ACCEL_RANGE_2G;
        /*!
            Bandwidth parameter, determines filter configuration, Optional parameters:
                - BMA4_ACCEL_OSR4_AVG1
                - BMA4_ACCEL_OSR2_AVG2
                - BMA4_ACCEL_NORMAL_AVG4
                - BMA4_ACCEL_CIC_AVG8
                - BMA4_ACCEL_RES_AVG16
                - BMA4_ACCEL_RES_AVG32
                - BMA4_ACCEL_RES_AVG64
                - BMA4_ACCEL_RES_AVG128
        */
        cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;

        /*! Filter performance mode , Optional parameters:
            - BMA4_CIC_AVG_MODE
            - BMA4_CONTINUOUS_MODE
        */
        cfg.perf_mode = BMA4_CONTINUOUS_MODE;

        // Configure the BMA423 accelerometer
        sensor->accelConfig(cfg);

        // Enable BMA423 accelerometer
        // Warning : Need to use steps, you must first enable the accelerometer
        // Warning : Need to use steps, you must first enable the accelerometer
        // Warning : Need to use steps, you must first enable the accelerometer
        sensor->enableAccel();

        sensor->enableFeature(BMA423_ACTIVITY, true);


        state = 6;
    }

}

void add_message(const char *txt)
{
    if (!txt || !ta1)return;
    if (strlen(lv_textarea_get_text(ta1)) >= lv_textarea_get_max_length(ta1)) {
        lv_textarea_set_text(ta1, "");
    }
    String str = txt;
    str.trim();
    str += "\n";
    lv_textarea_add_text(ta1, str.c_str());
}

static void menu1(){
  lv_obj_clean(gContainer);

    ta1 = NULL;
    lv_obj_t *label ;

    /*Create a brightness button*/
    btn_brightness_plus = lv_btn_create(gContainer, NULL);
    lv_obj_set_size(btn_brightness_plus, 50, 45);
    lv_obj_align(btn_brightness_plus, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -2, -2);
    lv_obj_set_event_cb(btn_brightness_plus, brightness_handler);

    /*Add a label to the button*/
    label = lv_label_create(btn_brightness_plus, NULL);
    lv_label_set_text(label, "+");

    btn_brightness_minus = lv_btn_create(gContainer, btn_brightness_plus);
    lv_obj_align(btn_brightness_minus, btn_brightness_plus, LV_ALIGN_OUT_LEFT_MID, -2, 0);
    lv_obj_set_event_cb(btn_brightness_minus, brightness_handler);

    label = lv_label_create(btn_brightness_minus, NULL);
    lv_label_set_text(label, "-");


    /*Create a normal button*/
    btn1 = lv_btn_create(gContainer, NULL);
    lv_obj_set_size(btn1, 115, 45);
    lv_obj_align(btn1, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 10);
    lv_obj_set_event_cb(btn1, event_handler);

    /*Add a label to the button*/
    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Sender");

    btn2 = lv_btn_create(gContainer, btn1);
    lv_obj_align(btn2, btn1, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_event_cb(btn2, event_handler);

    label = lv_label_create(btn2, NULL);
    lv_label_set_text(label, "Receiver");

    btn3 = lv_btn_create(gContainer, btn2);
    lv_obj_align(btn3, btn2, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_event_cb(btn3, event_handler);

    label = lv_label_create(btn3, NULL);
    lv_label_set_text(label, "Screen off");

    btn4 = lv_btn_create(gContainer, btn3);
    lv_obj_align(btn4, btn3, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_event_cb(btn4, event_handler);

    label = lv_label_create(btn4, NULL);
    lv_label_set_text(label, "battery");

    btn5 = lv_btn_create(gContainer, btn4);
    lv_obj_align(btn5, NULL, LV_ALIGN_IN_TOP_RIGHT, -5, 10);
    lv_obj_set_event_cb(btn5, event_handler);

    label = lv_label_create(btn5, NULL);
    lv_label_set_text(label, "lora thingspeak");

    btn6 = lv_btn_create(gContainer, btn5);
    lv_obj_align(btn6, btn5, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_event_cb(btn6, event_handler);

    label = lv_label_create(btn6, NULL);
    lv_label_set_text(label, "bma");
}

static void  createGui(int menu)
{
  menu1();
/*
  switch (menu) {
    case 1: menu1(); break;
    case 2: menu2(); break;
  }
*/  

}

void createWin()
{
    lv_obj_clean(gContainer);
    ta1 = lv_textarea_create(gContainer, NULL);
    lv_obj_set_size(ta1, LV_HOR_RES, LV_VER_RES);
    lv_obj_align(ta1, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_textarea_set_text(ta1, "");    /*Set an initial text*/
    lv_textarea_set_max_length(ta1, 128);
}


void pressed()
{
    isInit = false;
    state = 0;
    sendCount = 0;
    createGui(0);
}

void setup(void)
{
    Serial.begin(9600);

    ttgo = TTGOClass::getWatch();
    ttgo->begin();
    ttgo->tft->fillScreen(TFT_BLACK);
    ttgo->openBL();

    ttgo->setBrightness(brightness);

    ttgo->lvgl_begin();

    pinMode(AXP202_INT, INPUT_PULLUP);
    attachInterrupt(AXP202_INT, [] {
        irq = true;
    }, FALLING);

    //!Clear IRQ unprocessed  first
    ttgo->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
    ttgo->power->clearIRQ();

    //! Open lora module power
    ttgo->enableLDO3();

    ttgo->button->setPressedHandler(pressed);

    gContainer = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(gContainer,  LV_HOR_RES, LV_VER_RES);

    /*lv_obj_t *label = lv_label_create(gContainer, NULL);
    lv_label_set_text(label, "Begin Lora Module");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);*/


    LoraSPI.begin(TWATCH_LORA_SCK, TWATCH_LORA_MISO, TWATCH_LORA_MOSI, TWATCH_LORA_SS);
    LoRa.setSPI(LoraSPI);
    LoRa.setPins(TWATCH_LORA_SS, TWATCH_LORA_RST, TWATCH_LORA_DI0);
    LoRa.setSpreadingFactor(7);
    LoRa.setSyncWord(52);
    LoRa.setCodingRate4(5);
    LoRa.setSignalBandwidth(125000);
    //LoRa.setTxPower(20);

    int ret = 0;
    do {
        lv_task_handler();
        ret = LoRa.begin(BAND);
        delay(500);
    } while (!ret);
    Serial.println("LORA Begin PASS");

    createGui(0);

    lv_task_create([](lv_task_t *args) {
        ttgo->button->loop();
    }, 30, 1, nullptr);

}

uint32_t startmillis = 0;

void loop(void)
{
    if (irq) {
        irq = false;
        ttgo->power->readIRQ();
        if (ttgo->power->isPEKShortPressIRQ()) {
            isScreenOn = !isScreenOn;
            if(isScreenOn){ 
                ttgo->displayWakeup(); 
                ttgo->openBL();
            } else {
                ttgo->displayOff();
                ttgo->closeBL();
            }
        }
        ttgo->power->clearIRQ();
    }
    switch (state) {
    case 1:
        if (millis() - startmillis > 1000 ) {
            startmillis = millis();
            LoRa.beginPacket();
            LoRa.print("lora: ");
            LoRa.print(sendCount);
            LoRa.endPacket();
            ++sendCount;
            snprintf(buf, sizeof(buf), "Send %lu\n", sendCount);
            Serial.println(buf);
            add_message(buf);
        }
        break;
    case 2:
        if (LoRa.parsePacket()) {
            recv = "";
            while (LoRa.available()) {
                recv += (char)LoRa.read();
            }
            snprintf(buf, sizeof(buf), "Received:%s - rssi:%d\n", recv.c_str(), LoRa.packetRssi());
            Serial.println(buf);
            add_message(buf);
        }
        break;
    case 3:
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        //ttgo->closeBL();
        ttgo->displayOff();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        ttgo->displayWakeup();
        //ttgo->openBL();
        break;
    case 4:
        batteryState();
        break;
    case 5:
        lora_thingspeak();
        break;
    case 6:
        bma();
        break;
    default:
        break;
    }
    lv_task_handler();
    delay(5);
}

void bma(){

    
    String a = sensor->getActivity();
    Serial.println(a);
}

void lora_thingspeak(){
    new_time=millis();
    if (new_time - old_time >= 10000 || old_time == 0)
    {
      old_time = new_time;
      Serial.print("###########    ");
      Serial.print("COUNT=");
      Serial.print(count);
      Serial.println("    ###########");
      count++;
      ir();
      irWrite();
      SendData();
      LoRa.receive();
    }
}

void batteryState(){
    new_time=millis();
    char data[50];
    uint8_t charging;
    if (new_time - old_time >= 9000 || old_time == 0)
    {
        old_time = new_time;
        if (power->isVBUSPlug()) {
            //lv_label_set_text(isConnected, "Connected");
            //strcpy(data,"<5680>field1=1");
            charging = 1;
        } else
        {
            //lv_label_set_text(isConnected, "Disconnected");
            //strcpy(data,"<5680>field1=0");
            charging = 0;
        }
        sprintf((char *)datasend, "<5680>field1=%d&field2=%.4f", charging, (power->getBattVoltage()/1000));
        /*strcpy((char *)datasend,data);
        strcat((char *)datasend, "&");
        ftoa(power->getBattVoltage(), data);
        strcat((char *)datasend, data);*/
        Serial.println((char *)datasend);
        SendData();
        LoRa.receive();
        
    }
}