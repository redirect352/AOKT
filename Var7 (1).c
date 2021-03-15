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

//---------------------8 Задание --------------------------------


// Номер считываемого регистра - 6(начиная с 0), первый байт - Primary BN, 2 byte - Secondary BN, 3 byte - Subordinate BN
void outputBusData(long  regData) {
  fputs("           Задание 8\n", out);
     puts("           Задание 8");

  printf("\n Soderjimoe registra: %x \n", regData);
  
  int PrimaryBN = regData & 0xFF;
  int SecondaryBn = (regData >> 8) & 0xFF;
  int SubBN = (regData >> 16) & 0xFF;
  printf("\nНомер первичной шины: %d ", PrimaryBN);
  printf("\nНомер вторичной шины (номер моста): %d ", SecondaryBn);
  printf("\nМаксимальный номер подчиненной шины: %d \n", SubBN);
  fprintf(out,"\nНомер первичной шины: %d ", PrimaryBN);
  fprintf(out,"\nНомер вторичной шины (номер моста): %d ", SecondaryBn);
  fprintf(out,"\nМаксимальный номер подчиненной шины: %d \n", SubBN);
        
   



}

//---------------------6 и 7 Задание --------------------------------


void outputInterruptData(long regData){
     fputs("           Задание 6\n", out);
     puts("           Задание 6 ");

   int interruptLine = regData & 0xFF;

    fputs("Interrupt line: ", out);
    printf("Interrupt line: ");
    if (interruptLine == 0xFF) {
        fputs("unused/unknown input\n", out);
        printf("unused/unknown input\n");
    } else if (interruptLine < 16) {
        fprintf(out, "%s%d\n", "IRQ", interruptLine);
        printf("%s%d\n", "IRQ", interruptLine);
    } else {
        fputs("invalid line number\n", out);
        puts("invalid line number\n");
    }
     
     fputs("           Задание 7\n", out);
     puts("           Задание 7 ");


    int interruptPin = (regData >> 8) & 0xFF;
    char *interruptPinData;

    switch (interruptPin) {
        case 0:
            interruptPinData = "not used";
            break;
        case 1:
            interruptPinData = "INTA#";
            break;
        case 2:
            interruptPinData = "INTB#";
            break;
        case 3:
            interruptPinData = "INTC#";
            break;
        case 4:
            interruptPinData = "INTD#";
            break;
        case 5: 
            interruptPinData = "FFh(резерв)";
            break;
        
            
        default:
            interruptPinData = "invalid pin number";
            break;
    }

    fprintf(out, "Interrupt pin: %s\n", interruptPinData);
    printf( "Interrupt pin: %s\n", interruptPinData);


    
    
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
            outputBusData(readRegister(bus, device, function, 6));    
           
        } else {
            fprintf(out, "\nNot a bridge\n\n");
            printf("\nNot a bridge\n\n");
	    outputInterruptData(readRegister(bus, device, function, 15));

            
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
