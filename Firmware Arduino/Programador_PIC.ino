#define dato_ 2
#define reloj_ 7 //gris
#define vpp_ 4    //Naranjado pero manda la señal el 5(Verde) y salen los 12 amarillo
#define led 13


// define comandos de programacion
const  char     load_conf=0b000000;
const  char     load_prog=0b000010;
const  char read_dat_prog=0b000100;
const  char      inc_dir=0b000110;
const char begin_ere_prog=0b001000;
const  char    begin_prog=0b011000;
const  char  load_dat_dat=0b000011;
const  char  read_dat_dat=0b000101;
const  char bulk_ere_prog=0b001001;
const  char  bulk_ere_dat=0b001011;
const  char      chip_ere=0b111111;
const  char      begin_ere_prog_only=0b110000;

#include <SoftwareSerial.h>

const byte rxPin = 11;
const byte txPin = 12;

SoftwareSerial soft_serial (rxPin, txPin);

//Variables recepcion serial
char trama[200];
char info[50];
unsigned  char dato_rx;
char puntero=0;

//maquina de estado

enum estado_ {nada,programando,lineas,fin,leyendo,verifica,borrando};
char estado=nada;
char line_estado=0;

unsigned int direccion;
unsigned char cuenta;

unsigned int dato_leido,dir_prog,dato_leido2,direccion_rec1,config_temp,config_temp1;

unsigned int dato_conf[2];
unsigned int lee_conf[2];
String pic; 
unsigned int mascara[3];
char modo_mascara[2];
char prog_8=0;


//usada para pasar de string hex a decimal
char *endptr;

void delay_ms(unsigned long ret)
{
 delay(ret*64); 
 
 }

void delay_us(unsigned long ret)
{
 delayMicroseconds(ret);  
 }


void vpp(char pon)
{
   if (pon==0) digitalWrite(vpp_,1); 
  else digitalWrite(vpp_,0); 
}

void dato(char pon)
{
 digitalWrite(dato_,pon); 
 }

void reloj(char pon)
{
 digitalWrite(reloj_,pon); 
 }

void resetpic()
{
reloj(0);
dato(0);
vpp(0);
delay_ms(1);
  }

void inicio()
{
resetpic;
vpp(1); 
 }  

void write_bit(char bit_)
{
  reloj(1);
  delay_us(1);
  if  (bit_==0) dato(0);
  else dato(1);
  reloj(0);
  delay_us(1);
 }

 char read_bit()
{
 char bit_=0; 
   reloj(1);
 delay_us(1);
 if(digitalRead(dato_)==HIGH) bit_=1;
  else bit_=0; 
  reloj(0);
  delay_us(1);
  return (bit_);
}


//para variables
char lee_un_bit (unsigned int variable, char bit1)
{
    
    if ((variable & (1 << bit1))>0) return (1);
    else return (0);
}

void manda_comando(char coma)
{
  char cont;
  for (cont=0;cont<=5;cont++)
  {
        if (lee_un_bit(coma,cont)==0)     write_bit(0);
    else write_bit(1); 
   }
  
}

void manda_dato(unsigned int dat)
{
  signed char cont;
   write_bit(0); //necesario
  for (cont=0;cont<=13;cont++)
  {
    if (lee_un_bit(dat,cont)==0)     write_bit(0); 
    else write_bit(1); 
   }
    write_bit(0); //necesario para 16 bits
    
}

void detecta_pic()
{
  
                     lee_id1();
                      mascara[0]=0x3fff;
                      mascara[1]=0x700;
                      modo_mascara[0]=1;
                      modo_mascara[1]=0;
                       if ((dato_leido==8192)|(dato_leido==8320)|(dato_leido==8288)|(dato_leido==8224)|(dato_leido==8256)) 
                       { 
                        modo_mascara[0]=1;
                        modo_mascara[1]=1;
                        mascara[1]=0x700;// pic="16f882";
                        prog_8=1;
                       }

                       if ((dato_leido==3616)|(dato_leido==3584)|(dato_leido==3680)|(dato_leido==3648)) 
                       {
                        modo_mascara[0]=1;
                        modo_mascara[1]=1;
                        mascara[0]=0x2FCF;//mascara 16f87xa
                        prog_8=1;
                       }
                        direccion=0;
  }


