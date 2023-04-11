#include <EEPROM.h>

#define Led_1       40
#define Led_2       41
#define Led_3       42
#define Led_4       43
#define Led_5       44

#define b_Rst       45
#define b_Esc       46
#define b_Enter     47
#define b_Mais      48
#define b_Menos     49
#define b_Passa     50
#define b_Func_A    51

//###################################################################################################################################
//###################################################################################################################################
//###################################################################################################################################

#define Pin_TPS                          A0       // Pino entrada tps - é ANALOGICO
#define Pin_Sensor_Temp_Agua             A1       // Sensor Temperatura do Cabeçote
#define Pin_Sinal_Roda_Fonica            31       // Pino Sinal da Roda Fonica, Calcula RPM com isto
#define Pin_Gerador_Hidrogenio           A2       // Pino Sinal Gerador de Hidrogenio(BOTAO LIGA E DESLIGA )
#define Pin_Botao_BackFire               A3
#define Pin_Botao_Pipoco                 A4
#define Pin_Botao_PreCorte               48       // Pino do botao do precorte
#define Pin_Reator_Gerador_Hidro         35


#define Pin_Fonte_A                      52 //OK
#define Pin_Fonte_B                      53 //OK

#define Pin_Sinal_Dist                   26 //OK   - Pino Sinal do Distribuidor, Calcula RPM com isto
#define Pin_Rele_Bob                     32 //OK
#define Pin_Rele_BombaCombus             33 //OK

#define Pin_Recuo_Atuador_ML             30 //OK

#define Pin_Ci_Bico_1                    22 //OK
#define Pin_Ci_Bico_2                    23 //OK
#define Pin_Ci_Bico_3                    24 //OK
#define Pin_Ci_Bico_4                    25 //OK


#define RPM_Min_Padrao                   800
#define RPM_Max_Padrao                   6450
#define RPM_Max_PreCorte_Padrao          3000
#define Gerador_Hidrogen_TPS_Min_Padrao  15

#define Mult_Tempo_Inj_TPS_Padrao        1.2
#define Mult_Tempo_Inj_TPS_GH_Padrao     0.7
#define RPM_Variacao                     100

#define Bicos_DeadTime_BaixaTensao       20;
#define Bicos_DeadTime_AltaTensao        7;
#define Bicos_DeadTime_Padrao            20

#define Tempo_Injecao_Padrao             30 

//###################################################################################################################################
//###################################################################################################################################
//###################################################################################################################################
bool Debug_Painel                     = true;
bool Apertado[55] = {false};
int BT_Apertado = -1;
int Menu = 1;
int Menu_Dentro = 0;
int Led[6] = {0};

char Menu_Nome_1[32] = "Tempo de Injeção";
char Menu_Nome_2[32] = "RPM Max"; 
char Menu_Nome_3[32] = "RPM Max Pre-Corte";
char Menu_Nome_4[32] = "RPM Marcha Lenta";
char Menu_Nome_5[32] = "#####";

unsigned long Pisca_Mills_Passado     = 0;
const long Pisca_Intervalo            = 300;
int Pisca_LedIndex                    = -1;

unsigned long Alerta_Millis_Passado   = 0;
const long Alerta_Intervalo           = 900;
int Alerta_Estado                     = LOW;
bool Alerta                           = false;


//###################################################################################################################################
//###################################################################################################################################
//###################################################################################################################################

bool Debug                          = true;
bool Log_RPM                        = true;
char Msg[64];                       // void Log

int  Ja_Pulsado                     = 0;
bool Distribuidor                   = false;
int Distribuidor_Count              = 0;
int Jan_Dist_Count                  = 0;
unsigned long Tempo_RPM_Passado          = 0;        // tempo de 1500ms para zerar Rpm caso motor desligue
bool BackFire_Status                = false;
bool Pipoco_Status                  = false;
bool Ja_Pressurizado                = false;

int RPM                             = 0;
int RPM_Start                       = 600;
int RPM_Min                         = 0;      // EEPROM
int RPM_Max                         = 0;      // EEPROM
int RPM_Max_PreCorte                = 0;      // EEPROM

int PreCorte                        = 0;        // Botao PreCorte


bool CutOffStatus                   = false;
int Temp_Sensor_Cabecote            = 0;
int Gerador_Hidrogenio              = 0;
int Gerador_Hidrogen_TPS_Min        = 0;       // EEPROM // Valor para TPS Minimo ativar o G.H (Muda o tempo de injeção)
int Sequencia_CC                    = 5;       // 5 Para modo Distribuidor, 1 para Sequencializar 1-3-4-2, sem ser na Roda Fonica
unsigned long TPS                   = 0;
unsigned long Tempo_TensaoBat       = 0;
unsigned long Tempo_TensaoBat_Log   = 0;
float Voltagem_Bateria              = 0;

