#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdlib.h>

#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "bme68x.h"

static int fd;
static int buzzer_fd;

/* I2C read function */
BME68X_INTF_RET_TYPE user_i2c_read(uint8_t reg_addr,
                                   uint8_t *reg_data,
                                   uint32_t len,
                                   void *intf_ptr)
{
    write(fd, &reg_addr, 1);
    read(fd, reg_data, len);
    return BME68X_OK;
}

/* I2C write function */
BME68X_INTF_RET_TYPE user_i2c_write(uint8_t reg_addr,
                                    const uint8_t *reg_data,
                                    uint32_t len,
                                    void *intf_ptr)
{
    uint8_t buf[256];

    buf[0] = reg_addr;

    for (uint32_t i = 0; i < len; i++)
        buf[i + 1] = reg_data[i];

    write(fd, buf, len + 1);

    return BME68X_OK;
}

void user_delay_us(uint32_t period, void *intf_ptr)
{
    usleep(period);
}

/* AQI estimation from gas resistance */
float calculate_aqi(unsigned long gas)
{
    if (gas >= 40000)
        return 25;

    if (gas >= 20000)
        return (25 + (40000 - gas) * 75.0f / 20000.0f)-125;

    if (gas >= 10000)
        return (100 + (20000 - gas) * 100.0f / 10000.0f)-125;

    if (gas >= 5000)
        return (200 + (10000 - gas) * 100.0f / 5000.0f)-125;

    if (gas >= 2000)
        return (300 + (5000 - gas) * 200.0f / 3000.0f)-125;

    return 500-125;
}

/* Send telemetry to ThingsBoard */
void send_to_thingsboard(float temp,
                         float hum,
                         float pres,
                         float aqi,
                         int alarm)
{
    int sock;
    struct sockaddr_in server;

    char json[256];
    char request[1024];
    char response[512];

     sprintf(json,
            "{\"temperature\":%.2f,"
            "\"humidity\":%.2f,"
            "\"pressure\":%.2f,"
            "\"aqi\":%.0f,"
            "\"alarm\":%d}",
            temp,
            hum,
            pres,
            aqi,
            alarm);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("socket");
        return;
    }

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("192.168.0.10");

    if (connect(sock,
                (struct sockaddr *)&server,
                sizeof(server)) < 0)
    {
        perror("connect");
        close(sock);
        return;
    }

    sprintf(request,
            "POST /api/v1/6TAggUnIo0sIgVm8s9H6/telemetry HTTP/1.1\r\n"
            "Host: 192.168.0.10\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %ld\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",
            strlen(json),
            json);

    send(sock, request, strlen(request), 0);

    int len = recv(sock,
                   response,
                   sizeof(response) - 1,
                   0);

    if (len > 0)
    {
        response[len] = '\0';
        printf("ThingsBoard Response:\n%s\n",
               response);
    }

    close(sock);
}

int main(void)
{
    int led_fd;
    struct bme68x_dev bme;
    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr;
    struct bme68x_data data;
    uint8_t n_fields;

    fd = open("/dev/i2c-0", O_RDWR);

    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    if (ioctl(fd, I2C_SLAVE, 0x77) < 0)
    {
        perror("ioctl");
        return -1;
    }

    bme.intf = BME68X_I2C_INTF;
    bme.read = user_i2c_read;
    bme.write = user_i2c_write;
    bme.delay_us = user_delay_us;
    bme.intf_ptr = NULL;

    if (bme68x_init(&bme) != BME68X_OK)
    {
        printf("BME680 initialization failed\n");
        return -1;
    }
    system("echo out > /sys/class/gpio/PD4/direction");
    system("echo 0 > /sys/class/gpio/PD4/value");

    buzzer_fd = open("/sys/class/gpio/PD4/value", O_WRONLY);

    if (buzzer_fd < 0)
    {
        perror("buzzer");
    }
    system("echo out > /sys/class/gpio/PC13/direction");
    system("echo 0 > /sys/class/gpio/PC13/value");

    led_fd = open("/sys/class/gpio/PC13/value", O_WRONLY);

    if (led_fd < 0)
    {
    perror("led");
    }

    conf.os_hum  = BME68X_OS_2X;
    conf.os_pres = BME68X_OS_4X;
    conf.os_temp = BME68X_OS_8X;
    conf.filter  = BME68X_FILTER_SIZE_3;

    bme68x_set_conf(&conf, &bme);

    heatr.enable = BME68X_ENABLE;
    heatr.heatr_temp = 300;
    heatr.heatr_dur = 100;

    bme68x_set_heatr_conf(BME68X_FORCED_MODE,
                          &heatr,
                          &bme);

    while (1)
    {
        bme68x_set_op_mode(BME68X_FORCED_MODE,
                           &bme);

        usleep(200000);

        if (bme68x_get_data(BME68X_FORCED_MODE,
                            &data,
                            &n_fields,
                            &bme) == BME68X_OK)
        {
            unsigned long gas =
                (unsigned long)data.gas_resistance;

            float aqi =
                calculate_aqi(gas);

            int alarm = (data.temperature > 30.0) ? 1 : 0;

           if (data.temperature > 30.0)
	{
 	   /* Buzzer ON */
    	lseek(buzzer_fd, 0, SEEK_SET);
    	write(buzzer_fd, "1", 1);

    	/* LED ON */
    	lseek(led_fd, 0, SEEK_SET);
    	write(led_fd, "0", 1);

    	printf("ALARM! Buzzer ON, LED ON\n");
	}
	else
	{
	    /* Buzzer OFF */
    	lseek(buzzer_fd, 0, SEEK_SET);
    	write(buzzer_fd, "0", 1);

    	/* LED OFF */
    	lseek(led_fd, 0, SEEK_SET);
    	write(led_fd, "1", 1);

    	printf("Normal Condition\n");
	}

            printf("\n");
            printf("=================================\n");
            printf("Temp : %.2f C\n",
                   data.temperature);

            printf("Hum  : %.2f %%\n",
                   data.humidity);

            printf("Pres : %.2f hPa\n",
                   data.pressure / 100.0);

            printf("Gas  : %lu ohm\n",
                   gas);

            printf("AQI  : %.0f\n",
                   aqi);

            printf("=================================\n");

            send_to_thingsboard(
                data.temperature,
                data.humidity,
                data.pressure / 100.0,
                aqi,
                alarm);
        }

        sleep(5);
    }

    close(fd);

    return 0;
}