unsigned int lee_dato()
{
  char cont;
  unsigned int dato_read=0;
    manda_comando(read_dat_prog);// manda comando de lectura
         pinMode(dato_, INPUT);
         delay_us(3);
         read_bit();// lee bit 16
         //lee los bit validos
        for (cont=0;cont<=13;cont++)
        {
        dato_read= dato_read | (read_bit() <<cont);
       }// fin cont=13
      read_bit();// lee el ultimo
       pinMode(dato_, OUTPUT);
       delay_us(3);
       return (dato_read);
   }

unsigned int lee_dato8(unsigned int dir)
{
  String trama_tx;
  char conco;
  unsigned int datos_[8];

  while (direccion!=dir)
    {
      manda_comando(inc_dir);
      direccion++;
      } 

      
    for (conco=0;conco<7;conco++)
    {
     // inicio();
      datos_[conco]=lee_dato();
      manda_comando(inc_dir);
       direccion++;      
       }
   
  datos_[conco]=lee_dato();
    
    
         for (conco=0;conco<8;conco++)
          {
          trama_tx=trama_tx+String(datos_[conco],DEC)+"    ";
          }
           // soft_serial.println("lei esta vaina");
         Serial.println(trama_tx);
        //delay_ms(10);  
          
   
   }


 

void programa_dato(unsigned int dato)
{
   // inicio();
   manda_comando(load_prog);
   manda_dato(dato);
   manda_comando(begin_ere_prog);
   delay_ms(12);
   dato_leido=lee_dato();
   manda_comando(inc_dir);
  direccion++;
 }

void programa_dato8(unsigned int dato)
{
   manda_comando(load_prog);
   manda_dato(dato);
   manda_comando(inc_dir);
  direccion++;
 }

void des_protec1()
{
   manda_comando(load_conf);
   manda_dato(0x3ff0); 
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(1);
   manda_comando(7);
    manda_comando(8);
   delay_ms(12);
   manda_comando(1);
   manda_comando(7);
  }


void des_protec2()
{
   manda_comando(load_conf);
   manda_dato(0x2000); 
   manda_comando(chip_ere);
   
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(9);
   delay_ms(12);
   }

void  borra_todo()
{
  
  
  }


void  lee_id1()
{
  unsigned int dati;
   manda_comando(load_conf);
   manda_dato(0x3f8); 
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   dato_leido=lee_dato() & (0x3FE0);
   }


   void prog_config1()
   {
   unsigned int dati;
    manda_comando(load_conf);
 //   delay_us(10);
   manda_dato(dato_conf[0]);
   //manda_dato(0x3f8 | 0xf); 
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   
   manda_comando(begin_ere_prog);
   delay_ms(12);
     dato_leido=lee_dato()  & mascara[0] ;
   
        
    }