// - Bobina Def's
unsigned long delay_Bobina          = 0;
bool Bobina_Liberado                = true;

//- Bomba de Combustivel Def's
bool BombaCombus_Liberado           = true;
unsigned long delay_BombaCombus     = 0;  
unsigned long BombaCosbus_Duracao   = 350;      // - TEMPO DE PRESSURIZACAO A CADA PULSO DO DISTRIBUIDOR
int Bomba_Combus_Duracao_AoLigar    = 1000;     // - TEMPO DE PRESSURIZACAO AO VIRAR A CHAVE

// - Bicos (Injetar) Def's
unsigned long delay_Bico[5]         = {0};
int Pin_Ci_Bico[4]                  = {0};
bool Bico_Liberado[4]               = {true};

unsigned long Bicos_DeadTime                = 20;
unsigned long Bobina_DeadTime               = 22;
unsigned long Tempo_Injecao                 = 0;
float Mult_Tempo_Inj_TPS                    = 0;
float Mult_Tempo_Inj_TPS_GH                 = 0;

// - Atuador Marcha Lenta Def's
unsigned long delay_AML             = 0;
unsigned long Refresh_AML           = 350;
int Pin_Exec_ML                     = 0;
bool ML_Liberado                    = true;

int LerInt(int endereco1, int endereco2)
{
  int valor = 0;
  byte primeiroByte = EEPROM.read(endereco1);
  byte segundoByte = EEPROM.read(endereco2);
  valor = (segundoByte << 8) + primeiroByte;
  return valor;
}

void EscreveInt(int endereco1, int endereco2, int valor)
{
  int valorAtual = LerInt(endereco1,endereco2);
  if (valorAtual == valor)
  {
    return;
  }
  else
  {
      byte primeiroByte = valor&0xff;
      byte segundoByte = (valor >> 8) &0xff;
      EEPROM.write(endereco1,primeiroByte);
      EEPROM.write(endereco2,segundoByte);
  }
}

long LerLong(long address) 
{
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void EscreveLong(int address, long value) 
{
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);
  
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}

void Setup_ValoresPadroes()
{
  return;
  EscreveInt(1, 2, RPM_Min_Padrao);                        // Rpm_Min
  EscreveInt(3, 4, RPM_Max_Padrao);                        // Rpm Max
  EscreveInt(5, 6, RPM_Max_PreCorte_Padrao);               // Rpm Max PreCorte
  EscreveInt(7, 8, Gerador_Hidrogen_TPS_Min_Padrao);       // Tps Min Gerador Hid  
  //EscreveLong(9, Mult_Tempo_Inj_TPS_Padrao);               // Mult_Tempo_Inj_TPS
  //EscreveLong(13, Mult_Tempo_Inj_TPS_GH_Padrao);           // Mult_Tempo_Inj_TPS_GH
  EscreveLong(17, Bicos_DeadTime_Padrao);                  // Bicos_DeadTime
  EscreveLong(21, Tempo_Injecao_Padrao);                   // Tempo_Injecao
}

void Carrega_Valores_EEPROM()
{
  // Carrega Valores da EEPROM
  
  RPM_Min                           = LerInt(1, 2);
  RPM_Max                           = LerInt(3, 4);
  RPM_Max_PreCorte                  = LerInt(5, 6);
  Gerador_Hidrogen_TPS_Min          = LerInt(7, 8); 
  Mult_Tempo_Inj_TPS                = Mult_Tempo_Inj_TPS_Padrao; //LerLong(9);
  Mult_Tempo_Inj_TPS_GH             = Mult_Tempo_Inj_TPS_GH_Padrao; //LerLong(13);
  Bicos_DeadTime                    = LerLong(17);
  Tempo_Injecao                     = LerLong(21);
  
  //\\ Carrega Valores da EEPROM
}

void Log(const char *t_Msg)
{
  if(!Debug) return;
  Serial.println(t_Msg);
}

void Log2(const char *t_Msg)
{
  if(!Debug_Painel) return;
  Serial.println(t_Msg);
}

void Verifica_Alimentacao_Arduino()
{
  if(digitalRead(Pin_Fonte_A) == LOW) Log("[Alerta] Problema de Alimentação [Chip A]");
  if(digitalRead(Pin_Fonte_B) == LOW) Log("[Alerta] Problema de Alimentação [Chip B]");
}

