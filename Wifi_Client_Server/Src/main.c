/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private defines -----------------------------------------------------------*/

#define TERMINAL_USE
#define TTP229_SCL_GPIO_Port  GPIOB
#define TTP229_SCL_Pin        GPIO_PIN_8

#define TTP229_SDO_GPIO_Port  GPIOB
#define TTP229_SDO_Pin        GPIO_PIN_9

#define BUTTON1_GPIO_PORT GPIOA
#define BUTTON1_PIN       GPIO_PIN_0

/* Update SSID and PASSWORD with own Access point settings */
#define SSID     "Yocci"
#define PASSWORD "wuyouyou"

uint8_t RemoteIP[] = {172,20,10,3};
#define RemotePORT	65431

#define WIFI_WRITE_TIMEOUT 10000
#define WIFI_READ_TIMEOUT  10000

#define CONNECTION_TRIAL_MAX          10

#if defined (TERMINAL_USE)
#define TERMOUT(...)  printf(__VA_ARGS__)
#else
#define TERMOUT(...)
#endif

/* Private variables ---------------------------------------------------------*/
#if defined (TERMINAL_USE)
extern UART_HandleTypeDef hDiscoUart;
#endif /* TERMINAL_USE */
int32_t Socket = -1;

/* Private function prototypes -----------------------------------------------*/
#if defined (TERMINAL_USE)
#ifdef __GNUC__
/* With GCC, small TERMOUT (option LD Linker->Libraries->Small TERMOUT
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
#endif /* TERMINAL_USE */

static void SystemClock_Config(void);
extern  SPI_HandleTypeDef hspi;

/* Private functions ---------------------------------------------------------*/
// 初始化 SCL 为输出，SDO 为输入
void TTP229_GPIO_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // PB8 = SCL
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // PB9 = SDO
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

uint16_t TTP229_ReadKeys(void)
{
    uint16_t data = 0;

    for (int i = 0; i < 16; i++)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);  // CLK low
        HAL_Delay(1);
        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9) == GPIO_PIN_RESET)
        {
            data |= (1 << i);
        }
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);  // CLK high
        HAL_Delay(1);
    }
    return data;
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  uint8_t  MAC_Addr[6] = {0};
  uint8_t  IP_Addr[4] = {0};
  uint8_t TxData[] = "STM32 : Hello!\n";

  uint16_t Datalen;
  int32_t ret;
  int16_t Trials = CONNECTION_TRIAL_MAX;

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  BSP_COM_Init(COM1, &hDiscoUart);
  BSP_ACCELERO_Init();

  /* Configure the system clock */
  SystemClock_Config();
  /* Configure LED2 */
  BSP_LED_Init(LED2);
  // 启用 GPIOA（用于 PA0、PA1）
  __HAL_RCC_GPIOA_CLK_ENABLE();


  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = BUTTON1_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING; // 支持按下（下降）和松开（上升）中断
  GPIO_InitStruct.Pull = GPIO_PULLUP;  // 按键接 GND 时用上拉
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // 启用 EXTI 中断通道 0（PA0 对应 EXTI0）
  HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);


#if defined (TERMINAL_USE)
  /* Initialize all configured peripherals */
  hDiscoUart.Instance = DISCOVERY_COM1;
  hDiscoUart.Init.BaudRate = 115200;
  hDiscoUart.Init.WordLength = UART_WORDLENGTH_8B;
  hDiscoUart.Init.StopBits = UART_STOPBITS_1;
  hDiscoUart.Init.Parity = UART_PARITY_NONE;
  hDiscoUart.Init.Mode = UART_MODE_TX_RX;
  hDiscoUart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hDiscoUart.Init.OverSampling = UART_OVERSAMPLING_16;
  hDiscoUart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hDiscoUart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  BSP_COM_Init(COM1, &hDiscoUart);
