/***************************************************************************
 *   Copyright (C) 2021 by Federico Amedeo Izzo IU2NUO,                    *
 *                         Niccolò Izzo IU2KIN,                            *
 *                         Frederik Saraci IU2NRO,                         *
 *                         Silvano Seva IU2KWO                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <interfaces/delays.h>
#include <interfaces/audio.h>
#include <interfaces/audio_path.h>
#include <interfaces/audio_stream.h>
#include <interfaces/gpio.h>
#include <interfaces/platform.h>
#include <HR_C5000.h>
#include <hwconfig.h>
#include <cstring>

HR_C5000& c5000 = HR_C5000::instance();

extern "C"
{
    extern const unsigned char audio_raw[];
}

size_t audio_pos = 0;

void fillData()
{
    uint8_t sound[64];

    for(int i = 0; i < 64; i += 2)
    {
        sound[i + 1] = audio_raw[audio_pos + 0];
        sound[i + 0] = audio_raw[audio_pos + 1];

        audio_pos += 2;
        if(audio_pos >= 160582) audio_pos = 0;
    }

    c5000.sendAudio(sound);
}


int main()
{
    platform_init();

    gpio_setMode(SPK_MUTE, OUTPUT);
    gpio_clearPin(SPK_MUTE);

    gpio_setMode(AUDIO_AMP_EN, OUTPUT);
    gpio_setPin(AUDIO_AMP_EN);

    gpio_setMode(DMR_SW, OUTPUT);
    gpio_setPin(DMR_SW);


    c5000.init();
    c5000.fmMode();

    // LineOutEn = 1
    c5000.writeCfgRegister(0x0E, (1<<3));

    // HPoutEn = 1, HPOutVol = 6dB
    // c5000.writeCfgRegister(0x0E, (1<<3) | (1 << 5) | (1 << 4));


    platform_ledOff(GREEN);
    platform_ledOff(RED);

    // OpenMusic = 1, DMRFrom = input from SPI1
    c5000.writeCfgRegister(0x06, 0x02);

    while(1)
    {
        unsigned long long now = getTick();
        uint8_t reg = c5000.readCfgRegister(0x88);
        if((reg & 0x01) == 0)
        {
            platform_ledOn(GREEN);
            fillData();
        } else {
            platform_ledOn(RED);
        }

        now += 4;
        sleepUntil(now);

    }

    return 0;
}
