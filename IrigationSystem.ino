// ================================================== 
// === DEFINE ====
#define MODE_HIVER	(0)
#define MODE_ETE	(1)

#define VANNE_ON	(LOW)
#define VANNE_OFF	(HIGH)

#define EACH_DEMI_SECONDE (100) // 5m * 100 = 500ms
#define EACH_SECONDE (200) 		// 5m * 200 = 1000ms

#define PIN_RELAIS_1	(12) // Relay to operat the Electric Transformer (230V)
#define PIN_LED_HIVER	(7)  // 
#define PIN_LED_ETE		(8)  // 
#define PIN_BUTTON		(3)  // 

#define EACH_HIVER_ON	(5) 
#define EACH_HIVER_OFF	(10) 
#define EACH_ETE_ON		(2) 
#define EACH_ETE_OFF	(4) 

#define MODE_DEBUG		// Commenter ou pas pour activer le mode Debug

// ================================================== 
// === PROTOTYPES ====
void timerSetup(void);
void actionVanne(boolean );
void traintement(void);

// ================================================== 
// === VARIABLES DECLARATION ====
boolean relay1 = VANNE_OFF;
unsigned long gTimeLeft;
typedef enum  { STATE_RIEN, STATE_ON, STATE_OFF } ESTATE; 
ESTATE eEtat = STATE_OFF ;
volatile int gSeason = MODE_HIVER; // Variable for gSeason in ISR
volatile long debounce = 0; // To insure NO button bouncing

// ================================================== 
// === SETUP ====
void setup() 
{
	ESTATE eEtat = STATE_OFF ;
	gTimeLeft  = 1;
	gSeason = MODE_HIVER; // Set gSeason variable as Winter (0) - if Value = 1: Summer
	
	pinMode(PIN_LED_HIVER, OUTPUT); // Set ledhiv pin as Output
    pinMode(PIN_LED_ETE, OUTPUT); // Set ledete pin as Output
    pinMode(PIN_RELAIS_1, OUTPUT); // Set relay1 pin as Output
    pinMode(PIN_BUTTON, INPUT_PULLUP); // Set button pin as Interrupt
    actionVanne(VANNE_OFF);
	attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), timetogive, RISING); // Interrupt mode Definition (Pin, ISR loop to execute, Mode)
    
#ifdef MODE_DEBUG
	Serial.begin(9600); // Start Serial Monitor to follow step on Screen (Problem solving)
#endif

	cli();
	timerSetup();
	sei(); // enable interrupts
}
 
/* ************************************************************************ */
/* loop()                                                                   */
/* le micro dort tout le temps et actif toutes les 5ms                      */
/* ************************************************************************ */
void loop() 
{
}

/* ************************************************************************ */
/* timetogive()                                                             */
/* Name the ISR, it will be executed if the pushbutton pressed              */
// vue que l'on fonctionne par interuuption, la gesion de l'anti rebond 
// avec millis peut etre supprrimer !
/* ************************************************************************ */
void timetogive() 
{ 
static volatile long debounce = 0; // To insure NO button bouncing
    //Comparison to avoid Bouncing Button!
	if (millis() - debounce > 500) 
    { 	
		if (gSeason == MODE_ETE) 
        { // Change the Variable gSeason value IF == 1 turn it to 0
          gSeason = MODE_HIVER;
#ifdef MODE_DEBUG
		  Serial.println("HIVER");
#endif		  
        } else { 
            gSeason = MODE_ETE;
#ifdef MODE_DEBUG
			Serial.println("ETE");
#endif			
        }
        debounce = millis (); //New value for further comparison
    }
}




/* ************************************************************************ */
/* ON RENTRE DANS SE TIMER TOUTES LES 5ms
/* ************************************************************************ */
void actionVanne(boolean etatVanne)
{
	digitalWrite (PIN_RELAIS_1, etatVanne);
	relay1 = etatVanne;
	
	if (gSeason == MODE_ETE)	{
		if (!etatVanne)	{
			digitalWrite (PIN_LED_ETE, HIGH);
		} else {
			digitalWrite (PIN_LED_ETE, LOW);
		}		
	}
	else {
		if (!etatVanne)	{
			digitalWrite (PIN_LED_HIVER, HIGH);
		} else {
			digitalWrite (PIN_LED_HIVER, LOW);
		}	
	}
	
#ifdef MODE_DEBUG
// Ce qu il y a en dessous c'est pour du Debug sur RS232	
	if (gSeason == MODE_ETE)	{
		if (etatVanne)	{
			Serial.print(" --- ETE   watering ON --- "); // Serial Monitor Information
		} else {
			Serial.print(" --- ETE   watering OFF -- "); // Serial Monitor Information
		}	
	}
	else {
		if (etatVanne)	{
			Serial.print(" --- HIVER watering ON --- "); // Serial Monitor Information
		} else {
			Serial.print(" --- HIVER watering OFF -- "); // Serial Monitor Information
		}	
	}
#endif	
	
}

