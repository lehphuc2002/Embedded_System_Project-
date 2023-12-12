/*
 * encoder.c
 *
 *  Created on: Dec 2, 2023
 *      Author: lehuu
 */
#include "encoder.h"

void encoder()
{
  if (data_after_cut[0] == 0x44) //reset
  {
    htim2.Instance->CNT = 0;
    instance_enc.speed_by_encoder = 0;
    instance_enc.pre_speed_by_encoder = 0;
  }
  else
  {
    instance_enc.speed_by_encoder = htim2.Instance->CNT - instance_enc.pre_speed_by_encoder;   // so xung giua 2 lan doc encoder
    //	htim2.Instance->CNT = 0;
    instance_enc.pre_speed_by_encoder = htim2.Instance->CNT;
    //	instance_enc.speed_by_encoder = htim2.Instance->CNT;
    instance_enc.position += instance_enc.speed_by_encoder;
  }
//  //	htim2.Instance->CNT = 0;
}

