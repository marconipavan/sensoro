// Constantes dos pinos
#define PIN_CARREGADOR 		6
#define PIN_DESCARREGADOR 	7
#define PIN_MEDIDOR			A0

// Constantes para medição
#define CHARGED				1023
#define TIME_ON_OFF			10
#define DELTA_T				1000
#define THRESHOLD_UP		900
#define THRESHOLD_DOWN		10
#define RESISTOR_OHM		1000

// Macros para controle dos sinais dos pinos digitais
#define ON(p)	digitalWrite(p, HIGH);
#define OFF(p)	digitalWrite(p, LOW);

// Esqueleto da máquina de estados
enum Estado {
	CARREGAR = 0,
	DESCARREGAR,
	MEDIR,
	ESPERAR,
	NUMERO_DE_ESTADOS
};

// Definindo um tipo para um ponteir para uma função que retorna enum Estados
typedef enum Estado (*FuncaoDeEstado)();

// Funções referentes à máquina de estados

enum Estado carregar(){
	Serial.println("Carregando o capacitor...");

	OFF(PIN_DESCARREGADOR);
	delay(TIME_ON_OFF);
	ON(PIN_CARREGADOR);

	// Esperar carregar completamente
	while(analogRead(PIN_MEDIDOR) < CHARGED){}

	return DESCARREGAR;
}

enum Estado descarregar(){
	Serial.println("Descarregando o capacitor...");
	
	OFF(PIN_CARREGADOR);
	delay(TIME_ON_OFF);
	ON(PIN_DESCARREGADOR);

	return MEDIR;
}

enum Estado medir(){

	float leitura = analogRead(PIN_MEDIDOR);
	float dt;

	do{
		dt = millis();
    	leitura = analogRead(PIN_MEDIDOR);
	} while(leitura >= THRESHOLD_UP);
	
	do{
    	leitura = analogRead(PIN_MEDIDOR);
  	}while(leitura >= THRESHOLD_DOWN);

	dt = millis() - dt;

	// dt = RC
	float capacitance = dt/((float) RESISTOR_OHM);
	Serial.println(capacitance);	

	return ESPERAR;
}

enum Estado esperar(){
	Serial.println("Esperando para a próxima medição...");
	
	delay(DELTA_T);

	return CARREGAR;
}

// Lista com ponteiros de funções que retornam enum Estados
FuncaoDeEstado maquina_de_estados[NUMERO_DE_ESTADOS];
enum Estado estado_atual = CARREGAR;

void setup(){
	// Configurando a comunicação serial
	Serial.begin(9600);
	Serial.println("Iniciando o programa...");

	// Configuração dos pinos
	pinMode(PIN_CARREGADOR, OUTPUT);
	pinMode(PIN_DESCARREGADOR, OUTPUT);
	pinMode(PIN_MEDIDOR, INPUT);

	// Populando a máquina de estados
	maquina_de_estados[CARREGAR] = carregar;
	maquina_de_estados[DESCARREGAR] = descarregar;
	maquina_de_estados[MEDIR] = medir;
	maquina_de_estados[ESPERAR] = esperar;

	// Garantir que o capacitor está descarregado
	maquina_de_estados[DESCARREGAR]();
}

// Loop
void loop(){
	estado_atual = maquina_de_estados[estado_atual]();
}