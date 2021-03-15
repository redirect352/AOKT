#include <stdio.h>
#include <sys/io.h>

#include <errno.h>
#include <stdlib.h>

#include <string.h>
#include <stdbool.h>
#include <values.h>
#include "pci.h"

//-------------------------Конец INCLUDES------------------------

// Максимальное количество шин(256), устр подключенных к каждой шине и функция у кажд. устройства
#define MAX_BUS 256
#define MAX_DEVICE 32
#define MAX_FUNCTIONS 8

#define ID_REGISTER 0

//На сколько сдвинуть чтобы получить нужную инфу
#define DEVICEID_SHIFT 16
#define BUS_SHIFT 16
#define DEVICE_SHIFT 11
#define FUNCTION_SHIFT 8
#define REGISTER_SHIFT 2

// Адреса портов Управления и Инфы соответственно
#define CONTROL_PORT 0x0CF8
#define DATA_PORT 0x0CFC

//-------------------------Конец DEFINES------------------------




void PrintInfo(int bus, int device, int function);
bool IfBridge(int bus,int device, int function);
long readRegister(int bus, int device, int function, int reg);
void outputGeneralData(int bus, int device, int function, int regData);
char *getDeviceName(int vendorID, int deviceID);
char *getVendorName(int vendorID);
void outputBusData(long regData);
void outputInterruptData(long regData);


FILE *out;

//---------------------3 Задание --------------------------------


//Вывод информации базовых полей регистров памяти
void outputBARsData(int bus, int device, int function) {
     int flag = 1;
     fputs("           Задание 3\n", out);
     puts("           Задание 3");
     fputs("Базовые регистры ввода/вывода:\n", out);
     puts("Базовые регистры ввода/вывода:\n");
     int i;
     for (i = 0; i < 6; i++) {
        long regData = readRegister(bus, device, function, 4 + i);
        if (regData) {
            
            // Если 0 бит = 0 -это Регистр баз адреса памяти, если = 1 - это регистр портов 
            if ((regData & 1)) {
              fprintf(out, "\tРегистр ввода/вывода %d: ", i);
              printf("\tРегистр ввода/вывода %d: ", i);
 		
		 flag = 0;
                    
              outputIOMemorySpaceBARData(regData);
            }
        } 
    }
    
    if (flag)
    {
      printf("Отсутствую баз. регистры в\в");
      fprintf(out, "Отсутствую баз. регистры в\в");
    
    }
    
}

void outputIOMemorySpaceBARData(long regData) {
  unsigned long reg1Data=regData-1;
    fprintf(out, "%#lx, ", reg1Data); 
    fputs("I/O space register\n", out);
    printf("%#lx, ", reg1Data);
    printf("I/O space register\n");
}



//---------------------5 Задание --------------------------------
// 3 regester, 0 byte
void outputCacheLineData(long regdata)
{
       fputs("           Задание 5\n", out);
     puts("           Задание 5");

  
  int CacheLineSize = regdata && 0xFF;
  if (CacheLineSize>128)
    CacheLineSize = 0;
  printf("\nCashe Line Size data: %x", CacheLineSize);
  printf ("\nРазмер строки кэша: %d\n",CacheLineSize );
  fprintf(out,"\nCashe Line Size data: %x", CacheLineSize);
  fprintf (out,"\nРазмер строки кэша: %d\n",CacheLineSize );
  
}


//---------------------10 Задание --------------------------------
//  8 regester, 2-3 byte - Mem Limit, 0-1 byte - Mem Base
void outputMemoryBaseData(long regdata)
{
       fputs("           Задание 10\n", out);
     puts("           Задание 10");

  printf("\nregister 8  data: %x\n",regdata);
  
  long   MemBase = regdata & 0xFFFF;
  long  MemLim  = (regdata >> 16) & 0xFFFF;
  printf("Задают начальный и конечный адрес пам, на кот. отражены регистры ввода-вывода устройств за мостом");
  printf("\nMemory base data: %x",MemBase);
  printf ("\nMemory limit data: %x \n",MemLim );
  fprintf(out,"\nMemory base data: %x",MemBase);
  fprintf (out,"\nMemory limit data: %x \n",MemLim );
  
}




//=======================GENERAL=======================================================================