void Tensao_Bateria()
{
  unsigned long t_Agr = millis();
  float T_Volts;
  
  if(t_Agr - 150 > Tempo_TensaoBat)
  {
    Tempo_TensaoBat = t_Agr;
    /*float T_Total = float(analogRead(A1));
    float T_Acerto = ((T_Total / 100) / 100) * 10;
    T_Volts = ((T_Total / 100) * 2) + (T_Acerto * 2);
    Voltagem_Bateria = T_Volts;
    */
    float T_Total = (analogRead(A1)/2)*4;
    float DezPCent = (T_Total/100)*22;
    T_Total = T_Total + DezPCent;
    Voltagem_Bateria = T_Total / 100;
  }
  
  if(t_Agr - 1000 > Tempo_TensaoBat_Log)
  {
    //if(RPM < 100) return;
    Tempo_TensaoBat_Log = t_Agr;
    //sprintf(Msg, "Bat: %d.%02d", (int)T_Volts, (int)(T_Volts*100)%100);
    //sprintf(Msg, "Bat: %d", (int)Voltagem_Bateria);
    //sprintf(Msg, "%s%s", Msg, "v");
    //Log(Msg);
  }
}

void setup() 
{
  if(Debug) Serial.begin(9600);

  // ##################################################################################################
  Led[1] = Led_1;
  Led[2] = Led_2;
  Led[3] = Led_3;
  Led[4] = Led_4;
  Led[5] = Led_5;
  
  pinMode(b_Esc, INPUT);      // [<<] Voltar/Esc
  pinMode(b_Enter, INPUT);    // [o] Enter
  pinMode(b_Passa, INPUT);    // [>] Passa Lado
  pinMode(b_Mais, INPUT);     // [+]
  pinMode(b_Menos, INPUT);    // [-]
  pinMode(b_Rst, INPUT);      // [RST]
  pinMode(b_Func_A, INPUT);   // [??]

  pinMode(Led_1, OUTPUT); // Led Painel
  pinMode(Led_2, OUTPUT); // Led Painel
  pinMode(Led_3, OUTPUT); // Led Painel
  pinMode(Led_4, OUTPUT); // Led Painel
  pinMode(Led_5, OUTPUT); // Led Painel

  digitalWrite(Led_1, HIGH);
  // ##################################################################################################
  
  CutOffStatus                        = false;
  
  // - Bobina
  pinMode(Pin_Rele_Bob, OUTPUT);
  digitalWrite(Pin_Rele_Bob, HIGH);

  //Bomba Combustivel 
  pinMode(Pin_Rele_BombaCombus, OUTPUT);
  digitalWrite(Pin_Rele_BombaCombus, HIGH); 

  // - Atuador M.L
  pinMode(Pin_Recuo_Atuador_ML, OUTPUT);
  digitalWrite(Pin_Recuo_Atuador_ML, HIGH);
  
  // - Bicos
  Pin_Ci_Bico[1]                    = Pin_Ci_Bico_1;
  Pin_Ci_Bico[2]                    = Pin_Ci_Bico_2;
  Pin_Ci_Bico[3]                    = Pin_Ci_Bico_3;
  Pin_Ci_Bico[4]                    = Pin_Ci_Bico_4;

  Carrega_Valores_EEPROM();

  
  pinMode(Pin_Ci_Bico[1], OUTPUT);
  pinMode(Pin_Ci_Bico[2], OUTPUT);
  pinMode(Pin_Ci_Bico[3], OUTPUT);
  pinMode(Pin_Ci_Bico[4], OUTPUT);
  
  digitalWrite(Pin_Ci_Bico[1], LOW);
  digitalWrite(Pin_Ci_Bico[2], LOW);
  digitalWrite(Pin_Ci_Bico[3], LOW);
  digitalWrite(Pin_Ci_Bico[4], LOW);

  
  pinMode(Pin_Botao_PreCorte, INPUT);
  pinMode(Pin_Sinal_Dist, INPUT);
  pinMode(Pin_TPS, INPUT);
  pinMode(Pin_Sensor_Temp_Agua, INPUT);
  pinMode(Pin_Sinal_Roda_Fonica, INPUT);
  pinMode(Pin_Gerador_Hidrogenio, INPUT);
  pinMode(Pin_Botao_BackFire, INPUT);
  pinMode(Pin_Botao_Pipoco, INPUT);
  pinMode(Pin_Reator_Gerador_Hidro, INPUT);
  
  pinMode(Pin_Fonte_A, INPUT);
  pinMode(Pin_Fonte_B, INPUT);
  
  delay(50);
  Log("...");
  Log("######## [ MX Injection v0.01 ] ########");
  Log("Setup feito com sucesso!");

  Verifica_Alimentacao_Arduino();
}

