#include <Wire.h>
#include <Servo.h>

///////////////////////Entradas e Saídas///////////////////////
int Analog_in = A0;  // Sensor de distância
Servo myservo;       // Objeto servo conectado ao pino D9
//////////////////////////////////////////////////////////////

//////////////////////Variáveis///////////////////////////////
int Read = 0;
float distance = 0.0;
float elapsedTime, time, timePrev;
float distance_previous_error, distance_error;
int period = 50; // Atualização a cada 50 ms
//////////////////////////////////////////////////////////////

///////////////////Constantes do PID//////////////////////////
float kp = 8;      
float ki = 0.7;
float kd = 1000;    
float distance_setpoint = 13;  // Distância alvo em cm
float PID_p, PID_i, PID_d, PID_total, PID_total2;
//////////////////////////////////////////////////////////////

// === Novo: Ângulo de equilíbrio da calha (ajuste conforme necessário) ===
int angulo_neutro = 80;  // Comece testando 90, depois ajuste conforme equilíbrio real
int min_angulo = angulo_neutro - 55;
int max_angulo = angulo_neutro + 55;

void setup() {
  Serial.begin(115200);
  myservo.attach(9);
  myservo.write(angulo_neutro);  // Posição inicial da calha
  pinMode(Analog_in, INPUT);
  time = millis();
  while (Serial.available() && Serial.read()); // empty buffer
}

void loop() {
  if (millis() > time + period) {
    time = millis();
    
    distance = get_dist(30);  // Média de 30 leituras
    distance_error = distance_setpoint - distance;

    PID_p = kp * distance_error;
    PID_d = kd * ((distance_error - distance_previous_error) / period);

    if (-3 < distance_error && distance_error < 3) {
      PID_i = PID_i + (ki * distance_error);
    } else {
      PID_i = 0;
    }

    PID_total = -(PID_p + PID_i + PID_d);

    // Mapeamento centrado no angulo_neutro
    PID_total2 = map(PID_total, -100, 100, min_angulo, max_angulo);

    // Limites de segurança
    if (PID_total2 < min_angulo) PID_total2 = min_angulo;
    if (PID_total2 > max_angulo) PID_total2 = max_angulo;

    myservo.write(PID_total2);
   //myservo.write(angulo_neutro +0);

    distance_previous_error = distance_error;

    // Debug via Serial Monitor
    //Serial.print("Distancia (cm): ");
   //Serial.print(distance);
   //Serial.print(" | PID Total: ");
   //Serial.println(PID_total2);
  }

  
/* variables declaration and initialization                */
  int  val =  0;           /* generic value read from serial */
  int  agv =  0;           /* generic analog value           */
  float valor_referencia = distance_setpoint;; //MUDAR PARA TEUS VALORES DO PROGRAMA
  float valor_distancia = distance; //MUDAR PARA TEUS VALORES DO PROGRAMA
  float valor_PID = PID_total2; //MUDAR PARA TEUS VALORES DO PROGRAMA

  if (Serial.available()!=0) {
    val = Serial.read(); 
  };

  //case A -> Leitura Analógica
  if (val==65){//A -> Analog read
    while (Serial.available()==0) {}; // Waiting char
    val=Serial.read();
    if (val == 48) { //from pin 0,   0=48
      agv = valor_referencia;
      Serial.write((uint8_t*)&agv,2); /* send binary value via serial  */   
    }
    if (val == 49) { //from pin 1,   1=49
      agv=valor_distancia;
      Serial.write((uint8_t*)&agv,2); /* send binary value via serial  */   
    }
    if (val == 50) { //from pin 2,   2=50
      agv=valor_PID;
      Serial.write((uint8_t*)&agv,2); /* send binary value via serial  */   
    }
    val=-1;
  }//end if
  
  //case R -> Analog reference
  if(val==82){
    while (Serial.available()==0) {};                
    val = Serial.read();
    if (val==48) analogReference(DEFAULT);
    if (val==49) analogReference(INTERNAL1V1);
    if (val==50) analogReference(EXTERNAL);
    if (val==51) Serial.print("v5");
    val=-1;
  }//end if
} // end loop


float get_dist(int n) {
  long sum = 0;
  for (int i = 0; i < n; i++) {
    sum += analogRead(Analog_in);
  }
  float adc = sum / n;
  float distance_cm = 17569.7 * pow(adc, -1.2062);  // Curva típica do Sharp 2Y0A21
  return distance_cm;
}