//Получаем производителя
char *getVendorName(int vendorID) {
int i;
    for (i = 0; i < PCI_VENTABLE_LEN; i++) {
        if (PciVenTable[i].VendorId == vendorID) {
            return PciVenTable[i].VendorName;
        }
    }
    return NULL;
}

// Получаем имя устройства из списка
char *getDeviceName(int vendorID, int deviceID) {
int i;
    for ( i = 0; i < PCI_DEVTABLE_LEN; i++) {
        if (PciDevTable[i].VendorId == vendorID && PciDevTable[i].DeviceId == deviceID) {
            return PciDevTable[i].DeviceName;
        }
    }
    return NULL;
}

void outputVendorData(int vendorID)
{
    char *vendorName = getVendorName(vendorID);
    fprintf(out, "Vendor ID: %04d, %s\n", vendorID, vendorName ? vendorName : "unknown vendor");
   printf( "Vendor ID: %04d, %s\n", vendorID, vendorName ? vendorName : "Unknown vendor");
}

void outputDeviceData(int vendorID, int deviceID)
{
    char *deviceName = getDeviceName(vendorID, deviceID);
    fprintf(out, "Девайс ИД Device ID: %04d, %s\n", deviceID, deviceName ? deviceName : "unknown device");
	printf( "Device ID: %04d, %s\n", deviceID, deviceName ? deviceName : "Unknown device");
}


void outputGeneralData(int bus, int device, int function, int regData){
//Выводим номер шины, устр и функции, Id and VendorId
    fprintf(out, "%x:%x:%x\n", bus, device, function);
    printf( "%x:%x:%x\n", bus, device, function);
    int deviceID = regData >> DEVICEID_SHIFT;
    int vendorID = regData & 0xFFFF;
    outputVendorData(vendorID);
    outputDeviceData(vendorID, deviceID);
}


//Чтение нужного регистраШ(reg) функции function девайса  device шина bus
long readRegister(int bus, int device, int function, int reg) {
   // C помощью сдвигов формирум адрес нужного регистра
   long  configRegAddress = (1 << 31) | (bus << BUS_SHIFT) | (device << DEVICE_SHIFT) |(function << FUNCTION_SHIFT) | (reg << REGISTER_SHIFT);
    // загружаем получ. адрес в регистр управления
    outl(configRegAddress, CONTROL_PORT);
    // Читаем значение из реггистра данных
    return inl(DATA_PORT);

	return 0;
}

// Проверка, является ли это устройство мостом
bool IfBridge(int bus,int device, int function){
 //Читаем 3 регистр из простр. конфигураций
 long htypeRegData = readRegister(bus, device, function, 3);
//Если -0 бит HEader Type  - 1  это мост, возвращаем этот бит
    return ((htypeRegData >> 16) & 0xFF) & 1;
}


// Функция выводит информацию о заданной функции, заданного уст и моста
void PrintInfo(int bus, int device, int function) {
   long idRegData = readRegister(bus, device, function, ID_REGISTER);

// Если записано это значение, значит нет устройства 
   if (idRegData != 0xFFFFFFFF) { 
        outputGeneralData(bus, device, function, idRegData);

// Если это мост, выводим необх. информация, в ином случае выводим инфу о баз регистрах 
   if (IfBridge(bus, device, function)) {
            fprintf(out, "\nIs bridge\n\n");
	    printf("\nIs bridge\n\n");
            outputMemoryBaseData(readRegister(bus, device, function, 8));
        } else {
            fprintf(out, "\nNot a bridge\n\n");
            printf("\nNot a bridge\n\n");            
  	    outputBARsData( bus,  device, function);
   	    outputCacheLineData(readRegister(bus, device, function, 3));
           	       
        }
        fputs("---------------------------------------------------\n", out);
	   puts("---------------------------------------------------\n");
    }
}


int main() { 



// Изменение уровня привилегий программы	
    if (iopl(3)) {   
       printf("I/O Privilege level change error: %s\nTry running under ROOT user\n", strerror(errno));

       return 2;
    }

	
   int buses; 
   int device;
   int function;
   // Файл вывода 
   out = fopen("output.txt", "w");

// Циклы проходящие по всех шинам, устр-вам и их функциям
   for ( buses = 0; buses < MAX_BUS; buses++){
        for (device = 0; device < MAX_DEVICE; device++){
            for ( function = 0; function < MAX_FUNCTIONS; function++){
                PrintInfo(buses,device,function);
			}
		}
	}


    fclose(out);
    return 0;
}
