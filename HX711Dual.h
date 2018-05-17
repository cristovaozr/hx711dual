#ifndef HX711DUAL_H_
#define HX711DUAL_H_

#include <stdint.h>

typedef enum _channel_gain {
    CHANNEL_A_GAIN_128 = 25,
    CHANNEL_B_GAIN_32 = 26,
    CHANNEL_A_GAIN_64 = 27
} ChannelGain_e;

typedef enum _channel {
    CHANNEL_A,
    CHANNEL_B
} Channel_e;

class HX711Dual {

public:
    explicit HX711Dual(int DT, int SCK,
        ChannelGain_e ch_a_gain = CHANNEL_A_GAIN_128,
        ChannelGain_e ch_b_gain = CHANNEL_B_GAIN_32);
    virtual ~HX711Dual();

    void setChannelAGain(ChannelGain_e new_gain);
    void setChannelBGain(ChannelGain_e new_gain);

    void channelACalibrate(int sample_number = 16);
    void channelBCalibrate(int sample_number = 16);
    void channelCalibrate(Channel_e ch, int sample_number = 16);

    int32_t readChannelARaw();
    int32_t readChannelBRaw();
    int32_t readChannelRaw(Channel_e ch);

    float readChannelA();
    float readChannelB();
    float readChannel(Channel_e ch);

private:

    void _setReadingChannel(Channel_e ch);
    int32_t _readChannel(Channel_e ch);

    int _dt_pin;
    int _sck_pin;

    int32_t _ch_a_offset;
    int32_t _ch_b_offset;

    ChannelGain_e _ch_a_gain;
    ChannelGain_e _ch_b_gain;
};

#endif