#ifdef MODE_DEBUG
/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.   
 OLQ => C'est juste pour remplacer l'appui du bouton poussoir par l'envoi de 'a' sur la liason série,
 pour eviter que je ne cable un bt poussoir
 */
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == 'a') {
	  timetogive();
    }
  }
}
#endif


/* ************************************************************************ */
/* ON RENTRE DANS SE TIMER TOUTES LES 5ms
/* ************************************************************************ */
ISR(TIMER1_COMPA_vect) // 16 bit timer 1 compare 1A match
{
	
	static uint16_t iLed500 = EACH_DEMI_SECONDE;
	static uint16_t iLed = 0;

// La LED change d'etat toutes les 5ms
	if (--iLed500 == 0 ) {
		iLed = 1 - iLed ;
		if (iLed){
			traintement(); // toutes les seconde on va voir ce que le pgm doit faire
			digitalWrite(LED_BUILTIN, HIGH );
		} else {
			digitalWrite(LED_BUILTIN, LOW);
		}
		iLed500 = EACH_DEMI_SECONDE ;
	}
}

/* ************************************************** */
void timerSetup(void )
{	

 
	// TIMER 1 16BIT
	TCCR1A = 0;
	TCCR1B = ( 1<<WGM12) | (0<<CS12) | (1<<CS11) | (1<<CS10) ; // Prescaler / 64 ==> des PAS de 5ms
	TCCR1C = 0; // not forcing output compare
	TCNT1 = 0; // set timer counter initial value (16 bit value)
	OCR1A = 1250; // Closer to one second than value above, 1250 pas de 4µs ca fait
	TIMSK1 = 1 << OCIE1A; // enable timer compare match 1A interrupt
	
}

/* ************************************************************************ */
/* ON RENTRE DANS CETTE FONCTION TOUTE LES SECONDE                          */
/* ************************************************************************ */
void traintement(void)
{
#ifdef MODE_DEBUG
	Serial.println(gTimeLeft);
#endif	
	if (--gTimeLeft == 0)
	{
		switch(eEtat)
		{
			case STATE_ON : 
				actionVanne(VANNE_ON);

				if (gSeason == MODE_ETE) 
				{
					// digitalWrite (PIN_LED_ETE, HIGH); -> Fait dans actionVanne(..)
					gTimeLeft = EACH_ETE_OFF;
				}else {
					// digitalWrite (PIN_LED_HIVER, HIGH); -> Fait dans actionVanne(..)
					gTimeLeft = EACH_HIVER_OFF;	
				}
				eEtat = STATE_OFF;
			break;
			
		case STATE_OFF : 
				actionVanne(VANNE_OFF);
				
				if (gSeason == MODE_ETE) {
					digitalWrite (PIN_LED_ETE, LOW); // -> Fait dans actionVanne(..)
					gTimeLeft = EACH_ETE_ON;
				} else {
					digitalWrite (PIN_LED_HIVER, LOW); // -> Fait dans actionVanne(..)
					gTimeLeft = EACH_HIVER_ON;
				}
				eEtat = STATE_ON;
			break;
		default:
			gTimeLeft = 10;
			eEtat = STATE_OFF;
			break;
		}
	}
	return;
	
	/*
	// NON REPRIS, mais ceci pourait etre dans une fonction et appelé par timetogive()
    if (trigger != gSeason) 
	{ // This if detect the change of gSeason with the varible volatile in the ISR. Then it light the appropriate LED.
        trigger = gSeason; // We reset trigger as gSeason to detect further change
        if (gSeason == MODE_HIVER) 
		{ // If variable gSeason = MODE_HIVER : winter light activated
            actionVanne(VANNE_OFF);
			digitalWrite(PIN_LED_HIVER, HIGH);// Turn ON the LED indicating the WINTER mode. It will stay On until next watering.
            digitalWrite (PIN_LED_ETE, LOW); // Turn OFF the LED indicating the SUMMER mode
            Serial.println("button has been pressed now Hiver mode"); // Serial Monitor Information
		}
		else if (gSeason == MODE_ETE) 
		{ // If variable gSeason = MODE_ETE : summer light activated
			actionVanne(VANNE_OFF);
			digitalWrite (PIN_LED_ETE, HIGH);// Turn ON the LED indicating the SUMMER mode. It will stay On until next watering.
			digitalWrite (PIN_LED_HIVER, LOW); // Turn OFF the LED indicating the WINTER mode
			Serial.println("button has been pressed now summer mode"); // Serial Monitor Information
		} 
    }
	*/
}