void prog_config12(unsigned int dato_confu)
   {
   unsigned int dati;
 
    manda_comando(load_conf);
   manda_dato(dato_conf[0]);
  // manda_comando(bulk_ere_prog);
  // delay_ms(20);

    
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   manda_comando(inc_dir);
   
   manda_comando(load_prog);
   manda_dato(dato_conf[0]);      
    manda_comando(begin_ere_prog);
   delay_ms(12);
   if (modo_mascara[0]=1)   lee_conf[0]=lee_dato() & mascara[0] ;
   else lee_conf[0]=lee_dato() ;
   
   manda_comando(inc_dir);
  
   manda_comando(load_prog);
   manda_dato(dato_conf[1]);      
    manda_comando(begin_ere_prog);
   delay_ms(12);
  
  if (modo_mascara[1]==1)   lee_conf[1]=lee_dato() & mascara[1] ;
   else 
   {
    lee_conf[1]=lee_dato()  ;
    config_temp=lee_dato() & 0x3703;
    config_temp1=lee_dato() & 0x3713;
    
   
    
   }
   
    }

   


 void linea()
 {
  unsigned int direccion_rec,num_datos;
  unsigned int dat_prog;
  
                               // soft_serial.println(trama);
                                 strcpy(info, strtok(trama, ":"));  //inicia captura de tokens                                  
                                 strcpy(info, strtok(0, ":"));  //Captura #datos                             
                                 num_datos=atoi(info)-1;
                                 strcpy(info, strtok(0, ":"));  //Captura dirrecion en decimal                                 
                                 direccion_rec= strtol(info, &endptr, 16);
                             
                             // soft_serial.print("dir=");
                            //  soft_serial.println(direccion_rec);
                              //soft_serial.print("d=");                                
                                //soft_serial.println(num_datos);

                              // Serial.println(direccion_rec);
                               // Serial.println(num_datos);
                               // Serial.println("");
                                
                                   if (direccion_rec>=0x2007) 
                                   { 

                                    
                                             for (cuenta=0; cuenta<num_datos;cuenta++)
                                           {
                                          strcpy(info, strtok(0, "\t"));  //Captura dirrecion en decimal                                 
                                          dato_conf[cuenta]=strtol(info, &endptr, 16);// captura el dato a programar
                                          
                                           }//for
                                           
                                          if (num_datos==1)
                                          {
                                             prog_config1(); 
                                                    if (dato_leido!=dato_conf[0]) 
                                                   {
                                                    Serial.println("Error en la  Configuracion_");
                                                    estado=nada;
                                                    return;
                                                    }//si el igual dato es recibido
                                            
                                            }

                                      else
                                       {
                                        //Serial.println("Pendejo"); 
                                       // estado=nada;
                                        //return;                                      
                                       // Serial.println(lee_conf[0]); 
                                       //Serial.println(lee_conf[1]);
                                                   /*   prog_config11(dato_conf[0]); 
                                                    if (dato_leido!=dato_conf[0]) 
                                                   {
                                                    Serial.println("Error en la  Configuracion1");
                                                    estado=nada;
                                                    return;
                                                    }//si el igual dato es recibido*/
                                                 
                                                 prog_config12(dato_conf[0]); 
                                                 
                                                 
                                                 if (lee_conf[0]!=dato_conf[0]) 
                                                   {
                                                    Serial.println("Error en la  Configuracion1");
                                                    estado=nada;
                                                    return;
                                                    }//si el igual dato es recibido

                                                   if ((lee_conf[1]!=dato_conf[1]) && (config_temp!=dato_conf[1])&& (config_temp1!=dato_conf[1]))
                                                   {
                                                    Serial.println("Error en la  Configuracion2");
                                                    estado=nada;
                                                    return;
                                                    }//si el igual dato es recibido
                                               


                                                                                     
                                        }//sino tiene mas de una plabra de configuracion
                                                                         
                                                                           
                                    estado=fin;
                                    return;
                                   
                                  }// fin de direccion de configuracion


                                   
                                
                                    while (direccion!=direccion_rec)
                                        {
                                          manda_comando(inc_dir);
                                          direccion++;
                                         } 
                             
                                //  DESPUES DE ESO SI DAR COMANDO DE PROGRAMAR

                               // captura todos los datos a programar, debe incrementar la dirreccion
                                  for (cuenta=0; cuenta<num_datos;cuenta++)
                                   {
                                  strcpy(info, strtok(0, "\t"));  //Captura dirrecion en decimal                                 
                                  dat_prog=strtol(info, &endptr, 16);// captura el dato a programar
                                   //soft_serial.println(dat_prog); 
                                   programa_dato(dat_prog); 
                                   if (dato_leido!=dat_prog) 
                                   {
                                    Serial.println("ERROR EN LA PROGRAMACION");
                                    estado=nada;
                                    return;
                                    }//si el dato es recibido
                                                                 
                                    }//for
                         
  
                                 }//procedimeito 



