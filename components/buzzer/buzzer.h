#ifndef __BUZZER_H__
#define __BUZZER_H__

#define BUZZER_GPIO 18

/**
 * @brief initialize buzzer
 * 
 */
void init_buzzer();

/**
 * @brief function to alarm with buzzer
 */
void alarm_buzzer();

#endif