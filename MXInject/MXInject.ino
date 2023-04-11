#include <EEPROM.h>
#include <SoftwareSerial.h>

#define Pin_R1       31
#define Pin_R2       32 
#define Pin_R3       33  
#define Pin_R4       34  
#define Pin_R5       35  
#define Pin_R6       36  
#define Pin_R7       37   //Danificcpado
#define Pin_R8       38  

#define Pin_T1       22
#define Pin_T2       23
#define Pin_T3       24
#define Pin_T4       25
#define Pin_T5       26
#define Pin_T6       28
#define Pin_T7       29
#define Pin_T8       30

#define Tipo_Terra    0
#define Tipo_Rele     1
#define Tipo_Sensor   2
#define Tipo_Atuador  3

#define MAX_OBJ       32
int total_obj       = 0;
//################################

#define   Pin_Saida_Negativa_1           22 // A.M.L
#define   Pin_Saida_Negativa_2           23 // bicos
#define   Pin_Saida_Negativa_3           25 // rele bomba gas
#define   Pin_Saida_Negativa_4           26 // 
#define   Pin_Saida_Negativa_5           27 //
#define   Pin_Saida_Negativa_6           28
#define   Pin_Saida_Negativa_7           29
#define   Pin_Saida_Negativa_8           30


#define Pin_TPS                           A0 //ANALOG
#define Pin_Temperatura_AR                A2 //ANALOG
#define Pin_Temperatura_AGUA              A3 //ANALOG
#define Pin_Sensor_HALL                   2  //DIGITAL

#define   Pin_Voltimetro_A                A4 
#define   Pin_Voltimetro_B                A5 



typedef struct
  {
   
    bool Ativado;
    char Nome[32];
    int Tipo;
    int Index;
    
    int Pin_Entrada;
    int Pin_Saida;
    bool Saida_Status;
    
    unsigned long delay_Acc;
    unsigned long momento_Acc;
  }  objectstruct;
/*
#define Tipo_Terra    0
#define Tipo_Rele     1
#define Tipo_Sensor   2
#define Tipo_Atuador  3
*/
 
objectstruct mxObj[MAX_OBJ];


// - Voltimetro [A] & [B]

char TensaoA[6];
char TensaoB[6];

int TPS_Iddle;
int Tempo_Inject;
int ID_Bicos;
int ID_B1;
int ID_B2;
int ID_B3;
int ID_B4;
int Tempo_Bomba_Press;
int ID_BombaGas;
int Bomba_Count;
int Tempo_AML_Press;


int RPM;
int LastRPM;
unsigned long Tempo_RPM_Passado;

int SP_delayStartStop;
int AmplificadorSom       = 1;
int Milhas                = 1;
int CarregarBatAux        = 1;
int PainelStatus          = 1;
int PreCorteStatus        = 1;


//unsigned long delay_updaterpm = 0;


bool Procurar(char *OndeProcurar, char *Pesquisar, bool CaseSensitive)
{  
  int TotalEncontrado   = 0;
  int TamanhoOndeP    = strlen(OndeProcurar);
  int TamanhoPesq     = strlen(Pesquisar);
  int h = 0;

  if(TamanhoPesq > TamanhoOndeP) return false;
  for(int i = 0; i < TamanhoOndeP; i++)
  {
    char CaractereA = OndeProcurar[i];
    char CaractereB = Pesquisar[h];
    if(!CaseSensitive) CaractereA = tolower(OndeProcurar[h]);
    if(!CaseSensitive) CaractereB = tolower(Pesquisar[h]);

    if(CaractereA == CaractereB) 
    {
      TotalEncontrado++;
      h++;
    }else
    {
      TotalEncontrado = 0;
      h = 0;
    }
    if(TotalEncontrado >= TamanhoPesq) return true;
  }
  if(TotalEncontrado >= TamanhoPesq) return true;
  return false;
}

int LerInt(int endereco1, int endereco2)
{
  return (EEPROM.read(endereco2) << 8) + EEPROM.read(endereco1);
}

void EscreveInt(int endereco1, int endereco2, int valor)
{
  if(LerInt(endereco1, endereco2) == valor) return;
  EEPROM.write(endereco1, valor&0xff);
  EEPROM.write(endereco2, (valor >> 8) &0xff);
}