void Apaga_Leds()
{
    digitalWrite(Led[1], LOW);
    digitalWrite(Led[2], LOW);
    digitalWrite(Led[3], LOW);
    digitalWrite(Led[4], LOW);
    digitalWrite(Led[5], LOW);
}

void Pisca_Menu()
{
  if(Menu_Dentro == 1)
  {
    digitalWrite(Led[Pisca_LedIndex], HIGH);
    return;
  }
  
  if(Menu == 1) { digitalWrite(Led[1], HIGH); digitalWrite(Led[5], LOW); }
  if(Pisca_LedIndex == -1)  return;
  
  Apaga_Leds();
  unsigned long SubMenu_Intervalo = 0;
  if(Menu >= 6 && Menu <= 10) SubMenu_Intervalo = 250;
  if(Menu >= 11 && Menu <= 15) SubMenu_Intervalo = 550;
  
 if(Pisca_Mills_Passado == 0) 
  {
    sprintf(Msg, "Pisca Menu: %d", Pisca_LedIndex);
    //Log(Msg);
    Pisca_Mills_Passado = millis();
  }
  
  if(millis() >= (Pisca_Intervalo + SubMenu_Intervalo + Pisca_Mills_Passado))
  {
    digitalWrite(Led[Pisca_LedIndex], HIGH);
    Pisca_LedIndex = -1;
    Pisca_Mills_Passado = 0;
    return;
  }
}

void f_Alerta()
{
  if(!Alerta) return;
  if(Menu_Dentro == 1) return;
  unsigned long millisAgr = millis();
  
  if((millisAgr - Alerta_Millis_Passado) >= Alerta_Intervalo) 
  {
    if(Alerta_Estado == LOW)
    {
      Alerta_Estado = HIGH;
    }else
    {
      Alerta_Estado = LOW;
    }
    
    int l;
    for(l = 1; l < 6; l++)
    {
      digitalWrite(Led[l], Alerta_Estado);
    }
    
    Alerta_Millis_Passado = millisAgr;
  }
}

bool Verificar_Corte_Giro()
{
  PreCorte    = digitalRead(Pin_Botao_PreCorte);
  int RPM_Limit   = RPM_Max;
  
  if(PreCorte == HIGH) RPM_Limit = RPM_Max_PreCorte;
  if(RPM >= RPM_Limit) 
  {
    sprintf(Msg, "[C.G. %d RPM]", RPM_Limit);
    Log(Msg);
    return true;
  }

  return false;
}

void Verifica_DeadTime()
{    
  if(Voltagem_Bateria <= 10.00)
  {
    Bicos_DeadTime        = Bicos_DeadTime_BaixaTensao;
    Bobina_DeadTime       = 32;
  }else
  {
    Bicos_DeadTime        = Bicos_DeadTime_AltaTensao;
    Bobina_DeadTime       = 20;
  }
}

void Injetar(int bIndex)
{ 
  //if(RPM < 100) return;
  if(bIndex <= 0 || bIndex > 5)   return;
  if(!Bico_Liberado[bIndex] && bIndex < 5)      return;
  if(CutOffStatus)                return;
  if(Verificar_Corte_Giro())      return;
  
  unsigned long Agora     = millis();
 
  if(bIndex == 5)
  {
    digitalWrite(Pin_Ci_Bico[1], HIGH); 
    digitalWrite(Pin_Ci_Bico[2], HIGH); 
    digitalWrite(Pin_Ci_Bico[3], HIGH); 
    digitalWrite(Pin_Ci_Bico[4], HIGH); 
    
    delay_Bico[1]      = Agora;
    delay_Bico[2]      = Agora;
    delay_Bico[3]      = Agora;
    delay_Bico[4]      = Agora;
    
    Bico_Liberado[1]   = false;
    Bico_Liberado[2]   = false;
    Bico_Liberado[3]   = false;
    Bico_Liberado[4]   = false;

    char floatSTR[5];
    dtostrf(Voltagem_Bateria,4,2,floatSTR);
    sprintf(Msg, "Bat: %s%s", floatSTR, "v");
    Log(Msg);
    
    int TmpInt = Bicos_DeadTime / 10;
    sprintf(Msg, "DT Bicos: %d %s [%d]", TmpInt, "ms", Bicos_DeadTime);
    Log(Msg);

    TmpInt = Bobina_DeadTime / 10;
    sprintf(Msg, "DT Bobi: %d %s [%d]", TmpInt, "ms", Bobina_DeadTime);
    Log(Msg);

    int tmpDl = Tempo_Injecao / 10;
    sprintf(Msg, "T.i: %d%s [%d]", tmpDl, "ms", Tempo_Injecao);
    Log(Msg);


    sprintf(Msg, "Bicos [1~4] On");
    Log(Msg);
    
    return;
  }
  
  if(bIndex != -1) 
  {
    digitalWrite(Pin_Ci_Bico[bIndex], HIGH); 
    
    delay_Bico[bIndex]      = Agora;
    Bico_Liberado[bIndex]   = false;
    
    sprintf(Msg, "B. Inj. (%d) On!", bIndex);
    Log(Msg);
    return;
  }
}

