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
void outputIOMemorySpaceBARData(long regData);
void outputMemorySpaceBARData(long regData);
void outputBARsData(int bus, int device, int function);
void outputIOLimitBaseData(long regData);
void outputInterruptData(long regData);


FILE *out;

//---------------------12 Задание --------------------------------

void outputInterruptData(long regData){
     fputs("           TASK 12\n", out);
     puts("           TASK 12");


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
        default:
            interruptPinData = "invalid pin number";
            break;
    }

    fprintf(out, "Interrupt pin: %s\n", interruptPinData);
    printf( "Interrupt pin: %s\n", interruptPinData);




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




}




//---------------------9 Задание --------------------------------

void outputIOLimitBaseData(long  regData) {
 unsigned reg1Data=regData;
unsigned reg2Data=reg1Data >> 8;
     fputs("           TASK 9\n", out);
     puts("           TASK 9");
    long  IOBase = reg1Data & 0xFF;
    long  IOLimit = (reg2Data) & 0xFF;
 
if (IOBase==0){
    fprintf(out, "I/O Base:  0x00\n");
    printf( "I/O Base: 0x00\n");
} else {
 fprintf(out, "I/O Base:  %#lx\n", IOBase);
 printf( "I/O Base: %#lx\n", IOBase);
}

if (IOLimit==0){

    fprintf(out, "I/O Limit: 0x00\n");
    printf( "I/O Limit: 0x00\n");
}   else{
   
    fprintf(out, "I/O Limit: %#lx\n", IOLimit);
    printf( "I/O Limit: %#lx\n", IOLimit);
   }
}
//---------------------2 Задание --------------------------------

//Вывод информации базовых полей регистров памяти
void outputBARsData(int bus, int device, int function) {
     fputs("           TASK 2\n", out);
     puts("           TASK 2");
     fputs("Base Address Registers:\n", out);
     puts("Base Address Registers:\n");
     int i;
    for (i = 0; i < 6; i++) {
        fprintf(out, "\tRegister %d: ", i);
        printf("\tRegister %d: ", i);
        long regData = readRegister(bus, device, function, 4 + i);
        if (regData) {
            // Если 0 бит = 0 -это Регистр баз адреса памяти, если = 1 - это регистр портов 
            if (regData & 1) {
                outputIOMemorySpaceBARData(regData);
            } else {
                outputMemorySpaceBARData(regData);
            }
        } else {
            
            fprintf(out, "0x0000, ");
            printf("0x0000, ");
            fputs("unused register\n", out);
            puts("unused register");
        }
    }
}

void outputMemorySpaceBARData(long regData) {
unsigned long reg1Data=regData >> 4;
    fprintf(out, "%#lx, ", reg1Data );
    printf("%#lx, ", reg1Data ); 
    fputs("memory space register", out);
    printf("memory space register");
 int typeBits = (regData >> 1) & 3;

    switch (typeBits) {
        case 0:
            fprintf(out," (Память может быть отраженаany position in 32 bit address space)\n");
            printf(" (any position in 32 bit address space)\n");
            break;
        case 1:
            fprintf(out,"(below 1MB)\n");
            printf("(below 1MB)\n");
            break;
        case 2:
            fprintf(out,"(any position in 64 bit address space)\n");
            printf("(any position in 64 bit address space)\n");
            break;
        default:
            fprintf( out,"(reserved)\n");
            printf("(reserved)\n");
            break;
    }
}

void outputIOMemorySpaceBARData(long regData) {
  unsigned long reg1Data=regData-1;
    fprintf(out, "%#lx, ", reg1Data); 
    fputs("I/O space register\n", out);
    printf("%#lx, ", reg1Data);
    printf("I/O space register\n");
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
                outputIOLimitBaseData(readRegister(bus, device, function, 7));
                outputInterruptData(readRegister(bus, device, function, 15));

           
        } else {
            fprintf(out, "\nNot a bridge\n\n");
            printf("\nNot a bridge\n\n");
	    outputBARsData(bus, device, function);

            
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