long LerLong(long address) 
{
  return ((EEPROM.read(address) << 0) & 0xFF) + ((EEPROM.read(address + 1) << 8) & 0xFFFF) + ((EEPROM.read(address + 2) << 16) & 0xFFFFFF) + ((EEPROM.read(address + 3) << 24) & 0xFFFFFFFF);
}

void EscreveLong(int address, long value) 
{
  EEPROM.write(address, (value & 0xFF));
  EEPROM.write(address + 1, ((value >> 8) & 0xFF));
  EEPROM.write(address + 2, ((value >> 16) & 0xFF));
  EEPROM.write(address + 3, ((value >> 24) & 0xFF));
}

void Setup_ValoresPadroes()
{
  //EscreveInt(5, 6, 2500);     // - RPM PRE CORTE        #INT
  //EscreveLong(7, 150);        // - DEADTIME BOBINA #LONG

  EscreveInt(1, 2, 200);
}

void Carrega_Valores_EEPROM()
{
//  SP_TempoInjecao   = LerLong(15);
 // SP_TempoPressBomb = LerInt(19, 20);
 Tempo_Bomba_Press          = LerInt(1, 2);
}

void Salva_Valores_EEPROM()
{
  //EscreveInt(5 , 6, SP_PRECorte);
  //EscreveLong(7, SP_DT_Bobina);

  EscreveInt(1 , 2, Tempo_Bomba_Press);
}

void Log(const char *t_Msg)
{
  Serial.println(t_Msg);
}

void func_RPM()
{
  unsigned long T_Agora = millis();
  int TmpRPM = 30 * 1000 / (T_Agora - Tempo_RPM_Passado);
  
  if(LastRPM == 0) LastRPM = TmpRPM;
  if((TmpRPM - LastRPM) > 2000) return;
  if(TmpRPM == 0 )
  {
    if((LastRPM - TmpRPM) > 500) return;
  }
  RPM               = TmpRPM;
  LastRPM           = TmpRPM;
  Tempo_RPM_Passado = T_Agora;  
 
  //sprintf(Msg, "RPM: %d", RPM);
  //Log(Msg);
}


void AML_Liberar()
{
 // if(millis() >= (mxObj[6].delay_acc + Tempo_AML_Press))
  //  {
  //    digitalWrite(mxObj[6].Pin_Saida, LOW); 
  //    mxObj[6].Obj_Liberado  = true;
   // }
}
void AML_Press()
{
  //if(!mxObj[5].Obj_Liberado) return;

   // mxObj[6].delay_acc    = millis();
    //mxObj[6].Obj_Liberado  = false;
    mxObj[6].Saida_Status = HIGH;
    
    digitalWrite(mxObj[6].Pin_Saida, mxObj[6].Saida_Status); 
   // sprintf(Msg, "A.M.L: ON");
  //  Log(Msg);
}

void AML_Setup()
{
  //sprintf(Msg, "Configurando Atuador Marcha Lenta");
  //Log(Msg);

  mxObj[5].Pin_Saida = Pin_Saida_Negativa_1;
  sprintf(mxObj[6].Nome, "Atu. Mar. Len.");
   
  //mxObj[6].Obj_Liberado   = true;
 // mxObj[6].delay_acc      = 0;
  mxObj[6].Saida_Status   = LOW;
  mxObj[6].Ativado        = true;
  
  digitalWrite(mxObj[6].Pin_Saida, mxObj[6].Saida_Status); 
  //if(mxObj[6].Saida_Status == 0) sprintf(Msg, "A.M.L: [LOW]");
  //if(mxObj[6].Saida_Status == 1) sprintf(Msg, "A.M.L: [HIGH]");
 // Log(Msg);

}
void Bomba_Gas_Liberar()
{
  if(millis() >= (mxObj[ID_BombaGas].delay_Acc + Tempo_Bomba_Press))
    {
      digitalWrite(mxObj[ID_BombaGas].Pin_Saida, LOW); 
     // mxObj[ID_BombaGas].Obj_Liberado  = true;
    }
}

void Bomba_Gas_Press()
{
  
    mxObj[ID_BombaGas].delay_Acc    = millis();
    mxObj[ID_BombaGas].Saida_Status = HIGH;
    
    digitalWrite(mxObj[ID_BombaGas].Pin_Saida, mxObj[ID_BombaGas].Saida_Status); 
}


