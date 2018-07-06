#include "HX711Dual.h"

#include <Arduino.h>
#include <stdint.h>

HX711Dual::HX711Dual(int DT, int SCK, ChannelGain_e ch_a_gain,
    ChannelGain_e ch_b_gain)
    : _dt_pin(DT), _sck_pin(SCK), _ch_a_offset(0.0f), _ch_b_offset(0.0f),
    _ch_a_gain(ch_a_gain), _ch_b_gain(ch_b_gain)
{
    if(this->_ch_b_gain != CHANNEL_B_GAIN_32) {
        this->_ch_b_gain = CHANNEL_B_GAIN_32;
    }
    if(this->_ch_a_gain == CHANNEL_B_GAIN_32) {
        this->_ch_a_gain = CHANNEL_A_GAIN_128;
    }

    pinMode(this->_dt_pin,    INPUT);
    pinMode(this->_sck_pin,   OUTPUT);

    digitalWrite(this->_sck_pin, HIGH);
	delayMicroseconds(100);
	digitalWrite(this->_sck_pin, LOW);
}

HX711Dual::~HX711Dual() {
    pinMode(this->_dt_pin, INPUT);
    pinMode(this->_sck_pin, INPUT);
}

void HX711Dual::_setReadingChannel(Channel_e ch)
{

    int n_clk;

    switch(ch){
    case CHANNEL_A:
        n_clk = this->_ch_a_gain;
        break;

    case CHANNEL_B:
        n_clk = this->_ch_b_gain;
        break;

    default:
        return;
    }

    while(digitalRead(this->_dt_pin));
    for(int i = 0; i < n_clk; i++) {
        digitalWrite(this->_sck_pin, HIGH);
        digitalWrite(this->_sck_pin, LOW);
    }
}

int32_t HX711Dual::_readChannel(Channel_e ch)
{
    int32_t ret = 0;

    while(digitalRead(this->_dt_pin));
    for(int bit = 23; bit > 0; bit--) {
        digitalWrite(this->_sck_pin, HIGH);
        if(digitalRead(this->_dt_pin)) {
            ret |= (1 << bit);
        }
        digitalWrite(this->_sck_pin, LOW);
    }
    digitalWrite(this->_sck_pin, HIGH);
    digitalWrite(this->_sck_pin, LOW);

    // ret ^= 0x00800000;

    if(ret & 0x00800000) {
        // negative number... extend signal over 32 bits
        ret |= 0xff000000;
    }

    return ret;
}

void HX711Dual::setChannelAGain(ChannelGain_e new_gain)
{
    if(new_gain == CHANNEL_B_GAIN_32)   return ;

    this->_ch_a_gain = new_gain;
}

void HX711Dual::setChannelBGain(ChannelGain_e new_gain)
{
    if(new_gain != CHANNEL_B_GAIN_32)   return;

    this->_ch_b_gain = new_gain;
}

void HX711Dual::channelACalibrate(int sample_number)
{
    int64_t sum = 0;

    for(int i = 0; i < sample_number; i++) {
        _setReadingChannel(CHANNEL_A);
        sum += _readChannel(CHANNEL_A);
    }

    this->_ch_a_offset = (sum / sample_number);
}

void HX711Dual::channelBCalibrate(int sample_number)
{
    int64_t sum = 0;

    for(int i = 0; i < sample_number; i++) {
        _setReadingChannel(CHANNEL_B);
        sum += _readChannel(CHANNEL_B);
    }

    this->_ch_b_offset = (sum / sample_number);
}

int32_t HX711Dual::readChannelARaw()
{
    return readChannelRaw(CHANNEL_A);
}

int32_t HX711Dual::readChannelBRaw()
{
    return readChannelRaw(CHANNEL_B);
}

int32_t HX711Dual::readChannelRaw(Channel_e ch)
{
    _setReadingChannel(ch);
    return _readChannel(ch);
}

float HX711Dual::readChannelA()
{
    float v, gain;

    _setReadingChannel(CHANNEL_A);
    v = (float)(this->readChannelARaw() - this->_ch_a_offset);
    switch(this->_ch_a_gain){
    case CHANNEL_A_GAIN_128:
        gain = 128.0f;
        break;
    case CHANNEL_A_GAIN_64:
        gain = 64.0f;
        break;
    default:
        gain = 1.0f;
        break;
    }

    return v / gain;
}
float HX711Dual::readChannelB()
{
    float v;

    _setReadingChannel(CHANNEL_B);
    v = (float)(this->readChannelBRaw() - this->_ch_b_offset);
    return v / 32.0f;
}

float HX711Dual::readChannel(Channel_e ch)
{
    float v, gain;

    _setReadingChannel(ch);
    v = (float)this->readChannelRaw(ch);
    if(ch == CHANNEL_A) {
        v -= (float)this->_ch_a_offset;
    } else {
        v -= (float)this->_ch_b_offset;
    }

    switch(ch) {
    case CHANNEL_A:
        switch(this->_ch_a_gain){
            case CHANNEL_A_GAIN_128:
            gain = 128.0f;
            break;
            case CHANNEL_A_GAIN_64:
            gain = 64.0f;
            break;
            default:
            gain = 1.0f;
            break;
        }
        break;

    case CHANNEL_B:
        gain = 32.0f;
        break;

    default:
        gain = 1.0f;
    }

    return v / gain;
}

float averageChannelA(uint8_t averageFactor = 16)
{
  float ret;

  for (uint8_t i = 0; i < averageFactor; i++)
  {
    ret += readChannelA();
  }

  return (ret / averageFactor);
}

float averageChannelB(uint8_t averageFactor = 16)
{
  float ret;

  for (uint8_t i = 0; i < averageFactor; i++)
  {
    ret += readChannelB();
  }

  return (ret / averageFactor);
}

float averageChannel(Channel_e ch, uint8_t averageFactor = 16)
{
  float ret;

  for (uint8_t i = 0; i < averageFactor; i++)
  {
    ret += readChannel(ch);
  }

  return (ret / averageFactor);
}
