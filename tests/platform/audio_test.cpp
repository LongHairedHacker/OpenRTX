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
#include <HR_C6000.h>
#include <hwconfig.h>
#include <cstring>

HR_C6000& c6000 = HR_C6000::instance();

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

    c6000.sendAudio(sound);
}

// void TIM7_IRQHandler()
// {
//     TIM7->SR = 0;
//
//     uint8_t reg = c6000.readCfgRegister(0x88);
//     if((reg & 0x01) == 0)
//     {
//         fillData();
//     }
// }

int main()
{
    platform_init();
    audio_enableAmp();

    c6000.init();
    c6000.fmMode();
    c6000.writeCfgRegister(0x06, 0x22);

    gpio_setMode(RX_AUDIO_MUX, OUTPUT);
    gpio_setPin(RX_AUDIO_MUX);

//     RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
//
//     TIM7->PSC = 0;
//     TIM7->CNT = 0;
//     TIM7->ARR = 10499;
//     TIM7->EGR  = TIM_EGR_UG;
//     TIM7->DIER = TIM_DIER_UIE;
//     TIM7->CR1  = TIM_CR1_CEN;
//
//     NVIC_ClearPendingIRQ(TIM7_IRQn);
//     NVIC_SetPriority(TIM7_IRQn, 3);
//     NVIC_EnableIRQ(TIM7_IRQn);

    while(1)
    {
        unsigned long long now = getTick();
        uint8_t reg = c6000.readCfgRegister(0x88);
        if((reg & 0x01) == 0)
        {
            fillData();
        }

        now += 4;
        sleepUntil(now);

    }

    return 0;
}