void Faiscar_Bobina()
{
  if(Pipoco_Status)               return;
  if(BackFire_Status)             return;
  if(!Bobina_Liberado)            return;
  if(RPM < 15)                   return;
  if(CutOffStatus)                return;
  if(Verificar_Corte_Giro())      return;
  
  digitalWrite(Pin_Rele_Bob, LOW); 
  delay_Bobina            = millis();
  Bobina_Liberado         = false;
  
  Log("Bobina On");
}

void Pressurizar_Linha_Combustivel()
{
  if(!BombaCombus_Liberado)          return;
  digitalWrite(Pin_Rele_BombaCombus, LOW);
  delay_BombaCombus       = millis();
  BombaCombus_Liberado    = false;
  if(Ja_Pressurizado) Log("B. de Comb. On");
  if(!Ja_Pressurizado) Log("Pressurizando Linha de Comb.!");
  
}

void Atuador_Marcha_Lenta()
{
  if(!ML_Liberado)              return;
  digitalWrite(Pin_Recuo_Atuador_ML, LOW); 
  delay_AML                   = millis();
  ML_Liberado                 = false;
  sprintf(Msg, "At.ML. [%d / %d]", RPM_Min, RPM);
  Log(Msg);
}

void Delay_Atuador_Marcha_Lenta()
{
  if(millis() >= (delay_AML + Refresh_AML)) // < - Delay para Pulso Atuador Marcha Lenta
  {
    digitalWrite(Pin_Recuo_Atuador_ML, HIGH);
    ML_Liberado = true;
  }
}

void Libera_BombaCombus()
{
  if(millis() >= (delay_BombaCombus + BombaCosbus_Duracao)) // < - Delay para Pulso
  {
    digitalWrite(Pin_Rele_BombaCombus, HIGH); 
    BombaCombus_Liberado = true;
  }
}

void Libera_Bobina()
{
  if(millis() >= (delay_Bobina + Bobina_DeadTime)) // < - Delay para Pulso Bobina
  {
    digitalWrite(Pin_Rele_Bob, HIGH); 
    Bobina_Liberado = true;
  }
}

void Libera_Bicos()
{
  int i;
  unsigned long T_Agr = millis();
  for(i = 1; i < 5; i++)
  {
    if(T_Agr >= (delay_Bico[i] + Tempo_Injecao + Bicos_DeadTime))
    {
       digitalWrite(Pin_Ci_Bico[i], LOW); 
       Bico_Liberado[i] = true;
    }
  }
}

