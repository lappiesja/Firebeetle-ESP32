//
// Created by hiwsz on 2019/11/21.
//

#include <sys/types.h>
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include "include/oled_api.h"
#include "include/oled_data.h"
#include "include/oled_define.h"

void oled_module_open(void)
{
    oled_gpio_init();
    oled_reg_init();
    OLED_CS_L;
}

void oled_gpio_init(void)
{
    gpio_set_direction(OLED_SCLK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(OLED_SDIN_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(OLED_DC_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(OLED_RES_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(OLED_CS_PIN, GPIO_MODE_OUTPUT);
}

void delay_some_us(uint32_t us)
{
    for (int i = 0; i < us; ++i)
    {
        for (int j = 0; j < 26000; ++j)
        {
        }
    }
}

void misc_delay_ms(uint32_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void oled_reg_init(void)
{

    OLED_RES_L;            //�͵�ƽ��λ
    misc_delay_ms(500);
    OLED_RES_H;            //��λ���
    misc_delay_ms(200);

    oled_write_command_8_bit(0xae);    //����ʾ

    oled_write_command_8_bit(0xd5);    //����Ƶ��
    oled_write_command_8_bit(0x80);

    oled_write_command_8_bit(0xa8);    //duty����
    oled_write_command_8_bit(0x3f);    //duty=1/64


    oled_write_command_8_bit(0xdc);
    oled_write_command_8_bit(0x00);

    oled_write_command_8_bit(0xd3);
    oled_write_command_8_bit(0x60); //ƫ�Ƶ�ַ������Ҫ

    oled_write_command_8_bit(0x20);    //page address mode


    oled_write_command_8_bit(0xc0); //��ɨ��˳�򣺴�����  ����
    oled_write_command_8_bit(0xa0); //��ɨ��˳�򣺴��ϵ���  ����


    oled_write_command_8_bit(0x81); //΢���Աȶ�,��ָ���0x81��Ҫ�Ķ����������ֵ
    oled_write_command_8_bit(0xcf); //΢���Աȶȵ�ֵ�������÷�Χ0x00��0xff


    oled_write_command_8_bit(0xd9); //Set Pre-Charge Period
    oled_write_command_8_bit(0xf1);

    oled_write_command_8_bit(0xdb); //Set VCOMH Deselect Level
    oled_write_command_8_bit(0x40);

    //oled_clean_screen();
    oled_write_command_8_bit(0xaf); //����ʾ
}

void oled_write_command_8_bit(uint8_t command)
{
    OLED_DC_L;
    OLED_CS_L;
    for (volatile int i = 0; i < 8; ++i)
    {
        OLED_SCLK_L;
        if (command & 0x80)
            OLED_SDIN_H;
        else
            OLED_SDIN_L;
        OLED_SCLK_H;
        command <<= 1;
    }
    OLED_CS_H;
}

void oled_write_data_8_bit(uint8_t data)
{
    OLED_DC_H;
    OLED_CS_L;
    for (volatile int i = 0; i < 8; ++i)
    {
        OLED_SCLK_L;
        if (data & 0x80)
            OLED_SDIN_H;
        else
            OLED_SDIN_L;
        OLED_SCLK_H;
        data <<= 1;
    }
    OLED_CS_H;
}

void oled_write_data_16_bit(uint16_t data)
{
    OLED_DC_H;
    uint8_t data_8_bit;
    data_8_bit = data >> 8;
    for (volatile int j = 0; j < 2; ++j)
    {
        OLED_CS_L;
        for (volatile int i = 0; i < 8; ++i)
        {
            OLED_SCLK_L;
            if (data_8_bit & 0x80)
                OLED_SDIN_H;
            else
                OLED_SDIN_L;
            OLED_SCLK_H;
            data_8_bit <<= 1;
        }
        OLED_CS_H;
        data_8_bit = data;
    }
}

void lcd_address(uint8_t page, uint8_t column)
{
    column = column - 1;                            //����ƽ����˵�ĵ�1�У���LCD����IC���ǵ�0�С������������ȥ1.
    page = page - 1;
    oled_write_command_8_bit(0xb0 + page);            //����ҳ��ַ��ÿҳ��8�С�һ�������64�б��ֳ�8��ҳ������ƽ����˵�ĵ�1ҳ����LCD����IC���ǵ�0ҳ�������������ȥ1
    oled_write_command_8_bit(((column >> 4) & 0x0f) + 0x10);    //�����е�ַ�ĸ�4λ
    oled_write_command_8_bit(column & 0x0f);                //�����е�ַ�ĵ�4λ
}

void oled_clean_screen(void)
{
    unsigned char i, j;
    for (j = 0; j < 16; j++)
    {
        lcd_address(1 + j, 1);
        for (i = 0; i < 64; i++)
        {
            oled_write_data_8_bit(0x00);
        }
    }
}

//��ʾ128x64����ͼ��
void display_64x128(uint8_t *dp)
{
    uint i, j;
    for (j = 0; j < 16; j++)
    {
        lcd_address(j + 1, 1);
        for (i = 0; i < 64; i++)
        {
            oled_write_data_8_bit(*dp);                    //д���ݵ�LCD,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1
            dp++;
        }
    }
}

void full_display(uint8_t data1, uint8_t data2)
{
    int i, j;
    for (i = 0; i < 16; i++)
    {
        lcd_address(i + 1, 1);
        for (j = 0; j < 32; j++)
        {
            oled_write_data_8_bit(data1);
            oled_write_data_8_bit(data2);
        }
    }
}

//��ʾ128x16����ͼ��
void display_128x16(uint8_t page, uint8_t column, uint8_t *dp)
{
    uint i, j;
    for (j = 0; j < 2; j++)
    {
        lcd_address(page + j, column);
        for (i = 0; i < 128; i++)
        {
            oled_write_data_8_bit(*dp);                    //д���ݵ�LCD,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1
            dp++;
        }
    }
}

//��ʾ32x32����ͼ�񡢺��֡���Ƨ�ֻ�32x32���������ͼ��
void display_graphic_32x32(uint8_t page, uint8_t column, uint8_t *dp)
{
    uint8_t i, j;
    for (j = 0; j < 4; j++)
    {
        lcd_address(page + j, column);
        for (i = 0; i < 32; i++)
        {
            oled_write_data_8_bit(*dp);        //д���ݵ�LCD,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1
            dp++;
        }
    }
}

//��ʾ16x16����ͼ�񡢺��֡���Ƨ�ֻ�16x16���������ͼ��
void display_graphic_16x16(uint8_t page, uint8_t column, uint8_t *dp)
{
    uint8_t i, j;
    for (j = 0; j < 2; j++)
    {
        lcd_address(page + j, column);
        for (i = 0; i < 16; i++)
        {
            oled_write_data_8_bit(*dp);        //д���ݵ�LCD,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1
            dp++;
        }
    }
}

//��ʾ8x16����ͼ��ASCII, ��8x16����������ַ�������ͼ��
void display_graphic_8x16(uint8_t page, uint8_t column, uint8_t *dp)
{
    uint8_t i, j;
    for (j = 0; j < 2; j++)
    {
        lcd_address(page + j, column);
        for (i = 0; i < 8; i++)
        {
            oled_write_data_8_bit(*dp);                    //д���ݵ�LCD,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1
            dp++;
        }
    }
}

//��ʾ8x16�ĵ�����ַ�����������Ĳ����ֱ�Ϊ��ҳ,�У��ַ���ָ�룩
void display_string_8x16(uint page, uint column, uint8_t *text)
{
    uint i = 0, j, k, n;
    if (column > 57)
    {
        column = 1;
        page += 2;
    }
    while (text[i] > 0x00)
    {
        if ((text[i] >= 0x20) && (text[i] <= 0x7e))
        {
            j = text[i] - 0x20;
            for (n = 0; n < 2; n++)
            {
                lcd_address(page + n, column);
                for (k = 0; k < 8; k++)
                {
                    oled_write_data_8_bit(ascii_table_8x16[j][k + 8 * n]);    //д���ݵ�LCD,ÿд��1�ֽڵ����ݺ��е�ַ�Զ���1
                }
            }
            i++;
            column += 8;
        }
        else
            i++;
    }
}


//��ʾ5x8�ĵ�����ַ�����������Ĳ����ֱ�Ϊ��ҳ,�У��ַ���ָ�룩
void display_string_5x8(uint page, uint column, uint8_t reverse, uint8_t *text)
{
    uint i = 0, j, k, disp_data;
    while (text[i] > 0x00)
    {
        if ((text[i] >= 0x20) && (text[i] <= 0x7e))
        {
            j = text[i] - 0x20;
            lcd_address(page, column);
            for (k = 0; k < 5; k++)
            {
                if (reverse == 1)
                {
                    disp_data = ~ascii_table_5x8[j][k];
                }
                else
                {
                    disp_data = ascii_table_5x8[j][k];
                }

                oled_write_data_8_bit(disp_data);    //д���ݵ�LCD,ÿд��1�ֽڵ����ݺ��е�ַ�Զ���1
            }
            if (reverse == 1) oled_write_data_8_bit(0xff);    //д��һ�пհ��У�ʹ��5x8���ַ����ַ�֮����һ�м����������
            else oled_write_data_8_bit(0x00);            //д��һ�пհ��У�ʹ��5x8���ַ����ַ�֮����һ�м����������
            i++;
            column += 6;
            if (column > 60)
            {
                column = 1;
                page++;
            }
        }
        else
            i++;
    }
}


//д��һ��16x16����ĺ����ַ������ַ���������躬�д��֣�
//������Ĳ�����(ҳ���У������ַ�����
void display_string_16x16(uint8_t page, uint8_t column, uint8_t *text)
{
    uint8_t i, j, k;
    uint address;

    j = 0;
    while (text[j] != '\0')
    {
        i = 0;
        address = 1;
        while (Chinese_text_16x16[i] > 0x7e)      // >0x7f��˵������ASCII���ַ�
        {
            if (Chinese_text_16x16[i] == text[j])
            {
                if (Chinese_text_16x16[i + 1] == text[j + 1])
                {
                    address = i * 16;
                    break;
                }
            }
            i += 2;
        }

        if (column > 49)
        {
            column = 0;
            page += 2;
        }

        if (address != 1)// ��ʾ����
        {

            for (k = 0; k < 2; k++)
            {
                lcd_address(page + k, column);
                for (i = 0; i < 16; i++)
                {
                    oled_write_data_8_bit(Chinese_code_16x16[address]);
                    address++;
                }
            }
            j += 2;
        }
        else              //��ʾ�հ��ַ�            
        {
            for (k = 0; k < 2; k++)
            {
                lcd_address(page + k, column);
                for (i = 0; i < 16; i++)
                {
                    oled_write_data_8_bit(0x00);
                }
            }

            j++;
        }

        column += 16;
    }
}


//��ʾ16x16����ĺ��ֻ���ASCII��8x16������ַ�����ַ���
//������Ĳ�����(ҳ���У��ַ�����
void disp_string_8x16_16x16(uint8_t page, uint8_t column, uint8_t *text)
{
    uint8_t temp[3];
    uint8_t i = 0;

    while (text[i] != '\0')
    {
        if (text[i] > 0x7e)
        {
            temp[0] = text[i];
            temp[1] = text[i + 1];
            temp[2] = '\0';          //����Ϊ�����ֽ�
            display_string_16x16(page, column, temp);  //��ʾ����
            column += 16;
            i += 2;
        }
        else
        {
            temp[0] = text[i];
            temp[1] = '\0';          //��ĸռһ���ֽ�
            display_string_8x16(page, column, temp);  //��ʾ��ĸ
            column += 8;
            i++;
        }
    }
}

void oled_dis(void)
{
    display_64x128(BMP1);
/*    if (HAL_GPIO_ReadPin(USER_KEY_GPIO_Port, USER_KEY_Pin))
    {
        misc_delay_ms(100);
        oled_clean_screen();

        disp_string_8x16_16x16(1, 1, "OLEDҺ��");      //��ʾ�ַ�����������Ĳ����ֱ�Ϊ��PAGE,��,�ַ���ָ�룩
        disp_string_8x16_16x16(3, 1, "��ʾģ��");      //��ʾ�ַ�����������Ĳ����ֱ�Ϊ��PAGE,��,�ַ���ָ�룩
        disp_string_8x16_16x16(5, 1, "��Ϊרע");        //��ʾ16x16�����ִ���8x16������ַ�����������Ĳ����ֱ�Ϊ��ҳ,��,�ַ���ָ�룩
        disp_string_8x16_16x16(7, 1, "����רҵ");
        disp_string_8x16_16x16(9, 1, "�ṩ����");      //��ʾ�ַ�����������Ĳ����ֱ�Ϊ��PAGE,��,�ַ���ָ�룩
        disp_string_8x16_16x16(11, 1, "�������");
        disp_string_8x16_16x16(13, 1, "0.96���");        //��ʾ16x16�����ִ���8x16������ַ�����������Ĳ����ֱ�Ϊ��ҳ,��,�ַ���ָ�룩
        disp_string_8x16_16x16(15, 1, " 64*128 ");

    }
    else
    {
        misc_delay_ms(100);
        oled_clean_screen();
        //����5*8�ַ���ʾЧ��
        display_string_5x8(7, 1, 1, "    MENU   ");    //��ʾ5x8������ַ�����������Ĳ����ֱ�Ϊ��ҳ���У��Ƿ��ԣ�����ָ�룩
        display_string_5x8(8, 1, 0, "Select>>>>");
        display_string_5x8(9, 1, 1, "1.Graphic ");
        display_string_5x8(10, 1, 0, "2.Chinese ");
        display_string_5x8(11, 1, 0, "3.Movie   ");
        display_string_5x8(12, 1, 0, "4.Contrast");
        display_string_5x8(13, 1, 0, "5.Mirror  ");

    }*/
    //misc_delay_ms(100);
    //oled_clean_screen();
}