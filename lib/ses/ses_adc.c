#include "ses_adc.h"
#include <avr/io.h>

/* ADC clock prescaler: division factor is 128 */
#define ADC_PRESCALE            0x07
/* ADC clock prescaler mask */
#define ADC_PRESCALE_MASK       0x07
/* ADPS0-2 bit position in ADEN */
#define ADC_PRESCALE_BIT_POS    ADPS0

/* ADC external reference voltage source */
#define ADC_VREF_SRC            0x00
/* ADC reference voltage source mask */
#define ADC_VREF_SRC_MASK       0x03
/* REFS0-REFS1 bit position in ADMUX register */
#define ADC_VREF_BIT_POS        REFS0

/* ADC channel mux mask */
#define ADC_MUX_BIT_MASK        0x1F
/* MUX0-4 bit position in ADMUX register*/
#define ADC_MUX_BIT_POS         MUX0

/* Interpolation low temperature */
#define ADC_TEMP_LOW            10
/* Interpolation high temperature */
#define ADC_TEMP_HIGH           30

/* ADC raw value for the low temperature (10 Celsius) */
#define ADC_TEMP_RAW_LOW        553
/* ADC raw value for the high temperature (30 Celsius) */
#define ADC_TEMP_RAW_HIGH       857


/* SES-board potentiometer PORT, DDR and PIN register macros */
#define POTI_PORT            PORTF
#define POTI_DDR             DDRF
#define POTI_PIN             PINF
#define POTI_BIT             6

/* SES-board temperature sensor PORT, DDR and PIN register macros */
#define TEMP_PORT       PORTF
#define TEMP_DDR        DDRF
#define TEMP_PIN        PINF
#define TEMP_BIT        7

/* SES-board light sensor PORT, DDR and PIN register macros */
#define LIGHT_PORT     PORTF
#define LIGHT_DDR      DDRF
#define LIGHT_PIN      PINF
#define LIGHT_BIT      0

void adc_init(void){
    // Configure potentiometer analog input and disable internal pull-up resistor
    POTI_DDR &= ~(1 << POTI_BIT);
    POTI_PORT &= ~(1 << POTI_BIT);

    // Configure temperature sensor analog input and disable internal pull-up resistor
    TEMP_DDR &= ~(1 << TEMP_BIT);
    TEMP_PORT &= ~(1 << TEMP_BIT);

    // Configure temperature sensor analog input and disable internal pull-up resistor
    LIGHT_DDR &= ~(1 << LIGHT_BIT);
    LIGHT_PORT &= ~(1 << LIGHT_BIT);

    // Disable ADC power reduction mode
    PRR0 &= ~(1 << PRADC);

    // Select an external reference voltage
    ADMUX &= ~(ADC_VREF_SRC_MASK << ADC_VREF_BIT_POS);
    ADMUX |=  (ADC_VREF_SRC_MASK & ADC_VREF_SRC) << ADC_VREF_BIT_POS;


    // Right aligned setup
    ADMUX &= ~(1 << ADLAR); 
    
    // Set ADC prescale value
    ADCSRA &= ~(ADC_PRESCALE_MASK << ADC_PRESCALE_BIT_POS);
    ADCSRA |=  (ADC_PRESCALE_MASK & ADC_PRESCALE) << ADC_PRESCALE_BIT_POS;


    // Disable auto-triggering
    ADCSRA &= ~(1 << ADATE);

    // Enable ADC peripheral
    ADCSRA |= (1 << ADEN);

}



uint16_t adc_read(uint8_t adc_channel){

    // Check the channel correctness
    if((uint8_t)ADC_LIGHT_CH != adc_channel && (uint8_t)ADC_POTI_CH != adc_channel && (uint8_t)ADC_TEMP_CH != adc_channel){
        return ADC_INVALID_CHANNEL;
    }

    // Select the correct channel
    ADMUX &= ~(ADC_MUX_BIT_MASK << ADC_MUX_BIT_POS);
    ADMUX |=  (ADC_MUX_BIT_MASK & adc_channel) << ADC_MUX_BIT_POS;

    // Start a single conversion
    ADCSRA |= (1 << ADSC);

    // Wait until conversion is finished
    while(ADCSRA & (1 << ADSC));

    // Read the result
    return ADC;

}

int16_t adc_getTemperature(void){

    int32_t adc_raw = adc_read(ADC_TEMP_CH);
    return (int16_t)(((adc_raw - ADC_TEMP_RAW_LOW) * (ADC_TEMP_HIGH - ADC_TEMP_LOW)) / (ADC_TEMP_RAW_HIGH - ADC_TEMP_RAW_LOW)) + ADC_TEMP_LOW;

}