void Painel() 
{
  //Alerta = true;
  f_Alerta();

  int i;

  for(i = 45; i <= 51; i++) // NAO MUDAR PINAGEM NA PLACA, VAI INFLUENCIAR AQUI
  {
    int Bt = digitalRead(i);
    if(digitalRead(i) == HIGH)
    {
      BT_Apertado = i;
      sprintf(Msg, "%d", i);
      //Log(Msg);
      break;
    }
    else
    {
      BT_Apertado = -1;
      Apertado[i] = false;
    }
  }

  if(Menu >= 6 && Menu <= 15)
  {
    int SubTrai = 5;
    if(Menu >= 11) SubTrai = 10;
    int vLed = Menu - SubTrai;
    if(vLed < 0) vLed = 1;
    Pisca_LedIndex = vLed;
  }
  
  if(Apertado[BT_Apertado]) return;
  
  if(Menu_Dentro == 0)
  {
    if(BT_Apertado == b_Enter)
    {
        Menu_Dentro = 1;
        
        if(Menu == 1) sprintf(Msg, "[%s]", Menu_Nome_1);
        if(Menu == 2) sprintf(Msg, "[%s]", Menu_Nome_2);
        if(Menu == 3) sprintf(Msg, "[%s]", Menu_Nome_3);
        if(Menu == 4) sprintf(Msg, "[%s]", Menu_Nome_4);
        if(Menu == 5) sprintf(Msg, "[%s]", Menu_Nome_5);
        Log2(Msg);

        Pisca_LedIndex = Menu; // Fazer Piscar
        
        Apertado[BT_Apertado] = true;
        return;
    }

    if(BT_Apertado == b_Passa)
    {
      Apertado[BT_Apertado] = true;
      Menu++;
      
      if(Menu == 5) Menu = 1; // Limite de 4 Menus
      
      if(Menu == 1) sprintf(Msg, "[> %s]", Menu_Nome_1);
      if(Menu == 2) sprintf(Msg, "[> %s]", Menu_Nome_2);
      if(Menu == 3) sprintf(Msg, "[> %s]", Menu_Nome_3);
      if(Menu == 4) sprintf(Msg, "[> %s]", Menu_Nome_4);
      if(Menu == 5) sprintf(Msg, "[> %s]", Menu_Nome_5);
      Log2(Msg);

      Apaga_Leds();
      digitalWrite(Led[Menu], HIGH);
      
      return;
    }

    if(BT_Apertado == b_Func_A)
    {
      Apertado[BT_Apertado] = true;
      Log2("[LOG RPM]");

      if(!Log_RPM)
      {
        Log_RPM = true;
      }else
      {
        Log_RPM = false;
      }
      return;
    }
  }

  if(Menu_Dentro == 1)
  {
    if(BT_Apertado == b_Esc)
    {
      Menu_Dentro = 0;
      Log2("[<<]");
      return;
    }

    if(BT_Apertado == b_Mais) //+
    {
      //Log2("[+]");
      Pisca_LedIndex = Menu; // Fazer Piscar
      Apertado[BT_Apertado] = true;

      // - AQUI DIZ OQUE FAZER
      if(Menu == 1) 
      {
        if(Tempo_Injecao < 160) 
        {
          Tempo_Injecao += 5;
          EscreveLong(21, Tempo_Injecao);
        }

        sprintf(Msg, "Tempo Injecao: [+] %d", Tempo_Injecao);
        Log(Msg);
      }

      if(Menu == 2) 
      {
        if(RPM_Max < 7900) 
        {
          RPM_Max += 100;
          EscreveInt(3, 4, RPM_Max);
          sprintf(Msg, "RPM Max: [+] %d", RPM_Max);
          Log(Msg);
        }
      }

      if(Menu == 3)
      {
        if(RPM_Max_PreCorte > RPM_Min && RPM_Max_PreCorte < RPM_Max)
        {
          RPM_Max_PreCorte += 100;
          EscreveInt(5, 6, RPM_Max_PreCorte);
          sprintf(Msg, "RPM Max PreCorte: [+] %d", RPM_Max_PreCorte);
          Log(Msg);
        }
      }

      if(Menu == 4)
      {
        if(RPM_Min < 1500)
        {
          RPM_Min += 25;
          EscreveInt(1, 2, RPM_Min);
          sprintf(Msg, "RPM Marcha-Lenta: [+] %d", RPM_Min);
          Log(Msg);
        }
      }
      
      if(Menu == 5)
      {
        //EscreveInt(25, 26, Avanco_Graus);
        //sprintf(Msg, "Avanço de IGN: [+] %dº", Avanco_Graus);
        Log(Msg);
      }
      return;
    }

    if(BT_Apertado == b_Menos)// -
    {
      //Log2("[-]");
      Pisca_LedIndex = Menu; // Fazer Piscar
      Apertado[BT_Apertado] = true;

      // - AQUI DIZ OQUE FAZER
      if(Menu == 1) 
      {
        if(Tempo_Injecao > 100) 
        {
          Tempo_Injecao -= 5;
          EscreveLong(21, Tempo_Injecao);
        }
        sprintf(Msg, "Tempo Injecao: [-] %d", Tempo_Injecao);
        Log(Msg);
      }

      if(Menu == 2) 
      {
        if(RPM_Max > 3000) 
        {
          RPM_Max -= 100;
          EscreveInt(3, 4, RPM_Max);
          sprintf(Msg, "RPM Max: [-] %d", RPM_Max);
          Log(Msg);
        }
      }

      if(Menu == 3)
      {
        if(RPM_Max_PreCorte > RPM_Min && RPM_Max_PreCorte < RPM_Max)
        {
          RPM_Max_PreCorte -= 100;
          EscreveInt(5, 6, RPM_Max_PreCorte);
          sprintf(Msg, "RPM Max PreCorte: [-] %d", RPM_Max_PreCorte);
          Log(Msg);
        }
      }

      if(Menu == 4)
      {
        if(RPM_Min > 750)
        {
          RPM_Min -= 25;
          EscreveInt(1, 2, RPM_Min);
          sprintf(Msg, "RPM Marcha-Lenta: [-] %d", RPM_Min);
          Log(Msg);
        }
      }

      if(Menu == 5)
      {

        //EscreveInt(25, 26, Avanco_Graus);
        //sprintf(Msg, "Avanço de IGN: [-] %dº", Avanco_Graus);
        Log(Msg);
      }
      return;
    }

    if(BT_Apertado == b_Rst)// RST
    {
     // Log2("[Rst]");
      Pisca_LedIndex = Menu; // Fazer Piscar
      Apertado[BT_Apertado] = true;

      // - AQUI DIS OQUE FAZER
      if(Menu == 1)
      {
        Tempo_Injecao = Tempo_Injecao_Padrao;
        sprintf(Msg, "Tempo Injecao: [P] %d", Tempo_Injecao);
        Log(Msg);
      }

      if(Menu == 2)
      {
        RPM_Max = RPM_Max_Padrao;
        sprintf(Msg, "RPM Max: [P] %d", RPM_Max);
        Log(Msg);
      }

      if(Menu == 3)
      {
        RPM_Max_PreCorte = RPM_Max_PreCorte_Padrao;
        sprintf(Msg, "RPM Max PreCorte: [P] %d", RPM_Max_PreCorte);
        Log(Msg);
      }

      if(Menu == 4)
      {
        RPM_Min = RPM_Min_Padrao;
        EscreveInt(1, 2, RPM_Min);
        sprintf(Msg, "RPM Marcha-Lenta: [P] %d", RPM_Min);
        Log(Msg);
      }

      if(Menu == 5)
      {
        //EscreveInt(25, 26, Avanco_Graus);
        //sprintf(Msg, "Avanço de IGN: [P] %dº", Avanco_Graus);
        Log(Msg);
      }
      return;
    }
  }
  
}

