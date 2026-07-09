//
//  BluetoothSender.mm
//  SendingBluetoothSignalToESP32 - App
//
//  Created by Luke Amos on 29/06/2026.
//

//File where the objective C Scripting takes place

#include <string>
#import <CoreBluetooth/CoreBluetooth.h>
#include "BluetoothSender.h"


//Delegate that recieves Corebluetooth callbacks
@interface BLEDelegate : NSObject <CBCentralManagerDelegate , CBPeripheralDelegate>
@property (strong, nonatomic) CBCentralManager* manager;
@property (strong , nonatomic) CBPeripheral* targetPeripheral;
@property (strong , nonatomic) CBCharacteristic* synthCharacteristic;
@property (strong , nonatomic) CBCharacteristic* filterCharacteristic;
@property (strong , nonatomic) CBCharacteristic* reverbCharacteristic;
@property (strong , nonatomic) CBCharacteristic* distortionCharacteristic;
@end

@implementation BLEDelegate

//Bluetooth powered on ?

//Callback 1 power bluetooth on
-(void)centralManagerDidUpdateState:(CBCentralManager*)central{
    
    if(central.state == CBManagerStatePoweredOn){
        NSLog(@"Bluetooth Powered ON");
        CBUUID* serviceUUID = [CBUUID UUIDWithString:@"1f64540c-e392-4b1c-8561-a26baf3945d2"];
        [central scanForPeripheralsWithServices:@[serviceUUID] options:nil]; // scan for peripherals with ther service ID as stated above with nil options
        
    }
    else
        NSLog(@"Bluetooth not available state = %ld" , (long)central.state);
    
}

// --- callback 2: device discovered (the new block goes HERE) ---
- (void)centralManager:(CBCentralManager*)central
 didDiscoverPeripheral:(CBPeripheral*)peripheral
     advertisementData:(NSDictionary*)advertisementData
                  RSSI:(NSNumber*)RSSI
{
    if(self.targetPeripheral == nil){ // if not pointing to anything yet
        
        NSLog(@"Found device, connecting ...");
        self.targetPeripheral = peripheral; //retain it
        peripheral.delegate = self; //handle its call backs
        [central stopScan];
        [central connectPeripheral:peripheral options:nil];
    }
}


//Callback 3 connect to the device

-(void)centralManager:(CBCentralManager*)central
 didConnectPeripheral:(CBPeripheral *)peripheral
{
    NSLog(@"Connected Discovering services ");
    [peripheral discoverServices:nil]; //send discover services message to the peripheral object with nil argument
    
}

//Callback 4 call back the service
-(void) peripheral:(CBPeripheral*)peripheral
didDiscoverServices:( NSError *)error{
    
    if(error != nil){
        
        NSLog(@"Service discovery error: %@", error);
        return;
    }
    
    for(CBService* service in peripheral.services){
        
        NSLog(@"Found Service : %@" , service.UUID);
        
        if([service.UUID isEqual:[CBUUID UUIDWithString:@"1f64540c-e392-4b1c-8561-a26baf3945d2"]]){
            
            NSLog(@"Our service! Discovering characteristics ... ");
            [peripheral discoverCharacteristics:nil forService:service];
        }
        
    }
    
}

//CallBack 5 retrieve the characteristics
-(void) peripheral:(CBPeripheral*)peripheral
didDiscoverCharacteristicsForService:( CBService *)service error:( NSError *)error{
    
    if(error != nil){
        
        NSLog(@"Charcateristic discovery error : %@" , error);
        return;
    }
   

    
    for(CBCharacteristic * characteristic in service.characteristics){
        
        NSLog(@"Found Characteristic: %@" , characteristic.UUID);
        
        if([characteristic.UUID isEqual:[CBUUID UUIDWithString:@"1baa65f0-bc68-4e6a-99a1-b0145f960382"]]){
            
            NSLog(@"Got characteristic ready to write");
            self.synthCharacteristic = characteristic;
            
        }else if([characteristic.UUID isEqual:[CBUUID UUIDWithString:@"afda78fa-ec12-4bcb-8fca-791e43416598"]]){
            
            NSLog(@"Got characteristic ready to write");
            self.filterCharacteristic = characteristic;
            
        }else if([characteristic.UUID isEqual:[CBUUID UUIDWithString:@"b5ecef56-3dad-49c9-a7de-84f29933cf9b"]]){
            
            NSLog(@"Got characteristic ready to write");
            self.reverbCharacteristic = characteristic;
            
        }else if([characteristic.UUID isEqual:[CBUUID UUIDWithString:@"e1d5977d-8227-4d6d-94de-ba28562ac507"]]){
            
            NSLog(@"Got characteristic ready to write");
            self.distortionCharacteristic = characteristic;
            
        }
        
        
    }
}
@end

//C++ constructor creates the OBJ C delegate + manager

BluetoothSender::BluetoothSender(){
    
    BLEDelegate* delegate = [[BLEDelegate alloc] init];
    delegate.manager = [[CBCentralManager alloc] initWithDelegate:delegate queue:nil];
    
    impl = (void*)CFBridgingRetain(delegate); //Hand OwnerShip to C++
    
}

BluetoothSender::~BluetoothSender(){
    
    if(impl != nullptr)
        CFBridgingRelease(impl); //Release objC object 
    
}

//A send function that based on the paramter chosen takes the UUID and paramterID + the data and packages it so that it is parsable on the other side 
void BluetoothSender::sendValue(float value, const std::string paramID, const std::string uuid){
    
    if (impl == nullptr) return;
    
    BLEDelegate* delegate = (__bridge BLEDelegate*)impl ;
    
    //Take the value input and wrap the data with the paramID
    NSString* str = [NSString stringWithFormat:@"%s:%.2f", paramID.c_str(), value];
    NSData* data = [str dataUsingEncoding:NSUTF8StringEncoding];
    
    //Target UUID to be used to compare against the Characteristic UUIDS
    CBUUID* targetUUID = [CBUUID UUIDWithString:[NSString stringWithUTF8String:uuid.c_str()]];
    //Match the UUID to the correct characteristic , check that teh characteristic is not nil , then write the string based on teh param ID and the VAlye and use the delegate to write the data
    
    
    if([targetUUID isEqual:delegate.synthCharacteristic.UUID]){
        //Synth
        [delegate.targetPeripheral writeValue:data
            forCharacteristic:delegate.synthCharacteristic
                         type:CBCharacteristicWriteWithoutResponse];

    }else if([targetUUID isEqual:delegate.filterCharacteristic.UUID]){
        //Filter
        [delegate.targetPeripheral writeValue:data
            forCharacteristic:delegate.filterCharacteristic
                         type:CBCharacteristicWriteWithoutResponse];

    }else if([targetUUID isEqual:delegate.reverbCharacteristic.UUID]){
        //Reverb
        [delegate.targetPeripheral writeValue:data
            forCharacteristic:delegate.reverbCharacteristic
                         type:CBCharacteristicWriteWithoutResponse];

    }else if([targetUUID isEqual:delegate.distortionCharacteristic.UUID]){
        //Distortion
        [delegate.targetPeripheral writeValue:data
            forCharacteristic:delegate.distortionCharacteristic
                         type:CBCharacteristicWriteWithoutResponse];

    }else {
        DBG("Unavailable");
        return;
    }

}