void linea8()
 {
  unsigned int direccion_rec,num_datos;
  unsigned int dat_prog;
  
                               // soft_serial.println(trama);
                                 strcpy(info, strtok(trama, ":"));  //inicia captura de tokens                                  
                                 strcpy(info, strtok(0, ":"));  //Captura #datos                             
                                 num_datos=atoi(info)-1;
                                 strcpy(info, strtok(0, ":"));  //Captura dirrecion en decimal                                 
                                 direccion_rec= strtol(info, &endptr, 16);
                             
                             // soft_serial.print("dir=");
                            //  soft_serial.println(direccion_rec);
                              //soft_serial.print("d=");                                
                                //soft_serial.println(num_datos);

                              // Serial.println(direccion_rec);
                               // Serial.println(num_datos);
                               // Serial.println("");
                                
                                   if (direccion_rec>=0x2007) 
                                   { 

                                    
                                             for (cuenta=0; cuenta<num_datos;cuenta++)
                                           {
                                          strcpy(info, strtok(0, "\t"));  //Captura dirrecion en decimal                                 
                                          dato_conf[cuenta]=strtol(info, &endptr, 16);// captura el dato a programar
                                          
                                           }//for
                                           
                                          if (num_datos==1)
                                          {
                                             prog_config1(); 
                                                    if (dato_leido!=dato_conf[0]) 
                                                   {
                                                    Serial.println("Error en la  Configuracion_");
                                                    estado=nada;
                                                    return;
                                                    }//si el igual dato es recibido
                                            
                                            }

                                      else
                                       {
                                        //Serial.println("Pendejo"); 
                                       // estado=nada;
                                        //return;                                      
                                       // Serial.println(lee_conf[0]); 
                                       //Serial.println(lee_conf[1]);
                                                   /*   prog_config11(dato_conf[0]); 
                                                    if (dato_leido!=dato_conf[0]) 
                                                   {
                                                    Serial.println("Error en la  Configuracion1");
                                                    estado=nada;
                                                    return;
                                                    }//si el igual dato es recibido*/
                                                 
                                                 prog_config12(dato_conf[0]); 
                                                 
                                                 
                                                 if (lee_conf[0]!=dato_conf[0]) 
                                                   {
                                                    Serial.println("Error en la  Configuracion1");
                                                    estado=nada;
                                                    return;
                                                    }//si el igual dato es recibido

                                                   if (lee_conf[1]!=dato_conf[1]) 
                                                   {
                                                    Serial.println("Error en la  Configuracion2");
                                                    estado=nada;
                                                    return;
                                                    }//si el igual dato es recibido
                                               


                                                                                     
                                        }//sino tiene mas de una plabra de configuracion
                                                                         
                                     resetpic();
                                     inicio();
                                    direccion=0;
                                    Serial.println("veri");
                                    estado=verifica;                                       
                                    
                                    return;
                                   
                                  }// fin de direccion de configuracion


                                   
                                
                                    while (direccion!=direccion_rec)
                                        {
                                          manda_comando(inc_dir);
                                          direccion++;
                                         } 
                             
                                //  DESPUES DE ESO SI DAR COMANDO DE PROGRAMAR

                               // captura todos los datos a programar, debe incrementar la dirreccion
                                  for (cuenta=0; cuenta<num_datos-1;cuenta++)
                                   {
                                  strcpy(info, strtok(0, "\t"));  //Captura dirrecion en decimal                                 
                                  dat_prog=strtol(info, &endptr, 16);// captura el dato a programar
                                   //soft_serial.println(dat_prog); 
                                   programa_dato8(dat_prog);
                                                               
                                    }//for
                                     //Carga el último dato sin incrementar la direccion
                                  strcpy(info, strtok(0, "\t"));  //Captura dirrecion en decimal                                 
                                  dat_prog=strtol(info, &endptr, 16);// captura el dato a programar
                                  manda_comando(load_prog);
                                   manda_dato(dat_prog);
  
                                       //programa los ocho datos
                                       manda_comando(begin_ere_prog);
                                       delay_ms(20);
             
  
                                 }//procedimeito 


 
void setup()
 { // put your setup code here, to run once:
 
 // pines PWM1
 pinMode(5,OUTPUT);
 pinMode(vpp_,OUTPUT);
 pinMode(reloj_,OUTPUT);
 pinMode(dato_,OUTPUT);
 pinMode(led,OUTPUT);
 pinMode(8,OUTPUT);

 TCCR0B = TCCR0B & B11111000 | B00000001; // coloca la frecuencia del pwm en 600


  vpp(0);// quita los 12 voltios de vpp poniendolo en cero
 analogWrite(5,127); //crea una onda con cilco util de 50%
 digitalWrite(led,LOW);

 Serial.begin(115200);
 soft_serial.begin(115200);
// delay_ms(5000);
 soft_serial.println("Iniciando");
 //delay_ms(5000);
 resetpic();
 inicio();
                                 

 }