void Verificar_Marcha_lenta()
{
  if(RPM > RPM_Start)
  {
    bool RpmRef = false;
    if(RPM > 1000 && !RpmRef)  { Refresh_AML = 100; RpmRef = true; } 
    if(RPM > 950 &&  !RpmRef)  { Refresh_AML = 90;  RpmRef = true; }
    if(RPM > 900 &&  !RpmRef)  { Refresh_AML = 80;  RpmRef = true; }
    if(RPM > 875 &&  !RpmRef)  { Refresh_AML = 70;  RpmRef = true; }
    if(RPM > 850 &&  !RpmRef)  { Refresh_AML = 60;  RpmRef = true; }
    if(RPM > 825 &&  !RpmRef)  { Refresh_AML = 50;  RpmRef = true; }
    if(RpmRef) Atuador_Marcha_Lenta();
  }
}

void Sequencia_Explosao()
{  
  if(Ja_Pulsado >= 2) return;
  if(RPM < 15) return;
  
  Log(" ");

  Sequencia_CC = 5;
  switch(Sequencia_CC) 
  {
  case 5: Sequencia_CC = 5; break;
  case 1: Sequencia_CC = 3; break;
  case 3: Sequencia_CC = 4; break;
  case 4: Sequencia_CC = 2; break;
  case 2: Sequencia_CC = 1; break;
  }

  Injetar(Sequencia_CC);
  Faiscar_Bobina();
  if(Log_RPM)
  {
    sprintf(Msg, "%d RPM", RPM);
    Log(Msg);
  }
  
  sprintf(Msg, "%d%% TPS", TPS);
  Log(Msg);
  Log("___________________________________");
}
void(* resetFunc) (void) = 0;