void Bicos_Liberar()
{
  if(millis() >= (mxObj[ID_Bicos].delay_Acc + Tempo_Inject))
  {
    digitalWrite(mxObj[ID_Bicos].Pin_Saida, LOW); 
  }
}

void Bicos_Injetar()
{
    mxObj[ID_Bicos].delay_Acc    = millis();
    mxObj[ID_Bicos].Saida_Status = HIGH;
    digitalWrite(mxObj[ID_Bicos].Pin_Saida, mxObj[ID_Bicos].Saida_Status); 
}

void TPS_Setup()
 {

    pinMode(Pin_TPS, INPUT);
    sprintf(mxObj[7].Nome, "TPS");
    TPS_Iddle = 11;
    //sprintf(Msg, "Configurando TPS: [iddle %d%%]", TPS_Iddle);
    //Log(Msg);
    mxObj[7].Pin_Entrada = Pin_TPS;
    mxObj[7].Ativado = true;
 }

 void TempAr_Setup()
 {
  Log("Configurando Temp.  Ar");
  sprintf(mxObj[8].Nome, "TempAr");
  pinMode(Pin_Temperatura_AR, INPUT);
  mxObj[8].Pin_Entrada = Pin_Temperatura_AR;
  mxObj[8].Ativado = true;
 }

 void TempAgua_Setup()
 {
  Log("Configurando Temp.  Agua");
  sprintf(mxObj[9].Nome, "TempAgua");
  pinMode(Pin_Temperatura_AGUA, INPUT);
  mxObj[9].Pin_Entrada = Pin_Temperatura_AGUA;
  mxObj[9].Ativado = true;
 }

void SensorHall_interrup()
{  
  unsigned long T_Agora = millis();
  unsigned long TmpRPM = 30 * 1000 / (T_Agora - Tempo_RPM_Passado);

  Bomba_Count++;
  if(Bomba_Count == 4) { Bomba_Count = 0; Bomba_Gas_Press(); }

  if(LastRPM == 0) LastRPM = TmpRPM;

  if(TmpRPM < 8000)
  {
    RPM               = TmpRPM;
    LastRPM           = TmpRPM;
    Tempo_RPM_Passado = T_Agora;  

    char cMsg[32];
    sprintf(cMsg, "RPM: %d", RPM);
    Log(cMsg);
  }
}

void Volt_Setup()
{
    Log("Configurando Voltimetro [A] & [B]");
    pinMode(Pin_Voltimetro_A, INPUT);
    sprintf(mxObj[11].Nome, "VoltA");
    mxObj[11].Pin_Entrada = Pin_Voltimetro_A;
    mxObj[11].Ativado = true;
    
    pinMode(Pin_Voltimetro_B, INPUT);
    sprintf(mxObj[12].Nome, "VoltB");
    mxObj[12].Pin_Entrada = Pin_Voltimetro_B;
    mxObj[12].Ativado = true;
}


void preLoad_objStruct()
{
  for(int i = 0; i < MAX_OBJ; i++)
  {
    mxObj[i].Ativado          = -1;
    sprintf(mxObj[i].Nome, "");
    mxObj[i].Tipo             = -1;
    mxObj[i].Index            = -1;
    mxObj[i].Pin_Entrada      = -1;
    mxObj[i].Pin_Saida        = -1;
    mxObj[i].Saida_Status     = -1;
    mxObj[i].delay_Acc        = 0;
    mxObj[i].momento_Acc      = 0;
  }
}

void Add_objStruct(bool Ativado, const char *tNome, int rTipo, int pEntrada, int pSaida, bool pStatus)
{
  total_obj++;
  int i = total_obj;
  char cOut[32];

  sprintf(cOut, "Add '%s' [%d]", tNome, total_obj);
  Log(cOut);
  
  mxObj[i].Ativado          = Ativado;
  sprintf(mxObj[i].Nome, "%s",  tNome);
  mxObj[i].Tipo             = rTipo;
  mxObj[i].Index            = total_obj;
  mxObj[i].Pin_Entrada      = pEntrada;
  mxObj[i].Pin_Saida        = pSaida;
  mxObj[i].Saida_Status     = pStatus;
  mxObj[i].delay_Acc        = 0;
  mxObj[i].momento_Acc      = 0;

  if(rTipo == 1) pinMode(pSaida, OUTPUT);
  if(rTipo == 2) pinMode(pSaida, OUTPUT);
  if(rTipo == 3) pinMode(pEntrada, INPUT);
  if(rTipo == 4) pinMode(pSaida, OUTPUT);
  if(pStatus != -1) digitalWrite(pSaida, pStatus);

  if(pSaida == Pin_T2)
  {
    ID_Bicos = i;
  }
  
  if(pSaida == Pin_T4) 
  {
    sprintf(cOut, "Delay bomba %dms", Tempo_Bomba_Press);
    Log(cOut);
    ID_BombaGas = i;
  }

  if(pEntrada == Pin_Sensor_HALL)
  {
    pinMode(Pin_Sensor_HALL, INPUT);
    attachInterrupt(digitalPinToInterrupt(Pin_Sensor_HALL), SensorHall_interrup, FALLING); // 
  }
}