void loop()
 { // put your main code here, to run repeatedly:

   
   while (Serial.available()==0);
   Serial.readBytesUntil('*',trama,200);

    switch(estado)
    {

          case nada:
                    
                  if (strstr(trama,"prog" )) 
                  {
                    prog_8=0;
                    inicio();
                    detecta_pic();
                    if (dato_leido==1376) des_protec1();//  
                     //des_protec2();// 
                    manda_comando(bulk_ere_prog);
                    delay(20);
                    manda_comando(load_conf);
                    manda_dato(0x3fff);
                    manda_comando(bulk_ere_prog);
                    delay(20);
                    manda_comando(bulk_ere_dat);
                     delay(20);
                    resetpic();
                    estado=programando; 
                  
                      }

                    else if (strstr(trama,"mira" ))
                    {
                             resetpic();
                             inicio();
                             direccion=0;
                             estado=verifica;
                      
                      }
                   
                       else if (strstr(trama,"vpp0" )) vpp(0);
                     else if (strstr(trama,"vpp1" )
                     ) vpp(1);
                     else if (strstr(trama,"leeid" )) 
                     {
                  //  Serial.println("llegoid");
                  resetpic();
                 inicio();
                 des_protec1();//  
                   des_protec2();// 
                   manda_comando(bulk_ere_prog);
                    manda_comando(load_conf);
                    manda_dato(0x3fff);
                    manda_comando(bulk_ere_prog);
                    manda_comando(bulk_ere_dat);
                    resetpic();
                    inicio();
                   lee_id1();switch (dato_leido)
                     {
                      case  1376: pic="16f84A";
                               break;
                      case 3616: pic="16f877A";
                                break;
                      case 3584: pic="16f876A";
                       break;
                       
                       case 3680: pic="16f874A";
                       break;
                      
                       case 3648: pic="16f873A";
                       break; 
                         
                        case 8192 : pic="16f882";
                        break;  

                       case 8224: pic="16f883";
                       break; 

                       case 8256: pic="16f884";
                       break; 

                      case 8288: pic="16f886";
                       break; 
                       
                      case 8320: pic="16f887";
                       break; 

                       case 9984: pic="12f1822";
                       break; 

                      case 10048: pic="16f1824";
                       break;

                        case 10080: pic="16f1825";
                       break;

                        case 10112: pic="16f1825";
                       break;

                       

                       default:
                               pic="Informa a Alcides";
                       break; 
                                                  
                      
                      
                      }
                    Serial.println(dato_leido);
                    Serial.println(pic);
                   
                  resetpic();
                     
                      }
                       
                     break;
        
        case programando:
                     resetpic();
                        inicio();
                        direccion=0;
                        estado=lineas; 
                        
                       // break;

        case lineas:
                 if (strstr(trama,"line"))
                 {
                  if (prog_8==0) linea(); else linea8();
                  
                  }
                 break;

          case verifica:

                  if (strstr(trama,"veri" ))
                    {
                                 strcpy(info, strtok(trama, ":"));  //inicia captura de tokens                                  
                                 strcpy(info, strtok(0, ":"));  //Captura direccion                            
                                 direccion_rec1=atoi(info);
                               
                        //   soft_serial.println(direccion_rec1);
                      lee_dato8(direccion_rec1);
                      
                      }

                      else if (strstr(trama,"fin")) 
                  {       
                          Serial.println("PROGRAMACION EXITOSA");
                           estado=nada; 
                            break;
                       }
               
              else if (strstr(trama,"error")) 
                  {       
                          //Serial.println("PROGRAMACION EXITOSA");
                           estado=nada; 
                            break;
                       }
               
                          
                          break;       

           case fin:
          
                if (strstr(trama,"fin")) 
                  {       
                          Serial.println("PROGRAMACION EXITOSA");
                           estado=nada; 
                        
                       }
               
                    break;     
      
      
    }// fin switch

   
 }  