void Verificar_Distribuidor_Sinal()
{
  if(Distribuidor)
  {

      if(Distribuidor_Count == 0)
      {
        Distribuidor_Count = 1;

        Jan_Dist_Count++;
        unsigned long T_Agora = millis();
         
        RPM =  30*1000 / ( T_Agora - Tempo_RPM_Passado );
        Tempo_RPM_Passado = T_Agora;    

        //if(Jan_Dist_Count == 1) { if(RPM > 100) Pressurizar_Linha_Combustivel(); }
        if(RPM > 100) Pressurizar_Linha_Combustivel();

        if(Jan_Dist_Count == 5) Jan_Dist_Count = 0;

        Sequencia_Explosao();
      }
  }else
  {
    if(Distribuidor_Count == 1) Distribuidor_Count = 0;
  }

  if(RPM > 0 && ((millis() - Tempo_RPM_Passado) > 1500))
  {
    RPM = 0; // Zera rpm se passar mais de 1000ms
    //Jan_Dist_Count = 0;
    Tempo_RPM_Passado = 0;
    //resetFunc();
    Log("## Rotação Zerada ou Motor Off ##");
  }
}

void loop()
{
  Setup_ValoresPadroes();
  
  Delay_Atuador_Marcha_Lenta();
  Verifica_DeadTime();
  Libera_Bobina();
  Libera_Bicos();
  Libera_BombaCombus();
  Tensao_Bateria();
  
  //Tempo_Injecao = 3.06;
  //Injetar(5);
  //delay(10);
  //return;
  
  if(analogRead(Pin_Botao_BackFire) > 1000) 
  {
    //BackFire_Status = true;
  }
  else 
  {
    BackFire_Status = false;
  }

  if(analogRead(Pin_Botao_Pipoco) > 1000) 
  {
    //Pipoco_Status = true;
  }
  else 
  {
    Pipoco_Status = false;
  }
  
  // - ##################################
  // - ##### [ TPS & TEMPO INJEÇAO] #####
  // - ##################################
  TPS                     = analogRead(Pin_TPS) / 10;    /*Dividir por 10 para Transformar em %*/
  if(TPS > 100) TPS       = 100;  /*Correção 102%, Pode dar Bosta Aqui*/
  Temp_Sensor_Cabecote    = 0; //analogRead(Pin_Sensor_Temp_Agua);
  Gerador_Hidrogenio      = 0; //analogRead(Pin_Gerador_Hidrogenio);
  
  if(Temp_Sensor_Cabecote >  80)
  {
      
  }
  
  if(Gerador_Hidrogenio > 1000 && TPS >= Gerador_Hidrogen_TPS_Min)
  {
      //digitalWrite(Pin_Reator_Gerador_Hidro, HIGH); // LIGA GERADOR HIDROGENIO
      //Tempo_Injecao = Tempo_Injecao_Padrao + (TPS * Mult_Tempo_Inj_TPS_GH);
  }else
  {
      //digitalWrite(Pin_Reator_Gerador_Hidro, LOW); // DESLIGA GERADOR HIDROGENIO
      //Tempo_Injecao = Tempo_Injecao_Padrao + (TPS * Mult_Tempo_Inj_TPS); 
  }
  
  //Tempo_Injecao += Bicos_DeadTime;
  //if(Tempo_Injecao > 14.0)  Tempo_Injecao = 14.0;
  //if(BackFire_Status)     Tempo_Injecao = 14.0;
  if(BackFire_Status && Pipoco_Status)
  {
      //Tempo_Injecao = Tempo_Injecao_Padrao + (TPS * Mult_Tempo_Inj_TPS);  // ?????????????????????
  }
  else
  {
      Tempo_Injecao = Tempo_Injecao_Padrao + (TPS * Mult_Tempo_Inj_TPS);
  }
  

  //Ja_Pulsado = 0;  
  Distribuidor = false;
  bool sinal_DistPin = digitalRead(Pin_Sinal_Dist);
  //sprintf(Msg, "[dst: %d]", sinal_DistPin);
  //Log(Msg);
  
  if(sinal_DistPin == LOW) 
  {
    Ja_Pulsado++;    
    if(Ja_Pulsado == 1) { Distribuidor = true; }
    else { Distribuidor = false; }
  }else 
  {
    if(sinal_DistPin == true)
    {
      Ja_Pulsado = 0;
      Distribuidor = false;
    }
  }

  Verificar_Distribuidor_Sinal();
  
  if(TPS <= 11) 
  {
    if(RPM > (RPM_Min + RPM_Variacao))
    {
      CutOffStatus = true;
    }
    else 
    {
      CutOffStatus = false;
      Verificar_Marcha_lenta();
    }
    
  }

  //####################################################################################################
  Pisca_Menu();
  Painel();
  //####################################################################################################

  if(!Ja_Pressurizado)
  {
    Ja_Pressurizado = true;
    Pressurizar_Linha_Combustivel();
    delay(Bomba_Combus_Duracao_AoLigar);
  }
}
