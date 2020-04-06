# Samples: OTABlinky

The samples in this folder demonstrate the process of creating an OTA deployment. The lab is seperated into 2 parts:
  1. Setup Products, DeviceGroups and Deployments to deliver application image to the devices
  2. Update new application image to the deployment


## Samples

 * [Blinky2](Blinky2/) - Green LED blinking, and change the blink rate by pressing Button A
 * [Blinky3](Blinky3/) - Based on Blinky2, with additional functionality to change the LED color by pressing Button B

**Definitions**
* A Product is a container for devices grouped into device-groups
* A DeviceGroup defines what Operating System version is delivered to devices in the group and also allows to set a policy for application updates over-the-air.
* A Deployment defines what application software is being delivered to devices in a DeviceGroup

By default, creating a product in AS3 automatically creates 5 DeviceGroups
* "*Development*"
* "*Field Test*" 
* "*Production*" 
* "*Production OS Evaluation*" 
* "*Field Test OS Evaluation*"

**Creating Blinky2 OTA Feed**
1. To create a "*Blinky Product*"
```sh
azsphere product create --name "Blinky Product" --description "Blinky2 Appliance"
```
you should see an output like the following, indicating that the product itself and the 5 default devicegroups have been
successfully created. 
```
Created product 'Blinky Product' with default device groups:
ID                                   Name
--                                   ----
3560f888-d19b-47dd-ac3d-551bedb2af7b Development
d38ad51d-39df-4be8-be3d-a6f355a3dd2d Field Test
a8bbef58-6233-4f03-9c03-de390138682a Production
5ac74c7d-7f1c-4b39-b14c-d9eb097ecd2b Production OS Evaluation
5210b268-7150-4d99-880f-4f831df957db Field Test OS Evaluation
```
2. Now you have created a "Product", you can select which device group you would like to upload your image to. In our lab, we will choose "Field Test" as our Device Group
3. To upload a new image to your device group, you may use:
```
azsphere dg dep create -pn "Blinky Product" -dgn "Field Test" -p "<<.....\Blinky2\out\ARM-Debug-*\Blinky2.imagepackage"
```

*Congratulation! You have successfully setup an OTA feed*

**Preparing Device to receive OTA**

1. Now we need to enable a device for cloud loading by disabling development and debugging, and assigning it to a product and a device group that enable application updates from cloud.
```
azsphere dev ect -pn "Blinky Product" -dgn "Field Test"
```
2. Press the RESET button to receive the OTA update (By default, Sphere will check with AS3 if there’s new update
once power up or every 24 hrs. So, resetting the device will force the process.)

**Blinky3 Challenge**

Can you update the OTA feed for Blinky3 ?

