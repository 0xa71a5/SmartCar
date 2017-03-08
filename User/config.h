/*
******************************************************************************
* @file    config.h
* @author  Tacu Lee 
* @version V1.0.0
* @version V2.0.0 2016.12.1 (Li Zaihan)
* @brief   This file is a demo code for the contest of 2016 SEU intelligent car.
******************************************************************************
*/

#ifndef CONFIG_H_
#define CONFIG_H_

//本例程根据岱默科技OV7620编写，只是定义，没有具体作用。
//提供的DEMO板自带7474分频器，将像素为320*240，变为160*120。
#define OV7620

//在有OLED的情况下，如果要使用OLED，就设置为YES，否则设置为NO。
#define OLED	        YES
//在有OLED的情况下，如果要使用OLED显示图像，就设置为YES，否则设置为NO。
#define DISPIMAGE	YES
//在有OLED的情况下，如果要使用OLED显示边线，就设置为YES，否则设置为NO。
#define DISPEDGE	NO

#endif /* CONFIG_H_ */
