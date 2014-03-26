#define KEY_4  0x0020 		//GPIOB PB5 	//Keypad Pin F
#define KEY_5  0x0040 		//GPIOB PB6		//Keypad Pin D
#define KEY_6  0x0080 		//GPIOB PB7		//Keypad Pin A
#define KEY_7  0x0008 		//GPIOA PA3		//Keypad Pin N
#define KEY_8  0x0010 		//GPIOA PA4		//Keypad Pin K
#define KEY_9  0x0020 		//GPIOA PA5		//Keypad Pin H
#define KEY_STAR  0x0080 	//GPIOC PC7		//Keypad Pin M
#define KEY_HASH 0x0080		//GPIOA PA7		//Keypad Pin J
#define KEY_1  0x0040 		//GPIOC PC6		//Keypad Pin E
#define KEY_2  0x0004 		//GPIOA PA2		//Keypad Pin C
#define KEY_3  0x0010 		//GPIOC PC4		//Keypad Pin B
#define KEY_0  0x0020 		//GPIOC PC5		//Keypad Pin L
#define KEY_ALL1 0x00E0		//GPIOB	MASK
#define KEY_ALL2 0x00F8		//GPIOA	MASK
#define KEY_ALL3 0x00F0		//GPIOC	MASK

void Key_Init(void);

void Key_Init2(void);

//********* KeyScan ****************
// Checks the switch input
// Inputs: None
// Outputs: Key which was pressed in KeyPad
unsigned short keyScan(void);
