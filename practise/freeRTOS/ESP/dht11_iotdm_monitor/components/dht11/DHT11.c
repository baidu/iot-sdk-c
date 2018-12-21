#include "DHT11.h"


enum DHT_Status_t __DHT_STATUS;

#define __DHT_Temperature_Min	0
#define __DHT_Temperature_Max	50
#define __DHT_Humidity_Min		20
#define __DHT_Humidity_Max		90
#define __DHT_Delay_Read		50


static double DataToTemp(uint8_t Data3, uint8_t Data4);
static double DataToHum(uint8_t Data1, uint8_t Data2);

static int DHT11_PIN;

#define BitSet(		x, y)			(	x |=	 (1UL<<y)			)
//Setup sensor. 
void DHT_Setup(int pin)
{
	DHT11_PIN = pin;
	gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL<<DHT11_PIN);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

	__DHT_STATUS = DHT_Ok;
}

//Get sensor status. 
enum DHT_Status_t DHT_status(void)
{
	return (__DHT_STATUS);
}

static bool dht_wait(int pin, int lvl, uint32_t usecs) {
  uint32_t t = 0;
  while (gpio_get_level(pin) != lvl) {
    if (t == usecs) {
      t = 0;
      break;
    }
    ets_delay_us(1);
    t++;
  }
  return t != 0;
}

//Read raw buffer from sensor. 
static void DHT_ReadRaw(uint8_t Data[4])
{
	uint8_t buffer[5] = {0, 0, 0, 0, 0};
	uint8_t retries, i;
	int8_t j;
	__DHT_STATUS = DHT_Ok;
	retries = i = j = 0;

	//----- Step 1 - Start communication -----
	if (__DHT_STATUS == DHT_Ok)
	{
		//Request data
		gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT);
		ets_delay_us(10 * 1000);

		gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
		gpio_set_level(DHT11_PIN, 0);
		ets_delay_us(18 * 1000);

		gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT);
		ets_delay_us(40);
		
		//Setup DHT_PIN as input with pull-up resistor so as to read data
		gpio_set_level(DHT11_PIN, 1);  //DHT_PIN = 1 (Pull-up resistor)
		gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT);  //DHT_PIN = Input

		//Wait for response 
		if (!dht_wait(DHT11_PIN, 1, 90) || !dht_wait(DHT11_PIN, 0, 90)) {
			printf("Timeout waiting for....");
			__DHT_STATUS = DHT_Error_Timeout;	//Timeout error
		}

	}
	

	//----- Step 2 - Data transmission -----
	if (__DHT_STATUS == DHT_Ok)
	{
		//Reading 5 bytes, bit by bit
		for (i = 0 ; i < 5 ; i++)
			for (j = 7 ; j >= 0 ; j--)
			{
				//There is always a leading low level of 50 us
				retries = 0;
				while(!gpio_get_level(DHT11_PIN))
				{
					//_delay_us(2);
					ets_delay_us(2);
					retries += 2;
					if (retries > 70)
					{
						__DHT_STATUS = DHT_Error_Timeout;	//Timeout error
						j = -1;								//Break inner for-loop
						i = 5;								//Break outer for-loop
						break;								//Break while loop
					}
				}

				if (__DHT_STATUS == DHT_Ok)
				{
					//We read data bit || 26-28us means '0' || 70us means '1'
					//_delay_us(35);							//Wait for more than 28us
					ets_delay_us(35);
					if (gpio_get_level(DHT11_PIN))				//If HIGH
						BitSet(buffer[i], j);				//bit = '1'

					retries = 0;
					while(gpio_get_level(DHT11_PIN))
					{
						//_delay_us(2);
						ets_delay_us(2);
						retries += 2;
						if (retries > 100)
						{
							__DHT_STATUS = DHT_Error_Timeout;	//Timeout error
							break;
						}
					}
				}
			}
	}
	//--------------------------------------


	//----- Step 3 - Check checksum and return data -----
	if (__DHT_STATUS == DHT_Ok)
	{	
		if (((uint8_t)(buffer[0] + buffer[1] + buffer[2] + buffer[3])) != buffer[4])
		{
			__DHT_STATUS = DHT_Error_Checksum;	//Checksum error
		}
		else
		{
			//Build returning array
			//data[0] = Humidity		(int)
			//data[1] = Humidity		(dec)
			//data[2] = Temperature		(int)
			//data[3] = Temperature		(dec)
			//data[4] = Checksum
			for (i = 0 ; i < 4 ; i++)
				Data[i] = buffer[i];
		}
	}
}

//Read temperature and humidity. 
void DHT_Read(double *Temperature, double *Humidity)
{
	uint8_t data[4] = {0, 0, 0, 0};

	//Read data
	DHT_ReadRaw(data);
	
	//If read successfully
	if (__DHT_STATUS == DHT_Ok)
	{	
		//Calculate values
		*Temperature = DataToTemp(data[2], data[3]);
		*Humidity = DataToHum(data[0], data[1]);	
		
		//Check values
		if ((*Temperature < __DHT_Temperature_Min) || (*Temperature > __DHT_Temperature_Max))
			__DHT_STATUS = DHT_Error_Temperature;
		else if ((*Humidity < __DHT_Humidity_Min) || (*Humidity > __DHT_Humidity_Max))
			__DHT_STATUS = DHT_Error_Humidity;
	}
}

//Convert temperature data to double temperature.
static double DataToTemp(uint8_t Data2, uint8_t Data3)
{
	printf("DataToTemp: %x\n", Data2);
	double temp = 0.0;
	
	temp = Data2;
	
	return temp;
}

static double DataToHum(uint8_t Data0, uint8_t Data1)
{
	printf("DataToHum: %x\n", Data0);
	double hum = 0.0;
	
	hum = Data0;
	
	return hum;
}
//---------------------------------------------//