void Loading()
{
  Serial.println(".");
  delay(10);
  Serial.print(".");
  delay(10);
  Serial.print(".");
  delay(10);
}
void setup() 
{
  Loading();
  Serial.begin(9600);
  
  Carrega_Valores_EEPROM();
  preLoad_objStruct();

  Add_objStruct(true, "R1", 1, -1, Pin_R1, HIGH);
  Add_objStruct(true, "R2", 1, -1, Pin_R2, HIGH);
  Add_objStruct(true, "R3", 1, -1, Pin_R3, HIGH);
  Add_objStruct(true, "R4", 1, -1, Pin_R4, HIGH);
  Add_objStruct(true, "R5", 1, -1, Pin_R5, HIGH);
  Add_objStruct(true, "R6", 1, -1, Pin_R6, HIGH);
  Add_objStruct(false, "R7", 1, -1, Pin_R7, HIGH);
  Add_objStruct(true, "R8", 1, -1, Pin_R8, HIGH);

  //Add_objStruct(true, "T1", 1, -1, Pin_T1, HIGH);
  
  Add_objStruct(true, "Bico Inj.", 1, -1, Pin_T2, LOW); // LOW = Bico OFF
  
  //Add_objStruct(true, "T3", 1, -1, Pin_T3, HIGH);
  Add_objStruct(true, "BombaGas", 1, -1, Pin_T4, HIGH); // LOW = Bomba OFF
  Add_objStruct(true, "SensorHal", 2, Pin_Sensor_HALL, -1, -1);
  //Add_objStruct(true, "T5", 1, -1, Pin_T5, HIGH);
  //Add_objStruct(true, "T6", 1, -1, Pin_T6, HIGH);
  //Add_objStruct(true, "T7", 1, -1, Pin_T7, HIGH);
  //Add_objStruct(true, "T8", 1, -1, Pin_T8, HIGH);
/*
    bool Ativado;
    char Nome[32];
    int Tipo;
    
    int Pin_Entrada;
    int Pin_Saida;
    int Saida_Status;
    
    unsigned long delay_Acc;
    unsigned long momento_Acc;
    bool Obj_Liberado;

#define Tipo_Terra    1
#define Tipo_Rele     2
#define Tipo_Sensor   3
#define Tipo_Atuador  4
*/

 
  //Carrega_Valores_EEPROM();


  //Volt_Setup();
  //Saida_Posi_Setup();
  //Bicos_Setup();
  //Bomba_Gas_Setup();
  //AML_Setup();
  //TPS_Setup();
  //TempAr_Setup();
  //TempAgua_Setup();
  //SensorHall_Setup();

}

void loop()
{
  //Setup_ValoresPadroes(); return;
  //sprintf(mxObj[0].Nome, "%d %d", LOW, HIGH);
  //Log(mxObj[1].Nome);

  //Bomba_Gas_Liberar();

  Bicos_Liberar();
  //Bomba_Gas_Liberar();
  //AML_Liberar();
  Tempo_Inject = 100;

     if(Serial.available() > 0)
     {
        char tmp[25];
        int Recv = Serial.read();

        if(Recv == 10) return;
        
        sprintf(tmp, "recv %d", Recv);
        Log(tmp);

       switch(Recv)
       {
        case 65:
          Log("Exec Bomba_Gas_Press()");
          Bomba_Gas_Press();
        break;

        case 66:
          Log("Exec Bicos_Injetar()");
          Bicos_Injetar();
        break;
       }
     }
}