#endif /* TERMINAL_USE */

  TERMOUT("****** WIFI Module in TCP Client mode demonstration ****** \n\n");
  TERMOUT("TCP Client Instructions :\n");
  TERMOUT("1- Make sure your Phone is connected to the same network that\n");
  TERMOUT("   you configured using the Configuration Access Point.\n");
  TERMOUT("2- Create a server by using the android application TCP Server\n");
  TERMOUT("   with port(8002).\n");
  TERMOUT("3- Get the Network Name or IP Address of your Android from the step 2.\n\n");


  /*Initialize  WIFI module */
  if(WIFI_Init() ==  WIFI_STATUS_OK)
  {
    TERMOUT("> WIFI Module Initialized.\n");
    if(WIFI_GetMAC_Address(MAC_Addr, sizeof(MAC_Addr)) == WIFI_STATUS_OK)
    {
      TERMOUT("> es-wifi module MAC Address : %X:%X:%X:%X:%X:%X\n",
               MAC_Addr[0],
               MAC_Addr[1],
               MAC_Addr[2],
               MAC_Addr[3],
               MAC_Addr[4],
               MAC_Addr[5]);
    }
    else
    {
      TERMOUT("> ERROR : CANNOT get MAC address\n");
      BSP_LED_On(LED2);
    }

    if( WIFI_Connect(SSID, PASSWORD, WIFI_ECN_WPA2_PSK) == WIFI_STATUS_OK)
    {
      TERMOUT("> es-wifi module connected \n");
      if(WIFI_GetIP_Address(IP_Addr, sizeof(IP_Addr)) == WIFI_STATUS_OK)
      {
        TERMOUT("> es-wifi module got IP Address : %d.%d.%d.%d\n",
               IP_Addr[0],
               IP_Addr[1],
               IP_Addr[2],
               IP_Addr[3]);

        TERMOUT("> Trying to connect to Server: %d.%d.%d.%d:%d ...\n",
               RemoteIP[0],
               RemoteIP[1],
               RemoteIP[2],
               RemoteIP[3],
							 RemotePORT);

        while (Trials--)
        {
          if( WIFI_OpenClientConnection(0, WIFI_TCP_PROTOCOL, "TCP_CLIENT", RemoteIP, RemotePORT, 0) == WIFI_STATUS_OK)
          {
            TERMOUT("> TCP Connection opened successfully.\n");
            Socket = 0;
            break;
          }
        }
        if(Socket == -1)
        {
          TERMOUT("> ERROR : Cannot open Connection\n");
          BSP_LED_On(LED2);
        }
      }
      else
      {
        TERMOUT("> ERROR : es-wifi module CANNOT get IP address\n");
        BSP_LED_On(LED2);
      }
    }
    else
    {
      TERMOUT("> ERROR : es-wifi module NOT connected\n");
      BSP_LED_On(LED2);
    }
  }
  else
  {
    TERMOUT("> ERROR : WIFI Module cannot be initialized.\n");
    BSP_LED_On(LED2);
  }
  TTP229_GPIO_Init();
  while (1)
    {
        uint16_t keys = TTP229_ReadKeys();

        // 打印调试信息
        if (keys != 0)
        {
            TERMOUT("Keys Pressed: 0d%08X\n", keys);
            // 构造发送内容
            uint8_t tx_buf[2];
            tx_buf[0] = (keys >> 8) & 0xFF;
            tx_buf[1] = keys & 0xFF;

            uint16_t sentLen = 0;
            if (WIFI_SendData(Socket, tx_buf, sizeof(tx_buf), &sentLen, WIFI_WRITE_TIMEOUT) != WIFI_STATUS_OK)
            {
                TERMOUT("> ERROR: Failed to send data via WiFi\n");
            }
        }
        HAL_Delay(50); // 防抖
    }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            PLL_P                          = 7
  *            PLL_Q                          = 4
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */

static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* MSI is enabled after System reset, activate PLL with MSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLP = 7;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
}

#if defined (TERMINAL_USE)
/**
  * @brief  Retargets the C library TERMOUT function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
  HAL_UART_Transmit(&hDiscoUart, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}
#endif /* TERMINAL_USE */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: TERMOUT("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin)
  {
  	  case GPIO_PIN_0:
      {
        GPIO_PinState key_state = HAL_GPIO_ReadPin(BUTTON1_GPIO_PORT, BUTTON1_PIN);
        // 控制 LED2
        if (key_state == GPIO_PIN_RESET)
        {
            BSP_LED_On(LED2);  // 按下，点亮
        }
        else
        {
            BSP_LED_Off(LED2); // 松开，熄灭
        }
        uint8_t tx_buf[2];
        tx_buf[0] = 0xAA;  // 自定义标识符：PA0 按键事件
        tx_buf[1] = (key_state == GPIO_PIN_RESET) ? 0x01 : 0x00;

        uint16_t sentLen;
        if (Socket >= 0)
        {
            WIFI_SendData(Socket, tx_buf, sizeof(tx_buf), &sentLen, WIFI_WRITE_TIMEOUT);
        }

        break;
      }
    case (GPIO_PIN_1):
    {
      SPI_WIFI_ISR();
      break;
    }
    default:
    {
      break;
    }
  }
}
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void SPI3_IRQHandler(void)
{
  HAL_SPI_IRQHandler(&hspi);
}
