#ifndef __ANEMMQTT__
#define __ANEMMQTT__

#include "anemometer.h"
#include <PubSubClient.h>

class MQTTAnem{
  protected:
    char _buf[32];
    char _buf2[32];

    Anemometer *_anem;
    PubSubClient *_client;
    const char *_bname;
    uint16_t _avg;
    int32_t _frame[4];
    uint8_t _chans[16];
    uint8_t *_p;
    uint8_t *_h;
    uint8_t *_a;
    uint8_t *_t;
    void _callback(char* topic, byte* payload, unsigned int length);
    void _cb_bmp(char* topic, uint8_t *payload, unsigned int length);
    void _cb_dht(char* topic, uint8_t *payload, unsigned int length);
    void _cb_ai(char* topic, uint8_t *payload, unsigned int length);
    void _cb_temp(char* topic, uint8_t *payload, unsigned int length);


  public:
    MQTTAnem(const char *bname, Anemometer *anem, PubSubClient *client);
    void initialize(Anemometer *anem, PubSubClient *client);
    void publish_params();
    void clear_buffer(char *b, int n);
    void loop();
    void set_avg(uint16_t avg);
    int32_t *frames() { return _frame;}
    void scan();
    void bmp_chans(char *s);
    void dht_chans(char *s);
    void temp_chans(char *s);
    void ai_chans(char *s);
    void fill_chans(char *buf, uint8_t *c, uint8_t n);
    
    void set_callback();


};


#endif // __ANEMMQTT__
